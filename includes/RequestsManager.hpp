// First, add the ClientState structure to RequestsManager.hpp

#ifndef REQUESTSMANAGER_HPP
# define REQUESTSMANAGER_HPP
# include "Response.hpp"

// Client state tracking structure
struct ClientState {
    Request request;
    long long body_read;
    bool processing_cgi;
    
    ClientState() : body_read(-1), processing_cgi(false) {}
};

class RequestsManager {
    private:
        HttpConfig      *_config;
        int             _client_fd;
        MAP<int, STR>   _partial_requests;
        MAP<int, STR>   _partial_responses;
        MAP<int, Response*> _active_responses; // Track active responses, particularly CGI ones
        MAP<int, ClientState> _client_states;  // Track client state for each client fd

        int             HandleRead();               //*ints here should indicate next action like 1 = nothing, 0 = remove fd,
                                                    // 2 = update fd status
        int             HandleWrite();              //*
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
        Response* getCgiResponse(int client_fd);  // Get CGI response for client
};

#endif