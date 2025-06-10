#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

ServerConfig::ServerConfig() :
	_return_code(-1),
	_return_url(""),
	_listen_port(-1), //80 only if it's the only block
	_listen_server("0.0.0.0"),
	_server_name()
{
	_server_name.push_back("localhost");
	_server_name.push_back("127.0.0.1");
}

void ServerConfig::_self_destruct() {
	for (MAP<STR, LocationConfig*>::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->second)
			it->second->_self_destruct();
		it->second = NULL;
	}
	delete (this);
}
