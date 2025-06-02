#include "Parser.hpp"
#include "Logger.hpp"

Parser::Parser() {
	_config = new HttpConfig();
}

Parser::Parser(STR file) {
	_config = new HttpConfig();

	_filepath = file;
}

Parser::Parser(const Parser &obj) {
	_config = obj._config;
	_filepath = obj._filepath;
}

Parser &Parser::operator=(const Parser &obj) {
	(void)obj;
	return (*this);
}

Parser::~Parser() {
	if (_config)
		delete _config;
}

int Parser::verifyPort(STR port_str) {
	std::stringstream ss(port_str);
	int port;

	if (!(ss >> port) || port < 0 || port > 65335) {
		return -1;
	}
	return port;
}

bool Parser::verifyAutoIndex(STR autoindex_str) {
	bool autoindex = false;
	if (autoindex_str == "on") {
		autoindex = true;
	}
	else if (autoindex_str == "off") {
		autoindex = false;
	}
	return autoindex;
}

/*
	 * client_max_body_size we use MB NOT MiB
 * 1b = 1 byte
 * 1k = 1000 bytes
 * 1m = 1000 * 1000 bytes
 * 1g = 1000 * 1000 * 1000 bytes
 *
 * default is 1M
*/
long long Parser::verifyClientMaxBodySize(STR client_max_body_size_str) {
	std::stringstream ss(client_max_body_size_str);
	long long value;
	STR unit;

	if (!(ss >> value)) {
		return -1;
	}
	ss >> unit;

	if (value < 0) {
		return -1;
	}

	if (unit.empty() || unit == "b" || unit == "B") { // default byte
		value *= 1;
	}
	else if (unit == "k" || unit == "K") {
		value *= 1000;
	}
	else if (unit == "m" || unit == "M") {
		value *= 1000 * 1000;
	}
	else if (unit == "g" || unit == "G") {
		value *= 1000 * 1000 * 1000;
	}

	if (value > LLONG_MAX) {
		return -1;
	}

	return value;
}

VECTOR<STR>	split(STR string, char delim, bool use_whitespaces_delim) {
	VECTOR<STR>	result;
	STR					temp_line;
	std::istringstream	string_stream (string);

	// Split by any whitespace (>> skips at the beginning and then stops at any whitespace by default)
	if (use_whitespaces_delim) {
        STR token;

        while (string_stream >> token) {
            result.push_back(token);
        }
		return result;
	}

	while (getline(string_stream >> std::ws, temp_line, delim)) {
		result.push_back(temp_line);
	}

	return result;
}

bool	isDirectiveOk(STR line, int start, int end) {
	VECTOR<STR>	tokens;
	STR			trimmed_line;

	trimmed_line = line.substr(start, end - start);
	// std::cerr << "TRIMMED |" << trimmed_line << "|\n";
	tokens = split(trimmed_line, ' ', 1);
	if (tokens.size() < 2)
		return false;
	return true;
}

bool	isBlockOk(STR line, int start, int end) {
	VECTOR<STR>	tokens;
	STR			trimmed_line;
	STR			block_name;

	trimmed_line = line.substr(start, end - start + 1);

	block_name = trimmed_line.substr(0, trimmed_line.find('{'));

	if (block_name == "")
		return false;

	//checking text before block
	tokens = split(block_name, ' ', 1);
	if (tokens.size() != 1 && tokens.size() != 2)
		return false;

	if (tokens[0] != "events" && tokens[0] != "http" && tokens[0] != "server" && tokens[0] != "location")
		return false;

	if (tokens[0] == "location" && tokens.size() != 2)
		return false;

	if (tokens[0] != "location" && tokens.size() != 1)
		return false;

	return true;
}

bool	isBlockEndOk(STR line, int start) {
	std::istringstream	string_stream;
	STR					trimmed_line;
	STR					no_ws;

	trimmed_line = line.substr(start);

	string_stream.str(trimmed_line);
	string_stream >> no_ws;
	if (no_ws[0] != '}')
		return false;
	return true;
}

