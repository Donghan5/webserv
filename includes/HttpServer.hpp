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

class HttpServer {
protected:
    int server_fd;
    int port;
    std::string root_dir;
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
    void handle_client_read(int client_fd);
    void handle_client_write(int client_fd);
    void close_client(int client_fd);

public:
    HttpServer(int port_num = 8080, const std::string& root = "./www");
    void start();
    void stop();
    ~HttpServer();
};

#endif
