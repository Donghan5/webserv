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

    // Helper function to make socket non-blocking
    static void make_non_blocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error("Failed to get socket flags");
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw std::runtime_error("Failed to set socket non-blocking");
        }
    }

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

    // Process a complete HTTP request
    std::string process_request(const std::string& request) {
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

                    std::string response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: " + content_type + "\r\n";
                    response += "Content-Length: " + content_length.str() + "\r\n";
                    response += "\r\n";
                    response += content_str;
                    return response;
                } else {
                    return "HTTP/1.1 403 Forbidden\r\n\r\nForbidden";
                }
            } else {
                return "HTTP/1.1 404 Not Found\r\n\r\nNot Found";
            }
        } else {
            return "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed";
        }
    }

    void handle_client_read(int client_fd) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);

        if (bytes_read <= 0) {
            if (bytes_read == 0 || errno != EAGAIN) {
                close_client(client_fd);
            }
            return;
        }

        partial_requests[client_fd].append(buffer, bytes_read);

        // Check if we have a complete HTTP request
        size_t header_end = partial_requests[client_fd].find("\r\n\r\n");
        if (header_end != std::string::npos) {
            std::string response = process_request(partial_requests[client_fd]);
            partial_responses[client_fd] = response;
            partial_requests.erase(client_fd);

            // Modify the pollfd entry to monitor for writing
            for (size_t i = 0; i < poll_fds.size(); ++i) {
                if (poll_fds[i].fd == client_fd) {
                    poll_fds[i].events = POLLOUT;
                    break;
                }
            }
        }
    }

    void handle_client_write(int client_fd) {
        std::string& response = partial_responses[client_fd];
        ssize_t bytes_written = write(client_fd, response.c_str(), response.length());

        if (bytes_written <= 0) {
            if (errno != EAGAIN) {
                close_client(client_fd);
            }
            return;
        }

        response.erase(0, bytes_written);

        if (response.empty()) {
            close_client(client_fd);
        }
    }

    void close_client(int client_fd) {
        close(client_fd);
        partial_requests.erase(client_fd);
        partial_responses.erase(client_fd);

        // Remove from poll_fds
        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].fd == client_fd) {
                poll_fds.erase(poll_fds.begin() + i);
                break;
            }
        }
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

        // Make server socket non-blocking
        make_non_blocking(server_fd);

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
        if (listen(server_fd, SOMAXCONN) < 0) {
            throw std::runtime_error("Failed to listen");
        }

        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Serving files from " << root_dir << std::endl;

        // Add server socket to poll set
        struct pollfd server_pollfd;
        server_pollfd.fd = server_fd;
        server_pollfd.events = POLLIN;
        server_pollfd.revents = 0;
        poll_fds.push_back(server_pollfd);

        while (running) {
            int ready = poll(&poll_fds[0], poll_fds.size(), -1);
            if (ready < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("Poll failed");
            }

            // Make a copy because the vector might be modified during iteration
            std::vector<struct pollfd> current_fds = poll_fds;

            for (size_t i = 0; i < current_fds.size(); ++i) {
                if (current_fds[i].revents == 0) {
                    continue;
                }

                if (current_fds[i].fd == server_fd) {
                    // Handle new connection
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (client_fd < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            std::cerr << "Failed to accept connection" << std::endl;
                        }
                        continue;
                    }

                    make_non_blocking(client_fd);

                    // Add to poll set
                    struct pollfd client_pollfd;
                    client_pollfd.fd = client_fd;
                    client_pollfd.events = POLLIN;
                    client_pollfd.revents = 0;
                    poll_fds.push_back(client_pollfd);

                } else {
                    // Handle client socket
                    if (current_fds[i].revents & POLLIN) {
                        handle_client_read(current_fds[i].fd);
                    }
                    if (current_fds[i].revents & POLLOUT) {
                        handle_client_write(current_fds[i].fd);
                    }
                    if (current_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                        close_client(current_fds[i].fd);
                    }
                }
            }
        }
    }

    void stop() {
        running = false;
        close(server_fd);

        // Close all client connections
        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].fd != server_fd) {
                close(poll_fds[i].fd);
            }
        }
        poll_fds.clear();
        partial_requests.clear();
        partial_responses.clear();
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
