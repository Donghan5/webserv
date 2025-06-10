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

enum CgiStatus {
	CGI_RUNNING = 0,
	CGI_COMPLETED = 1,
	CGI_TIMED_OUT = -1,
	CGI_CODE_500 = 500, // Internal Server Error
	CGI_CODE_502 = 502, // Bad Gateway
	CGI_CODE_504 = 504 // Gateway Timeout
};

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

		bool setUpPipes(void);
		bool isTimedOut(void) const;
		char **convertEnvToCharArray(void);
		char **convertArgsToCharArray(const std::string &interpreter);
		std::string createErrorResponse(const std::string& status, const std::string& message);

		time_t _start_time;
    	int _timeout;

	public:
		CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &env, const std::string &body);
		~CgiHandler();

		bool startCgi();
		bool isCgiRunning() const { return _process_running; }
		int getOutputFd() const { return _output_pipe[0]; }
		bool closeAndExitUnusedPipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1);
		bool writeToCgi(const char* data, size_t len);
		std::string readFromCgi();
		void closeCgi();
		CgiStatus checkCgiStatus();
		void closePipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1);
};

#endif
