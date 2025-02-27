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
#include <algorithm>
#include "WebServConf.hpp"

#define REQUEST200 "HTTP/1.1 200 OK\r\n\r\nFile deleted successfully"
#define REQUEST201 "HTTP/1.1 201 Created\r\n\r\nFile uploaded successfully"
#define REQUEST403 "HTTP/1.1 403 Forbidden\r\n\r\nForbidden"
#define REQUEST404 "HTTP/1.1 404 Not Found\r\n\r\nFile not found"
#define REQUEST405 "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed"
#define REQUEST500 "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete file"

class HttpServer {
protected:
    int server_fd;
    int port;
	WebServConf _webconf;
    bool running;
    static const int BUFFER_SIZE = 4096;
    static const int MAX_EVENTS = 100;

	std::map<int, int> server_fds;
    std::vector<struct pollfd> poll_fds;
    std::map<int, std::string> partial_requests;
    std::map<int, std::string> partial_responses;

    static void make_non_blocking(int fd);
    static bool file_exists(const std::string& path);
    static bool ends_with(const std::string& str, const std::string& suffix);
    std::string process_request(const std::string &request);

	// Request handler
	std::string handleGetRequest(const std::string &fullPath, const std::string &path);
	void handlePostRequest(const std::string &fullPath, const std::string &body);
	std::string handleDeleteRequest(const std::string &fullPath);

	void handle_client_read(int client_fd);
    void handle_client_write(int client_fd);
    void close_client(int client_fd);

public:
    HttpServer(const WebServConf &webconf);
    void start();
    void stop();
    ~HttpServer();
};

#endif
