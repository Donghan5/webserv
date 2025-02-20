#include "../includes/ParseConf.hpp"
#include "../includes/Logger.hpp"

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

	std::cout << line << std::endl;
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

void ParseConf::handleLocationBlock(std::ifstream& file, ServerConf& serverConfig) {
    LocationConf locationConfig;
    string_vector tokens = confReadToken(file);

    if (tokens.size() < 2) {
        throw std::runtime_error("Invalid location block format");
    }

    locationConfig.setPath(tokens[1]); // Set path for location block
    locationConfig.setData("path", tokens[1]);

    tokens = confReadToken(file);

    while (true) {
        tokens = confReadToken(file);
        if (tokens.empty()) continue;

        e_type_key type = getKeyType(tokens[0]);
        if (type == KEY_CLOSING_BRACE) break;

        std::string value = tokens.size() > 1 ? tokens[1] : "";
        if (tokens[0] == "root") {
            locationConfig.setRootDir(value);
        } else if (tokens[0] == "index") {
            locationConfig.setData("index", value);
        } else if (tokens[0] == "autoindex") {
            locationConfig.setData("autoindex", value);
        } else {
            locationConfig.setData(tokens[0], value);
        }
    }
    serverConfig.addLocation(locationConfig);
	locationConfig.showLocationData();
}

/*
	Handle multiple Server block
*/
void ParseConf::handleServerBlock(std::ifstream &file, HttpConf &httpConfig) {
    ServerConf serverConfig;
    string_vector tokens = confReadToken(file);

	serverConfig.setData(tokens[0], tokens[1]);
    while (true) {
        tokens = confReadToken(file);
        if (tokens.size() == 0) continue;

        e_type_key typeToken = getKeyType(tokens[0]);
        if (typeToken == KEY_CLOSING_BRACE) break;  // 서버 블록 종료

        if (typeToken == KEY_LOCATION) {
            handleLocationBlock(file, serverConfig);
        }
        serverConfig.setData(tokens[0], tokens[1]);
    }
    httpConfig.addServer(serverConfig);
	serverConfig.showServerData();
}


/*
	Handle multiple http block
*/
void ParseConf::handleHttpBlock(std::ifstream &file) {
	HttpConf hconf = _webconf->getHttpBlock();
	string_vector tokens;

	while (true) {
		tokens = confReadToken(file);
		e_type_key typeToken = getKeyType(tokens[0]);
		if (tokens.empty()) continue;
		if (typeToken == KEY_CLOSING_BRACE) break; // closing brace
		else if (typeToken == KEY_SERVER) {
			handleServerBlock(file, hconf);
		} else {
			std::string value = tokens[1];
			if (!value.empty() && value[value.size() - 1] == ';') {
				value = value.substr(0, value.size() - 1);
			}
			hconf.setData(tokens[0], value);
		}
		_webconf->setHttpBlock(hconf);
	}
	hconf.showHttpData();
}

/*
	Handle event block
*/
void ParseConf::handleEventBlock(std::ifstream &file) {
    string_vector tokens = confReadToken(file);

    bool isEmpty = true;

    while (true) {
        tokens = confReadToken(file);
        if (tokens.size() == 0) continue;

        e_type_key typeToken = getKeyType(tokens[0]);
        if (typeToken == KEY_CLOSING_BRACE) break; // 블록 종료

        isEmpty = false;

        if (tokens.size() > 1) {
            std::string value = tokens[1];
            if (value.size() > 0 && value[value.size() - 1] == ';') {
                value.erase(value.size() - 1, 1);  // `;` 제거
            }

            if (tokens[0] == "worker_connections") {
                if (isdigit(value[0])) {  // 값이 숫자인지 확인
                    this->_webconf->getEventConf().setWorkerConnections(std::atoi(value.c_str()));
                } else {
                    Logger::log(Logger::ERROR, "Invalid argument for worker_connections: " + value);
                    throw std::runtime_error("Invalid argument in event conf");
                }
            } else if (tokens[0] == "use") {
                if (value.size() > 0) {
                    this->_webconf->getEventConf().setUseMethods(value);
                } else {
                    Logger::log(Logger::ERROR, "Invalid argument for use directive");
                    throw std::runtime_error("Invalid argument in event conf");
                }
            } else {
                Logger::log(Logger::ERROR, "Unknown directive inside events block: " + tokens[0]);
            }
        } else {
            Logger::log(Logger::ERROR, "Invalid event directive format");
        }
    }

    if (isEmpty) {
        _webconf->getEventConf().setUseMethods("");
        _webconf->getEventConf().setWorkerConnections(0);
    }
}


/*
	Parsing configuration file
	@param
		confFileName
*/
void ParseConf::ParsingConfigure(std::string confFileName) {
    std::ifstream file(confFileName.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open the file");
    }

    bool httpBlockFound = false;
    bool eventBlockProcessed = false;

    while (true) {
        string_vector tokens = confReadToken(file);
        if (tokens.size() == 0) break;

        e_type_key type = getKeyType(tokens[0]);

        switch (type) {
            case KEY_EVENT:
                if (eventBlockProcessed) {
                    throw std::logic_error("Duplicate 'events' block detected.");
                }
                handleEventBlock(file);
                eventBlockProcessed = true;
                break;
            case KEY_HTTP:
                handleHttpBlock(file);
                httpBlockFound = true;
                break;
            case KEY_LOCATION:
                throw std::logic_error("Invalid top-level directive: location (must be inside a server block)");
            default:
                throw std::logic_error("Invalid top-level directive: " + tokens[0]);
        }
    }

    if (!httpBlockFound) {
        throw std::logic_error("Missing required 'http' block in configuration.");
    }
}


ParseConf::ParseConf(std::string confFileName): _confFileName(confFileName) {
	_webconf = new WebServConf();
	try {
		ParsingConfigure(confFileName);
	} catch (const std::exception &e) {
		Logger::log(Logger::ERROR, "Parsing Error");
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
