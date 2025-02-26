
#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include "ConfigElement.hpp"
#include "ConfigBlock.hpp"
#include "ServerConf.hpp"
#include "ParseConf.hpp"
#include "ConfigDirective.hpp"

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

class ConfigParser {
	private:
		std::string input;
		size_t pos;

		void skipWhitespace();
		std::string readUntil(const std::string& delimiters);
		ConfigDirective* parseDirective();
		ConfigBlock* parseBlock();
		ConfigElement* parseElement();

	public:
		ConfigBlock* parseString(const std::string& configString);
		ConfigBlock* parseFile(const std::string& filename);
};

#endif // CONFIGPARSER_HPP
