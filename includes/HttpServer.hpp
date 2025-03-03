#ifndef HttpServer_HPP
#define HttpServer_HPP
#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <errno.h>
#include <algorithm>
#include "WebServConf.hpp"
#include "ParsedRequest.hpp"
#include "FileHandler.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"
#include "Utils.hpp"
#include "ClientManager.hpp"
#include "SocketManager.hpp"
#include "RequestHandler.hpp"

class HttpServer {
	private:
		WebServConf _webconf;
		bool running;
		static const int MAX_EVENTS = 100;
		std::map<int, int> server_fds;
		std::vector<struct pollfd> poll_fds;
		ClientManager clientManager;
		std::map<std::string, int> server_name_to_fd;

	public:
		HttpServer(const WebServConf &webconf);
		~HttpServer();

		void start();
		void runServerLoop(void);
		int waitForEvents(void);
		bool isServerSocket(int fd);
		void acceptNewClient(int server_fd);
		void handleClientEvents(struct pollfd &client_fd);
		void stop();
};

#endif
