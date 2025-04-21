#ifndef ACONFIGBASE_HPP
# define ACONFIGBASE_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <map>

#define STR 			std::string
#define CHAR_NOT_FOUND	(int)std::string::npos
#define	VECTOR			std::vector
#define MAP				std::map

enum ConfigBlock {
	HTTP,
	SERVER,
	LOCATION,
	ERROR
};

struct AConfigBase {
	STR					_add_header;
	STR					_root;
	long long			_client_max_body_size; //in bytes
	VECTOR<STR>			_index;
	MAP<int, STR>		_error_pages;

	AConfigBase			*back_ref;

	virtual void		_self_destruct() = 0;
	static	ConfigBlock	_identify(AConfigBase *elem);

	AConfigBase() :
        _add_header(""),
        _root(""),
        _client_max_body_size(-1),
        _index(),
        _error_pages(),
        back_ref(NULL)
    {
		_error_pages[400] = "default_errors/400.html";
		_error_pages[401] = "default_errors/401.html";
		_error_pages[402] = "default_errors/402.html";
		_error_pages[403] = "default_errors/403.html";
		_error_pages[404] = "default_errors/404.html";
		_error_pages[405] = "default_errors/405.html";
		_error_pages[406] = "default_errors/406.html";
		_error_pages[407] = "default_errors/407.html";
		_error_pages[408] = "default_errors/408.html";
		_error_pages[500] = "default_errors/500.html";
		_error_pages[502] = "default_errors/502.html";
		_error_pages[503] = "default_errors/503.html";
		_error_pages[504] = "default_errors/504.html";
	}

	virtual ~AConfigBase() {}
};

#endif
