#include "PollServer.hpp"
#include "Logger.hpp"

extern volatile sig_atomic_t g_signal_received;

PollServer::PollServer() : MAX_EVENTS(64) {
    config = NULL;
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
}

PollServer::PollServer(const PollServer &obj) : MAX_EVENTS(64) {
    this->config = obj.config;
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
}

PollServer::PollServer(HttpConfig *config) : MAX_EVENTS(64) {
    running = false;
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to create epoll file descriptor");
    }
    _events.resize(MAX_EVENTS);
    setConfig(config);
}

PollServer::~PollServer() {
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
    }
}

void PollServer::setConfig(HttpConfig *config) {
	if (!config)
		throw std::runtime_error("Config does not exist");

	this->config = config;

    MAP<int, STR> unique_servers;
    // iterate through the servers and add them to the map. changed to map
    for (size_t i = 0; i < config->_servers.size(); i++) {
		bool is_port_found = false;
        try
		{
			if (unique_servers[(config->_servers[i]->_listen_port)] != "") {
				is_port_found = true;
			}
		}
		catch(const std::exception& e)
		{
			is_port_found = false;
		}

		if (!is_port_found) {
			unique_servers[(config->_servers[i]->_listen_port)] = config->_servers[i]->_listen_server;
		}
    }

	 // Create sockets for unique server entries
	 for (MAP<int, STR>::iterator it = unique_servers.begin(); it != unique_servers.end(); ++it) {
        int port = it->first;
        STR server_addr_str = it->second;

        Logger::log(Logger::INFO, "Setting up server on " + server_addr_str + ":" + Utils::intToString(port));

        // Create socket
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            throw std::runtime_error("Failed to create socket: " + STR(strerror(errno)));
        }

        // // For server sockets, keep the timeout but consider using level-triggered mode
        // struct timeval timeout;
        // timeout.tv_sec = 5;  // Increased to 5 seconds for better handling
        // timeout.tv_usec = 0;

        // if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        //     close(server_socket);
        //     throw std::runtime_error("Failed to set receive timeout: " + STR(strerror(errno)));
        // }

        // if (setsockopt(server_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        //     close(server_socket);
        //     throw std::runtime_error("Failed to set send timeout: " + STR(strerror(errno)));
        // }

        // Set reuse address option
        int reuse = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to set reuse address: " + STR(strerror(errno)));
        }

        // Set non-blocking mode
        int flags = fcntl(server_socket, F_GETFL, 0);
        if (flags == -1) {
            close(server_socket);
            throw std::runtime_error("Failed to get socket flags: " + STR(strerror(errno)));
        }

        if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(server_socket);
            throw std::runtime_error("Failed to set non-blocking mode: " + STR(strerror(errno)));
        }

        // Set up server address
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

        // Bind socket
        if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to bind to " + server_addr_str + ":" +
                                     Utils::intToString(port) + " - " + STR(strerror(errno)));
        }

        // Listen for connections
        if (listen(server_socket, SOMAXCONN) < 0) {
            close(server_socket);
            throw std::runtime_error("Failed to listen on port " + Utils::intToString(port) +
                                     ": " + STR(strerror(errno)));
        }

        // Store socket in map
        _server_sockets[port] = server_socket;

        // for better handling of large requests
		if (!AddFd(server_socket, EPOLLIN, SERVER_FD)) {
			close(server_socket);
			throw std::runtime_error("Failed to add server socket to epoll");
		}

        Logger::log(Logger::INFO, "Server listening on " + server_addr_str + ":" + Utils::intToString(port));
    }
}

bool PollServer::AddFd(int fd, uint32_t events, FdType type) {
    if (fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Attempted to add invalid file descriptor: " + Utils::intToString(fd));
        return false;
    }

    // Set non-blocking mode for the fd
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to get flags for fd " + Utils::intToString(fd) + ": " + STR(strerror(errno)));
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking mode for fd " + Utils::intToString(fd) + ": " + STR(strerror(errno)));
        return false;
    }

    // Check if the fd is already being tracked
    if (_fd_types.find(fd) != _fd_types.end()) {
        Logger::cerrlog(Logger::WARNING, "File descriptor " + Utils::intToString(fd) + " is already tracked as type " +
                       Utils::intToString(_fd_types[fd]) + ", changing to " + Utils::intToString(type));
    }

    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to add fd " + Utils::intToString(fd) + " to epoll: " + STR(strerror(errno)));
        return false;
    }

    // Store the fd type
    _fd_types[fd] = type;

    // Log success
    std::string type_name;
    switch (type) {
        case SERVER_FD: type_name = "server"; break;
        case CLIENT_FD: type_name = "client"; break;
        case CGI_FD: type_name = "CGI"; break;
        case POST_FD: type_name = "POST"; break;
        default: type_name = "unknown"; break;
    }

    Logger::cerrlog(Logger::DEBUG, "Added " + type_name + " fd " + Utils::intToString(fd) + " to epoll");

    return true;
}

