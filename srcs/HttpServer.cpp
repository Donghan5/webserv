#include "../includes/HttpServer.hpp"

HttpServer::HttpServer(const WebServConf& webconf) : _webconf(webconf), running(false), clientManager(poll_fds) {
	const std::vector<ServerConf>& serverConfigs = _webconf.getHttpBlock().getServerConfig();

	if (serverConfigs.empty()) {
		throw std::runtime_error("No server configurations found in WebServConf");
	}

	std::map<int, int> port_to_fd;
	std::vector<int> unique_ports;

	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		int port = std::atoi(serverConfigs[i].getData("listen").c_str());
		std::string server_name = serverConfigs[i].getData("server_name");
		std::vector<int>::iterator it = std::find(unique_ports.begin(), unique_ports.end(), port);
		if (port > 0 && it == unique_ports.end()) {
			unique_ports.push_back(port);
		}

		if (!server_name.empty()) {
			if (server_name_to_fd.find(server_name) == server_name_to_fd.end()) {
				server_name_to_fd[server_name] = -1;
			}
		}

		if (port > 0 && server_fds.find(port) != server_fds.end()) {
			int server_fd = server_fds[port];
			server_name_to_fd[server_name] = server_fd;
			Logger::log(Logger::INFO, "Mapped Server Name: " + server_name + " -> FD: " + Utils::intToString(server_fd));
		}

		Logger::log(Logger::INFO, "Server Config - Port: " + Utils::intToString(port) +
                ", Server Name: " + server_name +
                ", Mapped FD: " + (server_name_to_fd.find(server_name) != server_name_to_fd.end() ?
                Utils::intToString(server_name_to_fd[server_name]) : "Not Assigned"));
	}

	Logger::log(Logger::INFO, "===================== Start DEBUG ========================");

	Logger::log(Logger::INFO, "===== Port to FD Mapping =====");
	for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
		Logger::log(Logger::INFO, "Port: " + Utils::intToString(it->first) + " -> FD: " + Utils::intToString(it->second));
	}

	Logger::log(Logger::INFO, "===== Server Name to FD Mapping =====");
	for (std::map<std::string, int>::iterator it = server_name_to_fd.begin(); it != server_name_to_fd.end(); ++it) {
		Logger::log(Logger::INFO, "Server Name: " + it->first + " -> FD: " + Utils::intToString(it->second));
	}

	Logger::log(Logger::INFO, "===================== End DEBUG ========================");


	for (size_t i = 0; i < unique_ports.size(); ++i) {
		int server_fd = SocketManager::createSocket();
		if (server_fd < 0) {
			throw std::runtime_error("Failed to create socket for port " + Utils::intToString(unique_ports[i]));
		}

		SocketManager::setSocket(server_fd);
		SocketManager::makeNonBlocking(server_fd);
		SocketManager::bindSocket(server_fd, unique_ports[i]);
		SocketManager::listenSocket(server_fd);

		server_fds[unique_ports[i]] = server_fd;
		std::cout << "Server bound and listening on port " << unique_ports[i] << std::endl;

		// Add server socket to poll set
		struct pollfd server_pollfd;
		server_pollfd.fd = server_fd;
		server_pollfd.events = POLLIN;
		server_pollfd.revents = 0;
		poll_fds.push_back(server_pollfd);
	}
}

/*
	Execute server loop (wait poll event and sent it to propre handler)
*/
void HttpServer::runServerLoop(void) {
	Logger::log(Logger::INFO, "Start Server");

	while (running) {
		int ready = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ready < 0) {
			if (errno == EINTR) continue;
			Logger::log(Logger::ERROR, "Poll failed");
			throw std::runtime_error("Poll failed");
		}

		// Make a copy because the vector might be modified during iteration
		std::vector<struct pollfd> current_fds = poll_fds;

		for (size_t i = 0; i < current_fds.size(); ++i) {
			if (current_fds[i].revents == 0) {
				continue;
			}

			if (isServerSocket(current_fds[i].fd)) {
				acceptNewClient(current_fds[i].fd);

			} else {
				handleClientEvents(current_fds[i]);
			}
		}
	}
}

/*
	Create poll event and wait
*/
int HttpServer::waitForEvents(void) {
	int ready = poll(&poll_fds[0], poll_fds.size(), -1);
	if (ready < 0) {
		if (errno == EINTR) return -1;
		Logger::log(Logger::ERROR, "Poll failed");
		throw std::runtime_error("Poll failed");
	}
	return ready;
}

/*
	verify server socket (current fd)
*/
bool HttpServer::isServerSocket(int fd) {
	for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
		if (fd == it->second) {
			return true;
		}
	}
	return false;
}

/*
	handle new client link
*/
void HttpServer::acceptNewClient(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

	if (client_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			Logger::log(Logger::ERROR, "Fail connection");
			std::cerr << "Failed to accept connection" << std::endl;
		}
		return ;
	}

	SocketManager::makeNonBlocking(client_fd);

	// Add to poll set
	struct pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;
	poll_fds.push_back(client_pollfd);

	Logger::log(Logger::INFO, "New client connected: fd=" + Utils::intToString(client_fd));
}

/*
	handle event in client socket
*/
void HttpServer::handleClientEvents(struct pollfd &client_fd) {
	if (client_fd.revents & POLLIN) {
		clientManager.handleClientRead(client_fd.fd);
	}
	if (client_fd.revents & POLLOUT) {
		clientManager.handleClientWrite(client_fd.fd);
	}
	if (client_fd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
		Logger::log(Logger::ERROR, "Detected closed connection or error: client_fd=" + Utils::intToString(client_fd.fd));
		clientManager.closeClient(client_fd.fd);
	}
}

/*
	run main server loop
*/
void HttpServer::start() {
	running = true;
	Logger::log(Logger::INFO, "Start Server");

	runServerLoop();
}

/*
	Stop main server loop
*/
void HttpServer::stop() {
	running = false;

	// Close all server sockets
	for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
		int fd = it->second;
		SocketManager::closeSocket(fd);
		clientManager.closeClient(fd);
	}
	server_fds.clear();
	poll_fds.clear();
}

/*
	destructor: stop server if running
*/
HttpServer::~HttpServer() {
	if (running) {
		Logger::log(Logger::INFO, "Stopping Server");
		stop();
	}
}
