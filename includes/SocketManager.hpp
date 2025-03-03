#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP
#pragma once

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdexcept>
#include "Logger.hpp"
#include "Utils.hpp"

class SocketManager {
	public:
		static int createSocket();
		static void setSocket(int fd);
		static void makeNonBlocking(int fd);
		static void bindSocket (int fd, int port);
		static void listenSocket (int fd);
		static void closeSocket (int fd);
};

#endif
