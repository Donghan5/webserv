#include "PollServer.hpp"
#include "Logger.hpp"

extern volatile sig_atomic_t g_signal_received;

PollServer::PollServer() : MAX_EVENTS(64) {
    config = NULL;
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::log(Logger::DEBUG, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
}

PollServer::PollServer(const PollServer &obj) : MAX_EVENTS(64) {
    this->config = obj.config;
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::log(Logger::DEBUG, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
}

PollServer::PollServer(HttpConfig *config) : MAX_EVENTS(64) {
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::log(Logger::DEBUG, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
    setConfig(config);
}

PollServer::~PollServer() {
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
    }
}

void PollServer::getUniqueServers(const HttpConfig *hcf, MAP<int, STR>& unique_servers) {
	if (!hcf)
		throw std::runtime_error("Config does not exist");

	// iterate through the servers and add them to the map. changed to map 
    for (size_t i = 0; i < hcf->_servers.size(); i++) {
		bool is_port_found = false;
        try {
			if (unique_servers[(hcf->_servers[i]->_listen_port)] != "") {
				is_port_found = true;
			}
		}
		catch(const std::exception& e) {
			is_port_found = false;
		}

		if (!is_port_found) {
			unique_servers[(hcf->_servers[i]->_listen_port)] = config->_servers[i]->_listen_server;
		}
    }
}

// new helper function to initialize server sockets
void PollServer::initializeServerSockets(MAP<int, STR>& unique_servers) {
    for (MAP<int, STR>::iterator it = unique_servers.begin(); it != unique_servers.end(); ++it) {
        int port = it->first;
        STR server_addr_str = it->second;

        Logger::log(Logger::DEBUG, "Setting up server on " + server_addr_str + ":" + Utils::intToString(port));

        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            throw std::runtime_error("Failed to create socket: " + STR(strerror(errno)));
        }

        int reuse = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to set reuse address: " + STR(strerror(errno)));
        }

        // non-blocking mode
        int flags = fcntl(server_socket, F_GETFL, 0);
        if (flags == -1) {
            close(server_socket);
            throw std::runtime_error("Failed to get socket flags: " + STR(strerror(errno)));
        }

        if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(server_socket);
            throw std::runtime_error("Failed to set non-blocking mode: " + STR(strerror(errno)));
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (server_addr_str == "0.0.0.0") {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            struct addrinfo hints, *result;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_NUMERICHOST;

            int status = getaddrinfo(server_addr_str.c_str(), NULL, &hints, &result);
            if (status != 0) {
                close(server_socket);
                throw std::runtime_error("Failed to parse IP address: " +
                                         STR(gai_strerror(status)));
            }

            memcpy(&addr.sin_addr,
                   &((struct sockaddr_in*)result->ai_addr)->sin_addr,
                   sizeof(struct in_addr));

            freeaddrinfo(result);
        }

        if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to bind to " + server_addr_str + ":" +
                                     Utils::intToString(port) + " - " + STR(strerror(errno)));
        }

        if (listen(server_socket, SOMAXCONN) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to listen on port " + Utils::intToString(port) +
                                     ": " + STR(strerror(errno)));
        }

        _server_sockets[port] = server_socket;

		if (!AddFd(server_socket, EPOLLIN, SERVER_FD)) {
			close(server_socket);
			throw std::runtime_error("Failed to add server socket to epoll");
		}

        Logger::log(Logger::INFO, "Server listening on " + server_addr_str + ":" + Utils::intToString(port));
    }
}

void PollServer::setConfig(HttpConfig *config) {
	if (!config)
		throw std::runtime_error("Config does not exist");

	this->config = config;
    MAP<int, STR> unique_servers;

	getUniqueServers(config, unique_servers);
	initializeServerSockets(unique_servers);
}

