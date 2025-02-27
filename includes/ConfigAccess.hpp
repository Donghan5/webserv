#ifndef CONFIGACCESS_HPP
#define CONFIGACCESS_HPP

#include <iostream>
#include <string>
#include <vector>
#include "ConfigBlock.hpp"
#include "ParseConf.hpp"

class ConfigAccess {
	public:
		static bool getDirectiveValue(const ConfigBlock* block, const std::string& directiveName, std::string& outValue, size_t paramIndex = 0);
		static ConfigBlock* findFirstServerBlock(const ConfigBlock* rootBlock);
		static ConfigBlock* findLocationBlock(const ConfigBlock* serverBlock, const std::string& path);
		static std::vector<ConfigBlock*> getAllServerBlocks(const ConfigBlock* rootBlock);
		static ConfigBlock *findEventBlock(ConfigBlock* rootBlock);
};

#endif // CONFIGACCESS_HPP
