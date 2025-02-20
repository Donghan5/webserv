#include "../includes/FileHandler.hpp"
#include "../includes/Logger.hpp"

/*
	Initialize mime_types
*/
std::map<std::string, std::string> FileHandler::initMimeTypes() {
	std::map<std::string, std::string> mimeTypes;
	mimeTypes[".html"] = "text/html";
	mimeTypes[".css"] = "text/css";
	mimeTypes[".js"] = "application/javascript";
	mimeTypes[".jpg"] = "image/jpeg";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".png"] = "image/png";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".svg"] = "image/svg+xml";
	mimeTypes[".pdf"] = "application/pdf";
	mimeTypes[".txt"] = "text/plain";
	return mimeTypes;
}

std::map<std::string, std::string> FileHandler::_mimeTypes = FileHandler::initMimeTypes();

/*
	GETTING THE MIME_TYPE
*/
std::string FileHandler::getContentType(const std::string &path) {
	const std::string defaultMime = "application/octet-stream";

	try {
		size_t dotPos = path.find_last_of(".");
		if (dotPos == std::string::npos) return defaultMime;

		std::string extension = path.substr(dotPos);
		for (size_t i (0); i < extension.length(); i++) {
			extension[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(extension[i])));
		}

		std::map<std::string, std::string>::const_iterator it = _mimeTypes.find(extension);
		return (it != _mimeTypes.end()) ? it->second : defaultMime;
	} catch (const std::exception &e) {
		Logger::log(Logger::ERROR, "Memory allocation failed");
		std::cerr << "Memory allocation failed in getContentType()" << std::endl;
		return defaultMime;
	}
}

/*
	Boolean value: existance of the file
*/
bool FileHandler::exists(const std::string &path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

/*
	Handling GET request
*/
std::string FileHandler::handleGetRequest(const std::string &path) {
	if (!FileHandler::exists(path)) {
		Logger::log(Logger::ERROR, "404 File Not Found\r\n\r\nCannot open file");
		return REQUEST404;
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) {
		Logger::log(Logger::ERROR, "403 Forbidden\r\n\r\n");
		return REQUEST403;
	}

	std::stringstream content;
	content << file.rdbuf();
	std::string content_str = content.str();

	// Determine content type
	std::string content_type = FileHandler::getContentType(path);

	std::ostringstream content_length;
	content_length << content_str.length();

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type + "\r\n";
	response += "Content-Length: " + content_length.str() + "\r\n";
	response += "\r\n";
	response += content_str;
	Logger::log(Logger::INFO, "200 OK in get request");
	return response;
}

/*
	Handling POST request
*/
std::string FileHandler::handlePostRequest(const std::string &path, const std::string &body) {
	if (body.empty()) {
		Logger::log(Logger::ERROR, "400 Bad request in post request: Empty body");
		throw std::runtime_error(std::string("400 Bad Request\r\n\r\nEmpty body"));
	}

	std::ofstream outFile(path.c_str(), std::ios::binary);
	if (!outFile.is_open()) {
		Logger::log(Logger::ERROR, "500 Internal Server Error in post request");
		throw std::runtime_error(std::string("500 Internal Server Errorr\r\n\r\nCannot open file"));
	}
	outFile.write(body.c_str(), body.size());
	outFile.close();
	Logger::log(Logger::INFO, "201 POST request accepted");
	return REQUEST201;
}

/*
	Handling DELETE request
*/
std::string FileHandler::handleDeleteRequest(const std::string &path) {
	if (!FileHandler::exists(path)) {
		Logger::log(Logger::ERROR, "404 Error in delete handle");
		return REQUEST404;
	}

	if (remove(path.c_str()) != 0) {
		Logger::log(Logger::ERROR, "500 Error in delete handle");
		return REQUEST500;
	}
	Logger::log(Logger::INFO, "200 OK in delete request");
	return "HTTP/1.1 200 OK\r\n\r\nFile deleted successfully";
}
