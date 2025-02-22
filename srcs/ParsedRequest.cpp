#include "../includes/ParsedRequest.hpp"
#include "../includes/Logger.hpp"

static std::string intToString(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

static std::string &trimString(std::string &str) {
	const std::string whitespace = " \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	size_t end = str.find_last_not_of(whitespace);

	if (start == std::string::npos) { // string is empty
		str.clear();
	} else {
		str = str.substr(start, end - start + 1);
	}

	if (!str.empty() && str[str.size() - 1] == '\r') {
		str.erase(str.size() - 1);
	}
	return str;
}

/*
	Setting headers data
*/
void ParsedRequest::setHeaders(std::string key, std::string value) {
	_headers.insert(std::make_pair(key, value));
}

/*
	Parse cookie request
*/
void ParsedRequest::parseCookie(const std::string &cookieHeader) {
	std::istringstream ss(cookieHeader);
	std::string token;

	while (std::getline(ss, token, ';')) {
		size_t pos = token.find('=');
		if (pos != std::string::npos) {
			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);

			trimString(key);
			trimString(value);

			if (!key.empty() && !value.empty())
				_cookies[key] = value;
		}
	}

	Logger::log(Logger::DEBUG, "Parsed cookies: ");
	for (string_map::iterator it = _cookies.begin(); it != _cookies.end(); ++it) {
		Logger::log(Logger::DEBUG, " - " + it->first + ": " + it->second);
	}
}

/*
	Get specific data in _headers
*/
std::string ParsedRequest::getData(std::string key) const {
	string_map::const_iterator it = this->_headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	}
	return "";
}

void ParsedRequest::parseHttpRequest(const std::string &request) {
	std::istringstream stream(request);
	std::string line;

	if (std::getline(stream, line)) {
		Logger::log(Logger::INFO, "Parsing Request Line: " + line);
		std::istringstream lineStream(line);
		lineStream >> _method >> _path >> _version;
	}

	if (_version != "HTTP/1.1") {
		Logger::log(Logger::ERROR, "400 Bad Request: Unsupported HTTP version");
		throw std::runtime_error("400 Bad Request\r\n\r\nUnsupported HTTP version");
	}

	while (std::getline(stream, line) && !line.empty()) {
		size_t pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			key = trimString(key);
			value = trimString(value);

			for (size_t i (0); i < key.size(); i++) {
				key[i] = std::tolower(key[i]);
			}

			setHeaders(key, value);

			if (key == "cookie") {
				parseCookie(value);
			}
		}
	}

	if (_headers.find("host") != _headers.end()) {
		_host = getData("host");
		size_t colon = _host.find(':');
		_host = _host.substr(0, colon);
		_headers["host"] = _host;
	} else {
		_host = "default";
	}

	if (_method == "POST") {
		if (_headers.find("transfer-encoding") != _headers.end() && getData("transfer-encoding") == "chunked") {
    		Logger::log(Logger::ERROR, "501 Not Implemented: Chunked Transfer-Encoding not supported");
    		throw std::runtime_error("501 Not Implemented: Chunked Transfer-Encoding not supported");
		}
		if (_headers.find("content-length") != _headers.end()) {
			std::string contentLengthStr = getData("content-length");
			std::cout << "Content length str: " + contentLengthStr << std::endl;
			if (!contentLengthStr.empty()) {
				int contentLength = std::atoi(contentLengthStr.c_str());

				size_t bodyStart = request.find("\r\n\r\n");
				if (bodyStart == std::string::npos) {
					Logger::log(Logger::ERROR, "400 Bad Request Incomplete headers");
					throw std::runtime_error("400 Bad Request Incomplete headers");
				}
				bodyStart += 4;

				_body = request.substr(bodyStart);

				if (_body.size() < (size_t)contentLength) {
					Logger::log(Logger::ERROR, "400 Bad Request: Incomplete Content-Length");
					throw std::logic_error("400 Bad Request: Incomplete Content-Length");
				}
				Logger::log(Logger::INFO, "Successfully received full request body (" + intToString(contentLength) + " bytes)");
			}
		}
	}
}

ParsedRequest::ParsedRequest(const std::string &request) {
	try {
		parseHttpRequest(request);
	} catch (const std::exception &e) {
		Logger::log(Logger::ERROR, "Error detected Request scope");
		std::cerr << e.what() << std::endl;
	}
}

ParsedRequest::ParsedRequest(const ParsedRequest &obj) {
	*this = obj;
}

ParsedRequest &ParsedRequest::operator=(const ParsedRequest &obj) {
	if (this != &obj) {
		this->_method = obj._method;
		this->_path = obj._path;
		this->_host = obj._host;
		this->_version = obj._version;
		this->_body = obj._body;
		this->_headers = obj._headers;
		this->_cookies = obj._cookies;
	}
	return *this;
}

ParsedRequest::~ParsedRequest() {}

std::string ParsedRequest::getMethod(void) const { return _method; }

std::string ParsedRequest::getPath(void) const { return _path; }

std::string ParsedRequest::getHost(void) const { return _host; }

std::string ParsedRequest::getVersion(void) const { return _version; }

std::string ParsedRequest::getBody(void) const { return _body; }

string_map ParsedRequest::getHeaders(void) const { return _headers; }