bool PollServer::AddFd(int fd, uint32_t events, FdType type) {
    if (fd < 0) {
        Logger::log(Logger::DEBUG, "Attempted to add invalid file descriptor: " + Utils::intToString(fd));
        return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        Logger::log(Logger::DEBUG, "Failed to get flags for fd " + Utils::intToString(fd) + ": " + STR(strerror(errno)));
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::log(Logger::DEBUG, "Failed to set non-blocking mode for fd " + Utils::intToString(fd) + ": " + STR(strerror(errno)));
        return false;
    }

    if (_fd_types.find(fd) != _fd_types.end()) {
        Logger::log(Logger::DEBUG, "File descriptor " + Utils::intToString(fd) + " is already tracked as type " +
                       Utils::intToString(_fd_types[fd]) + ", changing to " + Utils::intToString(type));
    }

    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        Logger::log(Logger::DEBUG, "Failed to add fd " + Utils::intToString(fd) + " to epoll: " + STR(strerror(errno)));
        return false;
    }

    _fd_types[fd] = type;

    std::string type_name;
    switch (type) {
        case SERVER_FD: type_name = "server"; break;
        case CLIENT_FD: type_name = "client"; break;
        case CGI_FD: type_name = "CGI"; break;
        case POST_FD: type_name = "POST"; break;
        default: type_name = "unknown"; break;
    }

    Logger::log(Logger::DEBUG, "Added " + type_name + " fd " + Utils::intToString(fd) + " to epoll");

    return true;
}

bool PollServer::AddCgiFd(int cgi_fd, int client_fd) {
    if (cgi_fd < 0 || client_fd < 0) {
        Logger::log(Logger::DEBUG, "Invalid file descriptors in AddCgiFd");
        return false;
    }

    if (fcntl(cgi_fd, F_GETFD) == -1) {
        Logger::log(Logger::ERROR, "CGI fd " + Utils::intToString(cgi_fd) + " is not valid");
        return false;
    }

    if (fcntl(client_fd, F_GETFD) == -1) {
        Logger::log(Logger::ERROR, "Client fd " + Utils::intToString(client_fd) + " is not valid");
        return false;
    }

    int flags = fcntl(cgi_fd, F_GETFL, 0);
    if ((flags & O_NONBLOCK) == 0) {
        if (fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            Logger::log(Logger::ERROR, "Failed to set non-blocking mode for CGI fd");
            return false;
        }
    }

    if (AddFd(cgi_fd, EPOLLIN | EPOLLET, CGI_FD)) { // Using edge-triggered mode
        _cgi_to_client[cgi_fd] = client_fd;
        Logger::log(Logger::DEBUG, "Successfully added CGI fd " + Utils::intToString(cgi_fd) +
                       " for client " + Utils::intToString(client_fd));
        return true;
    }

    return false;
}

bool PollServer::ModifyFd(int fd, uint32_t events) {
    if (fd < 0) {
        Logger::log(Logger::DEBUG, "Invalid fd in ModifyFd: " + Utils::intToString(fd));
        return false;
    }

    if (fcntl(fd, F_GETFD) == -1) {
        Logger::log(Logger::DEBUG, "Attempted to modify closed fd: " + Utils::intToString(fd));
        return false;
    }

    if (_fd_types.find(fd) == _fd_types.end()) {
        Logger::log(Logger::DEBUG, "Attempted to modify untracked fd: " + Utils::intToString(fd));
        return false;
    }

    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
        Logger::log(Logger::DEBUG, "Failed to modify fd in epoll: " + STR(strerror(errno)));
        return false;
    }

    return true;
}

bool PollServer::RemoveFd(int fd) {
    if (fd < 0) {
        Logger::log(Logger::DEBUG, "Invalid fd in RemoveFd: " + Utils::intToString(fd));
        return false;
    }

    if (_fd_types.find(fd) == _fd_types.end()) {
        Logger::log(Logger::DEBUG, "RemoveFd: File descriptor " + Utils::intToString(fd) +
                      " not found in tracking map");
        return true;
    }

    std::string type_name;
    switch (_fd_types[fd]) {
        case SERVER_FD: type_name = "server"; break;
        case CLIENT_FD: type_name = "client"; break;
        case CGI_FD: type_name = "CGI"; break;
        case POST_FD: type_name = "POST"; break;
        default: type_name = "unknown"; break;
    }

    Logger::log(Logger::DEBUG, "Removing " + type_name + " fd: " + Utils::intToString(fd));

    if (fcntl(fd, F_GETFD) != -1) {
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
            Logger::log(Logger::DEBUG, "RemoveFd: " + type_name + " fd " + Utils::intToString(fd) +
                          " could not be removed from epoll: " + STR(strerror(errno)));
        }
    } else {
        Logger::log(Logger::DEBUG, "RemoveFd: " + type_name + " fd " + Utils::intToString(fd) +
                      " was already closed or removed from epoll");
    }

    _fd_types.erase(fd);

    return true;
}

