#include "LocationConfig.hpp"

LocationConfig::LocationConfig() :
	_proxy_pass_host(""),
	_proxy_pass_port(8080),
	_path(""),
	_return_code(-1),
	_return_url(""),
	_autoindex(false),
	_upload_store(""),
	_alias("")
{
	_allowed_methods["GET"] = false;
	_allowed_methods["POST"] = false;
	_allowed_methods["DELETE"] = false;
}

void LocationConfig::_self_destruct() {
	for (MAP<STR, LocationConfig*>::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->second)
			it->second->_self_destruct();
		it->second = NULL;
	}
	delete (this);
}
