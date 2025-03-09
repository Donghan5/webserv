#include "../includes/HttpServer.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Socket.hpp"
#include "../includes/Utils.hpp"
#include "Config.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <sstream>
#include <algorithm>

// Go to Utils class.....?????
// Helper function to check if file exists
bool HttpServer::file_exists(const std::string &path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

// Helper function to check string ending
bool HttpServer::ends_with(const std::string &str, const std::string &suffix)
{
	if (str.length() < suffix.length())
	{
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool HttpServer::validatePath(const std::string& path) const {
    // Allow absolute paths
    if (path.empty()) {
        return false;
    }

    // Check for path traversal attempts
    if (path.find("..") != std::string::npos) {
        return false;
    }

    return true;
}

void HttpServer::handle_client_read(int client_fd) {
    try {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);

        if (bytes_read <= 0) {
            if (bytes_read == 0 || errno != EAGAIN) {
                close_client(client_fd);
            }
            return;
        }

        // Limit the total request size
        if (partial_requests[client_fd].length() + bytes_read > MAX_REQUEST_SIZE) {
            partial_requests.erase(client_fd);
            close_client(client_fd);
            return;
        }

        partial_requests[client_fd].append(buffer, bytes_read);

        // Check if we have a complete HTTP request
        size_t header_end = partial_requests[client_fd].find("\r\n\r\n");
        if (header_end != std::string::npos) {
            std::string response = process_request(partial_requests[client_fd]);
            partial_responses[client_fd] = response;
            partial_requests.erase(client_fd);

            // Update poll flags
            for (size_t i = 0; i < poll_fds.size(); ++i) {
                if (poll_fds[i].fd == client_fd) {
                    poll_fds[i].events = POLLOUT;
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in handle_client_read: " << e.what() << std::endl;
        close_client(client_fd);
    }
}

void HttpServer::handle_client_write(int client_fd)
{
	std::string &response = partial_responses[client_fd];
	ssize_t bytes_written = write(client_fd, response.c_str(), response.length());

	if (bytes_written <= 0)
	{
		if (errno != EAGAIN)
		{
			close_client(client_fd);
		}
		return;
	}

	response.erase(0, bytes_written);

	if (response.empty())
	{
		close_client(client_fd);
	}
}

void HttpServer::close_client(int client_fd)
{
	close(client_fd);
	partial_requests.erase(client_fd);
	partial_responses.erase(client_fd);

	// Remove from poll_fds
	for (size_t i = 0; i < poll_fds.size(); ++i)
	{
		if (poll_fds[i].fd == client_fd)
		{
			poll_fds.erase(poll_fds.begin() + i);
			break;
		}
	}
}

// Add HttpServer(Config*) constructor implementation
HttpServer::HttpServer(Config* conf) : running(false) {
    try {
        if (!conf) {
            throw std::runtime_error("Invalid configuration: null pointer");
        }

        // Get server blocks with safety check
        std::vector<ConfigBlock*> serverBlocks = ConfigAccess::getAllServerBlocks(conf);
        if (serverBlocks.empty()) {
            throw std::runtime_error("No server blocks found in configuration");
        }
        if (serverBlocks.size() > MAX_SERVERS) {
            throw std::runtime_error("Too many server blocks in configuration");
        }

        // Pre-allocate with safety checks
        try {
            server_configs.clear();
            server_configs.reserve(serverBlocks.size());
            std::vector<int> unique_ports;
            unique_ports.reserve(serverBlocks.size());  // Pre-allocate unique_ports

            // Initialize each server configuration
            for (size_t i = 0; i < serverBlocks.size(); ++i) {
                try {
                    ServerConfig config;
                    initializeServerBlock(serverBlocks[i], config);

                    // Check if port is already in use
                    if (std::find(unique_ports.begin(), unique_ports.end(), config.port) == std::find(unique_ports.begin(), unique_ports.end(), config.port)) {
                        if (unique_ports.size() >= MAX_SERVERS) {
                            throw std::runtime_error("Too many unique ports");
                        }
                        unique_ports.push_back(config.port);
                    }

                    server_configs.push_back(config);
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Failed to initialize server block " << i << ": " << e.what() << std::endl;
                    continue;
                }
            }

            if (server_configs.empty()) {
                throw std::runtime_error("No valid server configurations found");
            }

            // Create and bind sockets
            for (size_t i = 0; i < unique_ports.size(); ++i) {
                try {
					int server_fd = Socket::createSocket();

					Socket::setSocket(server_fd);
                    // int server_fd = socket(AF_INET, SOCK_STREAM, 0);
                    // if (server_fd < 0) {
                    //     throw std::runtime_error("Failed to create socket");
                    // }

                    // int opt = 1;
                    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                    //     close(server_fd);
                    //     throw std::runtime_error("Failed to set socket options");
                    // }

                    Socket::makeNonBlocking(server_fd);

                    struct sockaddr_in address;
                    memset(&address, 0, sizeof(address));
                    address.sin_family = AF_INET;
                    address.sin_addr.s_addr = INADDR_ANY;
                    address.sin_port = htons(unique_ports[i]);

					Socket::bindSocket(server_fd, unique_ports[i]);
                    // if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                    //     close(server_fd);
                    //     throw std::runtime_error("Failed to bind to port " + intToString(unique_ports[i]));
                    // }

					Socket::listenSocket(server_fd);
                    // if (listen(server_fd, SOMAXCONN) < 0) {
                    //     close(server_fd);
                    //     throw std::runtime_error("Failed to listen on port " + intToString(unique_ports[i]));
                    // }

                    server_fds[unique_ports[i]] = server_fd;
                    std::cout << "Server listening on port " << unique_ports[i] << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Failed to initialize port " << unique_ports[i] << ": " << e.what() << std::endl;
                    continue;
                }
            }

            if (server_fds.empty()) {
                throw std::runtime_error("No server sockets could be initialized");
            }

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to allocate memory for server configs: " + std::string(e.what()));
        }

    } catch (const std::exception& e) {
        clearResources();
        throw std::runtime_error("Failed to initialize server: " + std::string(e.what()));
    }
}

/*
	Find server block in the vector
*/
ServerConfig* HttpServer::findMatchingServer(const std::string& host, int port) {
    ServerConfig* default_server = NULL;

    for (size_t i = 0; i < server_configs.size(); ++i) {
        if (server_configs[i].port == port) {
            // Store first server as default
            if (!default_server) {
                default_server = &server_configs[i];
            }

            // Check server names
            for (size_t j = 0; j < server_configs[i].server_names.size(); ++j) {
                if (server_configs[i].server_names[j] == host) {
                    return &server_configs[i];
                }
            }
        }
    }

    return default_server;
}

/*
	Finding the location in the vector
*/
Location* HttpServer::findMatchingLocation(const ServerConfig& server, const std::string& path) {
    std::cerr << "DEBUG: Finding location for path: " << path << std::endl;

    // First check for exact match
    std::map<std::string, Location>::const_iterator exact_it = server.locations.find(path);
    if (exact_it != server.locations.end()) {
        std::cerr << "DEBUG: Found exact location match" << std::endl;
        return const_cast<Location*>(&exact_it->second);
    }

    // Then check for prefix match
    std::string best_match = "";
    Location* best_location = NULL;

    for (std::map<std::string, Location>::const_iterator it = server.locations.begin();
         it != server.locations.end(); ++it) {
        // Check if path starts with location path
        if (path.compare(0, it->first.length(), it->first) == 0) {
            if (it->first.length() > best_match.length()) {
                best_match = it->first;
                best_location = const_cast<Location*>(&it->second);
                std::cerr << "DEBUG: Found better location match: " << it->first << std::endl;
            }
        }
    }

    if (best_location) {
        return best_location;
    }

    // If no match found, use root location
    std::map<std::string, Location>::const_iterator root_it = server.locations.find("/");
    if (root_it != server.locations.end()) {
        std::cerr << "DEBUG: Using root location" << std::endl;
        return const_cast<Location*>(&root_it->second);
    }

    std::cerr << "DEBUG: No location found" << std::endl;
    return NULL;
}

void HttpServer::start()
{
    running = true;

    // Initialize poll_fds with all server sockets
    for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
        if (listen(it->second, SOMAXCONN) < 0) {
            throw std::runtime_error("Failed to listen on port " + Utils::intToString(it->first));
        }

        struct pollfd server_pollfd;
        server_pollfd.fd = it->second;
        server_pollfd.events = POLLIN;
        server_pollfd.revents = 0;
        poll_fds.push_back(server_pollfd);
		Logger::log(Logger::INFO, "Server listening on port " + Utils::intToString(it->first));
    }

    while (running)
    {
        int ready = poll(&poll_fds[0], poll_fds.size(), -1);
        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("Poll failed");
        }

        // Make a copy because the vector might be modified during iteration
        std::vector<struct pollfd> current_fds = poll_fds;

        for (size_t i = 0; i < current_fds.size(); ++i)
        {
            if (current_fds[i].revents == 0)
            {
                continue;
            }

            bool is_server_socket = false;
            for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
                if (current_fds[i].fd == it->second) {
                    is_server_socket = true;
                    break;
                }
            }

            if (is_server_socket)
            {
                // Handle new connection
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(current_fds[i].fd, (struct sockaddr *)&client_addr, &client_len);

                if (client_fd < 0)
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                    {
                        std::cerr << "Failed to accept connection" << std::endl;
                    }
                    continue;
                }

                Socket::makeNonBlocking(client_fd);

                // Add to poll set
                struct pollfd client_pollfd;
                client_pollfd.fd = client_fd;
                client_pollfd.events = POLLIN;
                client_pollfd.revents = 0;
                poll_fds.push_back(client_pollfd);
            }
            else
            {
                // Handle client socket
                if (current_fds[i].revents & POLLIN)
                {
                    std::cerr << "handle_client_read\n";
                    handle_client_read(current_fds[i].fd);
                }
                if (current_fds[i].revents & POLLOUT)
                {
                    std::cerr << "handle_client_write\n";
                    handle_client_write(current_fds[i].fd);
                }
                if (current_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                {
                    std::cerr << "else\n";
                    close_client(current_fds[i].fd);
                }
            }
        }
    }
}

void HttpServer::stop()
{
	running = false;
	for (std::map<int, int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
        close(it->second);
    }

	// Close all client connections
	for (size_t i = 0; i < poll_fds.size(); ++i)
	{
		if (server_fds.find(poll_fds[i].fd) == server_fds.end())
		{
			close(poll_fds[i].fd);
		}
	}
	poll_fds.clear();
	partial_requests.clear();
	partial_responses.clear();
}

HttpServer::~HttpServer() {
	try {
		if (running) {
			stop();
		}
		clearResources();
	} catch (const std::exception &e) {
		(void)e;
	}
}

void HttpServer::initializeServerBlock(ConfigBlock* serverBlock, ServerConfig& config) {
    if (!serverBlock) {
        throw std::runtime_error("Invalid server block");
    }

    // Get listen directive
    std::string port_str;
    if (ConfigAccess::getDirectiveValue(serverBlock, "listen", port_str)) {
        try {
            config.port = stringToInt(port_str);
            if (config.port <= 0 || config.port > 65535) {
                throw std::runtime_error("Port number out of range (1-65535): " + port_str);
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid port number: " + port_str);
        }
    }

    // Get server name
    std::string server_name;
    if (ConfigAccess::getDirectiveValue(serverBlock, "server_name", server_name)) {
        config.server_names.push_back(server_name);
    }

    // Handle root directory
    processRootDirectory(serverBlock, config);

    // Process all location blocks
    processLocationBlocks(serverBlock, config);

    // Check for SSL configuration
	// Remove ssl part
    std::string ssl_cert, ssl_key;
    if (ConfigAccess::getDirectiveValue(serverBlock, "ssl_certificate", ssl_cert)) {
        config.ssl_enabled = true;
        config.ssl_certificate = ssl_cert;
    }
    if (ConfigAccess::getDirectiveValue(serverBlock, "ssl_certificate_key", ssl_key)) {
        config.ssl_certificate_key = ssl_key;
    }

    // Parse error pages
    std::vector<ConfigDirective*> error_pages = ConfigAccess::getDirectives(serverBlock, "error_page");
    for (size_t i = 0; i < error_pages.size(); ++i) {
        const std::vector<std::string>& params = error_pages[i]->getParameters();
        if (params.size() >= 2) {
            int error_code = std::atoi(params[0].c_str());
            config.error_pages[error_code] = params[1];
        }
    }
}

void HttpServer::processRootDirectory(ConfigBlock* serverBlock, ServerConfig& config) {
    // Set default root if not specified
    if (!ConfigAccess::getDirectiveValue(serverBlock, "root", config.root)) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            config.root = std::string(cwd) + "/www3";
        } else {
            config.root = "www3";
        }
    }

    // Normalize path
    if (config.root[0] != '/') {  // If relative path
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            if (config.root.substr(0, 2) == "./") {
                config.root = config.root.substr(2);
            }
            config.root = std::string(cwd) + "/" + config.root;
        }
    }

    config.root = normalize_path(config.root);
    std::cerr << "DEBUG: Using normalized root directory: " << config.root << std::endl;

    // Create directory if it doesn't exist
    if (!directoryExists(config.root)) {
        std::string cmd = "mkdir -p '" + config.root + "'";
        system(cmd.c_str());
        chmod(config.root.c_str(), 0755);
    }

    // Create default index.html only if it doesn't exist
    std::string index_path = config.root + "/index.html";
    if (!file_exists(index_path)) {
        std::cerr << "DEBUG: Creating default index file at: " << index_path << std::endl;
        std::ofstream index_file(index_path.c_str());
        if (index_file) {
            index_file << "<html><body><h1>Welcome to Webserv</h1></body></html>";
            index_file.close();
            chmod(index_path.c_str(), 0644);
        }
    } else {
        std::cerr << "DEBUG: Using existing index file at: " << index_path << std::endl;
    }

    // Verify permissions
    if (access(config.root.c_str(), R_OK | X_OK) != 0) {
        throw std::runtime_error("Cannot access root directory: " + config.root);
    }
    if (access(index_path.c_str(), R_OK) != 0) {
        throw std::runtime_error("Cannot access index file: " + index_path);
    }
}

