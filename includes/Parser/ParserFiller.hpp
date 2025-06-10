#ifndef PARSERFILLER_HPP
#define PARSERFILLER_HPP

#include <string>
#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <limits.h>
#include "ParserUtils.hpp"

class ParserFiller {
public:
	static bool FillHttp(HttpConfig* httpConf, VECTOR<STR> tokens);
	static bool FillServer(ServerConfig* serverConf, VECTOR<STR> tokens);
	static bool FillLocation(LocationConfig* locConf, VECTOR<STR> tokens);
	static bool FillDirective(AConfigBase* block, STR line, int position);
};

#endif // PARSERFILLER_HPP
