#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "AConfigBase.hpp"

struct ServerConfig;

struct HttpConfig : AConfigBase
{
	STR						_global_user;
	STR						_global_worker_process;
	STR						_global_error_log;
	STR						_global_pid;
	STR						_keepalive_timeout;

	VECTOR<ServerConfig*>	_servers;
	void					_self_destruct();
	HttpConfig();
};

#endif
