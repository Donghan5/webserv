#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP
#pragma once

#define BUFFER_SIZE 4096

#include <map>
#include <vector>
#include <poll.h>
#include "Logger.hpp"
#include "SocketManager.hpp"
#include "ParsedRequest.hpp"
#include "RequestHandler.hpp"

class Client {
	private:
		std::vector<struct pollfd>& poll_fds;
		std::map<int, std::string> partial_requests;
		std::map<int, std::string> partial_responses;

	public:
		Client(std::vector<struct pollfd>& poll_fds);
		~Client();

		void handleClientRead(int client_fd);
		void handleClientWrite(int client_fd);
		void closeClient(int client_fd);
};

#endif