bool PollServer::AddCgiFd(int cgi_fd, int client_fd) {
    // Validate file descriptors
    if (cgi_fd < 0 || client_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Invalid file descriptors in AddCgiFd");
        return false;
    }

    // Make sure the CGI fd is valid
    if (fcntl(cgi_fd, F_GETFD) == -1) {
        Logger::cerrlog(Logger::ERROR, "CGI fd " + Utils::intToString(cgi_fd) + " is not valid");
        return false;
    }

    // Make sure the client fd is still valid
    if (fcntl(client_fd, F_GETFD) == -1) {
        Logger::cerrlog(Logger::ERROR, "Client fd " + Utils::intToString(client_fd) + " is not valid");
        return false;
    }

    // Set non-blocking mode for the CGI fd if not already set
    int flags = fcntl(cgi_fd, F_GETFL, 0);
    if ((flags & O_NONBLOCK) == 0) {
        if (fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking mode for CGI fd");
            return false;
        }
    }

    // Add the fd to epoll
    if (AddFd(cgi_fd, EPOLLIN | EPOLLET, CGI_FD)) { // Using edge-triggered mode
        _cgi_to_client[cgi_fd] = client_fd;
        Logger::cerrlog(Logger::INFO, "Successfully added CGI fd " + Utils::intToString(cgi_fd) +
                       " for client " + Utils::intToString(client_fd));
        return true;
    }

    return false;
}

bool PollServer::ModifyFd(int fd, uint32_t events) {
    if (fd < 0) {
        Logger::cerrlog(Logger::WARNING, "Invalid fd in ModifyFd: " + Utils::intToString(fd));
        return false;
    }

    // CRITICAL FIX: Check if fd is still valid
    if (fcntl(fd, F_GETFD) == -1) {
        Logger::cerrlog(Logger::ERROR, "Attempted to modify closed fd: " + Utils::intToString(fd));
        return false;
    }

    // Make sure we're tracking this fd
    if (_fd_types.find(fd) == _fd_types.end()) {
        Logger::cerrlog(Logger::ERROR, "Attempted to modify untracked fd: " + Utils::intToString(fd));
        return false;
    }

    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to modify fd in epoll: " + STR(strerror(errno)));
        return false;
    }

    return true;
}

bool PollServer::RemoveFd(int fd) {
    if (fd < 0) {
        Logger::cerrlog(Logger::WARNING, "Invalid fd in RemoveFd: " + Utils::intToString(fd));
        return false;
    }

    // Check if we're tracking this fd
    if (_fd_types.find(fd) == _fd_types.end()) {
        Logger::cerrlog(Logger::DEBUG, "RemoveFd: File descriptor " + Utils::intToString(fd) +
                      " not found in tracking map");
        return true; // Not an error if we weren't tracking it
    }

    // Get fd type for logging
    std::string type_name;
    switch (_fd_types[fd]) {
        case SERVER_FD: type_name = "server"; break;
        case CLIENT_FD: type_name = "client"; break;
        case CGI_FD: type_name = "CGI"; break;
        case POST_FD: type_name = "POST"; break;
        default: type_name = "unknown"; break;
    }

    Logger::cerrlog(Logger::DEBUG, "Removing " + type_name + " fd: " + Utils::intToString(fd));

    // CRITICAL: Check if fd is still valid before trying to use it
    if (fcntl(fd, F_GETFD) != -1) {
        // Only try to remove from epoll if the fd is valid
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
            Logger::cerrlog(Logger::DEBUG, "RemoveFd: " + type_name + " fd " + Utils::intToString(fd) +
                          " could not be removed from epoll: " + STR(strerror(errno)));
        }
    } else {
        Logger::cerrlog(Logger::DEBUG, "RemoveFd: " + type_name + " fd " + Utils::intToString(fd) +
                      " was already closed or removed from epoll");
    }

    // Always remove from tracking maps
    _fd_types.erase(fd);

    return true;
}

