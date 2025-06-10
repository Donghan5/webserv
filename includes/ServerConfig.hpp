#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP
# include "AConfigBase.hpp"

struct LocationConfig;

struct ServerConfig : AConfigBase {
	int								_return_code;				//server, location
	STR								_return_url;				//server, location
	int								_listen_port;
	STR								_listen_server;
	VECTOR<STR>						_server_name;

	MAP<STR, LocationConfig*>		_locations;
	void							_self_destruct();
	ServerConfig();
};

#endif
