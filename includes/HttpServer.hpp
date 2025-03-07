#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // Add this for inet_addr()
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
#include "Config.hpp"
#include "ConfigAccess.hpp"

class HttpServer {
private:
    static const size_t MAX_SERVERS = 100;
    static const size_t MAX_LOCATIONS = 100;
    static const size_t MAX_METHODS = 10;
    static const size_t MAX_CONFIGS = 1000;

    // Server socket and configuration
    std::map<int, int> server_fds;
    std::vector<ServerConfig> server_configs;
    std::string root_dir;
    bool running;

    // Buffer and event constants
    static const size_t MAX_REQUEST_SIZE = 10 * 1024 * 1024; // 10MB
    static const int BUFFER_SIZE = 4096;
    static const int MAX_EVENTS = 100;

    // Connection management
    std::vector<struct pollfd> poll_fds;
    std::map<int, std::string> partial_requests;
    std::map<int, std::string> partial_responses;

    // Virtual host support
    std::map<std::string, Location> locations;
    std::vector<ServerConfig> virtualHosts;

    // Helper methods
    static void make_non_blocking(int fd);
    static bool file_exists(const std::string& path);
    static bool ends_with(const std::string& str, const std::string& suffix);
    bool validatePath(const std::string& path) const;
	bool directoryExists(const std::string& path) ;
    // Configuration processing
    void initializeServerBlock(ConfigBlock* serverBlock, ServerConfig& config);
    void processRootDirectory(ConfigBlock* serverBlock, ServerConfig& config);
    void processLocationBlocks(ConfigBlock* serverBlock, ServerConfig& config);

    // Request handling
    std::string process_request(const std::string& request);
    void handle_client_read(int client_fd);
    void handle_client_write(int client_fd);
    void close_client(int client_fd);

    // Add new helper methods
    ServerConfig* findMatchingServer(const std::string& host, int port);
    Location* findMatchingLocation(const ServerConfig& server, const std::string& path);  // Changed from LocationConfig to Location
    std::string getLocationPath(const std::string& uri);

    // Request handling methods
    std::string handleGET(const std::string& path, Location* location);
    std::string handlePOST(const std::string& path, const std::string& body, Location* location);
    std::string handleDELETE(const std::string& path, Location* location);

    // Helper methods for file operations
    std::string saveUploadedFile(const std::string& path, const std::string& content);
    bool isMethodAllowed(const Location* location, const std::string& method);
    std::string getMimeType(const std::string& path);

    // Response generation helpers
    std::string createResponse(int statusCode, const std::string& contentType, const std::string& body);

    // Add these declarations
    std::string generateDirectoryListing(const std::string& path);
    std::string handleError(const ServerConfig& server, int error_code);
    std::string handleFastCGI(const std::string& path, Location* location);
    void processLocationBlock(ConfigBlock* locationBlock, Location& location);


	std::string normalize_path(const std::string& path);
    // Add utility functions
	
    static int stringToInt(const std::string& str) {
        std::istringstream ss(str);
        int result;
        ss >> result;
        return result;
    }

    // Add safety checks for memory operations
    void checkRequestSize(size_t current, size_t additional) {
        if (current + additional > MAX_REQUEST_SIZE) {
            throw std::runtime_error("Request too large");
        }
    }

    // Add method to safely clear resources
    void clearResources() {
        server_fds.clear();
        server_configs.clear();
        poll_fds.clear();
        partial_requests.clear();
        partial_responses.clear();
    }

public:
    // Update constructor
    explicit HttpServer(Config* conf);
    ~HttpServer();

    // Server control
    void start();
    void stop();

    // Disable copy constructor and assignment operator
    HttpServer(const HttpServer&);               // declared but not defined
    HttpServer& operator=(const HttpServer&);    // declared but not defined
};

#endif // HTTPSERVER_HPP