// Add server socket
bool PollServer::AddServerSocket(int port, int socket_fd) {
    _server_sockets[port] = socket_fd;
    return AddFd(socket_fd, EPOLLIN, SERVER_FD);
}

// Accept new client connection
void PollServer::AcceptClient(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		Logger::cerrlog(Logger::ERROR, "Failed to accept client connection");
		return;
	}

	// Set non-blocking
	int flags = fcntl(client_fd, F_GETFL, 0);
	fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

	// Add to epoll for read events
	if (!AddFd(client_fd, EPOLLIN , CLIENT_FD)) {
		Logger::cerrlog(Logger::ERROR, "Failed to add client fd to epoll");
		close(client_fd);
		return;
	}

	Logger::cerrlog(Logger::INFO, "New client connection accepted: " + Utils::intToString(client_fd));
}

void PollServer::HandleCgiOutput(int cgi_fd, RequestsManager &manager) {
    // Find the associated client
    MAP<int, int>::iterator it = _cgi_to_client.find(cgi_fd);
    if (it == _cgi_to_client.end()) {
        Logger::cerrlog(Logger::ERROR, "CGI fd without associated client: " + Utils::intToString(cgi_fd));

        // Safe cleanup since we don't know the client
        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            // Just clean up tracking
            _fd_types.erase(cgi_fd);
            _cgi_to_client.erase(cgi_fd);
        }
        return;
    }

    int client_fd = it->second;

    // CRITICAL FIX: Check if client is still valid
    if (_fd_types.find(client_fd) == _fd_types.end() ||
        fcntl(client_fd, F_GETFD) == -1) {
        Logger::cerrlog(Logger::WARNING, "Client fd " + Utils::intToString(client_fd) +
                      " associated with CGI fd " + Utils::intToString(cgi_fd) + " is invalid");

        // Clean up the orphaned CGI fd
        _cgi_to_client.erase(it); // Remove from mapping first

        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            // Just clean up tracking
            _fd_types.erase(cgi_fd);
        }

        return;
    }

    try {
        // Process the CGI output
        manager.setClientFd(client_fd);
        int result = manager.HandleCgiOutput(cgi_fd);

        if (result > 0) {
            // CGI completed, switch client to write mode
            _cgi_to_client.erase(it); // Remove mapping first

            // Check if client is still valid before modifying
            if (fcntl(client_fd, F_GETFD) != -1) {
                if (ModifyFd(client_fd, EPOLLOUT)) {
                    // Successfully modified
                    Logger::cerrlog(Logger::DEBUG, "Client fd " + Utils::intToString(client_fd) +
                                  " switched to write mode");
                } else {
                    Logger::cerrlog(Logger::ERROR, "Failed to modify client fd for writing, closing");
                    CloseClient(client_fd);
                }
            } else {
                Logger::cerrlog(Logger::WARNING, "Client fd invalid in HandleCgiOutput");
                // Client already closed, just clean up CGI
            }

            // Then handle the CGI fd
            if (fcntl(cgi_fd, F_GETFD) != -1) {
                RemoveFd(cgi_fd);
                close(cgi_fd);
            } else {
                // Just clean up tracking
                _fd_types.erase(cgi_fd);
            }
        } else if (result == 0) {
            // Error occurred, clean up both client and CGI
            _cgi_to_client.erase(it); // Remove mapping first

            // Clean up CGI fd
            if (fcntl(cgi_fd, F_GETFD) != -1) {
                RemoveFd(cgi_fd);
                close(cgi_fd);
            } else {
                // Just clean up tracking
                _fd_types.erase(cgi_fd);
            }

            // Close the client
            CloseClient(client_fd);
        }
        // result < 0 means CGI still running, keep monitoring
    } catch (const std::exception& e) {
        Logger::cerrlog(Logger::ERROR, "Error handling CGI output: " + STR(e.what()));

        // Remove mapping first
        _cgi_to_client.erase(it);

        // Clean up CGI fd
        if (fcntl(cgi_fd, F_GETFD) != -1) {
            RemoveFd(cgi_fd);
            close(cgi_fd);
        } else {
            // Just clean up tracking
            _fd_types.erase(cgi_fd);
        }

        // Close the client
        CloseClient(client_fd);
    }
}

