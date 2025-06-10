#include "Parser/ParserUtils.hpp"

int ParserUtils::verifyPort(std::string port_str) {
	std::stringstream ss(port_str);
	int port;

	if (!(ss >> port) || port < 0 || port > 65335) {
		return -1;
	}
	return port;
}

bool ParserUtils::verifyAutoIndex(STR autoindex_str) {
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
long long ParserUtils::verifyClientMaxBodySize(std::string client_max_body_size_str) {
	std::stringstream ss(client_max_body_size_str);
	long long value;
	std::string unit;

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

bool ParserUtils::isDirectiveOk(STR line, int start, int end) {
	VECTOR<STR>	tokens;
	STR			trimmed_line;

	trimmed_line = line.substr(start, end - start);
	tokens = Utils::split(trimmed_line, ' ', 1);
	if (tokens.size() < 2)
		return false;
	return true;
}

bool ParserUtils::isBlockOk(STR line, int start, int end) {
	VECTOR<STR>	tokens;
	STR			trimmed_line;
	STR			block_name;

	trimmed_line = line.substr(start, end - start + 1);

	block_name = trimmed_line.substr(0, trimmed_line.find('{'));

	if (block_name == "")
		return false;

	//checking text before block
	tokens = Utils::split(block_name, ' ', 1);
	if (tokens.size() != 1 && tokens.size() != 2)
		return false;

	if (tokens[0] != "http" && tokens[0] != "server" && tokens[0] != "location")
		return false;

	if (tokens[0] == "location" && tokens.size() != 2)
		return false;

	if (tokens[0] != "location" && tokens.size() != 1)
		return false;

	return true;
}

bool	ParserUtils::isBlockEndOk(STR line, int start) {
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

bool	ParserUtils::check_location_path_duplicate(STR new_path, MAP<STR, LocationConfig*> locs) {
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

bool	ParserUtils::minimum_value_check(HttpConfig *conf) {
	if (conf->_servers.empty()) {
		Logger::log(Logger::ERROR, "No servers found");
		return false;
	}

	for (size_t i = 0; i < conf->_servers.size(); i++)
	{
		//if it's the only block - defaults to 80
		if (conf->_servers[i]->_listen_port == -1 && conf->_servers.size() == 1) {
			conf->_servers[i]->_listen_port = 80;
		}
		if (conf->_servers[i]->_listen_port == -1) {
			Logger::log(Logger::ERROR, "Server without port found");
			return false;
		}
		if (conf->_servers[i]->_locations.empty()) {
			Logger::log(Logger::ERROR, "Server without location found");
			return false;
		}
	}
	return true;
}
