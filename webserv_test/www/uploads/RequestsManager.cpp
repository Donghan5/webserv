#include "RequestsManager.hpp"
#include "Logger.hpp"
#include "sys/epoll.h"

RequestsManager::RequestsManager() {
    _config = NULL;
    _client_fd = -1;
}

RequestsManager::RequestsManager(int client_fd) {
    _config = NULL;
    _client_fd = client_fd;
    _partial_requests[_client_fd] = "";
    _partial_responses[_client_fd] = "";
}

RequestsManager::RequestsManager(const RequestsManager &obj) {
    _config = obj._config;
    _client_fd = obj._client_fd;
    _partial_requests[_client_fd] = "";
    _partial_responses[_client_fd] = "";
}

RequestsManager::RequestsManager(HttpConfig *config) {
    _config = config;
    _client_fd = -1;
}

RequestsManager::RequestsManager(HttpConfig *config, int client_fd) {
    _config = config;
    _client_fd = client_fd;
    _partial_requests[_client_fd] = "";
    _partial_responses[_client_fd] = "";
}

RequestsManager::~RequestsManager() {
    // Clean up any active responses
    for (MAP<int, Response*>::iterator it = _active_responses.begin(); it != _active_responses.end(); ++it) {
        if (it->second) {
            delete it->second;
        }
    }
    _active_responses.clear();
}

void RequestsManager::setConfig(HttpConfig *config) {
    _config = config;
    _partial_requests.erase(_client_fd);
    _partial_responses.erase(_client_fd);
    _client_states.erase(_client_fd);
}

void RequestsManager::setClientFd(int client_fd) {
    _client_fd = client_fd;
}


int RequestsManager::RegisterCgiFd(int cgi_fd, int client_fd) {
    if (cgi_fd < 0 || client_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Invalid file descriptors in RegisterCgiFd");
        return 0;
    }

    Logger::cerrlog(Logger::INFO, "Registering CGI fd " + Utils::intToString(cgi_fd) +
                   " for client " + Utils::intToString(client_fd));

    // Ensure the CGI fd is non-blocking
    int flags = fcntl(cgi_fd, F_GETFL, 0);
    if (flags == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to get flags for CGI fd: " + STR(strerror(errno)));
        return 0;
    }

    if (fcntl(cgi_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::cerrlog(Logger::ERROR, "Failed to set non-blocking for CGI fd: " + STR(strerror(errno)));
        return 0;
    }

    // Return the special code for PollServer to add this fd
    return 4; // Special code meaning "add CGI fd to epoll"
}

Response* RequestsManager::getCgiResponse(int client_fd) {
    MAP<int, Response*>::iterator it = _active_responses.find(client_fd);
    if (it != _active_responses.end()) {
        return it->second;
    }
    return NULL;
}

int RequestsManager::HandleRead() {
    ClientState &client_state = _client_states[_client_fd];
    long long &body_read = client_state.body_read;
    Request &request = client_state.request;
    bool done = false;

    if (_client_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "HandleRead: Invalid client fd");
        return 0;
    }

    try {
        char buffer[4096];
        int nbytes = read(_client_fd, buffer, sizeof(buffer));

        if (nbytes <= 0) {
            if (nbytes == 0) {
                Logger::cerrlog(Logger::INFO, "Client disconnected (read returned 0)");
            } else {
                Logger::cerrlog(Logger::ERROR, "Error reading from client: " + STR(strerror(errno)));
            }
            return 0; // Close the connection
        }

        // Track body read progress if needed
        if (body_read != -1) {
            body_read += nbytes;
        }

        // Append to the partial request
        _partial_requests[_client_fd].append(buffer, nbytes);

        // Check if we have a complete header
        int header_end = _partial_requests[_client_fd].find("\r\n\r\n");
        if (body_read == -1 && header_end != CHAR_NOT_FOUND) {
            Logger::log(Logger::INFO, "Complete headers received");

            // Parse the request headers
            request.clear();
            if (!request.setRequest(_partial_requests[_client_fd])) {
                Logger::cerrlog(Logger::ERROR, "Failed to parse request headers");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2; // Switch to write mode
            }

            // Check if body is needed
            if (request._body_size > 0) {
                Logger::cerrlog(Logger::DEBUG, "Request has body of size " + Utils::intToString(request._body_size));

                // Calculate how much of the body we've already received
                body_read = _partial_requests[_client_fd].size() - header_end - 4;

                if (body_read < (long long)request._body_size) {
                    Logger::log(Logger::INFO, "Body partially received: " +
                              Utils::intToString(body_read) + "/" + Utils::intToString(request._body_size));
                    return 1; // Keep in read mode until we get the full body
                } else {
                    // Full body received
                    done = true;
                }
            } else {
                // No need for body
                done = true;
            }
        }

        // Check if we've received the full body
        if (done || (body_read != -1 && body_read >= (long long)request._body_size)) {
            Logger::cerrlog(Logger::INFO, "Complete request received");

            // Parse the request with body
            request.clear();
            if (!request.setRequest(_partial_requests[_client_fd])) {
                Logger::cerrlog(Logger::ERROR, "Failed to parse complete request");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2; // Switch to write mode
            }

            if (!request.parseBody()) {
                Logger::cerrlog(Logger::ERROR, "Failed to parse request body");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2; // Switch to write mode
            }

            // Process the request
            try {
                // Create a new Response object for this request
                Response* response = new Response();
                response->setConfig(_config);
                response->setRequest(request);

                // Get the response
                STR response_text = response->getResponse();

                // Check if we need to handle CGI
                if (response_text.empty() && !response->isResponseReady()) {
                    // This is a CGI request
                    client_state.processing_cgi = true;
                    _active_responses[_client_fd] = response;

                    // Get the CGI output fd
                    int cgi_fd = response->getCgiOutputFd();
                    if (cgi_fd != -1) {
                        Logger::cerrlog(Logger::INFO, "Starting CGI processing for client " + Utils::intToString(_client_fd));
                        return RegisterCgiFd(cgi_fd, _client_fd);
                    } else {
                        Logger::cerrlog(Logger::ERROR, "Invalid CGI output fd");
                        delete response;
                        _active_responses.erase(_client_fd);
                        client_state.processing_cgi = false;
                        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                        return 2; // Switch to write mode
                    }
                } else {
                    // Normal response (not CGI)
                    _partial_responses[_client_fd] = response_text;
                    delete response;

                    // Reset client state for next request
                    body_read = -1;
                    client_state.processing_cgi = false;

                    // Clear the request buffer
                    _partial_requests.erase(_client_fd);

                    return 2; // Switch to write mode
                }
            } catch (const std::exception& e) {
                Logger::cerrlog(Logger::ERROR, "Error processing request: " + STR(e.what()));
                _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);

                // Reset client state
                body_read = -1;
                client_state.processing_cgi = false;

                return 2; // Switch to write mode
            }
        }

        // Need more data
        return 1;
    } catch (const std::exception& e) {
        body_read = -1;
        client_state.processing_cgi = false;
        Logger::cerrlog(Logger::ERROR, "Exception in HandleRead: " + STR(e.what()));
        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
        return 0; // Close the connection
    }
}

