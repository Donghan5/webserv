#include "Response.hpp"
#include "Logger.hpp"

STR Response::createResponse(int statusCode, const STR& contentType, const STR& body, const STR& extra) {
    std::stringstream response;
    response << "HTTP/1.1 " << _all_status_codes[statusCode] << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Access-Control-Allow-Credentials: true\r\n"
			 << extra << ((extra.empty()) ? "" : "\r\n")
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    return response.str();
}

Response::Response() {
	init_mimetypes(_all_mime_types);
	init_status_codes(_all_status_codes);
	_request.clear();
	_config = NULL;
    _cgi_handler = NULL;
    _state = READY;
}

Response::Response(Request request, HttpConfig *config) {
	init_mimetypes(_all_mime_types);
	init_status_codes(_all_status_codes);
	_request.clear();
	_request = request;
	_config = config;
    _cgi_handler = NULL;
    _state = READY;
}

Response::Response(const Response &obj) {
	_all_mime_types = obj._all_mime_types;
	_all_status_codes = obj._all_status_codes;
	_request.clear();
	_request = obj._request;
	_config = obj._config;
    _cgi_handler = NULL; //  don't copy the CGI handler
    _state = READY;
}

Response::~Response() {
    if (_cgi_handler) {
        Logger::log(Logger::DEBUG, "Response destructor: cleaning up CGI handler");

        _cgi_handler->closeCgi();

        delete _cgi_handler;
        _cgi_handler = NULL;
    }
}

void Response::clear() {
    _request.clear();
    _config = NULL;

    if (_cgi_handler) {
        delete _cgi_handler;
        _cgi_handler = NULL;
    }

    _state = READY;
    _response_buffer.clear();
}

void Response::setRequest(Request request) {
	_request.clear();
	_request = request;
}

void Response::setConfig(HttpConfig *config) {
	_config = config;
}

// --- helper function to join two paths ---
std::string joinPaths(const std::string& p1, const std::string& p2) {
    if (p1.empty()) return p2;
    if (p2.empty()) return p1;

    char lastChar = p1[p1.length() - 1];
    char firstChar = p2[0];

    if (lastChar == '/' && firstChar == '/')
        return p1 + p2.substr(1);
    else if (lastChar != '/' && firstChar != '/')
        return p1 + '/' + p2;
    else
        return p1 + p2;
}

STR	Response::checkRedirect(LocationConfig *matchLocation) {
	AConfigBase*	back_ref = matchLocation;
	ServerConfig* tempServer;
	LocationConfig* tempLocation;

	while (back_ref && back_ref->_identify(back_ref) != HTTP) {
		switch (back_ref->_identify(back_ref))
		{
		case SERVER:
			 tempServer = dynamic_cast<ServerConfig*>(back_ref);

			if (tempServer->_return_url != "" && tempServer->_return_code == -1) {
				return createResponse(301, "text/plain", "Redirect", "Location: " + tempServer->_return_url);
			}
			if (tempServer->_return_url != "") {
				return createResponse(tempServer->_return_code, "text/plain", "Redirect", "Location: " + tempServer->_return_url);
			}
			break;
		case LOCATION:
			 tempLocation = dynamic_cast<LocationConfig*>(back_ref);

			if (tempLocation->_return_url != "" && tempLocation->_return_code == -1) {
				return createResponse(301, "text/plain", "Redirect", "Location: " + tempLocation->_return_url);
			}
			if (tempLocation->_return_url != "") {
				return createResponse(tempLocation->_return_code, "text/plain", "Redirect", "Location: " + tempLocation->_return_url);
			}
			break;
		case HTTP:
			return "";
		default:
			break;
		}

		back_ref = back_ref->back_ref;
	}
	return "";
}

STR	Response::createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base) {
	while (base) {
		try
		{
			if (strncmp(base->_error_pages[statusCode].c_str(), "default_errors/", 15) || base->_identify(base) == HTTP) {
				if (access(base->_error_pages[statusCode].c_str(), R_OK) == 0) {
					std::ifstream file(base->_error_pages[statusCode].c_str(), std::ios::binary);
					if (file) {
						std::stringstream content;
						content << file.rdbuf();
						return createResponse(statusCode, getMimeType(base->_error_pages[statusCode]), content.str(), "");
					}
				}
			}
		}
		catch(const std::exception& e)
		{
		}
		base = base->back_ref;
	}

	return createResponse(statusCode, contentType, body, "");
}