// Add helper function for path normalization
std::string HttpServer::normalize_path(const std::string& path) {
    std::string normalized = path;
    size_t pos;

    // Remove "./"
    while ((pos = normalized.find("./")) != std::string::npos) {
        normalized.erase(pos, 2);
    }

    // Remove double slashes
    while ((pos = normalized.find("//")) != std::string::npos) {
        normalized.erase(pos, 1);
    }

    // Remove trailing slash if not root
    if (normalized.length() > 1 && normalized[normalized.length() - 1] == '/') {
        normalized.erase(normalized.length() - 1);
    }

    return normalized;
}

bool HttpServer::directoryExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

void HttpServer::processLocationBlocks(ConfigBlock* serverBlock, ServerConfig& config) {
    if (!serverBlock) {
        throw std::runtime_error("Invalid server block");
    }

    config.locations.clear();

    try {
        // Create default location with proper initialization
        Location defaultLocation;
        defaultLocation.path = "/";
        defaultLocation.root = config.root;

        // Get index from server config
        std::string index_value;
        if (ConfigAccess::getDirectiveValue(serverBlock, "index", index_value)) {
            defaultLocation.index.push_back(index_value);
        } else {
            defaultLocation.index.push_back("index.html");
        }

        defaultLocation.client_max_body_size = config.client_max_body_size;

        // Set default allowed methods
        defaultLocation.allowed_methods.clear();
        defaultLocation.allowed_methods.push_back("GET");  // Only GET for default location

        config.locations["/"] = defaultLocation;

        // Process location blocks
        const std::vector<ConfigElement*>& children = serverBlock->getChildren();
        for (size_t i = 0; i < children.size() && config.locations.size() < MAX_LOCATIONS; ++i) {
            ConfigBlock* locationBlock = dynamic_cast<ConfigBlock*>(children[i]);
            if (!locationBlock || locationBlock->getName() != "location") {
                continue;
            }

            Location location;  // Start with fresh location instead of inheriting
            location.path = locationBlock->getParameters()[0];
            location.root = config.root;
            location.client_max_body_size = config.client_max_body_size;
            location.index = defaultLocation.index;  // Inherit only index files

            // Process location-specific directives
            processLocationBlock(locationBlock, location);

            // If no methods were specified in the location block, set default methods
            if (location.allowed_methods.empty()) {
                location.allowed_methods.push_back("GET");
                location.allowed_methods.push_back("POST");
                location.allowed_methods.push_back("DELETE");
                std::cerr << "DEBUG: Using default methods (GET, POST, DELETE) for location " << location.path << std::endl;
            }

            config.locations[location.path] = location;
            std::cerr << "DEBUG: Added location block for path: " << location.path << std::endl;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create location: " + std::string(e.what()));
    }
}

void HttpServer::processLocationBlock(ConfigBlock* locationBlock, Location& location) {
    // Process allowed methods first
    location.allowed_methods.clear();

    // First set the root by combining server root and location path
    if (location.path != "/") {
        location.root = normalize_path(location.root + "/" + location.path.substr(1));
        std::cerr << "DEBUG: Set combined root path for location " << location.path << ": " << location.root << std::endl;
    }

    // Create directory if it doesn't exist
    if (!directoryExists(location.root)) {
        std::string cmd = "mkdir -p '" + location.root + "'";
        system(cmd.c_str());
        chmod(location.root.c_str(), 0755);
        std::cerr << "DEBUG: Created directory: " << location.root << std::endl;
    }

    // Process other directives
    const std::vector<ConfigElement*>& children = locationBlock->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        const ConfigDirective* directive = dynamic_cast<const ConfigDirective*>(children[i]);
        if (!directive) continue;

        if (directive->getName() == "allowed_methods") {
            // Process methods
            const std::vector<std::string>& methods = directive->getParameters();
            for (size_t j = 0; j < methods.size(); ++j) {
                std::string upperMethod = methods[j];
                for (size_t k = 0; k < upperMethod.length(); k++) {
                    upperMethod[k] = std::toupper(upperMethod[k]);
                }
                location.allowed_methods.push_back(upperMethod);
                std::cerr << "DEBUG: Added allowed method: " << upperMethod << " for path: " << location.path << std::endl;
            }
        }
        else if (directive->getName() == "root") {  // root directive
            // If explicit root is specified, it overrides the combined path
            std::string path_value = directive->getParameters()[0];
            if (path_value[0] != '/') {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    if (path_value.substr(0, 2) == "./") {
                        path_value = path_value.substr(2);
                    }
                    path_value = std::string(cwd) + "/" + path_value;
                }
            }
            location.root = normalize_path(path_value);
            std::cerr << "DEBUG: Overrode root path for location " << location.path << ": " << location.root << std::endl;
        }
    }

    // If no methods were found, set defaults
    if (location.allowed_methods.empty()) {
        if (location.path == "/") {
            location.allowed_methods.push_back("GET");
            std::cerr << "DEBUG: Using default GET for root location" << std::endl;
        } else {
            location.allowed_methods.push_back("GET");
            location.allowed_methods.push_back("POST");
            location.allowed_methods.push_back("DELETE");
            std::cerr << "DEBUG: Using default methods (GET, POST, DELETE) for location " << location.path << std::endl;
        }
    }

    // Process root and alias directives
    std::string path_value;
    if (ConfigAccess::getDirectiveValue(locationBlock, "root", path_value)) {
        if (path_value[0] != '/') {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                if (path_value.substr(0, 2) == "./") {
                    path_value = path_value.substr(2);
                }
                path_value = std::string(cwd) + "/" + path_value;
            }
        }
        location.root = normalize_path(path_value);
        std::cerr << "DEBUG: Set root for location " << location.path << ": " << location.root << std::endl;
    }

    // Process index directive
    std::string index;
    if (ConfigAccess::getDirectiveValue(locationBlock, "index", index)) {
        location.index.clear();  // Clear inherited index
        std::istringstream iss(index);
        std::string index_file;
        while (iss >> index_file) {
            location.index.push_back(index_file);
        }
        std::cerr << "DEBUG: Location-specific index set to: " << index << std::endl;
    }

    // Process deny all
    std::string deny;
    if (ConfigAccess::getDirectiveValue(locationBlock, "deny", deny) && deny == "all") {
        location.deny_all = true;
    }

    // Process FastCGI settings
    std::string fastcgi_pass;
    if (ConfigAccess::getDirectiveValue(locationBlock, "fastcgi_pass", fastcgi_pass)) {
        location.fastcgi_pass = fastcgi_pass;

        std::string fastcgi_index;
        if (ConfigAccess::getDirectiveValue(locationBlock, "fastcgi_index", fastcgi_index)) {
            location.fastcgi_index = fastcgi_index;
        }
    }

    // Process rewrite rules
    std::string rewrite;
    if (ConfigAccess::getDirectiveValue(locationBlock, "rewrite", rewrite)) {
        size_t space_pos = rewrite.find(' ');
        if (space_pos != std::string::npos) {
            location.rewrite_url = rewrite.substr(0, space_pos);
            std::string modifier = rewrite.substr(space_pos + 1);
            location.rewrite_permanent = (modifier == "permanent");
        }
    }

    // Process autoindex autoindex in boolean value?
    std::string autoindex;
    if (ConfigAccess::getDirectiveValue(locationBlock, "autoindex", autoindex)) {
        location.autoindex = (autoindex == "on");
    }
}

