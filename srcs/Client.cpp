#include "../includes/Client.hpp"

// constructor
Client::Client(std::vector<struct pollfd>& poll_fds) : poll_fds(poll_fds) {
	partial_requests.clear();
	partial_responses.clear();
}

// destructor
Client::~Client() {
	for (std::map<int, std::string>::iterator it = partial_requests.begin(); it != partial_requests.end(); ++it) {
		closeClient(it->first);
	}
	partial_requests.clear();
	partial_responses.clear();
}

void Client::handleClientRead(int client_fd) {
	if (client_fd <= 0) {
		Logger::log(Logger::ERROR, "Attempted to read from an invalid client_fd: client_fd=" + Utils::intToString(client_fd));
		return;
	}

	int socket_status = fcntl(client_fd, F_GETFL, 0);
	if (socket_status == -1) {
		Logger::log(Logger::ERROR, "Invalid socket: client_fd=" + Utils::intToString(client_fd));
		closeClient(client_fd);
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
			closeClient(client_fd);
			return;
		}
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			Logger::log(Logger::DEBUG, "Temporary read error, retrying: client_fd=" + Utils::intToString(client_fd));
			return;
		}
		Logger::log(Logger::ERROR, "Socket error before read: client_fd=" + Utils::intToString(client_fd) + ", errno=" + strerror(errno));
		closeClient(client_fd);
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
			closeClient(client_fd);
		}
		return ;
	}

	if (bytes_read == 0) {
		Logger::log(Logger::ERROR, "Client disconnected: client_fd=" + Utils::intToString(client_fd));
		closeClient(client_fd);
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
		std::string response = RequestHandler::process_request(partial_requests[client_fd]);
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

void Client::handleClientWrite(int client_fd) {
	std::string& response = partial_responses[client_fd];
	ssize_t bytes_written = write(client_fd, response.c_str(), response.length());

	if (bytes_written <= 0) {
		if (errno != EAGAIN) {
			closeClient(client_fd);
		}
		return;
	}

	response.erase(0, bytes_written);

	if (response.empty()) {
		closeClient(client_fd);
	}
}

void Client::closeClient(int client_fd) {
	if (client_fd < 0) return;

	// Remove from poll_fds
	for (size_t i = 0; i < poll_fds.size(); ++i) {
		if (poll_fds[i].fd == client_fd) {
			poll_fds.erase(poll_fds.begin() + i);
			break;
		}
	}

	Logger::log(Logger::INFO, "Close client_fd=" + Utils::intToString(client_fd));
	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	partial_requests.erase(client_fd);
	partial_responses.erase(client_fd);
}
