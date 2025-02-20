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

class CgiHandler {
	private:
		std::string _scriptPath;
		std::map<std::string, std::string> _env;
		std::string _body;
		std::map<std::string, std::string> _interpreters;

		char **convertEnvToCharArray();
		char **convertArgsToCharArray(const std::string &interpreter);

	public:
		CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &env, const std::string &body);
		~CgiHandler();

		std::string executeCgi();
};

#endif