std::string HttpServer::handleGET(const std::string& path, Location* location) {
    if (!location) {
        std::cerr << "Error: No location found for path: " << path << std::endl;
        return createResponse(500, "text/plain", "Internal Server Error");
    }

    std::cerr << "DEBUG: Processing GET request for path: " << path << std::endl;
    std::cerr << "DEBUG: Location path: " << location->path << std::endl;
    std::cerr << "DEBUG: Location root: " << location->root << std::endl;
    std::cerr << "DEBUG: Location alias: " << location->alias << std::endl;

    // Get the normalized location root path
    std::string absolute_root;
    if (!location->alias.empty()) {
        absolute_root = location->alias;
    } else {
        absolute_root = location->root;
    }

    if (absolute_root.empty()) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            absolute_root = std::string(cwd);
        } else {
            return createResponse(500, "text/plain", "Internal Server Error");
        }
    }

    // Normalize the root path
    absolute_root = normalize_path(absolute_root);
    std::cerr << "DEBUG: Normalized root path: " << absolute_root << std::endl;

    // Build the full filesystem path differently based on whether alias is used
    std::string full_path;
    if (!location->alias.empty()) {
        // With alias, we replace the location path portion entirely
        std::string relative_path = path;
        if (path.find(location->path) == 0) {
            relative_path = path.substr(location->path.length());
        }
        if (relative_path.empty() || relative_path[0] != '/') {
            relative_path = "/" + relative_path;
        }
        full_path = absolute_root + relative_path;
    } else {
        // Without alias, use the normal path concatenation
        std::string relative_path = path.substr(location->path.length());
        if (relative_path.empty() || relative_path[0] != '/') {
            relative_path = "/" + relative_path;
        }
        full_path = absolute_root + relative_path;
    }

    full_path = normalize_path(full_path);
	Logger::log(Logger::DEBUG, "Final full path: " + full_path);

    // Check if the path is outside the root directory (path traversal protection)
    if (full_path.find(absolute_root) != 0) {
		Logger::log(Logger::DEBUG, "Path traversal attempt detected: " + full_path);
        return createResponse(403, "text/plain", "Forbidden");
    }

    // Check permissions and serve the file
    if (access(absolute_root.c_str(), R_OK | X_OK) != 0) {
		Logger::log(Logger::ERROR, "Cannot access root directory: " + absolute_root);
        return createResponse(403, "text/plain", "Forbidden");
    }

    struct stat path_stat;
    if (stat(full_path.c_str(), &path_stat) != 0) {
		Logger::log(Logger::ERROR, "File not found: " + full_path);
        return createResponse(404, "text/plain", "Not Found");
    }

    // Handle directory requests
    if (S_ISDIR(path_stat.st_mode)) {
        // If requesting a directory without trailing slash, redirect
        if (path[path.length() - 1] != '/') {
            return createResponse(301, "text/plain", "Moved Permanently\nLocation: " + path + "/\r\n");
        }

        // Try index files
        for (size_t i = 0; i < location->index.size(); ++i) {
            std::string index_path = full_path + "/" + location->index[i];
            std::cerr << "DEBUG: Trying index file: " << index_path << std::endl;

            if (access(index_path.c_str(), R_OK) == 0) {
                std::ifstream file(index_path.c_str(), std::ios::binary);
                if (file) {
                    std::stringstream content;
                    content << file.rdbuf();
                    std::string mime_type = getMimeType(index_path);
                    std::cerr << "DEBUG: Serving file: " << index_path << " with type: " << mime_type << std::endl;
                    return createResponse(200, mime_type, content.str());
                }
            }
        }

        // If no index file found, show directory listing if enabled
        if (location->autoindex) {
            return generateDirectoryListing(full_path);
        }

        return createResponse(403, "text/plain", "Forbidden");
    }
    // Handle regular files
    else {
        if (access(full_path.c_str(), R_OK) == 0) {
            std::ifstream file(full_path.c_str(), std::ios::binary);
            if (file) {
                std::stringstream content;
                content << file.rdbuf();
                return createResponse(200, getMimeType(full_path), content.str());
            }
        }
    }

	Logger::log(Logger::DEBUG, "Access forbidden for path: " + full_path);
    return createResponse(403, "text/plain", "Forbidden");
}

