#include "../includes/ParseConf.hpp"

/*
	Get type of configuration block
	@return
		each key type
*/
e_type_key ParseConf::getKeyType(const std::string &key) {
	if (key == "http") return KEY_HTTP;
	else if (key == "server") return KEY_SERVER;
	else if (key == "location") return KEY_LOCATION;
	else if (key == "}") return KEY_CLOSING_BRACE;
	else if (key == "event") return KEY_EVENT;
	return KEY_UNKNOWN;
}

const HttpConf &ParseConf::getHttpConfig(void) const {
	return this->_httpConfig;
}

const EventConf &ParseConf::getEventConfig(void) const {
	return this->_eventConfig;
}

void	ParseConf::addHttp(const HttpConf &http) {
	httpblocks.push_back(http);
}

/*
	Read and parse token by white spaces
	@param
		file: configuration file
	@return
		token: parsed token
*/
string_vector confReadToken(std::ifstream &file) {
	std::string line;
	string_vector tokens;

	while (std::getline(file, line)) {
		std::istringstream ss(line);
		std::string token;
		while (ss >> token) {
			if (token[0] == '#') break;
			tokens.push_back(token);
		}
		if (!tokens.empty()) return tokens;
	}
	return tokens;
}

void ParseConf::handleLocationBlock(std::ifstream &file, ServerConf &serverConfig) {
	string_vector tokens;
	LocationConf locationConfig;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (!tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) break;
		locationConfig.setData(tokens[0], tokens[1]);
	}
	serverConfig.addLocation(locationConfig);
	std::cout << "Verify Data (location): " << locationConfig.showLocationData() << std::endl;
}

/*
	Handle multiple Server block
*/
void ParseConf::handleServerBlock(std::ifstream &file, HttpConfig &httpConfig) {
	ServerConf serverConfig;
	string_vector tokens;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (!tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) break;

		if (typeToken == KEY_LOCATION) {
			if (tokens.size() < 2) throw std::logic_error("Invalid location block form");
			handleLocationBlock(file, serverConfig);
		} else {
			serverConfig.setData(tokens[0], tokens[1]);
		}
	}
	httpConfig.addServer(serverConfig);
	std::cout << "Verify Data (server): " << serverConfig.showServerData() << std::endl;
}

/*
	Handle multiple http block
*/
void ParseConf::handleHttpBlock(std::ifstream &file) {
	string_vector tokens;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (!tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) break;

		if (typeToken == KEY_SERVER) {
			handleServerBlock(file, this->_httpConfig);
		} else {
			_httpConfig.setData(tokens[0], tokens[1]);
		}
	}
	this->addHttp(this->_httpConfig);
	std::cout << "Verify Data (http): " << _httpConfig.showHttpData() << std::endl;
}

/*
	Handle event block
*/
void ParseConf::handleEventBlock(std::ifstream &file) {
	string_vector tokens;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0])
		if (tokens.empty()) continue; // tokens vector is empty
		if (typeToken == KEY_CLOSING_BRACE) break; // end of block
		if (tokens[0] == "worker_connections")
			this->_eventConfig.setWorkerConnections(std::atoi(token[1].c_str()));
		else if (tokens[0] == "use")
			this->_eventConfig.setUseMethods(token[1]);
	}
}

/*
	Parsing configuration file
	@param
		confFileName
*/
void	ParseConf::confParse(std::string confFileName) {
	std::ifstream file(confFileName.c_str());

	if (!file.is_open()) {
		throw std::runtime_error("Cannot open the file");
	}
	std::string line;
	while (std::getline(file, line)) {
		string_vector tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0])
		switch (typeToken) {
			case KEY_HTTP:
				handleHttpBlock(file);
				break;
			case KEY_EVENT:
				handleEventBlock(file);
				break;
			default:
				throw std::logic_error("Invalid top-level directvie: " + tokens[0]);
		}
	}
}

ParseConf::ParseConf(std::string confFileName): _confFileName(confFileName) {
	try {
		confParse(confFileName);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

ParseConf::~ParseConf() {}

ParseConf::ParseConf(const ParseConf &obj) {
	*this = obj;
}

ParseConf &ParseConf::operator=(const ParseConf &obj) {
	if (this != &obj) {
		this->_confFileName = obj._confFileName;
		this->_httpConfig = obj._httpConfig;
		this->_eventConfig = obj._eventConfig;
		this->httpblocks = obj.httpblocks;
	}
	return (*this);
}