bool PollServer::AddServerSocket(int port, int socket_fd) {
    _server_sockets[port] = socket_fd;
    return AddFd(socket_fd, EPOLLIN, SERVER_FD);
}

void PollServer::AcceptClient(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		Logger::log(Logger::ERROR, "Failed to accept client connection");
		return;
	}

	// non-blocking
	int flags = fcntl(client_fd, F_GETFL, 0);
	fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

	if (!AddFd(client_fd, EPOLLIN , CLIENT_FD)) {
		Logger::log(Logger::ERROR, "Failed to add client fd to epoll");
		close(client_fd);
		return;
	}

	Logger::log(Logger::INFO, "New client connection accepted: " + Utils::intToString(client_fd));
}

void PollServer::HandleCgiOutput(int cgi_fd, RequestsManager &manager) {
    MAP<int, int>::iterator it = _cgi_to_client.find(cgi_fd);
    if (it == _cgi_to_client.end()) {
        Logger::log(Logger::ERROR, "CGI fd without associated client: " + Utils::intToString(cgi_fd));

        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            _fd_types.erase(cgi_fd);
            _cgi_to_client.erase(cgi_fd);
        }
        return;
    }

    int client_fd = it->second;

    if (_fd_types.find(client_fd) == _fd_types.end() ||
        fcntl(client_fd, F_GETFD) == -1) {
        Logger::log(Logger::DEBUG, "Client fd " + Utils::intToString(client_fd) +
                      " associated with CGI fd " + Utils::intToString(cgi_fd) + " is invalid");

        _cgi_to_client.erase(it);

        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            _fd_types.erase(cgi_fd);
        }

        return;
    }

    try {
        manager.setClientFd(client_fd);
        int result = manager.HandleCgiOutput(cgi_fd);

        if (result > 0) {
            _cgi_to_client.erase(it);

            if (fcntl(client_fd, F_GETFD) != -1) {
                if (ModifyFd(client_fd, EPOLLOUT)) {
                    Logger::log(Logger::DEBUG, "Client fd " + Utils::intToString(client_fd) +
                                  " switched to write mode");
                } else {
                    Logger::log(Logger::ERROR, "Failed to modify client fd for writing, closing");
                    CloseClient(client_fd);
                }
            } else {
                Logger::log(Logger::DEBUG, "Client fd invalid in HandleCgiOutput");
            }

            if (fcntl(cgi_fd, F_GETFD) != -1) {
                RemoveFd(cgi_fd);
                close(cgi_fd);
            } else {
                _fd_types.erase(cgi_fd);
            }
        } else if (result == 0) {
            _cgi_to_client.erase(it);

            if (fcntl(cgi_fd, F_GETFD) != -1) {
                RemoveFd(cgi_fd);
                close(cgi_fd);
            } else {
                _fd_types.erase(cgi_fd);
            }

            CloseClient(client_fd);
        }
    } catch (const std::exception& e) {
        Logger::log(Logger::DEBUG, "Error handling CGI output: " + STR(e.what()));

        _cgi_to_client.erase(it);

        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            _fd_types.erase(cgi_fd);
        }

        CloseClient(client_fd);
    }
}

// check disconnect or timeout cgis (garbage collection)
void PollServer::processDisconnectOrTimeoutCgis(RequestsManager &manager) {
    std::vector<int> completed_cgis;
    for (MAP<int, int>::iterator it = _cgi_to_client.begin(); it != _cgi_to_client.end(); ++it) {
        int cgi_fd = it->first;
        int client_fd = it->second;

        if (fcntl(client_fd, F_GETFD) == -1) {
            completed_cgis.push_back(cgi_fd);
            continue;
        }

        if (fcntl(cgi_fd, F_GETFD) == -1) {
            completed_cgis.push_back(cgi_fd);
            continue;
        }

        try {
            manager.setClientFd(client_fd);
            HandleCgiOutput(cgi_fd, manager);
        } catch (const std::exception& e) {
            Logger::log(Logger::DEBUG, "Error checking CGI: " + STR(e.what()));
            completed_cgis.push_back(cgi_fd);
        }
    }

    for (size_t i = 0; i < completed_cgis.size(); ++i) {
        int cgi_fd = completed_cgis[i];
        MAP<int, int>::iterator it = _cgi_to_client.find(cgi_fd);
        if (it != _cgi_to_client.end()) {
            int client_fd = it->second;
            _cgi_to_client.erase(it);

            if (fcntl(client_fd, F_GETFD) != -1) {
                ModifyFd(client_fd, EPOLLOUT);
            }
        }
    }
}

