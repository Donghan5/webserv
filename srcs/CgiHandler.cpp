#include "../includes/CgiHandler.hpp"
#include "../includes/AConfigBase.hpp"
#include "Logger.hpp"

CgiHandler::CgiHandler(const STR &scriptPath, const MAP<STR, STR> &env, const STR &body):
    _scriptPath(scriptPath), _env(env), _body(body), _cgi_pid(-1), _process_running(false) {
	_interpreters[".py"] = "/usr/bin/python3";
	_interpreters[".php"] = "/usr/bin/php";
	_interpreters[".pl"] = "/usr/bin/perl";
	_interpreters[".sh"] = "/bin/bash";

    // Initialize pipes with invalid values
    _input_pipe[0] = _input_pipe[1] = -1;
    _output_pipe[0] = _output_pipe[1] = -1;
}

CgiHandler::~CgiHandler() {
    closeCgi();
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

// Traditional synchronous execution (for backward compatibility)
STR CgiHandler::executeCgi() {
	int pipefd_in[2], pipefd_out[2];

	if (pipe(pipefd_in) == -1 || pipe(pipefd_out) == -1) {
		return "500 Internal Server Error\r\n\r\nPipe error";
	}

	pid_t pid = fork();
	if (pid < 0) {
		return "500 Internal Server Error\r\n\r\nFork error";
	}

	if (pid == 0) {
		// Child process
		dup2(pipefd_out[1], STDOUT_FILENO);
		close(pipefd_out[0]);
		close(pipefd_out[1]);

		dup2(pipefd_in[0], STDIN_FILENO);
		close(pipefd_in[0]);
		close(pipefd_in[1]);

		char **envp = convertEnvToCharArray();
		STR extension = _scriptPath.substr(_scriptPath.find_last_of("."));
		MAP<STR, STR>::const_iterator it = _interpreters.find(extension);
		if (it == _interpreters.end()) { // free the memory
			for(size_t i = 0; envp[i] != NULL; i++) {
				free(envp[i]);
			}
			delete[] envp;
			exit(1);
		}

		char **args = convertArgsToCharArray(it->second);
		execve(args[0], args, envp);

		for (size_t i = 0; envp[i] != NULL; i++) {
			free(envp[i]);
		}
		delete[] envp;

		for (size_t i = 0; args[i] != NULL; i++) {
			free(args[i]);
		}
		delete[] args;
		exit(1);
	} else {
		// Parent process
		close(pipefd_out[1]);
		close(pipefd_in[0]);

		if (!_body.empty()) {
			write(pipefd_in[1], _body.c_str(), _body.size());
		}
		close(pipefd_in[1]);

		char buffer[4096];
		STR output;
		int byteRead;

		while ((byteRead = read(pipefd_out[0], buffer, sizeof(buffer))) > 0) {
			output.append(buffer, byteRead);
		}
		close(pipefd_out[0]);

		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + output;
		} else {
			return "500 Internal Server Error\r\n\r\nCGI Execution Failed";
		}
	}
}