std::string HttpServer::generateDirectoryListing(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return createResponse(500, "text/plain", "Failed to read directory");
    }

    std::stringstream html;
    html << "<html><head><title>Index of " << path << "</title></head><body>\n";
    html << "<h1>Index of " << path << "</h1><hr><pre>\n";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        struct stat st;
        std::string fullpath = path + "/" + name;

        if (stat(fullpath.c_str(), &st) == 0) {
            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));

            html << "<a href=\"" << name << (S_ISDIR(st.st_mode) ? "/" : "") << "\">"
                 << name << (S_ISDIR(st.st_mode) ? "/" : "") << "</a>"
                 << std::string(50 - name.length(), ' ')
                 << timeStr
                 << std::string(20, ' ')
                 << st.st_size << "\n";
        }
    }

    html << "</pre><hr></body></html>";
    closedir(dir);
    return createResponse(200, "text/html", html.str());
}

// Add this to handle error pages
std::string HttpServer::handleError(const ServerConfig& server, int error_code) {
    std::map<int, std::string>::const_iterator it = server.error_pages.find(error_code);
    if (it != server.error_pages.end()) {
        Location* error_location = findMatchingLocation(server, it->second);
        if (error_location) {
            return handleGET(it->second, error_location);
        }
    }
    return createResponse(error_code, "text/plain", "Error " + Utils::intToString(error_code));
}

