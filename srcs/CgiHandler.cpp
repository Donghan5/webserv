#include "../includes/CgiHandler.hpp"
#include "../includes/Logger.hpp"

CgiHandler::CgiHandler(const std::string &scriptPath, const std::map<std::string, std::string> &env, const std::string &body): _scriptPath(scriptPath), _env(env), _body(body) {
	_interpreters[".py"] = "/usr/bin/python3";
	_interpreters[".php"] = "/usr/bin/php";
	_interpreters[".pl"] = "/usr/bin/perl";
	_interpreters[".sh"] = "/bin/bash";
}

CgiHandler::~CgiHandler() {}

/*
	Convert to envp using execve function
*/
char **CgiHandler::convertEnvToCharArray() {
	char **envp = new char*[_env.size() + 1];
	int i = 0;
	std::map<std::string, std::string>::const_iterator it = _env.begin();
	for (; it != _env.end(); it++) {
		std::string envEntry = it->first + "=" + it->second;
		envp[i] = strdup(envEntry.c_str());
		i++;
	}
	envp[i] = NULL;
	return envp;
}

/*
	args to launch cgi
*/
char **CgiHandler::convertArgsToCharArray(const std::string &interpreter) {
	char **args = new char*[3];
	args[0] = strdup(interpreter.c_str());
	args[1] = strdup(_scriptPath.c_str());
	args[2] = NULL;
	return (args);
}

std::string CgiHandler::executeCgi() {
	int pipefd_in[2], pipefd_out[2];

	if (pipe(pipefd_in) == -1 || pipe(pipefd_out) == -1) {
		Logger::log(Logger::ERROR, "Fail to create pipe");
		return "500 Internal Server Error\r\n\r\nPipe error";
	}

	pid_t pid = fork();
	if (pid < 0) {
		Logger::log(Logger::ERROR, "Fail to create pid");
		return "500 Internal Server Error\r\n\r\nFork error";
	}

	if (pid == 0) {
		dup2(pipefd_out[1], STDOUT_FILENO);
		close(pipefd_out[0]);
		close(pipefd_out[1]);

		dup2(pipefd_in[0], STDIN_FILENO);
		close(pipefd_in[0]);
		close(pipefd_in[1]);

		char **envp = convertEnvToCharArray();
		std::string extension = _scriptPath.substr(_scriptPath.find_last_of("."));
		std::map<std::string, std::string>::const_iterator it = _interpreters.find(extension);
		if (it == _interpreters.end()) {
			Logger::log(Logger::ERROR, "Fail to search extension");
			exit(1);
		}

		char **args = convertArgsToCharArray(it->second);
		Logger::log(Logger::INFO, "Attempting to execute CGI script:");
		Logger::log(Logger::INFO, "Interpreter: " + std::string(args[0]));
		Logger::log(Logger::INFO, "Script Path: " + std::string(args[1]));
		execve(args[0], args, envp);

		Logger::log(Logger::ERROR, "Fail to execute CGI script: " + std::string(args[0]) + " " + std::string(args[1]));
		exit(1);
	} else {
		close(pipefd_out[1]);
		close(pipefd_in[0]);

		if (!_body.empty()) {
			write(pipefd_in[1], _body.c_str(), _body.size());
		}
		close(pipefd_in[1]);

		char buffer[4096];
		std::string output;
		int byteRead;

		while ((byteRead = read(pipefd_out[0], buffer, sizeof(buffer))) > 0) {
			output.append(buffer, byteRead);
		}
		close(pipefd_out[0]);

		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			Logger::log(Logger::INFO, "CGI execution successful");
			return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + output;
		} else {
			Logger::log(Logger::ERROR, "CGI Execution Failed");
			return "500 Internal Server Error\r\n\r\nCGI Execution Failed";
		}
	}
}
