#include "ParseConf.hpp"
#include "Logger.hpp"

ParseConf::ParseConf(const std::string& confFileName) {
	_webconf = new WebServConf();
	parseFile(confFileName);
}

ParseConf::~ParseConf() {
    delete _webconf;
}

void ParseConf::loadConfig(const std::string& confFileName) {
	ConfigParser parser;
	ConfigBlock* rootBlock = parser.parseFile(confFileName);

	if (!rootBlock) {
		throw std::runtime_error("Failed to parse configuration file");
	}

	parseFromConfigBlock(rootBlock);
	delete rootBlock;
}

void ParseConf::parseFromConfigBlock(ConfigBlock* rootBlock) {
	if (!rootBlock) {
		throw std::runtime_error("Root ConfigBlock is null");
	}

	HttpConf& httpConf = _webconf->getHttpBlock();
	EventConf& eventConf = _webconf->getEventConf();

	std::vector<ConfigBlock*> serverBlocks = ConfigAccess::getAllServerBlocks(rootBlock);
	for (size_t i = 0; i < serverBlocks.size(); ++i) {
		ServerConf serverConf;

		std::string listenValue, serverName;
		if (ConfigAccess::getDirectiveValue(serverBlocks[i], "listen", listenValue)) {
			serverConf.setData("listen", listenValue);
		}
		if (ConfigAccess::getDirectiveValue(serverBlocks[i], "server_name", serverName)) {
			serverConf.setData("server_name", serverName);
		}

		const std::vector<ConfigElement*>& children = serverBlocks[i]->getChildren();
		for (size_t j = 0; j < children.size(); ++j) {
			ConfigBlock* locationBlock = dynamic_cast<ConfigBlock*>(children[j]);
			if (locationBlock && locationBlock->getName() == "location") {
				LocationConf locationConf;
				std::vector<std::string> params = locationBlock->getParameters();
				if (!params.empty()) {
					locationConf.setPath(params[0]);
				}

				std::string root, index, autoindex;
				if (ConfigAccess::getDirectiveValue(locationBlock, "root", root)) {
					locationConf.setRootDir(root);
				}
				if (ConfigAccess::getDirectiveValue(locationBlock, "index", index)) {
					locationConf.setData("index", index);
				}
				if (ConfigAccess::getDirectiveValue(locationBlock, "autoindex", autoindex)) {
					locationConf.setData("autoindex", autoindex);
				}

				serverConf.addLocation(locationConf);
			}
		}

		httpConf.addServer(serverConf);
	}

	// event blocks
	ConfigBlock* eventBlock = ConfigAccess::findEventBlock(rootBlock);
	if (eventBlock) {
		std::string workerConnections, useMethod;

		if (ConfigAccess::getDirectiveValue(eventBlock, "worker_connections", workerConnections)) {
			try {
				int workerCount = std::atoi(workerConnections);
				if (workerCount < 1) {
					Logger::log(Logger::ERROR, "Invalid worker_connections value, must be >= 1");
				} else {
					eventConf.setWorkerConnections(workerCount);
				}
			} catch (const std::exception& e) {
				Logger::log(Logger::ERROR, "Invalid worker_connections format");
			}
		}

		if (ConfigAccess::getDirectiveValue(eventBlock, "use", useMethod)) {
			eventConf.setUseMethods(useMethod);
		}
	}
}

const WebServConf& ParseConf::getWebServConf() const {
    return *_webconf;
}