void PollServer::checkingEventError(const epoll_event& current_event, RequestsManager &manager, FdType fd_type, int fd) {
	if (current_event.events & (EPOLLERR | EPOLLHUP)) {
		if (fd_type == SERVER_FD) {
			Logger::log(Logger::ERROR, "Error on server socket: " + Utils::intToString(fd));
		} else if (fd_type == CLIENT_FD) {
			Logger::log(Logger::DEBUG, "Client connection error or hangup: " + Utils::intToString(fd));
			CloseClient(fd);
		} else if (fd_type == CGI_FD) {
			Logger::log(Logger::DEBUG, "CGI error or hangup: " + Utils::intToString(fd));

			MAP<int, int>::iterator it = _cgi_to_client.find(fd);
			if (it != _cgi_to_client.end()) {
				int client_fd = it->second;

				try {
					manager.setClientFd(client_fd);
					HandleCgiOutput(fd, manager);
				} catch (const std::exception& e) {
					Logger::log(Logger::DEBUG, "Error handling CGI hangup: " + STR(e.what()));

					RemoveFd(fd);
					_cgi_to_client.erase(it);
					close(fd);

					_partial_responses[client_fd] = "HTTP/1.1 500 Internal Server Error\r\n"
					       "Content-Type: text/plain\r\n"
					       "Content-Length: 9\r\n"
					       "\r\n"
					       "CGI Error";

					// switch to write mode
					ModifyFd(client_fd, EPOLLOUT);
				}
			} else {
				// orphaned CGI fd
				Logger::log(Logger::DEBUG, "Orphaned CGI fd: " + Utils::intToString(fd));
				RemoveFd(fd);
				close(fd);
			}
		}
	}
}
 // --- handle client event activity ---
void PollServer::handleClientEventActivity(const epoll_event& current_event, RequestsManager &manager, int fd, int status) {
	switch (status) {
		case 0: // Remove client
			CloseClient(fd);
			break;
		case 2: // Switch to write mode
			ModifyFd(fd, EPOLLOUT);
			break;
		case 3: // Switch to read mode
			ModifyFd(fd, EPOLLIN);
			break;
		case 4: { // Register CGI fd
			int cgi_fd = manager.getCurrentCgiFd();
			if (cgi_fd > 0) {
				AddCgiFd(cgi_fd, fd);
			} else {
				Logger::log(Logger::ERROR, "Invalid CGI fd");
				ModifyFd(fd, EPOLLOUT);
			}
			break;
		}
	}
}

// --- handle event based on fd type ---
void PollServer::handleEventBasedOnFdType(const epoll_event& current_event, RequestsManager &manager, int fd, FdType fd_type) {
	try {
		if (fd_type == SERVER_FD && (current_event.events & EPOLLIN)) {
			AcceptClient(fd);
		} else if (fd_type == CLIENT_FD) {
			manager.setClientFd(fd);
			int status = manager.HandleClient(current_event.events);
			handleClientEventActivity(current_event, manager, fd, status);
		} else if (fd_type == CGI_FD && (current_event.events & EPOLLIN)) {
			HandleCgiOutput(fd, manager);
		}
	} catch (const std::exception& e) {
		Logger::log(Logger::ERROR, "Exception in event handling: " + STR(e.what()));
		if (fd_type == CLIENT_FD) {
			CloseClient(fd);
		} else if (fd_type == CGI_FD) {
			MAP<int, int>::iterator it = _cgi_to_client.find(fd);
			if (it != _cgi_to_client.end()) {
				CloseClient(it->second);
			}
			RemoveFd(fd);
			close(fd);
		}
	}
}

void print_all_fds(const std::map<int, FdType>& fd_types) {
    Logger::log(Logger::DEBUG, "Current tracked file descriptors:");
    for (std::map<int, FdType>::const_iterator it = fd_types.begin(); it != fd_types.end(); ++it) {
        std::string type_name;
        switch (it->second) {
            case SERVER_FD: type_name = "SERVER_FD"; break;
            case CLIENT_FD: type_name = "CLIENT_FD"; break;
            case CGI_FD: type_name = "CGI_FD"; break;
            case POST_FD: type_name = "POST_FD"; break;
            default: type_name = "UNKNOWN_FD"; break;
        }
        Logger::log(Logger::DEBUG, "FD: " + Utils::intToString(it->first) + ", Type: " + type_name);
    }
    Logger::log(Logger::DEBUG, "End of tracked file descriptors.");
}

