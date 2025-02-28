#include "../includes/HttpServer.hpp"
#include "../includes/ParsedRequest.hpp"
#include "../includes/FileHandler.hpp"
#include "../includes/Logger.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Utils.hpp"


// Helper function to make socket non-blocking
void HttpServer::make_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		Logger::log(Logger::ERROR, "Fail socket flags");
		throw std::runtime_error("Failed to get socket flags");
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		Logger::log(Logger::ERROR, "Fail set non-blocking");
		throw std::runtime_error("Failed to set socket non-blocking");
	}
}

// Helper function to check if file exists
bool HttpServer::file_exists(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

// Helper function to check string ending
bool HttpServer::ends_with(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length()) {
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// Process a complete HTTP request
std::string HttpServer::process_request(const std::string &request) {
	ParsedRequest parser(request);

	std::string method = parser.getMethod();
	std::string path = parser.getPath();
	std::string host = parser.getHost();

	// Default to index.html for root path
	if (path == "/") {
		path = "/index.html";
	}

	std::string resolved_root = _webconf.resolveRoot(host, path);
	std::string full_path = resolved_root + path;

	Logger::log(Logger::INFO, "Resolved root: " + resolved_root);
	Logger::log(Logger::INFO, "Directive to: " + full_path);
	std::string extension = path.substr(path.find_last_of("."));
	if (extension == ".py" || extension == ".php" || extension == ".pl" || extension == ".sh") {
			CgiHandler cgi(full_path, parser.getHeaders(), parser.getBody());
			return cgi.executeCgi();
	}
	if (method == "GET") {
		return FileHandler::handleGetRequest(full_path);
	}
	else if (method == "POST") {
		try {
			std::string body = parser.getBody();
			return FileHandler::handlePostRequest(full_path, body);
		} catch (const std::exception &e) {
			return std::string(e.what()); // request 500
		}
	}
	else if (method == "DELETE") {
		return FileHandler::handleDeleteRequest(full_path);
	}
	else {
		Logger::log(Logger::ERROR, "405 error in process request");
		return REQUEST405;
	}
}

void HttpServer::handle_client_read(int client_fd) {
	if (client_fd <= 0) {
		Logger::log(Logger::ERROR, "Attempted to read from an invalid client_fd: client_fd=" + Utils::intToString(client_fd));
		return;
	}

	int socket_status = fcntl(client_fd, F_GETFL, 0);
	if (socket_status == -1) {
		Logger::log(Logger::ERROR, "Invalid socket: client_fd=" + Utils::intToString(client_fd));
		close_client(client_fd);
		return;
	}

	char buffer[BUFFER_SIZE];

	int check = recv(client_fd, buffer, 1, MSG_PEEK);

	if (check == 0) {
		Logger::log(Logger::DEBUG, "Client disconnected before read: client_fd=" + Utils::intToString(client_fd));
		return;
	}

	if (check < 0) {
		if (errno == ENOTCONN) {
			Logger::log(Logger::ERROR, "Client not connected: client_fd=" + Utils::intToString(client_fd));
			close_client(client_fd);
			return;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			Logger::log(Logger::DEBUG, "Temporary read error, retrying: client_fd=" + Utils::intToString(client_fd));
			return;
		}
		Logger::log(Logger::ERROR, "Socket error before read: client_fd=" + Utils::intToString(client_fd) + ", errno=" + strerror(errno));
		close_client(client_fd);
		return;
	}

	ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);

	if (bytes_read < 0) {
		if (errno == ECONNRESET) {
			Logger::log(Logger::ERROR, "Connection reset by peer: client_fd=" + Utils::intToString(client_fd));
		}
		else if (errno == EAGAIN || errno == EWOULDBLOCK) {
			Logger::log(Logger::ERROR, "Temporary read error, retrying: client_fd=" + Utils::intToString(client_fd));
			return;
		}
		else {
			Logger::log(Logger::ERROR, "Read failed (closing connection): client_fd=" + Utils::intToString(client_fd) + ", errno=" + strerror(errno));
			close_client(client_fd);
		}
		return ;
	}

	if (bytes_read == 0) {
		Logger::log(Logger::ERROR, "Client disconnected: client_fd=" + Utils::intToString(client_fd));
		close_client(client_fd);
		return;
	}

	partial_requests[client_fd].append(buffer, bytes_read);

	// Check if we have a complete HTTP request
	size_t header_end = partial_requests[client_fd].find("\r\n\r\n");
	if (header_end != std::string::npos) {
		ParsedRequest parser(partial_requests[client_fd]);
		parser.parseHttpRequest(partial_requests[client_fd]);

		std::string content_length_str = parser.getData("content-length");
        if (!content_length_str.empty()) {
            int content_length = std::atoi(content_length_str.c_str());
            size_t body_start = header_end + 4;

            if (partial_requests[client_fd].size() < body_start + content_length) {
                return;
            }
        }
		std::string response = process_request(partial_requests[client_fd]);
		partial_responses[client_fd] = response;
		partial_requests.erase(client_fd);

		// Modify the pollfd entry to monitor for writing
		for (size_t i = 0; i < poll_fds.size(); ++i) {
			if (poll_fds[i].fd == client_fd) {
				poll_fds[i].events = POLLOUT;
				break;
			}
		}
	}
}

void HttpServer::handle_client_write(int client_fd) {
	std::string& response = partial_responses[client_fd];
	ssize_t bytes_written = write(client_fd, response.c_str(), response.length());

	if (bytes_written <= 0) {
		if (errno != EAGAIN) {
			close_client(client_fd);
		}
		return;
	}

	response.erase(0, bytes_written);

	if (response.empty()) {
		close_client(client_fd);
	}
}

void HttpServer::close_client(int client_fd) {
	if (client_fd < 0) return;

	Logger::log(Logger::INFO, "Close client_fd=" + Utils::intToString(client_fd));
	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	partial_requests.erase(client_fd);
	partial_responses.erase(client_fd);

	// Remove from poll_fds
	for (size_t i = 0; i < poll_fds.size(); ++i) {
		if (poll_fds[i].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + i);
			break;
		}
	}
}

