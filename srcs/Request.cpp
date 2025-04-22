#include "Request.hpp"
#include <sstream>
#include "Logger.hpp"

// static bool isToken(uint8_t ch) {
// 	if (ch == '!' || ch == '#' || ch == '$' || ch == '%' || ch == '&' || ch == '\'' || ch == '*'
// 		|| ch == '+' || ch == '-' || ch == '.' || ch == '^' || ch == '_' || ch == '`' || ch == '|'
// 		|| ch == '~' || isalnum(ch)) {
// 		return true;
// 	}
// 	return false;
// }

static void trimSpace(std::string &str) {
	static const char *whitespace = " \r";
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1);
}

static void toLower(std::string &str) {
	for (size_t i = 0; i < str.length(); i++) {
		str[i] = tolower(str[i]);
	}
}

void remove_trailing_r(STR &str) {
	if (!str.empty() && str[str.length() - 1] == '\r') {
		str.replace(str.length() - 1, 1, "\0");
	}
}

void	process_path(STR &full_path, STR &file_name) {
	Logger::cerrlog(Logger::DEBUG, "Request::process_path: full path is " + full_path);

	if (full_path.find('.') != STR::npos) {
		file_name = full_path.substr(full_path.find_last_of('/') + 1, full_path.size());
		full_path = full_path.substr(0, full_path.find_last_of('/') + 1);
	} else {
		file_name = "";
	}

	std::cerr << "Full path after: " << full_path << "\n";
	std::cerr << "File name after: " << file_name << "\n";
}

bool Request::parseHeader() {
	std::istringstream	request_stream(_full_request);
    STR					temp_line;
	std::istringstream	temp_line_stream;
	STR 				temp_token;

	if (_full_request == "")
		return false;

	//parse first line		GET / HTTP/1.1
	std::getline(request_stream, temp_line);
	remove_trailing_r(temp_line);
	temp_line_stream.str(temp_line);
	getline(temp_line_stream, temp_token, ' ');
	_method = temp_token;
	getline(temp_line_stream, temp_token, ' ');
	_file_path = temp_token;
	parseQueryString(); // to parse query string, if it exists
	getline(temp_line_stream, temp_token, ' ');
	_http_version = temp_token;

	//parse the rest of request searching for data nedded
	while (std::getline(request_stream, temp_line)) {
		if (temp_line.find("\r\n\r\n") != STR::npos) {
			return true;
		}
		remove_trailing_r(temp_line);
		temp_line_stream.clear();
		temp_token.clear();
		temp_line_stream.str(temp_line);
		getline(temp_line_stream, temp_token, ' ');

		//searching for 	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
		if (temp_token == "Accept:" && _accepted_types.empty()) {
			STR			filetypes_line;
			std::istringstream	filetypes_stream;
			STR 		filetype_token;

			//getting the second part of line line with filetypes
			getline(temp_line_stream, filetypes_line, ' ');
			filetypes_stream.str(filetypes_line);

			//getting types from type line one by one
			while (getline(filetypes_stream, filetype_token, ',')) {
				std::istringstream	name_quality_stream;
				STR 		name_quality_token;
				STR			type_name;
				float				type_quality = 1.0;

				//separating by name and value (quality/preference)
				name_quality_stream.str(filetype_token);
				getline(name_quality_stream, name_quality_token, ';');
				type_name = name_quality_token.c_str();
				type_name += '\0';
				if (getline(name_quality_stream, name_quality_token, ';'))
					type_quality = atof(name_quality_token.c_str() + 2);
				_accepted_types[type_name.c_str()] = type_quality;
			}
		} else if (temp_token == "Cookie:" && _cookies == "") {
			//to do cookies
			_cookies = temp_line.substr(temp_line.find_first_of(':') + 2);
		} else if (temp_token == "Host:" && _host == "localhost") {
			int	host_start;
			int	host_end;
			int	delim_position;
			int	port_end;

			//extracting host and port from 		Host: localhost:8080
			host_start = temp_line.find_first_of(':') + 2;
			delim_position = temp_line.find(':', temp_line.find_first_of(':') + 1);

			if (delim_position == (int)STR::npos) { //Not tested
			//host without port
				host_end = temp_line.find_last_not_of(' '); //		Future check required: newline included or not to remove +1
				_host = temp_line.substr(host_start, host_end - host_start);
			} else {
			//host with port
				host_end = delim_position;
				port_end = temp_line.find_last_not_of(' ');
				_host = temp_line.substr(host_start, host_end - host_start);
				_port = atoi(temp_line.substr(delim_position + 1, delim_position + 1 - port_end).c_str());
			}
		} else if (temp_token == "Content-Type:" && _content_type == "") {	//Not tested
			int	delim_position;
			int	end_position;
			_http_content_type = temp_line.substr(temp_line.find_first_of(':') + 2);  // to test and this is chaned (added)

			delim_position = temp_line.find_first_of(':');
			end_position = temp_line.find_last_not_of(' ');
			_content_type = temp_line.substr(delim_position + 2, end_position - delim_position - 2);
			Logger::log(Logger::DEBUG, "Request::parseHeader() Content-Type: " + _content_type);
		} else if (temp_token == "Content-Length:" && _body_size == 0) {
			int	delim_position;
			int	end_position;

			delim_position = temp_line.find_first_of(':');
			end_position = temp_line.length();
			_body_size = atoi((temp_line.substr(delim_position + 1, delim_position + 1 - end_position)).c_str());
			_chunked_flag = false;  // if content-length is present, chunked transfer encoding is not used
		} else if (temp_token == "Transfer-Encoding:") {

			std::string encoding_value = temp_line.substr(temp_line.find_first_of(':') + 2);
			parseTransferEncoding(encoding_value);

			if (_chunked_flag) {  // if chunked transfer encoding ignore content-length
				_body_size = 0;
			}
		}
		temp_line.clear();
		temp_token.clear();
	}

	// process_path(_file_path, _file_name);
	return true;
}