/*
	paths with spaces are found now
*/
STR Response::getResponse() {
	ServerConfig*	matchServer = NULL;
	LocationConfig*	matchLocation = NULL;

	bool	isDIR = false;
	STR dir_path = "";
	STR	file_path = "";

	_request._file_path = urlDecode(_request._file_path);

	if (_request._full_request == "" || !_config) {
		Logger::log(Logger::DEBUG, "Response::getResponse error, no config or request");
		return "";
	}

	for (size_t i = 0; i < _config->_servers.size(); i++) {
		if (_config->_servers[i]->_listen_port != _request._port)
			continue;
		for (size_t k = 0; k < _config->_servers[i]->_server_name.size(); k++) {
			if (_config->_servers[i]->_server_name[k] == _request._host) {
				matchServer = _config->_servers[i];
				break;
			}
		}
	}

	//if no server matches - defauts to first one
	if (!matchServer)
		matchServer = _config->_servers[0];

	if (_request._file_path.size() > 1 && _request._file_path.at(_request._file_path.size() - 1) == '/') {
		Logger::log(Logger::DEBUG, "Response::getResponse: path is a directory");
		isDIR = true;
		_request._file_path = _request._file_path.substr(0, _request._file_path.size() - 1);
		_request._file_name += '\0';
	}

	matchLocation = buildDirPath(matchServer, dir_path, isDIR);

	// check body size
	if (!checkBodySize(matchLocation)) {
		Logger::log(Logger::ERROR, "Body size is too big");
		return createErrorResponse(413, "text/plain", "Payload Too Large", matchServer);
	}

	STR temp_str = checkRedirect(matchLocation);
	if (temp_str != "")
		return temp_str;

	//if it's a script file - execute it
	if (ends_with(dir_path, ".py") || ends_with(dir_path, ".php") || ends_with(dir_path, ".pl") || ends_with(dir_path, ".sh")) {
		MAP<STR, STR> env;

		env["REQUEST_METHOD"] = _request._method;
		env["SCRIPT_NAME"] = dir_path;
		env["QUERY_STRING"] = _request._query_string.empty() ? "" : _request._query_string;
		env["CONTENT_TYPE"] = _request._http_content_type.empty() ? "text/plain" : _request._http_content_type;
		env["CONTENT_LENGTH"] = Utils::intToString(_request._body.length());
		env["HTTP_HOST"] = _request._host;
		env["SERVER_PORT"] = Utils::intToString(_request._port);
		env["SERVER_PROTOCOL"] = _request._http_version;
		env["HTTP_COOKIE"] = _request._cookies;

		if (!matchLocation->_upload_store.empty()) {
			env["UPLOAD_STORE"] = matchLocation->_upload_store;
		} else {
			env["UPLOAD_STORE"] = "";
		}

        if (_cgi_handler) {
            delete _cgi_handler;
        }

        _cgi_handler = new CgiHandler(dir_path, env, _request._body);

        if (_cgi_handler->startCgi()) {
            _state = PROCESSING_CGI;
            return "";
        } else {
            delete _cgi_handler;
            _cgi_handler = NULL;
            return createErrorResponse(500, "text/plain", "Failed to start CGI process", matchServer);
        }
	}
	if (_request._method == "POST") {
		try {
			if (matchLocation->_upload_store != "") {
				if (dir_path.find_last_of('/') != STR::npos) {
					dir_path = matchLocation->_upload_store + "/" + dir_path.substr(dir_path.find_last_of('/') + 1);
				} else {
					dir_path = matchLocation->_upload_store + dir_path;
				}
			}
		}
		catch (const std::exception& e) {
			Logger::log(Logger::DEBUG, "Response::getResponse: upload_store error: " + STR(e.what()));
		}
		return (handlePOST(dir_path));
	}
	file_path = dir_path;

	//serve file if path is a file
	if (checkFile(file_path) == NormalFile) {
		return (matchMethod(file_path, false, matchLocation));
	}
	//--server file

	//if it's not a directory return error
	if (checkFile(dir_path) != Directory && !isDIR) {
		Logger::log(Logger::INFO, "File not found: " + dir_path);
		return createErrorResponse(404, "text/plain", "Not Found", matchServer);
	}
	else if (checkFile(dir_path) != Directory && isDIR) {
		Logger::log(Logger::INFO, "File is not a directory: " + dir_path);
		return createErrorResponse(403, "text/plain", "Forbidden", matchLocation);
	}
	//--check dir

	//add index file name to file_path
	buildIndexPath(matchLocation, file_path, dir_path);

	//index file exists - serve it
	if (checkFile(file_path) == NormalFile) {
		return (matchMethod(file_path, false, matchLocation));
	}

	//index file doesn't exist - create directory if autoindex is on
	if (matchLocation->_autoindex || isDIR) {
		//return directory listing		
		return matchMethod(dir_path, true, matchLocation);
	} else {
		//autoindex is off
		return createErrorResponse(403, "text/plain", "Forbidden", matchLocation);
	}

	return createErrorResponse(403, "text/plain", "Forbidden", matchLocation);
}

int Response::getCgiOutputFd() const {
    if (_state == PROCESSING_CGI && _cgi_handler) {
        return _cgi_handler->getOutputFd();
    }
    return -1;
}

