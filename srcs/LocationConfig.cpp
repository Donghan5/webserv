#include "LocationConfig.hpp"

void LocationConfig::_self_destruct() {
	for (MAP<STR, LocationConfig*>::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->second)
			it->second->_self_destruct();
		it->second = NULL;
	}
	delete (this);
}
