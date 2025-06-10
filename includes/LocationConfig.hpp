#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP
# include "AConfigBase.hpp"

struct LocationConfig : AConfigBase {
	STR								_proxy_pass_host;
	int								_proxy_pass_port;
	STR								_path;
	int								_return_code;				//server, location
	STR								_return_url;				//server, location
	bool							_autoindex;
	MAP<STR, bool>					_allowed_methods;
	MAP<STR, LocationConfig*>		_locations;
	STR								_upload_store;
	STR								_alias;

	void							_self_destruct();
	LocationConfig();
};

#endif
