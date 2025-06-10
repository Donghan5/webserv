#include "Parser/ParserFiller.hpp"

// Fill HTTP block
bool ParserFiller::FillHttp(HttpConfig* httpConf, VECTOR<STR> tokens){
	if (tokens[0] == "user") {
		httpConf->_global_user = tokens[1];
	} else if (tokens[0] == "worker_process") {
		httpConf->_global_worker_process = tokens[1];
	} else if (tokens[0] == "DEBUG_log") {
		httpConf->_global_error_log = tokens[1];
	} else if (tokens[0] == "pid") {
		httpConf->_global_pid = tokens[1];
	} else if (tokens[0] == "keepalive_timeout") {
		httpConf->_keepalive_timeout = tokens[1];
	} else if (tokens[0] == "add_header") {
		httpConf->_add_header = tokens[1];
	} else if (tokens[0] == "client_max_body_size") {
		httpConf->_client_max_body_size = ParserUtils::verifyClientMaxBodySize(tokens[1]);  // C++98 long long
		if (httpConf->_client_max_body_size == -1) {
			Logger::log(Logger::ERROR, "Invalid client_max_body_size value");
			return false;
		}
	} else if (tokens[0] == "root") {
		httpConf->_root = tokens[1];
	} else if (tokens[0] == "index") {
		for (size_t j = 1; j < tokens.size(); j++) {
			httpConf->_index.push_back(tokens[j]);
		}
	} else if (tokens[0] == "error_page") {
		// handle multiple error codes for a single page
		STR page_path = tokens[tokens.size() - 1];

		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) {
				httpConf->_error_pages[code] = page_path;
			} else {
				Logger::log(Logger::ERROR, "Invalid error code: " + tokens[j]);
			}
		}
	} else {
		Logger::log(Logger::ERROR, "HttpConfig has extra type " + tokens[0]);
		return false;
	}
	return true;
}

// Fill Server block
bool ParserFiller::FillServer(ServerConfig* serverConf, VECTOR<STR> tokens){
	if (tokens[0] == "add_header") {
		serverConf->_add_header = tokens[1];
	} else if (tokens[0] == "listen") {  // change listen server and port
		// parse server address and port
		size_t position = tokens[1].find(':');
		if (position == STR::npos) {
			serverConf->_listen_port = ParserUtils::verifyPort(tokens[1]);
			if (serverConf->_listen_port == -1) {
				Logger::log(Logger::ERROR, "Invalid port value");
				return false;
			}
		} else {
			serverConf->_listen_server = tokens[1].substr(0, position);
			if (serverConf->_listen_server == "") {
				Logger::log(Logger::ERROR, "Invalid server address");
				return false;
			}
			serverConf->_listen_port = ParserUtils::verifyPort(tokens[1].substr(position + 1));
			if (serverConf->_listen_port == -1) {
				Logger::log(Logger::ERROR, "Invalid port value");
				return false;
			}
		}

	} else if (tokens[0] == "server_name") {
		for (size_t j = 1; j < tokens.size(); j++) {
			serverConf->_server_name.push_back(tokens[j]);
		}
	} else if (tokens[0] == "root") {
		serverConf->_root = tokens[1];
	} else if (tokens[0] == "index") {
		for (size_t j = 1; j < tokens.size(); j++) {
			serverConf->_index.push_back(tokens[j]);
		}
	} else if (tokens[0] == "error_page") {
		STR page_path = tokens[tokens.size() - 1];

		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) {
				serverConf->_error_pages[code] = page_path;
			} else {
				Logger::log(Logger::ERROR, "Invalid error code: " + tokens[j]);
			}
		}
	} else if (tokens[0] == "client_max_body_size") {
		serverConf->_client_max_body_size = ParserUtils::verifyClientMaxBodySize(tokens[1]);
		if (serverConf->_client_max_body_size == -1) {
			Logger::log(Logger::ERROR, "Invalid client_max_body_size value");
			return false;
		}
	} else if (tokens[0] == "return") {  // indicate that it's a return directive
		if (tokens.size() < 2) {
			Logger::log(Logger::ERROR, "Invalid return directive");
			return false;
		}
		else if (tokens.size() == 3) {
			serverConf->_return_code = atoi(tokens[1].c_str());
			if (serverConf->_return_code < 300 || serverConf->_return_code > 400) {
				Logger::log(Logger::ERROR, "Invalid return code");
				return false;
			}
			serverConf->_return_url = tokens[2];
		}
		else // tokens.size() == 2
			serverConf->_return_url = tokens[1];
	} else {
		Logger::log(Logger::ERROR, "HttpConfig has extra type " + tokens[0]);
		return false;
	}
	return true;
}

