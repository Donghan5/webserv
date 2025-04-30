#ifndef POLLSERVER_HPP
# define POLLSERVER_HPP
# include "HttpConfig.hpp"
# include "RequestsManager.hpp"
# include <iostream>

//to clean
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
#include <cstring>
#include <fcntl.h>
#include <map>
#include <sys/epoll.h>

enum FdType {
    SERVER_FD,
    CLIENT_FD,
    CGI_FD,
    POST_FD  // 추가된 타입: POST 작업 파일 디스크립터
};

class PollServer {
	private:
	HttpConfig 					*config;
	bool						running;
	std::map<int, int>			_server_sockets;      // port -> socket_fd
	std::map<int, STR>			_partial_requests;
	std::map<int, STR>			_partial_responses;
	std::map<int, FdType>       _fd_types;           // Track fd types
	std::map<int, int>          _cgi_to_client;      // Map CGI fd to client fd
	int							_epoll_fd;
	VECTOR<struct epoll_event>	_events;
	const int 					MAX_EVENTS;

	bool						WaitAndService(RequestsManager &requests);
	void						AcceptClient(int new_fd);
	void 						CloseClient(int client_fd);
	void                        HandleCgiOutput(int cgi_fd, RequestsManager &requests);
	bool AddFd(int fd, uint32_t events, FdType type);
	bool ModifyFd(int fd, uint32_t events);
	bool RemoveFd(int fd);
	bool AddServerSocket(int port, int socket_fd);
	bool AddCgiFd(int cgi_fd, int client_fd);

	public:
		PollServer();
		PollServer(const PollServer &obj);
		PollServer(HttpConfig *config);
		~PollServer();

		void setConfig(HttpConfig *config);

		void start();
		void stop();
};

#endif
