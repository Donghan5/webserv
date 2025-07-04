#include "Response.hpp"

STR	Response::handleGET(STR full_path, bool isListing, bool isRedirect) {
	if (isListing) {
		return GenerateListing(full_path, isRedirect);
	}

	if (access(full_path.c_str(), R_OK) == 0) {
		std::ifstream file(full_path.c_str(), std::ios::binary);
		if (file) {
			std::stringstream content;
			content << file.rdbuf();
			Logger::log(Logger::INFO, "GET " + full_path + " 200 OK");
			return createResponse(200, getMimeType(full_path), content.str(), "");
		}
	}
	Logger::log(Logger::INFO, "GET " + full_path + " 404 Not Found");
	return createErrorResponse(403, "text/plain", "Forbidden", _config);
}

STR Response::handlePOST(STR full_path) {
    Logger::log(Logger::DEBUG, "Response::handlePOST: start for " + full_path);

    STR dir_path = full_path.substr(0, full_path.find_last_of('/'));
    Logger::log(Logger::DEBUG, "Response::handlePOST: dir_path is " + dir_path);

    if (access(dir_path.c_str(), W_OK) != 0) {
		Logger::log(Logger::INFO, "POST " + full_path + " 403 Forbidden - Cannot write to directory");
        return createErrorResponse(403, "text/plain", "Forbidden - Cannot write to directory", _config);
    }

    bool file_exists = (access(full_path.c_str(), F_OK) == 0);

    std::ofstream file(full_path.c_str(), std::ios::binary);
    if (!file) {
		Logger::log(Logger::INFO, "POST " + full_path + " 500 Internal Server Error - Cannot create file");
        return createErrorResponse(500, "text/plain", "Internal Server Error - Cannot create file", _config);
    }

    file << _request._body;
    file.close();

    // 201 Created or 200 OK if updated
    STR status_message = file_exists ? "OK - File Updated" : "Created";
    int status_code = file_exists ? 200 : 201;

	Logger::log(Logger::DEBUG, "Response::handlePOST end");
	Logger::log(Logger::INFO, "POST " + full_path + " " + Utils::intToString(status_code) + " " + status_message);
    return createResponse(status_code, "text/plain", status_message, "");
}

STR Response::handleDELETE(STR full_path) {
    if (access(full_path.c_str(), F_OK) != 0) {
		Logger::log(Logger::INFO, "DELETE " + full_path + " 404 Not Found");
        return createErrorResponse(404, "text/plain", "Not Found", _config);
    }

    if (access(full_path.c_str(), W_OK) != 0) {
		Logger::log(Logger::INFO, "DELETE " + full_path + " 403 Forbidden - Cannot delete file");
        return createErrorResponse(403, "text/plain", "Forbidden", _config);
    }

    if (remove(full_path.c_str()) != 0) {
		Logger::log(Logger::INFO, "DELETE " + full_path + " 500 Internal Server Error - Failed to delete");
        return createErrorResponse(500, "text/plain", "Internal Server Error - Failed to delete", _config);
    }
	Logger::log(Logger::INFO, "DELETE " + full_path + " 204 No Content");
    return createResponse(204, "text/plain", "", "");
}

bool	check_method_allowed(STR method, LocationConfig *matchLocation) {
	AConfigBase* local_ref = matchLocation;
	while (local_ref && local_ref->_identify(local_ref) != SERVER) {
		LocationConfig* location = dynamic_cast<LocationConfig*>(local_ref);
		if (location->_allowed_methods[method] == true)
			return true;
		local_ref = local_ref->back_ref;
	}
	return false;
}

STR	Response::matchMethod(STR path, bool isDIR, LocationConfig *matchLocation, bool isRedirect) {
	if (_request._method == "GET") {
		if (!check_method_allowed("GET", matchLocation)) {
			Logger::log(Logger::INFO, "GET " + path + " 405 Method Not Allowed");
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
		}
		Logger::log(Logger::DEBUG, "Response::matchMethod GET path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handleGET(path, isDIR, isRedirect));
	} else if (_request._method == "POST") {
		if (!check_method_allowed("POST", matchLocation)) {
			Logger::log(Logger::INFO, "POST " + path + " 405 Method Not Allowed");
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
		}
		Logger::log(Logger::DEBUG, "Response::matchMethod POST path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handlePOST(path));
	} else if (_request._method == "DELETE") {
		if (!check_method_allowed("DELETE", matchLocation)) {
			Logger::log(Logger::INFO, "DELETE " + path + " 405 Method Not Allowed");
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
		}
		Logger::log(Logger::DEBUG, "Response::matchMethod DELETE path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handleDELETE(path));
	} else {
		Logger::log(Logger::DEBUG, "Response::matchMethod: UNUSUAL METHOD ERROR: " + _request._method);
		Logger::log(Logger::INFO, "Method " + _request._method + " not allowed for path: " + path);
		return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
	}
}