//  -- to fill location config --
bool ParserFiller::FillLocation(LocationConfig* locConf, VECTOR<STR> tokens){
	if (tokens[0] == "proxy_pass") {
		int	host_start;
		int	host_end;
		int	delim_position;
		int	port_end;

		//extracting host and port from		Host: localhost:8080
		host_start = tokens[1].find_first_of(':') + 3;
		delim_position = tokens[1].find(':', tokens[1].find_first_of(':') + 1);

		if (delim_position == (int)STR::npos) {
		//host without port
			host_end = tokens[1].find_last_not_of(' ');
			locConf->_proxy_pass_host = tokens[1].substr(host_start, host_end - host_start);
		} else {
		//host with port
			host_end = delim_position;
			port_end = tokens[1].find_last_not_of(' ');
			locConf->_proxy_pass_host = tokens[1].substr(host_start, host_end - host_start);
			locConf->_proxy_pass_port = atoi(tokens[1].substr(delim_position + 1, delim_position + 1 - port_end).c_str());
		}
	} else if (tokens[0] == "path") {
		locConf->_path = tokens[1];
	} else if (tokens[0] == "add_header") {
		locConf->_add_header = tokens[1];
	} else if (tokens[0] == "return") {
		if (tokens.size() < 2) {
			Logger::log(Logger::ERROR, "Invalid return directive");
			return false;
		}
		else if (tokens.size() == 3) {
			locConf->_return_code = atoi(tokens[1].c_str());
			if (locConf->_return_code < 300 || locConf->_return_code > 400) {
				Logger::log(Logger::ERROR, "Invalid return code");
				return false;
			}
			locConf->_return_url = tokens[2];
		}
		else // tokens.size() == 2
		locConf->_return_url = tokens[1];
	} else if (tokens[0] == "root") {
		locConf->_root = tokens[1];
	} else if (tokens[0] == "client_max_body_size") {
		locConf->_client_max_body_size = ParserUtils::verifyClientMaxBodySize(tokens[1]);
		if (locConf->_client_max_body_size == -1) {
			Logger::log(Logger::ERROR, "Invalid client_max_body_size value");
			return false;
		}
	} else if (tokens[0] == "autoindex") {
		// locConf->_autoindex = (tokens[1] == "on");  // Simple bool conversion
		locConf->_autoindex = ParserUtils::verifyAutoIndex(tokens[1]);
	} else if (tokens[0] == "index") {
		for (size_t j = 1; j < tokens.size(); j++) {
			locConf->_index.push_back(tokens[j]);
		}
	} else if (tokens[0] == "error_page") {
		STR page_path = tokens[tokens.size() - 1];

		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) {
				locConf->_error_pages[code] = page_path;
			} else {
				Logger::log(Logger::ERROR, "Invalid error code: " + tokens[j]);
			}
		}
	} else if (tokens[0] == "allowed_methods") {
		for (size_t j = 1; j < tokens.size(); j++) {
			if (tokens[j] != "GET" && tokens[j] != "POST" && tokens[j] != "DELETE")
				return false;
			locConf->_allowed_methods[tokens[j]] = true;
		}
	} else if (tokens[0] == "upload_store") {
		locConf->_upload_store = tokens[1];
	} else if (tokens[0] == "alias") {
		locConf->_alias = tokens[1];
	} else {
		Logger::log(Logger::ERROR, "Location has extra type " + tokens[0]);
		return false;
	}
	return true;
}

bool ParserFiller::FillDirective(AConfigBase* block, STR line, int position) {
	VECTOR<STR> tokens;
	STR 		trimmed_line;

	int semicol = line.find(';', position);
	trimmed_line = line.substr(position, semicol - position);
	tokens = Utils::split(trimmed_line, ' ', 1);

	if (HttpConfig* httpConf = dynamic_cast<HttpConfig*>(block)) {
		return FillHttp(httpConf, tokens);
    }
	else if (ServerConfig* serverConf = dynamic_cast<ServerConfig*>(block)) {
		return FillServer(serverConf, tokens);
    }
    else if (LocationConfig* locConf = dynamic_cast<LocationConfig*>(block)) {
		return FillLocation(locConf, tokens);
    }
	Logger::log(Logger::ERROR, "Config has unknown block type " + tokens[0]);

    return false;  // Unknown block type
}
