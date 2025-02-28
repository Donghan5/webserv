#include "../includes/SocketManager.hpp"

/*
	creating socket
*/
int SocketManager::createSocket(void) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) throw std::runtime_error("Fail to create socket");

	return fd;
}

// Helper function to make socket non-blocking
void SocketManager::makeNonBlocking(int fd) {
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


void SocketManager::bindSocket(int fd, int port) {
	struct sockaddr_in address;
	memset(&adderss, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		close(fd);
		throw std::runtime_error("Failed to bind to port" + Utils::intToString(port));
	}
}

void SocketManager::listenSocket(int fd) {
	if (listen(fd, SOMAXCONN) < 0) {
		close(fd);
		throw std::runtime_error("Failed to listen on socket");
	}
}

void SocketManager::closeSocket(int fd) {
	shutdown(fd, SHUT_RDWR);
	close(fd);
}
