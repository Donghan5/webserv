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

	ServerConfig() :
		_return_code(-1),
		_return_url(""),
        _listen_port(-1), //80 only if it's the only block
        _listen_server("0.0.0.0"),
        _server_name()
    {
		_server_name.push_back("localhost");
		_server_name.push_back("127.0.0.1");
	}
};

#endif
