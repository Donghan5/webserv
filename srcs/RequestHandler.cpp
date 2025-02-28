#include "../includes/RequestHandler.hpp"

bool RequestHandler::file_exists(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

// Helper function to check string ending
bool RequestHandler::ends_with(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length()) {
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

// Process a complete HTTP request
std::string RequestHandler::process_request(const std::string &request) {
	ParsedRequest parser(request);

	std::string method = parser.getMethod();
	std::string path = parser.getPath();
	std::string host = parser.getHost();

	// Default to index.html for root path
	if (path == "/") {
		path = "/index.html";
	}

	std::string resolved_root = _webconf.resolveRoot(host, path);
	std::string full_path = resolved_root + path;

	Logger::log(Logger::INFO, "Resolved root: " + resolved_root);
	Logger::log(Logger::INFO, "Directive to: " + full_path);
	std::string extension = path.substr(path.find_last_of("."));
	if (extension == ".py" || extension == ".php" || extension == ".pl" || extension == ".sh") {
			CgiHandler cgi(full_path, parser.getHeaders(), parser.getBody());
			return cgi.executeCgi();
	}
	if (method == "GET") {
		return FileHandler::handleGetRequest(full_path);
	}
	else if (method == "POST") {
		try {
			std::string body = parser.getBody();
			return FileHandler::handlePostRequest(full_path, body);
		} catch (const std::exception &e) {
			return std::string(e.what()); // request 500
		}
	}
	else if (method == "DELETE") {
		return FileHandler::handleDeleteRequest(full_path);
	}
	else {
		Logger::log(Logger::ERROR, "405 error in process request");
		return REQUEST405;
	}
}