std::string HttpServer::handlePOST(const std::string& path, const std::string& body, Location* location) {
    Logger::log(Logger::INFO, "Starting POST handler");

    // Explicit method check
    if (!isMethodAllowed(location, "POST")) {
        std::cerr << "DEBUG: POST method not allowed for path: " << path << std::endl;
        return createResponse(405, "text/plain", "Method Not Allowed");
    }

    if (body.empty()) {
        std::cerr << "DEBUG: Empty request body" << std::endl;
        return createResponse(400, "text/plain", "Empty request body");
    }

    // Create uploads directory if it doesn't exist
    std::string upload_dir = location->root;
	Logger::log(Logger::DEBUG, "Using upload directory: " + upload_dir);

    if (!directoryExists(upload_dir)) {
        std::string cmd = "mkdir -p '" + upload_dir + "'";
        system(cmd.c_str());
        chmod(upload_dir.c_str(), 0755);
		Logger::log(Logger::INFO, "Created upload directory");
    }

    // Extract filename from path
    std::string filename = path;
    size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) {
        filename = path.substr(last_slash + 1);
    }

    std::string full_path = upload_dir + "/" + filename;
    full_path = normalize_path(full_path);
    std::cerr << "DEBUG: Writing to file: " << full_path << std::endl;

    try {
        std::ofstream file(full_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
			Logger::log(Logger::ERROR, "Failed to open file for writing");
            std::cerr << "DEBUG: Failed to open file for writing" << std::endl;
            return createResponse(500, "text/plain", "Failed to create file");
        }

        file.write(body.c_str(), body.length());
        if (file.fail()) {
            std::cerr << "DEBUG: Failed to write to file" << std::endl;
            file.close();
            return createResponse(500, "text/plain", "Failed to write file");
        }

        file.close();
        chmod(full_path.c_str(), 0644);
        std::cerr << "DEBUG: File saved successfully" << std::endl;

        return createResponse(201, "text/plain", "File uploaded successfully");
    } catch (const std::exception& e) {
        std::cerr << "DEBUG: Exception during file operations: " << e.what() << std::endl;
        return createResponse(500, "text/plain", "Internal server error");
    }
}

