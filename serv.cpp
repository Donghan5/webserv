#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>

struct ClientArgs {
    int socket;
    std::string root_dir;
};

class HttpServer {
private:
    int server_fd;
    int port;
    std::string root_dir;
    bool running;
    std::vector<pthread_t> worker_threads;
    
    static const int MAX_CONNECTIONS = 10;
    static const int BUFFER_SIZE = 4096;

    // Helper function to check if file exists
    static bool file_exists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    // Helper function to check string ending
    static bool ends_with(const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length()) {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    static void* handle_client_thread(void* arg) {
        ClientArgs* args = static_cast<ClientArgs*>(arg);
        int client_socket = args->socket;
        std::string root_dir = args->root_dir;
        delete args;

        char buffer[BUFFER_SIZE] = {0};
        read(client_socket, buffer, BUFFER_SIZE);
        
        // Parse the HTTP request
        std::string request(buffer);
        std::string method = request.substr(0, request.find(' '));
        std::string path = request.substr(
            request.find(' ') + 1,
            request.find(" HTTP/") - request.find(' ') - 1
        );
        
        // Default to index.html for root path
        if (path == "/") {
            path = "/index.html";
        }

        std::string full_path = root_dir + path;
        std::string response;
        
        if (method == "GET") {
            if (file_exists(full_path)) {
                std::ifstream file(full_path.c_str(), std::ios::binary);
                if (file) {
                    // Read file content
                    std::stringstream content;
                    content << file.rdbuf();
                    std::string content_str = content.str();
                    
                    // Determine content type
                    std::string content_type = "text/plain";
                    if (ends_with(path, ".html")) content_type = "text/html";
                    else if (ends_with(path, ".css")) content_type = "text/css";
                    else if (ends_with(path, ".js")) content_type = "application/javascript";
                    else if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) content_type = "image/jpeg";
                    else if (ends_with(path, ".png")) content_type = "image/png";
                    
                    std::ostringstream content_length;
                    content_length << content_str.length();
                    
                    response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: " + content_type + "\r\n";
                    response += "Content-Length: " + content_length.str() + "\r\n";
                    response += "\r\n";
                    response += content_str;
                } else {
                    response = "HTTP/1.1 403 Forbidden\r\n\r\nForbidden";
                }
            } else {
                response = "HTTP/1.1 404 Not Found\r\n\r\nNot Found";
            }
        } else {
            response = "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed";
        }
        
        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
        return NULL;
    }

public:
    HttpServer(int port_num = 8080, const std::string& root = "./www")
        : port(port_num), root_dir(root), running(false) {
        
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            throw std::runtime_error("Failed to set socket options");
        }
        
        // Bind socket
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }
    }
    
    void start() {
        running = true;
        
        // Listen for connections
        if (listen(server_fd, MAX_CONNECTIONS) < 0) {
            throw std::runtime_error("Failed to listen");
        }
        
        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Serving files from " << root_dir << std::endl;
        
        while (running) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }
            
            // Create thread arguments
            ClientArgs* args = new ClientArgs;
            args->socket = client_socket;
            args->root_dir = root_dir;
            
            // Create new thread for client
            pthread_t thread;
            if (pthread_create(&thread, NULL, handle_client_thread, args) != 0) {
                delete args;
                close(client_socket);
                std::cerr << "Failed to create thread" << std::endl;
                continue;
            }
            worker_threads.push_back(thread);
        }
    }
    
    void stop() {
        running = false;
        close(server_fd);
        
        // Join all worker threads
        for (size_t i = 0; i < worker_threads.size(); ++i) {
            pthread_join(worker_threads[i], NULL);
        }
        worker_threads.clear();
    }
    
    ~HttpServer() {
        if (running) {
            stop();
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        if (argc > 1) {
            port = atoi(argv[1]);
        }
        std::string root_dir = "./www";
        if (argc > 2) {
            root_dir = argv[2];
        }
        
        HttpServer server(port, root_dir);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}