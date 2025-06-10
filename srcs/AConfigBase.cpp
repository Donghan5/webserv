#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

AConfigBase::AConfigBase() :
	_add_header(""),
	_root(""),
	_client_max_body_size(-1),
	_index(),
	_error_pages(),
	back_ref(NULL)
{
	_error_pages[400] = "default_errors/400.html";
	_error_pages[401] = "default_errors/401.html";
	_error_pages[402] = "default_errors/402.html";
	_error_pages[403] = "default_errors/403.html";
	_error_pages[404] = "default_errors/404.html";
	_error_pages[405] = "default_errors/405.html";
	_error_pages[406] = "default_errors/406.html";
	_error_pages[407] = "default_errors/407.html";
	_error_pages[408] = "default_errors/408.html";
	_error_pages[500] = "default_errors/500.html";
	_error_pages[502] = "default_errors/502.html";
	_error_pages[503] = "default_errors/503.html";
	_error_pages[504] = "default_errors/504.html";
}

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
