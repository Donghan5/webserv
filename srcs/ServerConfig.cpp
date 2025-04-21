#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

void ServerConfig::_self_destruct() {
	for (std::map<STR, LocationConfig*>::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->second)
			it->second->_self_destruct();
		it->second = NULL;
	}
	delete (this);
}
