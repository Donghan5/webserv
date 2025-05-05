#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP
#pragma once

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <map>
#include <cstring>
#include "Utils.hpp"

#include <cerrno>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sstream>

class CgiHandler {
	private:
		std::string _scriptPath;
		std::map<std::string, std::string> _env;
		std::string _body;
		std::map<std::string, std::string> _interpreters;
        
		// For non-blocking operation
		pid_t _cgi_pid;
		int _input_pipe[2];
		int _output_pipe[2];
		bool _process_running;
		std::string _output_buffer;

		char **convertEnvToCharArray(void);
		char **convertArgsToCharArray(const std::string &interpreter);
		std::string createErrorResponse(const std::string& status, const std::string& message);

		time_t _start_time;
    	int _timeout;

	public:
		CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &env, const std::string &body);
		~CgiHandler();
        
		// New asynchronous methods for use with epoll
		bool startCgi(); // Returns true if successfully started
		bool isCgiRunning() const { return _process_running; }
		int getOutputFd() const { return _output_pipe[0]; }
		bool writeToCgi(const char* data, size_t len); // Write data to CGI input
		std::string readFromCgi(); // Read data from CGI output
		void closeCgi(); // Clean up resources
		bool checkCgiStatus(); // Check if CGI has completed, returns true if done
};

#endif