ElemType	Parser::DetectNextType(STR line, int position, int &block_size) {
	int	semicol = line.find(';', position);
	int	open_brace = line.find('{', position);
	int	close_brace = line.find('}', position);
	static int depth;

	block_size = 0;

	VECTOR<STR> last_char_check = split(line.c_str() + position, ' ', 1);
	if (last_char_check.size() == 1 &&
		last_char_check[0].size() == 1 &&
		last_char_check[0][0] == '}' &&
		depth != 1) {
			return BAD_TYPE;
	}

	if (semicol != CHAR_NOT_FOUND) {
		if (open_brace != CHAR_NOT_FOUND && close_brace != CHAR_NOT_FOUND) {
			if (semicol < open_brace && semicol < close_brace) {
				if (isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else if (open_brace != CHAR_NOT_FOUND) {
			if (semicol < open_brace) {
				if (isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else if (close_brace != CHAR_NOT_FOUND) {
			if (semicol < close_brace) {
				if (isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else {	//no open and no close braces
			if (isDirectiveOk(line, position, semicol)) {
				block_size = semicol - position + 1;
				return DIRECTIVE;
			}
			else
				return BAD_TYPE;
		}
	}
	if (open_brace != CHAR_NOT_FOUND && close_brace != CHAR_NOT_FOUND) {
		if (close_brace < open_brace && depth <= 0)
			return BAD_TYPE;
		if (close_brace < open_brace) {
			if (isBlockEndOk(line, position)) {
				block_size = close_brace - position + 1;
				depth--;
				return BLOCK_END;
			}
		} else if (isBlockOk(line, position, close_brace)) {
			block_size = open_brace - position + 1;
			depth++;
			return BLOCK;
		}
	} else if (close_brace != CHAR_NOT_FOUND) {
		if (depth <= 0)
			return BAD_TYPE;
		if (isBlockEndOk(line, position)) {
			block_size = close_brace - position + 1;
			depth--;
			return BLOCK_END;
		}
	}
	// else if (open_brace != CHAR_NOT_FOUND) {
	// 	if (isBlockOk(line, position, close_brace)) {
	// 		block_size = open_brace - position + 1;
	// 		depth++;
	// 		return BLOCK;
	// 	}
	// }

	return BAD_TYPE;
}

bool	Parser::ValidateConfig(STR full_config) {
	int	size = 1;

	for (size_t i = 0; i < full_config.size(); i += size)
	{
		switch (DetectNextType(full_config, i, size))
		{
		case DIRECTIVE:
			// std::cerr << (full_config.c_str() + i) << "\nTYPE: DIRECTIVE\n\n\n\n";
			break;
		case BLOCK:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BLOCK\n\n";
			break;
		case BLOCK_END:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BLOCK_END\n\n";
			break;
		default:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BAD\n\n";
			size = -1;
			break;
		}
		if (size == -1)
			break;
	}
	if (size == -1)
		return false;
	return true;
}

// Fill HTTP block
bool FillHttp(HttpConfig* httpConf, VECTOR<STR> tokens){
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
		httpConf->_client_max_body_size = Parser::verifyClientMaxBodySize(tokens[1]);  // C++98 long long
		if (httpConf->_client_max_body_size == -1) {
			std::cerr << "Invalid client_max_body_size value" << std::endl;
			return false;
		}
	} else if (tokens[0] == "root") {
		httpConf->_root = tokens[1];
	} else if (tokens[0] == "index") {
		for (size_t j = 1; j < tokens.size(); j++) {
			httpConf->_index.push_back(tokens[j]);
		}
	} else if (tokens[0] == "error_page") {
		// Handle multiple error codes for a single page
		STR page_path = tokens[tokens.size() - 1]; // Last token is the path

		// Process all error codes (all tokens except first and last)
		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) { // Only process valid numeric codes
				httpConf->_error_pages[code] = page_path;
			} else {
				Logger::cerrlog(Logger::ERROR, "Invalid error code: " + tokens[j]);
			}
		}
	} else {
		Logger::cerrlog(Logger::ERROR, "CHECKFillDirective HttpConfig extra type " + tokens[0]);
		return false;
	}
	return true;
}

bool FillServer(ServerConfig* serverConf, VECTOR<STR> tokens){
	if (tokens[0] == "add_header") {
		serverConf->_add_header = tokens[1];
	} else if (tokens[0] == "listen") {  // change listen server and port
		// parse server address and port
		size_t position = tokens[1].find(':');
		if (position == STR::npos) {
			serverConf->_listen_port = Parser::verifyPort(tokens[1]);
			if (serverConf->_listen_port == -1) {
				Logger::cerrlog(Logger::ERROR, "Invalid port value");
				return false;
			}
		} else {
			serverConf->_listen_server = tokens[1].substr(0, position);
			if (serverConf->_listen_server == "") {
				Logger::cerrlog(Logger::ERROR, "Invalid server address");
				return false;
			}
			serverConf->_listen_port = Parser::verifyPort(tokens[1].substr(position + 1));
			if (serverConf->_listen_port == -1) {
				Logger::cerrlog(Logger::ERROR, "Invalid port value");
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
		// Handle multiple error codes for a single page
		STR page_path = tokens[tokens.size() - 1]; // Last token is the path

		// Process all error codes (all tokens except first and last)
		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) { // Only process valid numeric codes
				serverConf->_error_pages[code] = page_path;
			} else {
				Logger::cerrlog(Logger::ERROR, "Invalid error code: " + tokens[j]);
			}
		}
	} else if (tokens[0] == "client_max_body_size") {
		serverConf->_client_max_body_size = Parser::verifyClientMaxBodySize(tokens[1]);
		if (serverConf->_client_max_body_size == -1) {
			Logger::cerrlog(Logger::ERROR, "Invalid client_max_body_size value");
			return false;
		}
	} else if (tokens[0] == "return") {  // indicate that it's a return directive
		if (tokens.size() < 2) {
			Logger::cerrlog(Logger::ERROR, "Invalid return directive");
			return false;
		}
		else if (tokens.size() == 3) {
			serverConf->_return_code = atoi(tokens[1].c_str());
			if (serverConf->_return_code < 300 || serverConf->_return_code > 400) {
				Logger::cerrlog(Logger::ERROR, "Invalid return code");
				return false;
			}
			serverConf->_return_url = tokens[2];
		}
		else // tokens.size() == 2
			serverConf->_return_url = tokens[1];
	} else {
		Logger::cerrlog(Logger::ERROR, "CHECKFillDirective ServerConfig extra type " + tokens[0]);
		return false;
	}
	return true;
}

//  -- to fill location config --
bool FillLocation(LocationConfig* locConf, VECTOR<STR> tokens){
	if (tokens[0] == "proxy_pass") {
		int	host_start;
		int	host_end;
		int	delim_position;
		int	port_end;

		//extracting host and port from 		Host: localhost:8080
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
			Logger::cerrlog(Logger::ERROR, "Invalid return directive");
			return false;
		}
		else if (tokens.size() == 3) {
			locConf->_return_code = atoi(tokens[1].c_str());
			if (locConf->_return_code < 300 || locConf->_return_code > 400) {
				Logger::cerrlog(Logger::ERROR, "Invalid return code");
				return false;
			}
			locConf->_return_url = tokens[2];
		}
		else // tokens.size() == 2
		locConf->_return_url = tokens[1];
	} else if (tokens[0] == "root") {
		locConf->_root = tokens[1];
	} else if (tokens[0] == "client_max_body_size") {
		locConf->_client_max_body_size = Parser::verifyClientMaxBodySize(tokens[1]);
		if (locConf->_client_max_body_size == -1) {
			std::cerr << "Invalid client_max_body_size value" << std::endl;
			return false;
		}
	} else if (tokens[0] == "autoindex") {
		// locConf->_autoindex = (tokens[1] == "on");  // Simple bool conversion
		locConf->_autoindex = Parser::verifyAutoIndex(tokens[1]);
	} else if (tokens[0] == "index") {
		for (size_t j = 1; j < tokens.size(); j++) {
			locConf->_index.push_back(tokens[j]);
		}
	} else if (tokens[0] == "error_page") {
		// Handle multiple error codes for a single page
		STR page_path = tokens[tokens.size() - 1]; // Last token is the path

		// Process all error codes (all tokens except first and last)
		for (size_t j = 1; j < tokens.size() - 1; j++) {
			int code = atoi(tokens[j].c_str());
			if (code > 0) { // Only process valid numeric codes
				locConf->_error_pages[code] = page_path;
			} else {
				Logger::cerrlog(Logger::ERROR, "Invalid error code: " + tokens[j]);
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
		Logger::cerrlog(Logger::ERROR, "CHECKFillDirective LocationConfig extra type " + tokens[0]);
		return false;
	}
	return true;
}

bool FillDirective(AConfigBase* block, STR line, int position) {
	VECTOR<STR> tokens;
	STR 		trimmed_line;

	int semicol = line.find(';', position);
	trimmed_line = line.substr(position, semicol - position);
	tokens = split(trimmed_line, ' ', 1);

	if (HttpConfig* httpConf = dynamic_cast<HttpConfig*>(block)) {
		return FillHttp(httpConf, tokens);
    }
	else if (ServerConfig* serverConf = dynamic_cast<ServerConfig*>(block)) {
		return FillServer(serverConf, tokens);
    }
    else if (LocationConfig* locConf = dynamic_cast<LocationConfig*>(block)) {
		return FillLocation(locConf, tokens);
    }
	Logger::cerrlog(Logger::ERROR, "CHECKFillDirective Unknown block type " + tokens[0]);

    return false;  // Unknown block type
}

AConfigBase	*CreateBlock(STR line, int start) {
	AConfigBase *block;
	VECTOR<STR>	tokens;
	STR			trimmed_line;
	STR			block_name;
	STR			block_body;
	int	close_brace = line.find('}', start);

	trimmed_line = line.substr(start, close_brace - start);
	block_name = trimmed_line.substr(0, trimmed_line.find('{'));
	tokens = split(block_name, ' ', 1);

	block = NULL;
	if (tokens[0] == "events" || tokens[0] == "http") {
		HttpConfig *conf = new HttpConfig();
		block = conf;
		Logger::log(Logger::DEBUG, "AConfigBase *CreateBlock CREATED http");

	} else if (tokens[0] == "server") {
		ServerConfig *conf = new ServerConfig();
		block = conf;
		Logger::log(Logger::DEBUG, "AConfigBase *CreateBlock CREATED server");
	} else if (tokens[0] == "location") {
		LocationConfig *conf = new LocationConfig();
		conf->_path = tokens[1];
		block = conf;
		Logger::log(Logger::DEBUG, "AConfigBase *CreateBlock CREATED location");
	}
	Logger::log(Logger::DEBUG, "AConfigBase *CreateBlock CREATED block2");

	return block;
}

bool	check_location_path_duplicate(STR new_path, MAP<STR, LocationConfig*> locs) {
	try
	{
		MAP<STR, LocationConfig*> loc_loc = locs;
		while (loc_loc.size() > 0) {
			MAP<STR, LocationConfig*>::iterator it = loc_loc.begin();
			if (it->first == new_path)
				return false;
			if (it->second->_locations.size() > 0) {
				loc_loc.insert(it->second->_locations.begin(), it->second->_locations.end());
			}
			loc_loc.erase(it);
		}
	}
	catch(const std::exception& e)
	{
		return false;
	}
	return true;
}

AConfigBase	*AddBlock(AConfigBase *prev_block, STR line, int start) {
	AConfigBase	*child = NULL;
	ConfigBlock parent_type = ERROR;

	parent_type = prev_block->_identify(prev_block);
	if (parent_type == HTTP) {
		HttpConfig* httpConf = dynamic_cast<HttpConfig*>(prev_block);
		ServerConfig* serverConf;

		child = CreateBlock(line, start);
		if (!child || !httpConf || httpConf->_identify(child) != SERVER) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		serverConf = dynamic_cast<ServerConfig*>(child);
		httpConf->_servers.push_back(serverConf);
		child->back_ref = prev_block;
	} else if (parent_type == SERVER) {
		ServerConfig* serverConf = dynamic_cast<ServerConfig*>(prev_block);
		LocationConfig* location;

		child = CreateBlock(line, start);
		if (!child || !serverConf || serverConf->_identify(child) != LOCATION) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		location = dynamic_cast<LocationConfig*>(child);

		if (!check_location_path_duplicate(location->_path, serverConf->_locations)) {
			child->_self_destruct();

			Logger::cerrlog(Logger::ERROR, "AConfigBase *AddBlock LOCATION DUPLICATE " + location->_path);

			return NULL;
		}

		serverConf->_locations[location->_path] = location;
		// location->back_ref = serverConf;
		child->back_ref = prev_block;
	} else if (parent_type == LOCATION) {
		LocationConfig* locParent = dynamic_cast<LocationConfig*>(prev_block);
		LocationConfig* locChild;

		// this is changed, adding verication dup server name
		ServerConfig* serverConf = NULL;

		child = CreateBlock(line, start);
		if (!child || !locParent || locParent->_identify(child) != LOCATION) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		locChild = dynamic_cast<LocationConfig*>(child);

		// find server
		AConfigBase* temp = prev_block;
		while (temp && temp->_identify(temp) != HTTP) {
			if (temp->_identify(temp) == SERVER) {
				serverConf = dynamic_cast<ServerConfig*>(temp);
				break;
			}
			temp = temp->back_ref;
		}

		if (!serverConf || serverConf->_identify(serverConf) != SERVER) {
			if (child)
				child->_self_destruct();
			std::cerr << "DEBUG AConfigBase	*AddBlock !serverConf" << !serverConf << "\n";
			return NULL;
		}

		// new thing, check dup server name
		if (!check_location_path_duplicate(locChild->_path, serverConf->_locations)) {
			child->_self_destruct();

			Logger::cerrlog(Logger::ERROR, "AConfigBase *AddBlock LOCATION DUPLICATE " + locChild->_path);

			return NULL;
		}

		locParent->_locations[locChild->_path] = locChild;
		// locChild->back_ref = locParent;
		child->back_ref = prev_block;
	} else {
		std::stringstream ss;
		ss << "AddBlockNULL Type " << static_cast<int>(parent_type);
		Logger::cerrlog(Logger::ERROR, ss.str());
		return NULL;
	}
	Logger::log(Logger::INFO, "AConfigBase *AddBlock exit");

	return child;
}

bool	minimum_value_check(HttpConfig *conf) {
	if (conf->_servers.empty()) {
		Logger::cerrlog(Logger::ERROR, "No servers found");
		return false;
	}

	for (size_t i = 0; i < conf->_servers.size(); i++)
	{
		//if it's the only block - defaults to 80
		if (conf->_servers[i]->_listen_port == -1 && conf->_servers.size() == 1) {
			conf->_servers[i]->_listen_port = 80;
		}
		if (conf->_servers[i]->_listen_port == -1) {
			Logger::cerrlog(Logger::ERROR, "Server without port found");
			return false;
		}
		if (conf->_servers[i]->_locations.empty()) {
			Logger::cerrlog(Logger::ERROR, "Server without location found");
			return false;
		}
	}
	return true;
}

HttpConfig *Parser::Parse() {
	STR			line;
	VECTOR<STR>	tokens;
	STR			full_config;

	if (_filepath == "")
		_file.open("config.conf");
	else
		_file.open(_filepath.c_str());
	if (!_file.is_open()) {
		Logger::cerrlog(Logger::ERROR, "Could not open file " + _filepath);
		return (NULL);
	}

	STR temp_line;
	while (getline(_file, temp_line)) {
		full_config += temp_line;
    }
	_file.close();

	if (!ValidateConfig(full_config)) {
		Logger::cerrlog(Logger::ERROR, "Config is not correct!");
		return NULL;
	}

	int	size = 1;

	HttpConfig *base = new HttpConfig;
	base->back_ref = NULL;
	AConfigBase	*currentBlock = base;

	bool skipped_block1 = -1;
	bool skipped_block2 = -1;
	MAP<int, int>	directives_per_block;
	int depth = 0;
	for (size_t i = 0; i < full_config.size(); i += size)
	{
		switch (DetectNextType(full_config, i, size))
		{
		case DIRECTIVE:
			directives_per_block[depth]++;
			Logger::log(Logger::DEBUG, "DIRECTIVE (" + full_config.substr(i, 50) + ")");
			if (!FillDirective(currentBlock, full_config, i)) {
				base->_self_destruct();

				Logger::cerrlog(Logger::ERROR, "CHECKFillDirective");
				return NULL;
			}
			std::cerr << "--DIRECTIVE\n";
			break;
		case BLOCK:
			depth++;
			directives_per_block[depth] = 0;
			Logger::log(Logger::DEBUG, "BLOCK (" + full_config.substr(i, 50) + ")");
			//if event/http, we already have default one
			if (!strncmp(full_config.c_str() + i, "http", 4) || !strncmp(full_config.c_str() + i, "events", 6)) {
				skipped_block1 = depth;
				std::cerr << "--(skip)BLOCK\n";
				continue;
			}

			currentBlock = AddBlock(currentBlock, full_config, i);
			if (!currentBlock) {
				base->_self_destruct();

				Logger::log(Logger::ERROR, "CHECKAddBlock");
				return NULL;
			}
			std::cerr << "--BLOCK\n";
			break;
		case BLOCK_END:
			Logger::log(Logger::DEBUG, "BLOCK_END (" + full_config.substr(i, 50) + ")");
			if (skipped_block1 == depth)
				skipped_block1 = -1;
			else if (skipped_block2 == depth)
				skipped_block2 = -1;
			else
				currentBlock = currentBlock->back_ref;
			if (directives_per_block[depth] == 0) {
				//ERROR: block doesn't have it's own directives!
				base->_self_destruct();

				Logger::cerrlog(Logger::ERROR, "CHECKFillDirective block doesn't have it's own directives!");
				return NULL;
			}
			depth--;
			std::cerr << "--BLOCK_END\n";
			break;
		default:
			size = -1;
			break;
		}
		if (size == -1)
			break;
	}

	if (base->_identify(currentBlock) != HTTP || currentBlock != base || depth != 0) {
		base->_self_destruct();
		return NULL;
	}

	if (!minimum_value_check(base)) {
		base->_self_destruct();
		return NULL;
	}

	return (base);
}
