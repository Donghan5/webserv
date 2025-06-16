#include "../includes/CgiHandler.hpp"
#include "../includes/AConfigBase.hpp"
#include "Logger.hpp"

CgiHandler::CgiHandler(const STR &scriptPath, const MAP<STR, STR> &env, const STR &body):
    _scriptPath(scriptPath), _env(env), _body(body), _cgi_pid(-1), _process_running(false),
    _start_time(time(NULL)), _timeout(30)
{
    _interpreters[".py"] = "/usr/bin/python3";
    _interpreters[".php"] = "/usr/bin/php";
    _interpreters[".pl"] = "/usr/bin/perl";
    _interpreters[".sh"] = "/bin/bash";

    _input_pipe[0] = _input_pipe[1] = -1;
    _output_pipe[0] = _output_pipe[1] = -1;
}

CgiHandler::~CgiHandler() {
    closeCgi();
}

bool CgiHandler::isTimedOut(void) const {
	time_t now = time(NULL);
	if (now == (time_t)(-1)) {
			return false;
	}
	return (now - _start_time) > _timeout;
}

bool CgiHandler::setUpPipes(void) {
	if (pipe(_input_pipe) == -1) {
		Logger::log(Logger::ERROR, "Failed to create input pipe: " + STR(strerror(errno)));
		_input_pipe[0] = _input_pipe[1] = -1;
		return false;
	}

	if (pipe(_output_pipe) == -1) {
		Logger::log(Logger::ERROR, "Failed to create output pipe: " + STR(strerror(errno)));
		close(_input_pipe[0]);
		close(_input_pipe[1]);
		_output_pipe[0] = _output_pipe[1] = -1;
		return false;
	}

	int flags = fcntl(_output_pipe[0], F_GETFL, 0);
	if (flags == -1) {
		Logger::log(Logger::ERROR, "Failed to set non-blocking modes for output pipe: " + STR(strerror(errno)));
		closeCgi();
		return false;
	}

	if (fcntl(_output_pipe[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::log(Logger::ERROR, "Failed to set non-blocking mode: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

	return true;
}
/*
	Convert to envp using execve function
*/
char **CgiHandler::convertEnvToCharArray(void) {
	char **envp = new char*[_env.size() + 1];
	int i = 0;
	MAP<STR, STR>::const_iterator it = _env.begin();
	for (; it != _env.end(); it++) {
		STR envEntry = it->first + "=" + it->second;
		envp[i] = strdup(envEntry.c_str());
		i++;
	}
	envp[i] = NULL;
	return envp;
}

/*
	args to launch cgi
*/
char **CgiHandler::convertArgsToCharArray(const STR &interpreter) {
	char **args = new char*[3];
	args[0] = strdup(interpreter.c_str());
	args[1] = strdup(_scriptPath.c_str());
	args[2] = NULL;
	return (args);
}

STR CgiHandler::createErrorResponse(const STR& status, const STR& message) {
    return "HTTP/1.1 " + status + " Error\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: " + Utils::intToString(message.length()) + "\r\n"
           "\r\n"
           + message;
}

bool CgiHandler::closeAndExitUnusedPipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1) {
	if (close(input_pipe1) == -1) {
		Logger::log(Logger::ERROR, "Child: Failed to close input_pipe[1]: " + STR(strerror(errno)));
        return false;
	}

	if (close(output_pipe0) == -1) {
		Logger::log(Logger::ERROR, "Child: Failed to close output_pipe[0]: " + STR(strerror(errno)));
        return false;
	}

	// Set up stdin from input pipe
	if (dup2(input_pipe0, STDIN_FILENO) == -1) {
		Logger::log(Logger::ERROR, "Child: Failed to redirect stdin: " + STR(strerror(errno)));
        return false;
	}
	close(input_pipe0);

	// Set up stdout to output pipe
	if (dup2(output_pipe1, STDOUT_FILENO) == -1) {
		Logger::log(Logger::ERROR, "Child: Failed to redirect stdout: " + STR(strerror(errno)));
        return false;
	}
	close(output_pipe1);
    return true;
}

bool CgiHandler::startCgi() {
    Logger::log(Logger::INFO, "Starting CGI script: " + _scriptPath);
	if (!setUpPipes()) {
		return false;
	}

    if (access(_scriptPath.c_str(), F_OK | X_OK) != 0) {
        Logger::log(Logger::ERROR, "CGI script not executable: " + _scriptPath + " - " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    int input_pipe0 = _input_pipe[0];
    int input_pipe1 = _input_pipe[1];
    int output_pipe0 = _output_pipe[0];
    int output_pipe1 = _output_pipe[1];

    _cgi_pid = fork();

    if (_cgi_pid < 0) {
        Logger::log(Logger::ERROR, "Failed to fork: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    if (_cgi_pid == 0) {
        _start_time = time(NULL);
        _timeout = 30;

		if (!closeAndExitUnusedPipes(input_pipe0, input_pipe1, output_pipe0, output_pipe1))
            return 1;

        char **envp = convertEnvToCharArray();
        if (!envp) {
            Logger::log(Logger::ERROR, "Child: Failed to create environment");
            return 1;
        }

        STR extension = _scriptPath.substr(_scriptPath.find_last_of("."));
        MAP<STR, STR>::const_iterator it = _interpreters.find(extension);
        if (it == _interpreters.end()) {
            Logger::log(Logger::ERROR, "Child: No interpreter for " + extension);
            Utils::cleanUpDoublePointer(envp);
            return 1;
        }

        char **args = convertArgsToCharArray(it->second);
        if (!args) {
            Logger::log(Logger::ERROR, "Child: Failed to create args");
            Utils::cleanUpDoublePointer(envp);
            return 1;
        }

        // Logger::log(Logger::INFO, "Executing: " + it->second + " " + _scriptPath);
        
        // Execute the script
        execve(args[0], args, envp);

        Logger::log(Logger::ERROR, "Child: execve failed: " + STR(strerror(errno)));

        Utils::cleanUpDoublePointer(envp);
        Utils::cleanUpDoublePointer(args);

        return 1;
    } else {
        // Parent process
        if (close(input_pipe0) == -1) {
            Logger::log(Logger::ERROR, "Parent: Failed to close input_pipe[0]: " + STR(strerror(errno)));
        }
        _input_pipe[0] = -1;

        if (close(output_pipe1) == -1) {
            Logger::log(Logger::ERROR, "Parent: Failed to close output_pipe[1]: " + STR(strerror(errno)));
        }
        _output_pipe[1] = -1;

        _start_time = time(NULL);
        _timeout = 30; // 30 seconds timeout
        _process_running = true;

        if (!_body.empty()) {
            Logger::log(Logger::DEBUG, "Sending body to CGI (size: " + Utils::intToString(_body.size()) + " bytes)");
            if (!writeToCgi(_body.c_str(), _body.size())) {
                Logger::log(Logger::ERROR, "Failed to write request body to CGI");
                closeCgi();
                return false;
            }
        }

        if (_input_pipe[1] >= 0) {
            if (close(_input_pipe[1]) == -1) {
                Logger::log(Logger::ERROR, "Parent: Failed to close input_pipe[1] after writing: " + STR(strerror(errno)));
            }
            _input_pipe[1] = -1; 
        }

        return true;
    }
}

STR CgiHandler::readFromCgi() {
    if (!_process_running || _output_pipe[0] < 0) {
        return "";
    }

    char buffer[8192];

    ssize_t bytes_read = read(_output_pipe[0], buffer, sizeof(buffer));

    if (bytes_read > 0) {
        Logger::log(Logger::DEBUG, "Read " + Utils::intToString(bytes_read) +
                       " bytes from CGI output");
        _output_buffer.append(buffer, bytes_read);
        return STR(buffer, bytes_read);
    }
	else if (bytes_read == 0) {  // EOF - pipe closed
        _process_running = false;
    }

    return "";
}

bool CgiHandler::writeToCgi(const char* data, size_t len) {
    if (!_process_running || _input_pipe[1] < 0) {
        Logger::log(Logger::ERROR, "Cannot write to CGI: process not running or pipe closed");
        return false;
    }

    ssize_t bytes_written = write(_input_pipe[1], data, len);

    if (bytes_written < 0) {
        return false;
    }

    Logger::log(Logger::DEBUG, "Successfully wrote " + Utils::intToString(bytes_written) +
                  " bytes to CGI input");
    return true;
}

CgiStatus CgiHandler::checkCgiStatus() {
    // Check for timeout
    if (isTimedOut()) {
        Logger::log(Logger::WARNING, "CGI process timed out after " +
                       Utils::intToString(_timeout) + " seconds");
        closeCgi();
        _process_running = false;
        return CGI_TIMED_OUT;
    }

    if (_cgi_pid <= 0) {
        _process_running = false;
        return CGI_COMPLETED;
    }

    if (_process_running && isTimedOut()) {
        Logger::log(Logger::WARNING, "CGI process timed out after " +
                      Utils::intToString(_timeout) + " seconds");
        closeCgi();
        return CGI_TIMED_OUT;
    }

    int status_code = 0;
    pid_t result = waitpid(_cgi_pid, &status_code, WNOHANG);

    if (result == 0) {
        return CGI_RUNNING;
    }
	else if (result == _cgi_pid) {
        _process_running = false;

        if (WIFEXITED(status_code)) {
            int exit_code = WEXITSTATUS(status_code);
            if (exit_code == 0) {
                return CGI_COMPLETED;
            } else {
                Logger::log(Logger::WARNING, "CGI process exited with code: " +
                          Utils::intToString(exit_code));
                return CGI_CODE_502;
            }
        } else if (WIFSIGNALED(status_code)) {
            Logger::log(Logger::WARNING, "CGI process terminated by signal: " +
                      Utils::intToString(WTERMSIG(status_code)));
            _process_running = false;
            return CGI_CODE_500; 
        }
        return CGI_COMPLETED;
    }
    _process_running = false;
    return CGI_COMPLETED;
}

void CgiHandler::closePipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1) {
    if (input_pipe0 >= 0) {
        if (close(input_pipe0) < 0 && errno != EBADF) {
            Logger::log(Logger::DEBUG, "Failed to close input pipe[0]: " + STR(strerror(errno)));
        }
    }

    if (input_pipe1 >= 0) {
        if (close(input_pipe1) < 0 && errno != EBADF) {
            Logger::log(Logger::DEBUG, "Failed to close input pipe[1]: " + STR(strerror(errno)));
        }
    }

    if (output_pipe0 >= 0) {
        if (close(output_pipe0) < 0 && errno != EBADF) {
            Logger::log(Logger::DEBUG, "Failed to close output pipe[0]: " + STR(strerror(errno)));
        }
    }

    if (output_pipe1 >= 0) {
        if (close(output_pipe1) < 0 && errno != EBADF) {
            Logger::log(Logger::DEBUG, "Failed to close output pipe[1]: " + STR(strerror(errno)));
        }
    }
}

void CgiHandler::closeCgi() {
    Logger::log(Logger::DEBUG, "CgiHandler::closeCgi: Cleaning up resources");

    _process_running = false;

    int input_pipe0 = _input_pipe[0];
    int input_pipe1 = _input_pipe[1];
    int output_pipe0 = _output_pipe[0];
    int output_pipe1 = _output_pipe[1];

    _input_pipe[0] = _input_pipe[1] = -1;
    _output_pipe[0] = _output_pipe[1] = -1;

	closePipes(input_pipe0, input_pipe1, output_pipe0, output_pipe1);

    pid_t pid = _cgi_pid;
    _cgi_pid = -1;
    if (pid > 0) {
        Logger::log(Logger::DEBUG, "Terminating CGI process " + Utils::intToString(pid));
        Logger::log(Logger::INFO, "Closing CGI process: " + _scriptPath);

        kill(pid, SIGTERM);
        int status;
        if (waitpid(pid, &status, WNOHANG) == 0) {
            usleep(500);

            if (waitpid(pid, &status, WNOHANG) == 0) {
                // still running, use SIGKILL
                Logger::log(Logger::WARNING, "CGI process didn't terminate gracefully, using SIGKILL");
                kill(pid, SIGKILL);
                // Non-blocking wait
                waitpid(pid, &status, WNOHANG);
            }
        }
    }
    Logger::log(Logger::DEBUG, "CgiHandler::closeCgi: Resources cleaned up");
}