int RequestsManager::HandleWrite() {
    try {
        STR &response = _partial_responses[_client_fd];

        // Log response size for debugging
        Logger::cerrlog(Logger::DEBUG, "HandleWrite: Writing response of size " +
                        Utils::intToString(response.length()) + " bytes");

        // Try to write as much as possible
        ssize_t bytes_written = write(_client_fd, response.c_str(), response.length());

        if (bytes_written <= 0) {
            if (bytes_written == 0) {
                // Socket closed by peer
                Logger::cerrlog(Logger::INFO, "HandleWrite: Socket closed by peer");
                CloseClient();
                return 0;
            }
            // Real error
            Logger::cerrlog(Logger::ERROR, "HandleWrite error: " + STR(strerror(errno)));
            CloseClient();
            return 0;
        }

        Logger::cerrlog(Logger::INFO, "HandleWrite: Wrote " + Utils::intToString(bytes_written) +
                        " bytes out of " + Utils::intToString(response.length()));

        // Update response to remove written portion
        response.erase(0, bytes_written);

        if (response.empty()) {
            // All data has been sent, we're done with this client for now
            Logger::cerrlog(Logger::INFO, "HandleWrite: Response sent completely");

            // Reset the client state for the next request
            ClientState &client_state = _client_states[_client_fd];
            client_state.body_read = -1;
            client_state.processing_cgi = false;
            client_state.request.clear();

            // Clear the request buffer
            _partial_requests.erase(_client_fd);

            return 3; // Switch back to read mode
        } else {
            // More data to write, continue monitoring for write events
            Logger::cerrlog(Logger::DEBUG, "HandleWrite: Still have " +
                          Utils::intToString(response.length()) + " bytes to write");
            return 2; // Keep monitoring for write events
        }
    }
    catch(const std::exception& e) {
        Logger::cerrlog(Logger::ERROR, "HandleWrite error: " + STR(e.what()));
        CloseClient();
        return 0;
    }
}