std::string HttpServer::handleDELETE(const std::string& path, Location* location) {
    std::cerr << "DEBUG: Starting DELETE handler for path: " << path << std::endl;

    if (!isMethodAllowed(location, "DELETE")) {
        std::cerr << "DEBUG: DELETE method not allowed" << std::endl;
        return createResponse(405, "text/plain", "Method Not Allowed");
    }

    // Build the full path correctly
    std::string full_path = location->root;
    if (!full_path.empty() && full_path[full_path.length() - 1] != '/') {
        full_path += "/";
    }

    // Remove the location path prefix from the request path
    std::string relative_path = path;
    if (path.find(location->path) == 0) {
        relative_path = path.substr(location->path.length());
    }
    if (!relative_path.empty() && relative_path[0] == '/') {
        relative_path = relative_path.substr(1);
    }

    full_path += relative_path;
    full_path = normalize_path(full_path);
    std::cerr << "DEBUG: Attempting to delete file: " << full_path << std::endl;

    if (!file_exists(full_path)) {
        std::cerr << "DEBUG: File not found: " << full_path << std::endl;
        return createResponse(404, "text/plain", "Not Found");
    }

    // Additional security check
    struct stat path_stat;
    if (stat(full_path.c_str(), &path_stat) == 0) {
        if (S_ISDIR(path_stat.st_mode)) {
            std::cerr << "DEBUG: Cannot delete directory: " << full_path << std::endl;
            return createResponse(403, "text/plain", "Cannot delete directory");
        }
    }

    if (remove(full_path.c_str()) == 0) {
        std::cerr << "DEBUG: File deleted successfully: " << full_path << std::endl;
        return createResponse(200, "text/plain", "File deleted successfully\n");
    } else {
        std::cerr << "DEBUG: Failed to delete file: " << full_path << " (errno: " << errno << ")" << std::endl;
        return createResponse(500, "text/plain", "Failed to delete file");
    }
}

