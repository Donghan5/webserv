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


	void							_self_destruct();

	LocationConfig() :
		_proxy_pass_host(""),
		_proxy_pass_port(8080),
        _path(""),
		_return_code(-1),
		_return_url(""),
        _autoindex(false),
		_upload_store("")
    {
		_allowed_methods["GET"] = false;
		_allowed_methods["POST"] = false;
		_allowed_methods["DELETE"] = false;
	}
};

#endif
