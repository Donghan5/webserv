#ifndef REQUESTSMANAGER_HPP
# define REQUESTSMANAGER_HPP
# include "Response.hpp"

class RequestsManager {
	private:
		HttpConfig 		*_config;
		int				_client_fd;
		MAP<int, STR>	_partial_requests;
		MAP<int, STR>	_partial_responses;
        MAP<int, Response*> _active_responses; // Track active responses, particularly CGI ones

		int				HandleRead();               //*ints here should indicate next action like 1 = nothing, 0 = remove fd,
													// 2 = update fd status
		int 			HandleWrite();				//*
        STR             createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base);

		public:
		RequestsManager();
		RequestsManager(int client_fd);
		RequestsManager(HttpConfig *config);
		RequestsManager(HttpConfig *config, int client_fd);
		RequestsManager(const RequestsManager &obj);
		~RequestsManager();

		void setConfig(HttpConfig *config);
		void setClientFd(int client_fd);
		int HandleClient(short int revents);
		void CloseClient();

        // Methods for CGI management
        int RegisterCgiFd(int cgi_fd, int client_fd);
        void CleanupClient(int client_fd);
        int getCurrentCgiFd() const; // Get current CGI fd for the client
        int HandleCgiOutput(int fd);    // Handle CGI output ready event - moved to public
};

#endif
