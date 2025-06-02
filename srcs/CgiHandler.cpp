#include "../includes/CgiHandler.hpp"
#include "../includes/AConfigBase.hpp"
#include "Logger.hpp"

CgiHandler::CgiHandler(const STR &scriptPath, const MAP<STR, STR> &env, const STR &body):
    _scriptPath(scriptPath), _env(env), _body(body), _cgi_pid(-1), _process_running(false),
    _start_time(time(NULL)), _timeout(30)  // Add timeout initialization (30 seconds)
{
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

// check timeout logic
bool CgiHandler::isTimedOut(void) const {
	time_t now = time(NULL);
	if (now == (time_t)(-1)) {
			return false;
	}
	return (now - _start_time) > _timeout;
}


// Set-up pipes
bool CgiHandler::setUpPipes(void) {
	if (pipe(_input_pipe) == -1) {
		Logger::cerrlog(Logger::ERROR, "Failed to create input pipe: " + STR(strerror(errno)));
		_input_pipe[0] = _input_pipe[1] = -1;
		return false;
	}

	if (pipe(_output_pipe) == -1) {
		Logger::cerrlog(Logger::ERROR, "Failed to create output pipe: " + STR(strerror(errno)));
		close(_input_pipe[0]);
		close(_input_pipe[1]);
		_output_pipe[0] = _output_pipe[1] = -1;
		return false;
	}

	int flags = fcntl(_output_pipe[0], F_GETFL, 0);
	if (flags == -1) {
		Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking modes for output pipe: " + STR(strerror(errno)));
		closeCgi();
		return false;
	}

	if (fcntl(_output_pipe[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking mode: " + STR(strerror(errno)));
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

void CgiHandler::closeAndExitUnusedPipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1) {
	if (close(input_pipe1) == -1) {
		Logger::cerrlog(Logger::ERROR, "Child: Failed to close input_pipe[1]: " + STR(strerror(errno)));
		exit(1);
	}

	if (close(output_pipe0) == -1) {
		Logger::cerrlog(Logger::ERROR, "Child: Failed to close output_pipe[0]: " + STR(strerror(errno)));
		exit(1);
	}

	// Set up stdin from input pipe
	if (dup2(input_pipe0, STDIN_FILENO) == -1) {
		Logger::cerrlog(Logger::ERROR, "Child: Failed to redirect stdin: " + STR(strerror(errno)));
		exit(1);
	}
	close(input_pipe0);

	// Set up stdout to output pipe
	if (dup2(output_pipe1, STDOUT_FILENO) == -1) {
		Logger::cerrlog(Logger::ERROR, "Child: Failed to redirect stdout: " + STR(strerror(errno)));
		exit(1);
	}
	close(output_pipe1);
}

bool CgiHandler::startCgi() {
    // Initialize pipes with error checking
	if (!setUpPipes()) {  // set up the pipes (non-blocking etc...)
		return false;
	}

    // Check if the script exists and is executable
    if (access(_scriptPath.c_str(), F_OK | X_OK) != 0) {
        Logger::cerrlog(Logger::ERROR, "CGI script not executable: " + _scriptPath + " - " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    // Store pipe file descriptors before fork to avoid any possible race conditions
    int input_pipe0 = _input_pipe[0];
    int input_pipe1 = _input_pipe[1];
    int output_pipe0 = _output_pipe[0];
    int output_pipe1 = _output_pipe[1];

    // Fork a child process
    _cgi_pid = fork();

    if (_cgi_pid < 0) {
        Logger::cerrlog(Logger::ERROR, "Failed to fork: " + STR(strerror(errno)));
        closeCgi();
        return false;
    }

    if (_cgi_pid == 0) {
        // Child process
        _start_time = time(NULL);
        _timeout = 30; // 30 seconds timeout

        // Close unused pipe ends first
		closeAndExitUnusedPipes(input_pipe0, input_pipe1, output_pipe0, output_pipe1);

        // Convert environment variables for execve
        char **envp = convertEnvToCharArray();
        if (!envp) {
            Logger::cerrlog(Logger::ERROR, "Child: Failed to create environment");
            exit(1);
        }

        // Find the appropriate interpreter
        STR extension = _scriptPath.substr(_scriptPath.find_last_of("."));
        MAP<STR, STR>::const_iterator it = _interpreters.find(extension);
        if (it == _interpreters.end()) {
            Logger::cerrlog(Logger::ERROR, "Child: No interpreter for " + extension);
            Utils::cleanUpDoublePointer(envp);
            exit(1);
        }

        // Prepare arguments for execve
        char **args = convertArgsToCharArray(it->second);
        if (!args) {
            Logger::cerrlog(Logger::ERROR, "Child: Failed to create args");
            Utils::cleanUpDoublePointer(envp);
            exit(1);
        }

        Logger::cerrlog(Logger::INFO, "Executing: " + it->second + " " + _scriptPath);

        // Execute the script
        execve(args[0], args, envp);

        // If execve returns, an error occurred
        Logger::cerrlog(Logger::ERROR, "Child: execve failed: " + STR(strerror(errno)));

        // Clean up envp
        Utils::cleanUpDoublePointer(envp);

		// Clean up args
        Utils::cleanUpDoublePointer(args);

        exit(1);
    } else {
        // Parent process
        // CRITICAL: Close the pipes that the child process uses
        if (close(input_pipe0) == -1) {
            Logger::cerrlog(Logger::ERROR, "Parent: Failed to close input_pipe[0]: " + STR(strerror(errno)));
            // Don't return false yet, continue with cleanup
        }
        _input_pipe[0] = -1; // Mark as closed

        if (close(output_pipe1) == -1) {
            Logger::cerrlog(Logger::ERROR, "Parent: Failed to close output_pipe[1]: " + STR(strerror(errno)));
            // Don't return false yet, continue with cleanup
        }
        _output_pipe[1] = -1; // Mark as closed

        _start_time = time(NULL);
        _timeout = 30; // 30 seconds timeout
        _process_running = true;

        // Write request body to CGI script's stdin
        if (!_body.empty()) {
            Logger::cerrlog(Logger::DEBUG, "Sending body to CGI (size: " + Utils::intToString(_body.size()) + " bytes)");
            if (!writeToCgi(_body.c_str(), _body.size())) {
                Logger::cerrlog(Logger::ERROR, "Failed to write request body to CGI");
                closeCgi();
                return false;
            }
        }

        // CRITICAL: Close input pipe after writing to prevent SIGPIPE in child process
        if (_input_pipe[1] >= 0) {
            if (close(_input_pipe[1]) == -1) {
                Logger::cerrlog(Logger::ERROR, "Parent: Failed to close input_pipe[1] after writing: " + STR(strerror(errno)));
            }
            _input_pipe[1] = -1; // Mark as closed
        }

        return true;
    }
}

STR CgiHandler::readFromCgi() {
    if (!_process_running || _output_pipe[0] < 0) {
        return "";
    }

    char buffer[8192];

    // Simple read without poll
    ssize_t bytes_read = read(_output_pipe[0], buffer, sizeof(buffer));

    if (bytes_read > 0) {
        Logger::cerrlog(Logger::DEBUG, "Read " + Utils::intToString(bytes_read) +
                       " bytes from CGI output");
        _output_buffer.append(buffer, bytes_read);
        return STR(buffer, bytes_read);
    }
	else if (bytes_read == 0) {  // EOF - pipe closed
        _process_running = false;
    }
	else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        Logger::cerrlog(Logger::ERROR, "Failed to read from CGI: " + STR(strerror(errno)));
    }

    return "";
}

bool CgiHandler::writeToCgi(const char* data, size_t len) {
    if (!_process_running || _input_pipe[1] < 0) {
        Logger::cerrlog(Logger::ERROR, "Cannot write to CGI: process not running or pipe closed");
        return false;
    }

    // Simple write
    ssize_t bytes_written = write(_input_pipe[1], data, len);

    if (bytes_written < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Logger::cerrlog(Logger::ERROR, "Write to CGI failed: " + STR(strerror(errno)));
            return false;
        }
        return false; // Would block
    }

    Logger::cerrlog(Logger::DEBUG, "Successfully wrote " + Utils::intToString(bytes_written) +
                  " bytes to CGI input");
    return true;
}

bool CgiHandler::checkCgiStatus() {
    if (!_process_running) {
        return true; // Already done
    }

    // Check for timeout
    if (isTimedOut()) {
        Logger::cerrlog(Logger::WARNING, "CGI process timed out after " +
                       Utils::intToString(_timeout) + " seconds");
        closeCgi();
        return true; // Report as completed (timed out)
    }

    if (_cgi_pid <= 0) {
        _process_running = false;
        return true;
    }

    if (_process_running && isTimedOut()) {
        Logger::cerrlog(Logger::WARNING, "CGI process timed out after " +
                      Utils::intToString(_timeout) + " seconds");
        closeCgi();
        return true; // Report as completed (timed out)
    }

    int status;
    pid_t result = waitpid(_cgi_pid, &status, WNOHANG);

    if (result == 0) {
        // Process is still running
        return false;
    }
	else if (result == _cgi_pid) {
        // Process has exited
        _process_running = false;

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                Logger::cerrlog(Logger::INFO, "CGI process exited successfully");
            } else {
                Logger::cerrlog(Logger::WARNING, "CGI process exited with code: " +
                          Utils::intToString(exit_code));
            }
        } else if (WIFSIGNALED(status)) {
            Logger::cerrlog(Logger::WARNING, "CGI process terminated by signal: " +
                      Utils::intToString(WTERMSIG(status)));
        }
        return true;
    } else {
        // Error checking status
        Logger::cerrlog(Logger::ERROR, "Failed to check CGI status: " + STR(strerror(errno)));
        _process_running = false;
        return true;
    }
}

// Close all pipes safely
void CgiHandler::closePipes(int input_pipe0, int input_pipe1, int output_pipe0, int output_pipe1) {
    if (input_pipe0 >= 0) {
        if (close(input_pipe0) < 0 && errno != EBADF) {
            Logger::cerrlog(Logger::DEBUG, "Failed to close input pipe[0]: " + STR(strerror(errno)));
        }
    }

    if (input_pipe1 >= 0) {
        if (close(input_pipe1) < 0 && errno != EBADF) {
            Logger::cerrlog(Logger::DEBUG, "Failed to close input pipe[1]: " + STR(strerror(errno)));
        }
    }

    if (output_pipe0 >= 0) {
        if (close(output_pipe0) < 0 && errno != EBADF) {
            Logger::cerrlog(Logger::DEBUG, "Failed to close output pipe[0]: " + STR(strerror(errno)));
        }
    }

    if (output_pipe1 >= 0) {
        if (close(output_pipe1) < 0 && errno != EBADF) {
            Logger::cerrlog(Logger::DEBUG, "Failed to close output pipe[1]: " + STR(strerror(errno)));
        }
    }
}

// Close all pipes and clean up resources related to CGI
void CgiHandler::closeCgi() {
    // Log when we're cleaning up resources
    Logger::cerrlog(Logger::DEBUG, "CgiHandler::closeCgi: Cleaning up resources");

    // Set process as not running first to prevent further reads/writes
    _process_running = false;

    // Store fd values and set to -1 immediately to prevent double-close
    int input_pipe0 = _input_pipe[0];
    int input_pipe1 = _input_pipe[1];
    int output_pipe0 = _output_pipe[0];
    int output_pipe1 = _output_pipe[1];

    _input_pipe[0] = _input_pipe[1] = -1;
    _output_pipe[0] = _output_pipe[1] = -1;

	closePipes(input_pipe0, input_pipe1, output_pipe0, output_pipe1);

    // Store and clear pid
    pid_t pid = _cgi_pid;
    _cgi_pid = -1;

    // Terminate child process safely if it's still running
    if (pid > 0) {
        Logger::cerrlog(Logger::INFO, "Terminating CGI process " + Utils::intToString(pid));

        // Send SIGTERM first for graceful shutdown
        kill(pid, SIGTERM);

        // Use nonblocking waitpid to check if process exited
        int status;
        if (waitpid(pid, &status, WNOHANG) == 0) {
            // Process didn't exit immediately, wait briefly
            usleep(50000); // 50ms

            // Check again
            if (waitpid(pid, &status, WNOHANG) == 0) {
                // Still running, use SIGKILL
                Logger::cerrlog(Logger::WARNING, "CGI process didn't terminate gracefully, using SIGKILL");
                kill(pid, SIGKILL);
                // Non-blocking wait to avoid hanging if process is already gone
                waitpid(pid, &status, WNOHANG);
            }
        }
    }

    Logger::cerrlog(Logger::DEBUG, "CgiHandler::closeCgi: Resources cleaned up");
}