// Fix the CgiHandler::startCgi method to better handle the CGI script
bool CgiHandler::startCgi() {
    // Create pipes for communication with the CGI process
    if (pipe(_input_pipe) == -1 || pipe(_output_pipe) == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to create pipes for CGI: " + STR(strerror(errno)));
        return false;
    }

    // Set the output pipe to non-blocking mode for epoll
    int flags = fcntl(_output_pipe[0], F_GETFL, 0);
    if (flags == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to get flags for CGI output pipe: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }
    if (fcntl(_output_pipe[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking mode for CGI output pipe: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    // Check if the script exists and is executable
    if (access(_scriptPath.c_str(), F_OK | X_OK) != 0) {
        Logger::cerrlog(Logger::ERROR, "CGI script does not exist or is not executable: " + _scriptPath);
        closeCgi();
        return false;
    }

    // Fork a child process to execute the CGI script
    _cgi_pid = fork();
    if (_cgi_pid < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to fork for CGI: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    if (_cgi_pid == 0) {
        // Child process

        // Redirect stdin to input pipe
        dup2(_input_pipe[0], STDIN_FILENO);
        close(_input_pipe[0]);
        close(_input_pipe[1]);

        // Redirect stdout to output pipe
        dup2(_output_pipe[1], STDOUT_FILENO);
        close(_output_pipe[0]);
        close(_output_pipe[1]);

        // Convert environment variables for execve
        char **envp = convertEnvToCharArray();

        // Find the appropriate interpreter
        STR extension = _scriptPath.substr(_scriptPath.find_last_of("."));
        MAP<STR, STR>::const_iterator it = _interpreters.find(extension);
        if (it == _interpreters.end()) {
            Logger::cerrlog(Logger::ERROR, "No interpreter found for extension: " + extension);
            for(size_t i = 0; envp[i] != NULL; i++) {
                free(envp[i]);
            }
            delete[] envp;
            exit(1);
        }

        // Prepare arguments for execve
        char **args = convertArgsToCharArray(it->second);

        // Debug output
        Logger::cerrlog(Logger::INFO, "Executing CGI script: " + it->second + " " + _scriptPath);

        execve(args[0], args, envp);

        //----------------------------------------------------------------------------------RECHECK, SOMETIMES FAILS (bad python)
        // If execve() returns, it means it failed
        int err = errno; // Save errno because other system calls might change it
        Logger::cerrlog(Logger::ERROR, "Failed to execute CGI script: " + STR(strerror(err)));

        // Clean up resources
        for (size_t i = 0; envp[i] != NULL; i++) {
            free(envp[i]);
        }
        delete[] envp;

        for (size_t i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        delete[] args;

        // Exit the child process with error code
        exit(1);
    } else {
        // Parent process
        // Close unused pipe ends
        close(_input_pipe[0]);
        close(_output_pipe[1]);

		_process_running = true;

		// Write request body to CGI script's stdin
        if (!_body.empty()) {
			// Logger::cerrlog(Logger::INFO, "Sending body to CGI script, size: " + Utils::intToString(_body.size()) + " bytes");
			bool write_success = writeToCgi(_body.c_str(), _body.size());

			if (!write_success) {
				Logger::cerrlog(Logger::ERROR, "Failed to write request body to CGI");
				closeCgi();
				return false;
			}
			// writeToCgi(_body.c_str(), _body.size());
        }

        // Close input pipe after writing, so the CGI process gets EOF on stdin
        close(_input_pipe[1]);
        _input_pipe[1] = -1;

        // Mark the process as running
		// _process_running = true;

        return true;
    }
}

// bool CgiHandler::writeToCgi(const char* data, size_t len) {
//     if (!_process_running || _input_pipe[1] == -1) {
//         return false;
//     }

//     // Write data to the CGI process
//     ssize_t written = write(_input_pipe[1], data, len);
//     if (written < 0) {
//         Logger::cerrlog(Logger::ERROR, "Failed to write to CGI: " + STR(strerror(errno)));
//         return false;
//     }

//     // If we're done writing, close the pipe
//     if ((size_t)written == len) {
//         close(_input_pipe[1]);
//         _input_pipe[1] = -1;
//     }

//     return true;
// }

// testing
bool CgiHandler::writeToCgi(const char* data, size_t len) {
    if (!_process_running || _input_pipe[1] == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to write to CGI: process not running or pipe closed");
        return false;
    }

    Logger::cerrlog(Logger::INFO, "Starting to write " + Utils::intToString(len) + " bytes to CGI script");

    const size_t chunk_size = 1024;  // chunk size for writing
    size_t total_written = 0;

    while (total_written < len) {
        size_t to_write = std::min(chunk_size, len - total_written);
        ssize_t written = write(_input_pipe[1], data + total_written, to_write);

        if (written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // wait for a moment and redo
                continue;
            }
            Logger::cerrlog(Logger::ERROR, "Failed to write to CGI: " + STR(strerror(errno)));
            return false;
        }

        if (written == 0) {
            Logger::cerrlog(Logger::ERROR, "Write returned 0 bytes unexpectedly");
            break;
        }

        total_written += written;

        // Logging progress
        if (len > 10000 && (total_written % (len / 10) < written)) {
            Logger::cerrlog(Logger::INFO, "Written " + Utils::intToString(total_written) +
                            " of " + Utils::intToString(len) + " bytes (" +
                            Utils::intToString((total_written * 100) / len) + "%)");
        }
    }

    if (total_written < len) {
        Logger::cerrlog(Logger::ERROR, "Only wrote " + Utils::intToString(total_written) +
                       " of " + Utils::intToString(len) + " bytes");
        return false;
    }

    Logger::cerrlog(Logger::INFO, "Successfully wrote all " + Utils::intToString(len) + " bytes to CGI");
    return true;
}

// bool CgiHandler::writeToCgi(const char* data, size_t len) {
//     if (!_process_running || _input_pipe[1] == -1) {
//         return false;
//     }

//     // Write data to the CGI process
//     size_t total_written = 0;
//     while (total_written < len) {
//         ssize_t written = write(_input_pipe[1], data + total_written, len - total_written);

//         if (written < 0) {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 // Resource temporarily unavailable, try again
//                 continue;
//             }
//             Logger::cerrlog(Logger::ERROR, "Failed to write to CGI: " + STR(strerror(errno)));
//             return false;
//         }

//         if (written == 0) {
//             // This shouldn't happen, but if it does, avoid infinite loop
//             break;
//         }

//         total_written += written;
//     }

//     // Only close the pipe after ALL data has been written --> will be testing after check the leak
//     // if (total_written == len) {
//     //     close(_input_pipe[1]);
//     //     _input_pipe[1] = -1;
//     // }

//     return true;
// }

STR CgiHandler::readFromCgi() {
    if (!_process_running || _output_pipe[0] == -1) {
        return "";
    }

    char buffer[4096];
    STR output;
    ssize_t bytesRead;

    // Read available data from the CGI process
    while ((bytesRead = read(_output_pipe[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytesRead);
    }

    if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        Logger::cerrlog(Logger::ERROR, "Failed to read from CGI: " + STR(strerror(errno)));
    }

    _output_buffer += output;
    return output;
}

bool CgiHandler::checkCgiStatus() {
    if (!_process_running) {
        return true; // Already done
    }

    int status;
    pid_t result = waitpid(_cgi_pid, &status, WNOHANG);

    if (result == 0) {
        // Process is still running
        return false;
    } else if (result == _cgi_pid) {
        // Process has exited
        _process_running = false;

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Successful exit
            return true;
        } else {
            // Error exit
            Logger::cerrlog(Logger::ERROR, "CGI process exited with error: " + Utils::intToString(WEXITSTATUS(status)));
            return true;
        }
    } else {
        // Error checking status
        Logger::cerrlog(Logger::ERROR, "Failed to check CGI status: " + STR(strerror(errno)));
        _process_running = false;
        return true;
    }
}

void CgiHandler::closeCgi() {
    // Close pipes
    if (_input_pipe[0] >= 0) {
        close(_input_pipe[0]);
        _input_pipe[0] = -1;
    }
    if (_input_pipe[1] >= 0) {
        close(_input_pipe[1]);
        _input_pipe[1] = -1;
    }
    if (_output_pipe[0] >= 0) {
        close(_output_pipe[0]);
        _output_pipe[0] = -1;
    }
    if (_output_pipe[1] >= 0) {
        close(_output_pipe[1]);
        _output_pipe[1] = -1;
    }

    // If the process is still running, terminate it
    if (_process_running && _cgi_pid > 0) {
        kill(_cgi_pid, SIGTERM);
        int status;
        waitpid(_cgi_pid, &status, 0);
        _process_running = false;
    }
}
