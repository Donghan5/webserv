#ifndef PARSECONF_HPP
#define PARSECONF_HPP

#include <string>
#include "ConfigParser.hpp"
#include "ConfigAccess.hpp"
#include "WebServConf.hpp"

class ParseConf {
	private:
		WebServConf* _webconf;

	public:
		ParseConf(const std::string& confFileName);
		~ParseConf();

		void loadConfig(const std::string& confFileName);
		void parseFromConfigBlock(ConfigBlock* rootBlock);
		const WebServConf& getWebServConf() const;
};

#endif // PARSECONF_HPP
