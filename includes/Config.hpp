#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "ConfigAccess.hpp"
#include "ConfigBlock.hpp"
#include "ConfigDirective.hpp"
#include "ConfigElement.hpp"

class Config {
private:
    std::vector<ConfigElement*> elements;
    std::vector<ServerConfig> servers;

public:
    Config();
    ~Config();

    void parseFile(const std::string& filename);
    const std::vector<ServerConfig>& getServers() const;
    ServerConfig* findServer(const std::string& host, int port);

    void addElement(ConfigElement* element) { elements.push_back(element); }
    const std::vector<ConfigElement*>& getElements() const { return elements; }
};

#endif