std::string HttpServer::process_request(const std::string& request) {
    // Parse method and path
    size_t method_end = request.find(' '); // method is in front of all
    if (method_end == std::string::npos) {
        return createResponse(400, "text/plain", "Bad Request");
    }

    std::string method = request.substr(0, method_end);
    size_t path_start = method_end + 1;
    size_t path_end = request.find(' ', path_start);
    std::string path = request.substr(path_start, path_end - path_start);

    // Parse headers
    size_t headers_start = request.find("\r\n") + 2;
    size_t headers_end = request.find("\r\n\r\n");
    std::string headers = request.substr(headers_start, headers_end - headers_start);

    // Parse host header
    std::string host = "localhost";
    int port = 8080;
    size_t host_start = request.find("\r\nHost: ");
    if (host_start != std::string::npos) {
        host_start += 8;
        size_t host_end = request.find("\r\n", host_start);
        if (host_end != std::string::npos) {
            std::string host_port = request.substr(host_start, host_end - host_start);
            size_t colon_pos = host_port.find(':');
            if (colon_pos != std::string::npos) {
                host = host_port.substr(0, colon_pos);
                port = atoi(host_port.substr(colon_pos + 1).c_str());
            } else {
                host = host_port;
            }
        }
    }

    try {
        ServerConfig* server = findMatchingServer(host, port);
        if (!server) {
            return handleError(*server_configs.begin(), 404);
        }

        Location* location = findMatchingLocation(*server, path);
        if (!location) {
            return handleError(*server, 404);
        }

        if (location->deny_all) {
            return handleError(*server, 403);
        }

        if (!isMethodAllowed(location, method)) {
            return handleError(*server, 405);
        }

        if (method == "GET") {
            return handleGET(path, location);
        }
        else if (method == "POST") {
            // Find the actual body after headers
            std::string body;
            if (headers_end != std::string::npos) {
                body = request.substr(headers_end + 4);
                std::cerr << "DEBUG: POST body length: " << body.length() << std::endl;
            }

            if (body.empty()) {  // Empty body
                std::cerr << "DEBUG: Empty POST body" << std::endl;
                return createResponse(400, "text/plain", "Empty request body");
            }

            if (body.length() > location->client_max_body_size) {
                std::cerr << "DEBUG: Body too large: " << body.length() << " > " << location->client_max_body_size << std::endl;
                return handleError(*server, 413);
            }

            return handlePOST(path, body, location);
        }
        else if (method == "DELETE") {
            return handleDELETE(path, location);
        }
        return handleError(*server, 405);
    } catch (const std::exception& e) {
        std::cerr << "Error processing request: " << e.what() << std::endl;
        if (!server_configs.empty()) {
            return handleError(server_configs[0], 500);
        }
        return createResponse(500, "text/plain", "Internal Server Error");
    }
}

