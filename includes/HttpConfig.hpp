#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP

# include "AConfigBase.hpp"

/*
	Needed for sure:					Subject line
	!	error_pages						Your server must have default error pages if none are provided
		host and port					Choose the port and host of each ’server’.
		server_name						Set up the server_names or not
		max_body_size					Set the maximum allowed size for client request bodies
		allow_method					Define a list of accepted HTTP methods for the route.
		return							Define an HTTP redirect
		root							Define a directory or file where the requested file should be located (e.g.,
										if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is
										/tmp/www/pouic/toto/pouet).
		autoindex						Enable or disable directory listing
		index							Set a default file to serve when the request is for a directory.
		(cgi support)					Execute CGI based on certain file extension (for example .php).
	!	upload_store??					Allow the route to accept uploaded files and configure where they should be
										saved.
*/

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

	HttpConfig() :
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
};

#endif