bool PollServer::WaitAndService(RequestsManager &manager) {
    int num_events = epoll_wait(_epoll_fd, &_events[0], MAX_EVENTS, -1); // Use a timeout

    std::vector<int> completed_cgis;
    for (MAP<int, int>::iterator it = _cgi_to_client.begin(); it != _cgi_to_client.end(); ++it) {
        int cgi_fd = it->first;
        int client_fd = it->second;

        // Skip if client fd is invalid
        if (fcntl(client_fd, F_GETFD) == -1) {
            completed_cgis.push_back(cgi_fd);
            continue;
        }

        // Skip if CGI fd is invalid
        if (fcntl(cgi_fd, F_GETFD) == -1) {
            completed_cgis.push_back(cgi_fd);
            continue;
        }

        // Force CGI output processing to check for timeout
        try {
            manager.setClientFd(client_fd);
            HandleCgiOutput(cgi_fd, manager);
        } catch (const std::exception& e) {
            Logger::cerrlog(Logger::ERROR, "Error checking CGI: " + STR(e.what()));
            completed_cgis.push_back(cgi_fd);
        }
    }

    // Clean up any completed CGIs
    for (size_t i = 0; i < completed_cgis.size(); ++i) {
        int cgi_fd = completed_cgis[i];
        MAP<int, int>::iterator it = _cgi_to_client.find(cgi_fd);
        if (it != _cgi_to_client.end()) {
            int client_fd = it->second;
            _cgi_to_client.erase(it);

            // Try to mark client as writable
            if (fcntl(client_fd, F_GETFD) != -1) {
                ModifyFd(client_fd, EPOLLOUT);
            }
        }
    }

    if (num_events < 0) {
        if (errno == EINTR) {
            // Just a signal interruption, not a real error
            // Logger::cerrlog(Logger::DEBUG, "Epoll wait was interrupted by a signal");
            return true;
        } else {
            Logger::cerrlog(Logger::ERROR, "Epoll wait failed: " + STR(strerror(errno)));
            return false;
        }
    }

    for (int i = 0; i < num_events; i++) {
        int fd = _events[i].data.fd;

        // Skip invalid file descriptors
        if (fd < 0) {
            Logger::cerrlog(Logger::WARNING, "Received event for invalid fd");
            continue;
        }

        // Get the fd type - CRITICAL CHANGE: if not found, try to remove it from epoll
        if (_fd_types.find(fd) == _fd_types.end()) {
            Logger::cerrlog(Logger::WARNING, "Unknown fd type for fd: " + Utils::intToString(fd));

            // Try to remove the fd from epoll to prevent future unknown fd errors
            if (fcntl(fd, F_GETFD) != -1) { // Check if fd is still valid
                Logger::cerrlog(Logger::DEBUG, "Removing untracked fd " + Utils::intToString(fd) + " from epoll");
                epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);

                // If it's a valid fd, close it to prevent leaks
                close(fd);
            }
            continue; // Skip this event
        }

        FdType fd_type = _fd_types[fd];

        // Check for errors first
        if (_events[i].events & (EPOLLERR | EPOLLHUP)) {
            if (fd_type == SERVER_FD) {
                Logger::cerrlog(Logger::ERROR, "Error on server socket: " + Utils::intToString(fd));
                // Could try to restart the server socket here
            } else if (fd_type == CLIENT_FD) {
                Logger::cerrlog(Logger::INFO, "Client connection error or hangup: " + Utils::intToString(fd));
                CloseClient(fd);
            } else if (fd_type == CGI_FD) {
                Logger::cerrlog(Logger::INFO, "CGI error or hangup: " + Utils::intToString(fd));

                // Find the associated client
                MAP<int, int>::iterator it = _cgi_to_client.find(fd);
                if (it != _cgi_to_client.end()) {
                    int client_fd = it->second;

                    try {
                        // Try to read any available data
                        manager.setClientFd(client_fd);
                        HandleCgiOutput(fd, manager);
                    } catch (const std::exception& e) {
                        Logger::cerrlog(Logger::ERROR, "Error handling CGI hangup: " + STR(e.what()));

                        // Clean up CGI resources
                        RemoveFd(fd);
                        _cgi_to_client.erase(it);
                        close(fd);

                        // Create an error response
                        _partial_responses[client_fd] = "HTTP/1.1 500 Internal Server Error\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 9\r\n"
                               "\r\n"
                               "CGI Error";

                        // Switch to write mode
                        ModifyFd(client_fd, EPOLLOUT);
                    }
                } else {
                    // Orphaned CGI fd
                    Logger::cerrlog(Logger::WARNING, "Orphaned CGI fd: " + Utils::intToString(fd));
                    RemoveFd(fd);
                    close(fd);
                }
            }
            continue;
        }

        // Handle events based on fd type
        try {
            if (fd_type == SERVER_FD && (_events[i].events & EPOLLIN)) {
                // Server socket has incoming connection
                AcceptClient(fd);
            } else if (fd_type == CLIENT_FD) {
                // Client activity
                manager.setClientFd(fd);
                int status = manager.HandleClient(_events[i].events);

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
                            Logger::cerrlog(Logger::ERROR, "Invalid CGI fd returned from manager");
                            ModifyFd(fd, EPOLLOUT);
                        }
                        break;
                    }
                }
            } else if (fd_type == CGI_FD && (_events[i].events & EPOLLIN)) {
                // CGI output ready
                HandleCgiOutput(fd, manager);
            }
        } catch (const std::exception& e) {
            Logger::cerrlog(Logger::ERROR, "Exception in event handling: " + STR(e.what()));

            // Clean up based on fd type
            if (fd_type == CLIENT_FD) {
                CloseClient(fd);
            } else if (fd_type == CGI_FD) {
                // Find and clean up associated client
                MAP<int, int>::iterator it = _cgi_to_client.find(fd);
                if (it != _cgi_to_client.end()) {
                    CloseClient(it->second);
                }

                // Clean up CGI fd
                RemoveFd(fd);
                close(fd);
            }
        }
    }

    return true;
}