bool Request::parseBody() {
	int	body_beginning = -1;
	if (_full_request == "") {
		Logger::cerrlog(Logger::ERROR, "Request::parseBody: Empty request");
		return false;
	}

	body_beginning = _full_request.find("\r\n\r\n");
	if (body_beginning == CHAR_NOT_FOUND) {
		Logger::cerrlog(Logger::ERROR, "Request::parseBody: No body beginning found");
		return false;
	}

	if (_content_type.find("multipart/form-data") != STR::npos) {
		_body = _full_request.substr(body_beginning + 4, _full_request.length() - (body_beginning + 4));
		// std::cerr << "multipart/form-data\n";
		return true;
	}
	if (_chunked_flag) {
		const char *data = _full_request.c_str() + body_beginning + 4;  // skip \r\n\r\n (4 bytes)
		size_t size = _full_request.length() - (body_beginning + 4);  // skip \r\n\r\n (4 bytes)
		// std::cerr << "_chunked_flag\n";

		return processTransferEncoding(data, size);
	}

	// std::cerr << "DEBUG Request::parseBody _full_request = |" << _full_request << "|\n";

	try {
		_body = _full_request.substr(body_beginning + 4, _full_request.length() - (body_beginning + 4));
	} catch (std::exception &e) {
		std::cerr << "Error parsing body: " << e.what() << "\n";
		return false;
	}

	// std::cerr << "endeded body beginning = " << body_beginning << "\n";
	//if _full_request.length() - (body_beginning + 4) != _body_size 		potential error

	// std::cerr << "DEBUG Requet::parseBody _body = |" << _body << "|\n";
	return true;
}

void Request::parseQueryString(void) {
	size_t query_pos = _file_path.find('?');

	if (query_pos != STR::npos) {
		_query_string = _file_path.substr(query_pos + 1);
		_file_path = _file_path.substr(0, query_pos);
	}
	else {
		_query_string = "";
	}
}

void Request::parseTransferEncoding(const std::string &header) {
	std::vector<std::string> encodings;
	size_t start = 0, end;

	while ((end = header.find(',', start)) != std::string::npos) {
		std::string encoding = header.substr(start, end - start);
		trimSpace(encoding);
		toLower(encoding);
		encodings.push_back(encoding);
		start = end + 1;
	}

	std::string last_encoding = header.substr(start);
	trimSpace(last_encoding);
	toLower(last_encoding);
	encodings.push_back(last_encoding);

	if (encodings.empty() || encodings.back() == "chunked") {
		_chunked_flag = true;
		_transfer_encoding = encodings;
	}
	else {
		std::cerr << "HTTP/1.1\r\n\r\n400 Bad Request";
	}
}