int RequestsManager::HandleCgiOutput(int cgi_fd) {
    if (cgi_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "Invalid CGI fd in HandleCgiOutput");
        return 0;
    }

    // Find the client associated with this CGI
    int client_fd = -1;
    Response* response = NULL;

    for (MAP<int, Response*>::iterator it = _active_responses.begin(); it != _active_responses.end(); ++it) {
        if (it->second && it->second->getCgiOutputFd() == cgi_fd) {
            client_fd = it->first;
            response = it->second;
            break;
        }
    }

    if (client_fd == -1 || !response) {
        Logger::cerrlog(Logger::ERROR, "CGI fd " + Utils::intToString(cgi_fd) + " has no associated client");
        return 0;
    }

    // Get client state
    ClientState &client_state = _client_states[client_fd];

    // Make sure we're processing CGI
    if (!client_state.processing_cgi) {
        Logger::cerrlog(Logger::WARNING, "Client " + Utils::intToString(client_fd) +
                        " not marked as processing CGI, but received CGI output");
        // Continue processing anyway since we have a response object
    }

    try {
        // Process the CGI output
        bool completed = response->processCgiOutput();

        if (completed) {
            // CGI has finished
            Logger::cerrlog(Logger::INFO, "CGI processing completed for client " + Utils::intToString(client_fd));

            // Get the final response
            _partial_responses[client_fd] = response->getFinalResponse();

            // Clear the request buffer
            _partial_requests.erase(client_fd);

            // Reset client state
            client_state.body_read = -1;
            client_state.processing_cgi = false;

            // Clean up
            delete response;
            _active_responses.erase(client_fd);

            // Return value to update epoll to monitor client for writing
            return client_fd;
        }

        // CGI still running, continue monitoring
        return -1;
    } catch (const std::exception& e) {
        Logger::cerrlog(Logger::ERROR, "Error processing CGI output: " + STR(e.what()));

        // Create an error response
        _partial_responses[client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);

        // Reset client state
        client_state.body_read = -1;
        client_state.processing_cgi = false;

        // Clean up
        delete response;
        _active_responses.erase(client_fd);

        return client_fd;
    }
}

int RequestsManager::HandleClient(short int revents) {
    if (_client_fd == -1) {
        return 0;
    }
    if (revents & EPOLLIN) {
        Logger::cerrlog(Logger::DEBUG, "RequestsManager::HandleClient: POLLIN event");
        return HandleRead();
    }
    if (revents & EPOLLOUT) {
        Logger::cerrlog(Logger::DEBUG, "RequestsManager::HandleClient: POLLOUT event");
        return HandleWrite();
    }
    if (revents & (EPOLLERR | EPOLLHUP)) {
        Logger::cerrlog(Logger::INFO, "Socket error or hangup");
        CloseClient();
        return 0;
    }
    return 0;
}

void RequestsManager::CloseClient() {
    if (_client_fd < 0) {
        return; // Nothing to do
    }

    Logger::cerrlog(Logger::INFO, "RequestsManager::CloseClient: Cleaning up client " + Utils::intToString(_client_fd));

    // Clean up any active responses for this client
    MAP<int, Response*>::iterator it = _active_responses.find(_client_fd);
    if (it != _active_responses.end()) {
        if (it->second) {
            // Make sure the CGI handler is closed properly
            int cgi_fd = it->second->getCgiOutputFd();
            if (cgi_fd > 0) {
                Logger::cerrlog(Logger::INFO, "Closing CGI fd " + Utils::intToString(cgi_fd) +
                                " for client " + Utils::intToString(_client_fd));
                // Only close if still valid
                if (fcntl(cgi_fd, F_GETFD) != -1) {
                    close(cgi_fd);
                }
            }

            delete it->second;
        }
        _active_responses.erase(it);
    }

    _client_states.erase(_client_fd);

    if (fcntl(_client_fd, F_GETFD) != -1) {
        close(_client_fd);
    }

    _partial_requests.erase(_client_fd);
    _partial_responses.erase(_client_fd);

    _client_fd = -1;
}

int RequestsManager::getCurrentCgiFd() const {
    MAP<int, Response*>::const_iterator it = _active_responses.find(_client_fd);
    if (it != _active_responses.end() && it->second) {
        return it->second->getCgiOutputFd();
    }
    return -1;
}

// Clean up a client and associated resources
void RequestsManager::CleanupClient(int client_fd) {
    // Clean up any active responses for this client
    MAP<int, Response*>::iterator it = _active_responses.find(client_fd);
    if (it != _active_responses.end()) {
        if (it->second) {
            delete it->second;
        }
        _active_responses.erase(it);
    }

    _partial_requests.erase(client_fd);
    _partial_responses.erase(client_fd);
    _client_states.erase(_client_fd);
}

// Helper function to create error responses
STR RequestsManager::createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base) {
    Response tempResponse;
    tempResponse.setConfig(_config);
    return tempResponse.createErrorResponse(statusCode, contentType, body, base);
}
