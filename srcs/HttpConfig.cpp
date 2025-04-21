#include "HttpConfig.hpp"
#include "ServerConfig.hpp"

void HttpConfig::_self_destruct() {
	for (size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i])
			_servers[i]->_self_destruct();
		_servers[i] = NULL;
	}
	delete (this);
}
