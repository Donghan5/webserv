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
}

void RequestsManager::setClientFd(int client_fd) {
    _client_fd = client_fd;
}

static STR intToString(int num) {
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

int RequestsManager::HandleRead() {
    static Request request;
    static long long body_read = -1;

    // Figure out why this here doesn't work (instead of two inside)
    // if (body_read != -1 && body_read >= (long long)request._body_size) {
    //     Logger::cerrlog(Logger::INFO, "RequestsManager::HandleRead Full body read!");
    //     std::cerr << "body_read is " << body_read << "\n";
    //     body_read = -1;

    //     request.clear();
    //     if (!request.setRequest(_partial_requests[_client_fd])) {
    //         Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing request");
    //         _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
    //         return 2;
    //     }
    //     if (!request.parseBody()) {
    //         Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing body");
    //         _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
    //         return 2;
    //     }

    //     // Create a response object for this request
    //     Response* response = new Response();
    //     response->setConfig(_config);
    //     response->setRequest(request);

    //     // Process the request
    //     STR response_text = response->getResponse();

    //     // Check if we need to handle CGI
    //     if (response_text.empty() && !response->isResponseReady()) {
    //         // This is a CGI request that's being processed asynchronously
    //         _active_responses[_client_fd] = response;

    //         // Register the CGI output file descriptor with epoll
    //         int cgi_fd = response->getCgiOutputFd();
    //         if (cgi_fd != -1) {
    //             return RegisterCgiFd(cgi_fd, _client_fd);
    //         } else {
    //             Logger::cerrlog(Logger::ERROR, "Invalid CGI file descriptor");
    //             delete response;
    //             _active_responses.erase(_client_fd);
    //             _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
    //             return 2;
    //         }
    //     } else {
    //         // Normal response
    //         _partial_responses[_client_fd] = response_text;
    //         delete response;
    //         return 2;
    //     }
    // }

    try {
        //Figure out mystery about why small ones don't work
        char buffer[1500000];
        int nbytes = 1;

        nbytes = read(_client_fd, buffer, 1500000);
        if (nbytes <= 0) {
            if (nbytes == 0) {
                std::cerr << "nbytes == 0\n";
                CloseClient();
            }
            Logger::cerrlog(Logger::ERROR, "RequestManager::HandleRead: Error reading request");
            CloseClient();
            return 0;
        }

        if (body_read != -1) {
            // std::cerr << "body_read is " << body_read << "\n";
            body_read += nbytes;
        }

        _partial_requests[_client_fd].append(buffer, nbytes);

        int header_end = _partial_requests[_client_fd].find("\r\n\r\n");
        if (body_read == -1 && header_end != CHAR_NOT_FOUND) {
            Logger::log(Logger::INFO, "Requests HandleRead: Full message received");
            request.clear();
            if (!request.setRequest(_partial_requests[_client_fd])) {
                Logger::cerrlog(Logger::ERROR, "Requests HandleRead: Error parsing request");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2;
            }

            //HANDLE BODY IN if (body_read < (long long)request._body_size) else CASE, DON'T GO TO POLLOUT, body needed
            //           it's the same function - unify
            
            if (request._body_size > 0) {
                Logger::cerrlog(Logger::DEBUG, "RequestManager::HandleRead Body needed of size " + intToString(request._body_size));
                body_read = _partial_requests[_client_fd].size() - header_end - 4;
				if (body_read < (long long)request._body_size) {
                    Logger::log(Logger::INFO, "Body partially received: " +
                               intToString(body_read) + "/" + intToString(request._body_size));
                    return 1; // Keep in POLLIN mode
                } else {
                    Logger::cerrlog(Logger::INFO, "RequestsManager::HandleRead Full body read!");
                    std::cerr << "body_read is " << body_read << "\n";
                    body_read = -1;

                    request.clear();
                    if (!request.setRequest(_partial_requests[_client_fd])) {
                        Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing request");
                        _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                        return 2;
                    }
                    if (!request.parseBody()) {
                        Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing body");
                        _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                        return 2;
                    }

                    // Create a response object for this request
                    Response* response = new Response();
                    response->setConfig(_config);
                    response->setRequest(request);

                    // Process the request
                    STR response_text = response->getResponse();

                    // Check if we need to handle CGI
                    if (response_text.empty() && !response->isResponseReady()) {
                        // This is a CGI request that's being processed asynchronously
                        _active_responses[_client_fd] = response;

                        // Register the CGI output file descriptor with epoll
                        int cgi_fd = response->getCgiOutputFd();
                        if (cgi_fd != -1) {
                            return RegisterCgiFd(cgi_fd, _client_fd);
                        } else {
                            Logger::cerrlog(Logger::ERROR, "Invalid CGI file descriptor");
                            delete response;
                            _active_responses.erase(_client_fd);
                            _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                            return 2;
                        }
                    } else {
                        // Normal response
                        _partial_responses[_client_fd] = response_text;
                        delete response;
                        return 2;
                    }
                }
            } else {
                // Create a response object for this request
                Response* response = new Response();
                response->setConfig(_config);
                response->setRequest(request);

                // Process the request
                STR response_text = response->getResponse();

                // Check if we need to handle CGI
                if (response_text.empty() && !response->isResponseReady()) {
                    // This is a CGI request that's being processed asynchronously
                    _active_responses[_client_fd] = response;

                    // Register the CGI output file descriptor with epoll
                    int cgi_fd = response->getCgiOutputFd();
                    if (cgi_fd != -1) {
                        return RegisterCgiFd(cgi_fd, _client_fd);
                    } else {
                        Logger::cerrlog(Logger::ERROR, "Invalid CGI file descriptor");
                        delete response;
                        _active_responses.erase(_client_fd);
                        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                        return 2;
                    }
                } else {
                    // Normal response
                    _partial_responses[_client_fd] = response_text;
                    delete response;
                    _partial_requests.erase(_client_fd);
                    return 2;
                }
            }
        }

        if (body_read != -1 && body_read >= (long long)request._body_size) {
            Logger::cerrlog(Logger::INFO, "RequestsManager::HandleRead Full body read!");
			std::cerr << "body_read is " << body_read << "\n";
            body_read = -1;

            request.clear();
            if (!request.setRequest(_partial_requests[_client_fd])) {
                Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing request");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2;
            }
            if (!request.parseBody()) {
                Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error parsing body");
                _partial_responses[_client_fd] = createErrorResponse(400, "text/plain", "Bad Request", NULL);
                return 2;
            }

            // Create a response object for this request
            Response* response = new Response();
            response->setConfig(_config);
            response->setRequest(request);

            // Process the request
            STR response_text = response->getResponse();

            // Check if we need to handle CGI
            if (response_text.empty() && !response->isResponseReady()) {
                // This is a CGI request that's being processed asynchronously
                _active_responses[_client_fd] = response;

                // Register the CGI output file descriptor with epoll
                int cgi_fd = response->getCgiOutputFd();
                if (cgi_fd != -1) {
                    return RegisterCgiFd(cgi_fd, _client_fd);
                } else {
                    Logger::cerrlog(Logger::ERROR, "Invalid CGI file descriptor");
                    delete response;
                    _active_responses.erase(_client_fd);
                    _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
                    return 2;
                }
            } else {
                // Normal response
                _partial_responses[_client_fd] = response_text;
                delete response;
                return 2;
            }
        }
    } catch (const std::exception& e) {
        body_read = -1;
        Logger::cerrlog(Logger::ERROR, "Request HandleRead: Error reading request: " + STR(e.what()));
        _partial_responses[_client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);
        return 0;
    }
    return 1;
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
            // if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //     // Socket buffer is full, try again later
            //     Logger::cerrlog(Logger::DEBUG, "HandleWrite: Socket buffer full, will retry");
            //     return 2; // Keep monitoring for write events
            // } else {
            //     // Real error
            //     Logger::cerrlog(Logger::ERROR, "HandleWrite error: " + STR(strerror(errno)));
            //     CloseClient();
            //     return 0;
            // }
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

            // Check if we need to keep the connection alive or close it
            bool keep_alive = false; // Implement keep-alive logic here if needed

            if (!keep_alive) {
                CloseClient();
                return 1; // Done
            } else {
                return 3; // Switch back to read mode for persistent connections
            }
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

// Handle CGI output when data is available
int RequestsManager::HandleCgiOutput(int cgi_fd) {
    // Find the client fd associated with this CGI fd
    int client_fd = -1;
    Response* response = NULL;

    // Find which client this CGI belongs to
    for (MAP<int, Response*>::iterator it = _active_responses.begin(); it != _active_responses.end(); ++it) {
        if (it->second && it->second->getCgiOutputFd() == cgi_fd) {
            client_fd = it->first;
            response = it->second;
            break;
        }
    }

    if (client_fd == -1 || !response) {
        Logger::cerrlog(Logger::ERROR, "CGI output for unknown client");
        return 0;
    }

    try {
        // Process the CGI output
        bool completed = response->processCgiOutput();

        if (completed) {
            // CGI has finished
            Logger::cerrlog(Logger::INFO, "CGI processing completed for client " + Utils::intToString(client_fd));

            // Get the final response
            _partial_responses[client_fd] = response->getFinalResponse();

            // Clean up
            delete response;
            _active_responses.erase(client_fd);

            // Return value to update epoll to monitor client for writing
            return client_fd;
        }
    } catch (const std::exception& e) {
        Logger::cerrlog(Logger::ERROR, "Error processing CGI output: " + STR(e.what()));

        // Create an error response
        _partial_responses[client_fd] = createErrorResponse(500, "text/plain", "Internal Server Error", NULL);

        // Clean up
        delete response;
        _active_responses.erase(client_fd);

        return client_fd;
    }

    // CGI still running, continue monitoring
    return -1;
}

// Register a CGI fd with epoll and associate it with a client
int RequestsManager::RegisterCgiFd(int cgi_fd, int client_fd) {
    // This would need to add the CGI fd to epoll
    // Implementation depends on how your PollServer is structured
    // Returns a code that indicates PollServer should add this fd
    Logger::cerrlog(Logger::INFO, "Registering CGI fd " + Utils::intToString(cgi_fd) +
                    " for client " + Utils::intToString(client_fd));

    // Return a special code to tell PollServer to add this fd
    // This is implementation-specific, you'll need to handle this in PollServer
    return 4; // Special code for "add CGI fd to epoll"
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
    // Clean up any active responses for this client
    MAP<int, Response*>::iterator it = _active_responses.find(_client_fd);
    if (it != _active_responses.end()) {
        if (it->second) {
            delete it->second;
        }
        _active_responses.erase(it);
    }

    close(_client_fd);
    _partial_requests.erase(_client_fd);
    _partial_responses.erase(_client_fd);
}

// Get the current CGI file descriptor for the active client
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
}

// Helper function to create error responses
STR RequestsManager::createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base) {
    Response tempResponse;
    tempResponse.setConfig(_config);
    return tempResponse.createErrorResponse(statusCode, contentType, body, base);
}