bool Response::processCgiOutput() {
    if (_state != PROCESSING_CGI || !_cgi_handler) {
        return true;
    }

    try {
        STR output = _cgi_handler->readFromCgi();
        if (!output.empty()) {
            Logger::log(Logger::DEBUG, "Read " + Utils::intToString(output.length()) +
                          " bytes from CGI output");
            _response_buffer += output;
        }

		CgiStatus status = _cgi_handler->checkCgiStatus();
		if (status == CGI_RUNNING) {
			return false;
		} else if (status == CGI_COMPLETED) {
			Logger::log(Logger::DEBUG, "CGI process has completed successfully");
			_state = COMPLETE;
			return true;
		} else if (status == CGI_TIMED_OUT) {
			Logger::log(Logger::ERROR, "CGI process has timed out");
			_response_buffer = createErrorResponse(504, "text/html", "Gateway Timeout", _config);
			_state = COMPLETE;
			return true;
		} else {
			Logger::log(Logger::ERROR, "CGI process has failed with status: " + Utils::intToString(status));
			_response_buffer = createErrorResponse(status, "text/html", "Internal Server Error", _config);
			_state = COMPLETE;
			return true;
		}

        return false;
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, "Error in processCgiOutput: " + STR(e.what()));

        _response_buffer = createErrorResponse(500, "text/html", "Internal Server Error", _config);

        _state = COMPLETE;
        return true;
    }
}

STR Response::getFinalResponse() {
    if (_state != COMPLETE || !_cgi_handler) {
        return createErrorResponse(500, "text/plain", "Internal Server Error", _config);
    }

    try {
        if (_response_buffer.find("HTTP/") == 0) {
            STR response = _response_buffer;
            _response_buffer.clear();
            _state = READY;

            return response;
        }

        size_t header_end = _response_buffer.find("\r\n\r\n");
        if (header_end != STR::npos) {
            STR headers_section = _response_buffer.substr(0, header_end);
            STR body = _response_buffer.substr(header_end + 4); // +4 to skip "\r\n\r\n"

            VECTOR<std::pair<STR, STR> > headersList;
            std::istringstream headerStream(headers_section);
            STR headerLine;
            int statusCode = 200; // Default status code
            STR contentType = "text/html"; // Default content type

            while (std::getline(headerStream, headerLine)) {
                // Remove any trailing \r
                if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r') {
                    headerLine = headerLine.substr(0, headerLine.length() - 1);
                }

                if (headerLine.empty()) {
                    continue;
                }

                if (headerLine.find("Status:") == 0) {
                    STR status = headerLine.substr(7); // Skip "Status:"
                    status.erase(0, status.find_first_not_of(" \t"));
                    statusCode = atoi(status.c_str());
                    continue;
                }

                size_t colonPos = headerLine.find(':');
                if (colonPos != STR::npos) {
                    STR name = headerLine.substr(0, colonPos);
                    STR value = headerLine.substr(colonPos + 1);

                    value.erase(0, value.find_first_not_of(" \t"));

                    headersList.push_back(std::make_pair(name, value));

                    if (name == "Content-Type") {
                        contentType = value;
                    }
                }
            }

            std::stringstream response;
            response << "HTTP/1.1 " << statusCode << " ";

            if (_all_status_codes.find(statusCode) != _all_status_codes.end()) {
                response << _all_status_codes[statusCode].substr(4); 
            } else {
                response << "OK";
            }
            response << "\r\n";
            bool hasContentType = false;
            bool hasContentLength = false;

            for (size_t i = 0; i < headersList.size(); i++) {
                const std::pair<STR, STR>& header = headersList[i];
                response << header.first << ": " << header.second << "\r\n";

                if (header.first == "Content-Type") hasContentType = true;
                if (header.first == "Content-Length") hasContentLength = true;
            }

            if (!hasContentLength) {
                response << "Content-Length: " << body.length() << "\r\n";
            }
            if (!hasContentType) {
                response << "Content-Type: " << contentType << "\r\n";
            }

            response << "\r\n" << body;
            _response_buffer.clear();
            _state = READY;

            Logger::log(Logger::DEBUG, "Generated HTTP response from CGI output");
            return response.str();
        }

        std::stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << _response_buffer.length() << "\r\n"
                 << "\r\n"
                 << _response_buffer;

        _response_buffer.clear();
        _state = READY;

        Logger::log(Logger::DEBUG, "Generated default HTTP response for CGI output");
        return response.str();
    } catch (const std::exception& e) {
        Logger::log(Logger::DEBUG, "Error in getFinalResponse: " + STR(e.what()));

        _response_buffer.clear();
        _state = READY;
        return createErrorResponse(500, "text/plain", "Internal Server Error", _config);
    }
}
