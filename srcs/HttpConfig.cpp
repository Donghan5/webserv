#include "HttpConfig.hpp"
#include "ServerConfig.hpp"

HttpConfig::HttpConfig() :
	_global_user(""),
	_global_worker_process("1"),
	_global_error_log("logs/error.log"),
	_global_pid("logs/nginx.pid"),
	_keepalive_timeout("65"),
	_servers()
{
	_root = "./www";
	_client_max_body_size = 1000000; // 1MB
}

void HttpConfig::_self_destruct() {
	for (size_t i = 0; i < _servers.size(); i++) {
		if (_servers[i])
			_servers[i]->_self_destruct();
		_servers[i] = NULL;
	}
	delete (this);
}
