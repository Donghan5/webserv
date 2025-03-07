#include "Location.hpp"

Location::Location() : autoindex(false), client_max_body_size(DEFAULT_BODY_SIZE), deny_all(false), rewrite_permanent(false) {
			// Pre-allocate with reasonable sizes
			index.reserve(5);
			allowed_methods.reserve(5);
			fastcgi_params.reserve(10);

			// Set default allowed methods
			allowed_methods.push_back("GET");
			allowed_methods.push_back("POST");
			allowed_methods.push_back("DELETE");

			index.push_back("index.html");     // Default index file
}

Location::~Location() {}  // destructor

Location::Location(const Location& other) {
	if (this != &other) {
		try {
			*this = other;
		} catch (...) {
			// Ensure object is in valid state even if copy fails
			autoindex = false;
			client_max_body_size = 1024 * 1024;
			throw;
		}
	}
}

Location::Location& operator=(const Location& other) {
	if (this != &other) {
		path = other.path;
		root = other.root;
		alias = other.alias;
		index = other.index;
		autoindex = other.autoindex;
		allowed_methods = other.allowed_methods;
		return_url = other.return_url;
		client_max_body_size = other.client_max_body_size;
		deny_all = other.deny_all;
		fastcgi_pass = other.fastcgi_pass;
		fastcgi_index = other.fastcgi_index;
		fastcgi_params = other.fastcgi_params;
		rewrite_url = other.rewrite_url;
		rewrite_permanent = other.rewrite_permanent;
		error_pages = other.error_pages;
	}
	return *this;
}