void PollServer::CloseClient(int client_fd) {
    if (client_fd < 0) {
        return;
    }

    Logger::cerrlog(Logger::INFO, "Closing client connection: " + Utils::intToString(client_fd));

    // First find any CGI fds associated with this client
    std::vector<int> cgi_fds;
    for (MAP<int, int>::iterator it = _cgi_to_client.begin(); it != _cgi_to_client.end(); ++it) {
        if (it->second == client_fd) {
            cgi_fds.push_back(it->first);
        }
    }

    // Remove client from epoll
    if (_fd_types.find(client_fd) != _fd_types.end()) {
        RemoveFd(client_fd);
    }

    // Close client socket
    if (fcntl(client_fd, F_GETFD) != -1) {
        close(client_fd);
    }

    // Clean up client data
    _partial_requests.erase(client_fd);
    _partial_responses.erase(client_fd);

    // Handle each orphaned CGI fd
    for (size_t i = 0; i < cgi_fds.size(); i++) {
        int cgi_fd = cgi_fds[i];

        Logger::cerrlog(Logger::INFO, "Cleaning up orphaned CGI fd: " + Utils::intToString(cgi_fd));

        // Remove from tracking map first
        _cgi_to_client.erase(cgi_fd);

        // Then remove from epoll
        if (_fd_types.find(cgi_fd) != _fd_types.end()) {
            RemoveFd(cgi_fd);
        }

        // Finally close the fd
        if (fcntl(cgi_fd, F_GETFD) != -1) {
            close(cgi_fd);
        }
    }
}

void PollServer::start() {
	RequestsManager					manager;

	if (!config) {
		Logger::cerrlog(Logger::ERROR, "Can't start server: config is not set");
	}
	manager.setConfig(config);
	running = true;

	do {
		if (!WaitAndService(manager))
			throw std::runtime_error("Poll error");
		if (g_signal_received != 0) {
			Logger::cerrlog(Logger::INFO, "Signal received: " + Utils::intToString(g_signal_received));
			running = false;
			break;
		}
	} while (running);

	Logger::log(Logger::INFO, "Stopped server loop. Clearing resources...");

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

    Logger::log(Logger::INFO, "End to terminate server.");
}

void PollServer::stop() {
	if (!config) {
		Logger::cerrlog(Logger::ERROR, "Can't stop server: config is not set");
	}
	running = false;
}
