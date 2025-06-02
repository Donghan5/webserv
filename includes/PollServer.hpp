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
    POST_FD
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

		bool	WaitAndService(RequestsManager &requests);
		void	AcceptClient(int new_fd);
		void	CloseClient(int client_fd);
		void	HandleCgiOutput(int cgi_fd, RequestsManager &requests);
		bool	AddFd(int fd, uint32_t events, FdType type);
		bool	ModifyFd(int fd, uint32_t events);
		bool	RemoveFd(int fd);
		bool	AddServerSocket(int port, int socket_fd);
		bool	AddCgiFd(int cgi_fd, int client_fd);
		void	getUniqueServers(const HttpConfig *hcf, MAP<int, STR>& unique_servers);
		void	processDisconnectOrTimeoutCgis(RequestsManager &manager);
		void	handleSingleEpollEvent(const epoll_event& current_event, RequestsManager &manager);
		void	checkingEventError(const epoll_event& current_event, RequestsManager &manager, FdType fd_type, int fd);
		void	handleClientEventActivity(const epoll_event& current_event, RequestsManager &manager, int fd, int status);
		void	handleEventBasedOnFdType(const epoll_event& current_event, RequestsManager &manager, int fd, FdType fd_type);
		void	initializeServerSockets(MAP<int, STR>& unique_servers);

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
