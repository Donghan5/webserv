#ifndef HttpServer_HPP
#define HttpServer_HPP
#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <poll.h>
#include <fcntl.h>
#include <map>
#include <errno.h>
#include "WebServConf.hpp"

class HttpServer {
protected:
    int server_fd;
    int port;
	WebServConf _webconf;
    bool running;
    static const int BUFFER_SIZE = 4096;
    static const int MAX_EVENTS = 100;

    std::vector<struct pollfd> poll_fds;
    std::map<int, std::string> partial_requests;
    std::map<int, std::string> partial_responses;

    static void make_non_blocking(int fd);
    static bool file_exists(const std::string& path);
    static bool ends_with(const std::string& str, const std::string& suffix);
    std::string process_request(const std::string& request);
	void handlePostRequest(const std::string &fullPath, const std::string &body);
	std::string handleDeleteRequest(const std::string &fullPath);
    void handle_client_read(int client_fd);
    void handle_client_write(int client_fd);
    void close_client(int client_fd);

public:
    HttpServer(int port_num, const WebServConf &webconf);
    void start();
    void stop();
    ~HttpServer();
};

#endif
