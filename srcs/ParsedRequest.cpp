#include "../includes/ParsedRequest.hpp"


static std::string &trimString(std::string &str) {
	const std::string whitespace = " \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	size_t end = str.find_last_not_of(whitespace);

	if (start == std::string::npos) { // string is empty
		str.clear();
	} else {
		str = str.substr(start, end - start + 1);
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
		std::istringstream lineStream(line);
		lineStream >> _method >> _path >> _version;
	}

	if (_version != "HTTP/1.1") {
		throw std::runtime_error("400 Bad Request: Unsupported HTTP version");
	}

	while (std::getline(stream, line)) {
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

	if (_method == "POST" || _method == "PUT") {
		if (_headers.find("content-length") != _headers.end()) {
			int contentLength = std::atoi(getData("Content-Length").c_str());
			if (contentLength < 0) {
				throw std::logic_error("Invalid Content-Length value");
			}
			_body.resize(contentLength);
			stream.read(&_body[0], contentLength); // body 0 is the first address of string

			if (stream.gcount() < contentLength) {
				throw std::logic_error("400 Bad Request: Incomplete Content-Length");
			}
		}
	}
}

ParsedRequest::ParsedRequest(const std::string &request) {
	try {
		parseHttpRequest(request);
	} catch (const std::exception &e) {
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