void PollServer::handleSingleEpollEvent(const epoll_event& current_event, RequestsManager &manager) {
	int fd = current_event.data.fd;

	if (fd < 0) {
		Logger::log(Logger::WARNING, "Received event for invalid fd");
		return;
	}

	if (_fd_types.find(fd) == _fd_types.end()) {
		if (fcntl(fd, F_GETFD) != -1) {
			Logger::log(Logger::DEBUG, "Removing untracked fd " + Utils::intToString(fd) + " from epoll");
			epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);

			close(fd);
		}
		return;
	}

	FdType fd_type = _fd_types[fd];

	checkingEventError(current_event, manager, fd_type, fd);

	handleEventBasedOnFdType(current_event, manager, fd, fd_type);
}

bool PollServer::WaitAndService(RequestsManager &manager) {
    int num_events = epoll_wait(_epoll_fd, &_events[0], MAX_EVENTS, 1000);

	processDisconnectOrTimeoutCgis(manager);

    if (num_events < 0) {
        if (errno == EINTR) {
            // just a signal interruption
            // Logger::log(Logger::DEBUG, "Epoll wait was interrupted by a signal");
            return true;
        } else {
            Logger::log(Logger::ERROR, "Epoll wait failed: " + STR(strerror(errno)));
            return false;
        }
    }

    for (int i = 0; i < num_events; i++) {
		handleSingleEpollEvent(_events[i], manager);
    }
    return true;
}


void PollServer::CloseClient(int client_fd) {
    if (client_fd < 0) {
        return;
    }

    Logger::log(Logger::INFO, "Closing client connection: " + Utils::intToString(client_fd));

    std::vector<int> cgi_fds;
    for (MAP<int, int>::iterator it = _cgi_to_client.begin(); it != _cgi_to_client.end(); ++it) {
        if (it->second == client_fd) {
            cgi_fds.push_back(it->first);
        }
    }

    if (_fd_types.find(client_fd) != _fd_types.end()) {
        RemoveFd(client_fd);
    }

    if (fcntl(client_fd, F_GETFD) != -1) {
        close(client_fd);
    }

    _partial_requests.erase(client_fd);
    _partial_responses.erase(client_fd);

    for (size_t i = 0; i < cgi_fds.size(); i++) {
        int cgi_fd = cgi_fds[i];

        Logger::log(Logger::DEBUG, "Cleaning up orphaned CGI fd: " + Utils::intToString(cgi_fd));

        _cgi_to_client.erase(cgi_fd);

        if (_fd_types.find(cgi_fd) != _fd_types.end()) {
            RemoveFd(cgi_fd);
        }

        if (fcntl(cgi_fd, F_GETFD) != -1) {
            close(cgi_fd);
        }
    }
}

void PollServer::start() {
	RequestsManager	manager;

	if (!config) {
		Logger::log(Logger::ERROR, "Can't start server: config is not set");
	}
	manager.setConfig(config);
	running = true;

	do {
		if (!WaitAndService(manager))
			throw std::runtime_error("Poll error");
		if (g_signal_received != 0) {
			Logger::log(Logger::DEBUG, "Signal received: " + Utils::intToString(g_signal_received));
			running = false;
			break;
		}
	} while (running);

	Logger::log(Logger::DEBUG, "Stopped server loop. Clearing resources...");

    for (std::map<int, int>::iterator it = _server_sockets.begin(); it != _server_sockets.end(); ++it) {
        RemoveFd(it->second);
        close(it->second);
    }
    _server_sockets.clear();

    std::vector<int> clients_to_close;
    for (std::map<int, FdType>::iterator it = _fd_types.begin(); it != _fd_types.end(); ++it) {
        if (it->second == CLIENT_FD) {
            clients_to_close.push_back(it->first);
        }
    }
    for (size_t i = 0; i < clients_to_close.size(); ++i) {
        CloseClient(clients_to_close[i]);
    }
    _fd_types.clear();
    _cgi_to_client.clear();

    Logger::log(Logger::DEBUG, "End to terminate server.");
}

void PollServer::stop() {
	if (!config) {
		Logger::log(Logger::ERROR, "Can't stop server: config is not set");
	}
	running = false;
}
