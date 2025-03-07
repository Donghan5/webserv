#include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig(): port(80), client_max_body_size(DEFAULT_BODY_SIZE), ssl_enabled(false) {
	server_names.reserve(MAX_SERVER_NAMES);
	index.reserve(5);
}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig& other) {
	*this = other;
}

ServerConfig::ServerConfig& operator=(const ServerConfig& other) {
	if (this != &other) {
		host = other.host;
		port = other.port;
		server_names = other.server_names;
		locations = other.locations;
		root = other.root;
		index = other.index;
		client_max_body_size = other.client_max_body_size;
		ssl_enabled = other.ssl_enabled;
		ssl_certificate = other.ssl_certificate;
		ssl_certificate_key = other.ssl_certificate_key;
		error_pages = other.error_pages;
	}
	return *this;
}