HttpServer::HttpServer(const WebServConf& webconf) : _webconf(webconf), running(false) {
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
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0) {
			throw std::runtime_error("Failed to create socket for port " + Utils::intToString(unique_ports[i]));
		}

		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
			close(server_fd);
			throw std::runtime_error("Failed to set socket options");
		}

		make_non_blocking(server_fd);

		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(unique_ports[i]);

		if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
			close(server_fd);
			throw std::runtime_error("Failed to bind to port " + Utils::intToString(unique_ports[i]));
		}

		if (listen(server_fd, SOMAXCONN) < 0) {
			close(server_fd);
			throw std::runtime_error("Failed to listen on port " + Utils::intToString(unique_ports[i]));
		}

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

void HttpServer::start() {
	running = true;
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

			if (server_fds.find(current_fds[i].fd) != server_fds.end()) {
				// Handle new connection
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				int client_fd = accept(current_fds[i].fd, (struct sockaddr*)&client_addr, &client_len);

				if (client_fd < 0) {
					if (errno != EAGAIN && errno != EWOULDBLOCK) {
						Logger::log(Logger::ERROR, "Fail connection");
						std::cerr << "Failed to accept connection" << std::endl;
					}
					continue;
				}

				if (client_fd < 0) {
					Logger::log(Logger::ERROR, "Invalid client_fd" + Utils::intToString(client_fd));
					close(client_fd);
					continue;
				}

				make_non_blocking(client_fd);

				// Add to poll set
				struct pollfd client_pollfd;
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;
				poll_fds.push_back(client_pollfd);

			} else {
				// Handle client socket
				if (current_fds[i].revents & POLLIN) {
					handle_client_read(current_fds[i].fd);
				}
				if (current_fds[i].revents & POLLOUT) {
					handle_client_write(current_fds[i].fd);
				}
				if (current_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					Logger::log(Logger::ERROR, "Detected closed connection or error: client_fd=" + Utils::intToString(current_fds[i].fd));
					close_client(current_fds[i].fd);
				}
			}
		}
	}
}

void HttpServer::stop() {
	running = false;
	close(server_fd);

	// Close all client connections
	for (size_t i = 0; i < poll_fds.size(); ++i) {
		if (poll_fds[i].fd != server_fd) {
			close(poll_fds[i].fd);
		}
	}
	for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
		close(it->second);  // close socket
	}
	server_fds.clear();
	poll_fds.clear();
	partial_requests.clear();
	partial_responses.clear();
}

HttpServer::~HttpServer() {
	if (running) {
		Logger::log(Logger::INFO, "Stopping Server");
		stop();
	}
}
