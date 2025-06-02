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

int RequestsManager::PerformSocketRead() {
    if (_client_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "PerformSocketRead: Invalid client fd");
        return -1;
    }

    char buffer[4096];
    ssize_t nbytes = read(_client_fd, buffer, sizeof(buffer));

    if (nbytes <= 0) {
        if (nbytes == 0) {
            Logger::cerrlog(Logger::INFO, "Client disconnected (read returned 0)");
        } else {
            Logger::cerrlog(Logger::ERROR, "Error reading from client: " + STR(strerror(errno)));
        }
        return 0;
    }

    _partial_requests[_client_fd].append(buffer, nbytes);

    // update the body_read
    ClientState &client_state = _client_states[_client_fd];
    if (client_state.body_read != -1) {
        client_state.body_read += nbytes;
    }
    return static_cast<int>(nbytes); // return the number of bytes read
}

// Helper function 2: Process the buffered request data
int RequestsManager::ProcessBufferedData() {
    ClientState &client_state = _client_states[_client_fd];
    long long &body_read = client_state.body_read; // init value is -1
    Request &request = client_state.request;
    bool done = false;

    try {
        // if the header is not parsed and the current body is not being read, try to parse the header
        if (body_read == -1) {
            size_t header_end_pos = _partial_requests[_client_fd].find("\r\n\r\n");
            if (header_end_pos != STR::npos) { // find the header delimiter
                Logger::log(Logger::INFO, "Complete headers received in buffer");

                request.clear(); // clear the previous request information
                // note: setRequest is the original code's way.
                // it might be more efficient to modify the Request class to parse only the header.
                if (!request.setRequest(_partial_requests[_client_fd])) {
                    Logger::cerrlog(Logger::ERROR, "Failed to parse request headers");
                    _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                    return 2; // change to write mode (error response)
                }

                if (request._body_size > 0) { // if the request has a body
                    Logger::cerrlog(Logger::DEBUG, "Request has body of size " + Utils::intToString(request._body_size));
                    // the remaining part of the current buffer is the body data
                    body_read = static_cast<long long>(_partial_requests[_client_fd].size()) - (header_end_pos + 4);
                    if (body_read < 0) body_read = 0; // prevent any errors

                    if (body_read >= (long long)request._body_size) {
                        done = true; // the body is already in the buffer
                    } else {
                        Logger::log(Logger::INFO, "Body partially received with headers: " +
                                  Utils::intToString(body_read) + "/" + Utils::intToString(request._body_size));
                        return 1; // more body data needed (read mode)
                    }
                } else { // if the request has no body
                    done = true;
                }
            } else { // if the header delimiter is not found
                return 1; // more header data needed (read mode)
            }
        } else { // body_read != -1, receving body (header already parsed)
            if (body_read >= (long long)request._body_size) {
                done = true; // the body is already in the buffer
            } else {
                 Logger::log(Logger::INFO, "Body still partially received: " +
                              Utils::intToString(body_read) + "/" + Utils::intToString(request._body_size));
                return 1; // more body data needed (read mode)
            }
        }

        // if 'done' is true, the request is fully received (header and body if needed)
        if (done) {
            Logger::cerrlog(Logger::INFO, "Complete request received, processing...");

            // in the original code, it parses the request again here.
            // this assumes that Request::setRequest can handle the entire request string.
            request.clear();
            if (!request.setRequest(_partial_requests[_client_fd])) {
                Logger::cerrlog(Logger::ERROR, "Failed to parse complete request (second pass)");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2; // change to write mode
            }

            if (request._body_size > 0) { // if the request has a body
                if (!request.parseBody()) {
                    Logger::cerrlog(Logger::ERROR, "Failed to parse request body");
                    _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                    return 2; // change to write mode
                }
            }

            // --- actual request processing logic (CGI or normal response) ---
            // (this part is almost the same as the try-catch block in the original HandleRead function)
            try {
                Response* res_obj = new Response(); // change variable name (response -> res_obj)
                res_obj->setConfig(_config);
                res_obj->setRequest(request); // request object should be fully filled

                STR response_text = res_obj->getResponse();

                if (response_text.empty() && !res_obj->isResponseReady()) {
                    // CGI request processing
                    client_state.processing_cgi = true;
                    _active_responses[_client_fd] = res_obj;
                    int cgi_fd = res_obj->getCgiOutputFd();
                    if (cgi_fd != -1) {
                        Logger::cerrlog(Logger::INFO, "Starting CGI processing for client " + Utils::intToString(_client_fd));
                        return RegisterCgiFd(cgi_fd, _client_fd); // return 4 (register CGI fd)
                    } else {
                        Logger::cerrlog(Logger::ERROR, "Invalid CGI output fd");
                        delete res_obj;
                        _active_responses.erase(_client_fd);
                        client_state.processing_cgi = false;
                        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                        return 2;
                    }
                } else {
                    // normal response processing
                    _partial_responses[_client_fd] = response_text;
                    delete res_obj;

                    // initialize the client state for the next request
                    client_state.body_read = -1;
                    client_state.processing_cgi = false;
                    // request.clear(); // will be cleared when starting to process the next request
                    _partial_requests.erase(_client_fd); // request processing complete, clear the buffer

                    return 2; // change to write mode
                }
            } catch (const std::exception& e) {
                Logger::cerrlog(Logger::ERROR, "Error processing request: " + STR(e.what()));
                _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                client_state.body_read = -1;
                client_state.processing_cgi = false;
                _partial_requests.erase(_client_fd); // even if there is an error, clear the buffer
                return 2; // change to write mode
            }
        }
        // if we get here, done is false and the above return 1 (more data needed) should have been called.
        // as a safety net, if the logic reaches here, continue to read.
        return 1;

    } catch (const std::exception& e) {
        Logger::cerrlog(Logger::ERROR, "Exception in ProcessBufferedData: " + STR(e.what()));
        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
        client_state.body_read = -1;
        client_state.processing_cgi = false;
        _partial_requests.erase(_client_fd);
        return 2; // change to write mode to send the error response
    }
}

// the original HandleRead function is now more concise
int RequestsManager::HandleRead() {
    if (_client_fd < 0) {
        Logger::cerrlog(Logger::ERROR, "HandleRead: Invalid client fd");
        return 0; // connection closed or error
    }

    int read_status = PerformSocketRead();
    if (read_status <= 0) {
        return 0; // connection closed
    }

    return ProcessBufferedData();
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
