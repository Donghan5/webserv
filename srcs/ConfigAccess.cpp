#include "../includes/ConfigAccess.hpp"

static bool ConfigAccess::getDirectiveValue(const ConfigBlock* block, const std::string& directiveName, std::string& outValue, size_t paramIndex) {
	const std::vector<ConfigElement*>& children = block->getChildren();
	for (size_t i = 0; i < children.size(); ++i) {
		const ConfigDirective* directive = dynamic_cast<const ConfigDirective*>(children[i]);
		if (directive && directive->getName() == directiveName) {
			const std::vector<std::string>& params = directive->getParameters();
			if (paramIndex < params.size()) {
				outValue = params[paramIndex];
				return true;
			}
		}
	}
	return false;
}

static ConfigBlock* ConfigAccess::findFirstServerBlock(const Config* config) {
	const std::vector<ConfigElement*>& elements = config->getElements();
	for (size_t i = 0; i < elements.size(); ++i) {
		ConfigBlock* httpBlock = dynamic_cast<ConfigBlock*>(elements[i]);
		if (httpBlock && httpBlock->getName() == "http") {
			const std::vector<ConfigElement*>& children = httpBlock->getChildren();
			for (size_t j = 0; j < children.size(); ++j) {
				ConfigBlock* serverBlock = dynamic_cast<ConfigBlock*>(children[j]);
				if (serverBlock && serverBlock->getName() == "server") {
					return serverBlock;
				}
			}
		}
	}
	return NULL;
}

static ConfigBlock* ConfigAccess::findLocationBlock(
	const ConfigBlock* serverBlock,
	const std::string& path
) {
	const std::vector<ConfigElement*>& children = serverBlock->getChildren();
	for (size_t i = 0; i < children.size(); ++i) {
		ConfigBlock* locationBlock = dynamic_cast<ConfigBlock*>(children[i]);
		if (locationBlock && locationBlock->getName() == "location") {
			const std::vector<std::string>& params = locationBlock->getParameters();
			if (!params.empty() && params[0] == path) {
				return locationBlock;
			}
		}
	}
	return NULL;
}

static std::vector<ConfigBlock*> ConfigAccess::getAllServerBlocks(const Config* config) {
	std::vector<ConfigBlock*> servers;
	const std::vector<ConfigElement*>& elements = config->getElements();
	for (size_t i = 0; i < elements.size(); ++i) {
		ConfigBlock* httpBlock = dynamic_cast<ConfigBlock*>(elements[i]);
		if (httpBlock && httpBlock->getName() == "http") {
			const std::vector<ConfigElement*>& children = httpBlock->getChildren();
			for (size_t j = 0; j < children.size(); ++j) {
				ConfigBlock* serverBlock = dynamic_cast<ConfigBlock*>(children[j]);
				if (serverBlock && serverBlock->getName() == "server") {
					servers.push_back(serverBlock);
				}
			}
		}
	}
	return servers;
}

static std::vector<ConfigDirective*> ConfigAccess::getDirectives(const ConfigBlock* block, const std::string& directiveName) {
	std::vector<ConfigDirective*> directives;
	const std::vector<ConfigElement*>& children = block->getChildren();
	for (size_t i = 0; i < children.size(); ++i) {
		ConfigDirective* directive = dynamic_cast<ConfigDirective*>(children[i]);
		if (directive && directive->getName() == directiveName) {
			directives.push_back(directive);
		}
	}
	return directives;
}
