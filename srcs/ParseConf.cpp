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
	else if (key == "events") return KEY_EVENT;
	else if (key == "if") return KEY_IF;
	return KEY_UNKNOWN;
}

/*
	Read and parse token by white spaces
	@param
		file: configuration file
	@return
		token: parsed token
*/
string_vector ParseConf::confReadToken(std::ifstream &file) {
	std::string line;
	string_vector tokens;

	while (std::getline(file, line)) {
		std::cout << "DEBUG: Read line: [" << line << "] (size: " << line.size() << ")" << std::endl;

		if (line.empty()) continue;

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
	std::ostringstream ifStatement;
	bool insideIf = false;

	if (tokens.size() > 2) {
		locationConfig.setPath(tokens[1]);
		locationConfig.setData("path", tokens[1]);
	}

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) {

			if (insideIf  == true) {
				ifStatement << "}";
				locationConfig.setData("if_statement", ifStatement.str());
				insideIf = false;
			}
			else {
				std::cout << "Closing brace in location block" << std::endl;
				break;
			}
		}
		else if (typeToken == KEY_IF) {
			insideIf = true;
			ifStatement.str("");
			for (size_t i (0); i < tokens.size(); i++) {
				ifStatement << tokens[i] << " ";
			}
		}
		else if (insideIf) {
			for (size_t i (0); i < tokens.size(); i++) {
				ifStatement << tokens[i] << " ";
			}
		}
		locationConfig.setData(tokens[0], tokens[1]);
	}
	serverConfig.addLocation(locationConfig);
	std::cout << "==Verify Data (location)==" << std::endl;
	locationConfig.showLocationData();
}

/*
	Handle multiple Server block
*/
void ParseConf::handleServerBlock(std::ifstream &file, HttpConf &httpConfig) {
	ServerConf serverConfig;
	string_vector tokens;
	std::ostringstream ifStatement;
	bool insideIf = false;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) {
			if (insideIf == true) {
				ifStatement << "}";
				serverConfig.setData("if_statement", ifStatement.str());
				insideIf = false;
			}
			else {
				std::cout << "Closing brace in server block" << std::endl;
				break;
			}
		}

		if (typeToken == KEY_LOCATION) {
			if (tokens.size() < 2) throw std::logic_error("Invalid location block form");
			handleLocationBlock(file, serverConfig);
		}
		else if (typeToken == KEY_IF) {
			insideIf = true;
			ifStatement.str("");
			for (size_t i (0); i < tokens.size(); i++) {
				ifStatement << tokens[i] << " ";
			}
		}
		else if (insideIf) {
			for (size_t i (0); i < tokens.size(); i++) {
				ifStatement << tokens[i] << " ";
			}
		}
		else {
			serverConfig.setData(tokens[0], tokens[1]);
		}
	}
	httpConfig.addServer(serverConfig);
	std::cout << "==Verify Data (server)==" << std::endl;
	serverConfig.showServerData();
}

/*
	Handle multiple http block
*/
void ParseConf::handleHttpBlock(std::ifstream &file) {
	string_vector tokens;
	HttpConf hconf;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) {
			std::cout << "Closing brace in http block" << std::endl;
			break;
		}

		if (typeToken == KEY_SERVER) {
			handleServerBlock(file, hconf);
		} else {
			hconf.setData(tokens[0], tokens[1]);
		}
	}
	_webconf->addHttpBlock(hconf);
	std::cout << "==Verify Data (http)==" << std::endl;
	hconf.showHttpData();
}

/*
	Handle event block
*/
void ParseConf::handleEventBlock(std::ifstream &file) {
	string_vector tokens;
	bool isEmpty = true;

	while (!file.eof()) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (tokens.empty()) continue; // tokens vector is empty
		if (typeToken == KEY_CLOSING_BRACE) break; // end of block

		isEmpty = false;
		if (tokens[0] == "worker_connections")
			this->_webconf->getEventConf().setWorkerConnections(std::atoi(tokens[1].c_str()));
		else if (tokens[0] == "use")
			this->_webconf->getEventConf().setUseMethods(tokens[1]);
	}
	if (isEmpty == true) {
		_webconf->getEventConf().setUseMethods("");
		_webconf->getEventConf().setWorkerConnections(0);
	}
}

/*
	Parsing configuration file
	@param
		confFileName
*/
void	ParseConf::ParsingConfigure(std::string confFileName) {
	std::ifstream file(confFileName.c_str());

	if (!file.is_open()) {
		throw std::runtime_error("Cannot open the file");
	}

	std::string firstLine;
	if (std::getline(file, firstLine)) {
		// if (firstLine.empty()) continue;
		std::cout << "DEBUG: First line read: [" << firstLine << "]" << std::endl;
		std::istringstream ss(firstLine);
		std::string token;
		string_vector firstTokens;
		while (ss >> token) {
			if (token[0] == '#') break;
			firstTokens.push_back(token);
		}

		if (!firstTokens.empty()) {
			e_type_key typeToken = getKeyType(firstTokens[0]);
			if (typeToken == KEY_EVENT) {
				std::cout << "DEBUG: Handling first-line EVENTS block" << std::endl;
				handleEventBlock(file);
			}
		}
	}

	std::string line;
	while (std::getline(file, line)) {
		string_vector tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
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
	_webconf = new WebServConf();
	try {
		ParsingConfigure(confFileName);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

ParseConf::~ParseConf() {
	delete _webconf;
}

ParseConf::ParseConf(const ParseConf &obj) {
	*this = obj;
}

ParseConf &ParseConf::operator=(const ParseConf &obj) {
	if (this != &obj) {
		this->_confFileName = obj._confFileName;
		delete _webconf;
		this->_webconf = new WebServConf(*obj._webconf);
	}
	return (*this);
}

const WebServConf &ParseConf::getWebServConf() const {
	return *_webconf;
}
