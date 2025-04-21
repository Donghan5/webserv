#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

ConfigBlock AConfigBase::_identify(AConfigBase* elem) {
	if (HttpConfig* httpConf = dynamic_cast<HttpConfig*>(elem)) {
		return HTTP;
	} else if (ServerConfig* serverConf = dynamic_cast<ServerConfig*>(elem)) {
		return SERVER;
	} else if (LocationConfig* locConf = dynamic_cast<LocationConfig*>(elem)) {
		return LOCATION;
	}
	return ERROR;
}
