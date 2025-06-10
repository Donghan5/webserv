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
	virtual ~AConfigBase() {}
	AConfigBase();
};

#endif
