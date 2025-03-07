#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Config::Config() {}
Config::~Config() {
    for (size_t i = 0; i < elements.size(); ++i) {
        delete elements[i];
    }
}

void Config::parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file");

    ServerConfig current_server;
    Location current_location;
    std::string line;
    bool in_server = false;
    bool in_location = false;
    
    while (std::getline(file, line)) {
        // Remove comments and trim whitespace
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
            line = line.substr(0, comment_pos);
            
        // Parse server and location blocks
        if (line.find("server {") != std::string::npos) {
            in_server = true;
            current_server = ServerConfig();
        }
        else if (line.find("location") != std::string::npos && in_server) {
            in_location = true;
            current_location = Location();
            // Parse location path
            size_t path_start = line.find_first_of(" \t") + 1;
            size_t path_end = line.find("{") - 1;
            current_location.path = line.substr(path_start, path_end - path_start);
        }
        else if (line.find("}") != std::string::npos) {
            if (in_location) {
                in_location = false;
                current_server.locations[current_location.path] = current_location;
            }
            else if (in_server) {
                in_server = false;
                servers.push_back(current_server);
            }
        }
        else {
            // Parse directives
            // Implementation of directive parsing goes here
        }
    }
}

const std::vector<ServerConfig>& Config::getServers() const {
    return servers;
}

ServerConfig* Config::findServer(const std::string& host, int port) {
    for (size_t i = 0; i < servers.size(); ++i) {
        if (servers[i].port == port) {
            for (size_t j = 0; j < servers[i].server_names.size(); ++j) {
                if (servers[i].server_names[j] == host)
                    return &servers[i];
            }
        }
    }
    return NULL;
}