bool Request::processTransferEncoding(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		char ch = data[i];

		switch (_chunked_state) {
			case CHUNK_SIZE:
				if (isxdigit(ch)) {
					_chunk_buffer += ch;
				}
				else if (ch == ';') { // after semicolon is chunk extension
					if (_chunk_buffer.empty()) {
						return false;
					}

					_chunk_size = std::strtol(_chunk_buffer.c_str(), NULL, 16);
					_chunk_buffer.clear();
					_chunked_state = CHUNK_EXT;
				}
				else if (ch == '\r') {
					if (_chunk_buffer.empty()) {
						return false;
					}
					_chunk_size = std::strtol(_chunk_buffer.c_str(), NULL, 16);
					_chunk_buffer.clear();
					_chunked_state = CHUNK_LF;
				}
				else {
					return false;
				}
				break;
			case CHUNK_EXT:  // ignore extension and search for CRLF
				if (ch == '\r') {
					_chunked_state = CHUNK_LF;
				}
				break;
			case CHUNK_LF:
				if (ch == '\n') {
					_chunk_data_read = 0;
					if (_chunk_size == 0) {
						_chunked_state = CHUNK_TRAILER;
					}
					else {
						_chunked_state = CHUNK_DATA;
					}
				}
				else {
					return false;
				}
				break;
			case CHUNK_DATA:
				_body += ch;
				_chunk_data_read++;
				if (_chunk_data_read == _chunk_size) {
					_chunked_state = CHUNK_CR;
				}
				break;
			case CHUNK_CR:
				if (ch == '\r') {
					_chunked_state = CHUNK_LF;
				}
				else {
					return false; // missing CRLF
				}
				break;
			case CHUNK_TRAILER:
				if (ch == '\r') {
					_chunked_state = TRAILER_FINAL_LF;
				}
				break;
			case TRAILER_FINAL_LF:
				if (ch == '\n') {
					_chunked_state = CHUNK_COMPLETE;
				}
				else {
					_chunked_state = CHUNK_TRAILER;
					if (ch == '\r') {
						_chunked_state = TRAILER_FINAL_LF;
					}
				}
				break;
			case CHUNK_COMPLETE:
				break;
		}
	}
	return true;
}

Request::Request() {
	_cookies = "";
	_full_request = "";
	_file_path = "";
	_method = "";
	_http_version = "";
	_host = "localhost";
	_port = 80;
	_content_type = "";
	_body = "";
	_body_size = 0;
}

Request::Request(STR request) {
	_full_request = request;
	_cookies = "";
	_file_path = "";
	_method = "";
	_http_version = "";
	_host = "localhost";
	_port = 80;
	_content_type = "";
	_http_content_type = "";  // added
	_body = "";
	_body_size = 0;
	parseHeader();
	_chunked_flag = false;  // adding for transfer-encoding
	_chunked_state = CHUNK_SIZE;  // adding for transfer-encoding
	_chunk_size = 0;  // adding for transfer-encoding
	_chunk_data_read = 0;  // adding for transfer-encoding
}

Request::Request(const Request &obj) {
	_cookies = obj._cookies;
	_full_request = obj._full_request;
	_file_path = obj._file_path;
	_method = obj._method;
	_http_version = obj._http_version;
	_host = obj._host;
	_port = obj._port;
	_content_type = obj._content_type;
	_http_content_type = obj._http_content_type;
	_accepted_types = obj._accepted_types;
	_body = obj._body;
	_body_size = obj._body_size;
}

Request::~Request() {

}

void Request::clear() {
	_cookies = "";
	_full_request = "";
	_file_path = "";
	_method = "";
	_http_version = "";
	_host = "localhost";
	_port = 80;
	_content_type = "";
	_body = "";
	_body_size = 0;
}

bool Request::setRequest(STR request) {
	_full_request = request;
	_body = "";

	if (!parseHeader()) {
		Logger::cerrlog(Logger::ERROR, "Request::setRequest: Header Parsing error!");
		return false;
	}
	return true;
}