std::string HttpServer::saveUploadedFile(const std::string& path, const std::string& content) {
    // Create directories if they don't exist
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        system(("mkdir -p '" + dir + "'").c_str()); // create directory
    }

    // Save the file
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not create file");
    }

    file.write(content.c_str(), content.length());
    if (!file) {
        throw std::runtime_error("Failed to write to file");
    }

    return path;
}

std::string HttpServer::getMimeType(const std::string& path) {
    // Update MIME type detection with more common types
    if (ends_with(path, ".html") || ends_with(path, ".htm")) return "text/html; charset=utf-8";
    if (ends_with(path, ".css")) return "text/css; charset=utf-8";
    if (ends_with(path, ".js")) return "text/javascript; charset=utf-8";
    if (ends_with(path, ".json")) return "application/json; charset=utf-8";
    if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) return "image/jpeg";
    if (ends_with(path, ".png")) return "image/png";
    if (ends_with(path, ".gif")) return "image/gif";
    if (ends_with(path, ".svg")) return "image/svg+xml";
    if (ends_with(path, ".ico")) return "image/x-icon";
    if (ends_with(path, ".txt")) return "text/plain; charset=utf-8";
    if (ends_with(path, ".pdf")) return "application/pdf";
    return "text/plain; charset=utf-8";  // Default to text/plain instead of application/octet-stream
}

bool HttpServer::isMethodAllowed(const Location* location, const std::string& method) {
    if (!location) { // invalid format?
        std::cerr << "DEBUG: No location for method check" << std::endl;
        return false;
    }

    std::string upperMethod = method;
    for (size_t i = 0; i < upperMethod.length(); i++) {
        upperMethod[i] = std::toupper(upperMethod[i]);
    }

    std::cerr << "DEBUG: Method check - Path: " << location->path << ", Method: " << upperMethod << std::endl;
    std::cerr << "DEBUG: Available methods: ";
    for (std::vector<std::string>::const_iterator it = location->allowed_methods.begin();
         it != location->allowed_methods.end(); ++it) {
        std::cerr << *it << " ";
    }
    std::cerr << std::endl;

    for (std::vector<std::string>::const_iterator it = location->allowed_methods.begin();
         it != location->allowed_methods.end(); ++it) {
        if (*it == upperMethod) {
            std::cerr << "DEBUG: Method " << upperMethod << " is allowed for path " << location->path << std::endl;
            return true;
        }
    }

    std::cerr << "DEBUG: Method " << upperMethod << " is NOT allowed for path " << location->path << std::endl;
    return false;
}

std::string HttpServer::createResponse(int statusCode, const std::string& contentType, const std::string& body) {
    std::string status;
    switch (statusCode) {
        case 200: status = "200 OK"; break;
        case 201: status = "201 Created"; break;
        case 301: status = "301 Moved Permanently"; break;
        case 302: status = "302 Found"; break;
        case 400: status = "400 Bad Request"; break;
        case 403: status = "403 Forbidden"; break;
        case 404: status = "404 Not Found"; break;
        case 405: status = "405 Method Not Allowed"; break;
        case 413: status = "413 Payload Too Large"; break;
        case 500: status = "500 Internal Server Error"; break;
        default: status = "500 Internal Server Error";
    }

    std::stringstream response;
    response << "HTTP/1.1 " << status << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

    return response.str();
}

// Fix handleFastCGI implementation
std::string HttpServer::handleFastCGI(const std::string& path, Location* location) {
    (void)path; // Suppress unused parameter warning
    if (!location->fastcgi_pass.empty()) {
        return createResponse(501, "text/plain", "FastCGI support not implemented");
    }
    return createResponse(500, "text/plain", "Internal Server Error");
}
