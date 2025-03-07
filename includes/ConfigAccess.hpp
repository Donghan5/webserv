#ifndef CONFIGACCESS_HPP
#define CONFIGACCESS_HPP

#include <iostream>

class ConfigAccess {
	public:
		static bool getDirectiveValue(const ConfigBlock* block, const std::string& directiveName, std::string& outValue, size_t paramIndex = 0);

		static ConfigBlock* findFirstServerBlock(const Config* config);

		static ConfigBlock* findLocationBlock(const ConfigBlock* serverBlock, const std::string& path);

		static std::vector<ConfigBlock*> getAllServerBlocks(const Config* config);

		static std::vector<ConfigDirective*> getDirectives(const ConfigBlock* block, const std::string& directiveName);
};

#endif // CONFIGACCESS_HPP
