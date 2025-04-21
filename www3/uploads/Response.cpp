#include "Response.hpp"
#include "Logger.hpp"

void	init_mimetypes(std::map<STR, STR>	&mime_types) {
	mime_types[".html"] = "text/html";
	mime_types[".htm"] = "text/html";
	mime_types[".shtml"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".gif"] = "image/gif";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".js"] = "application/javascript";
	mime_types[".atom"] = "application/atom+xml";
	mime_types[".rss"] = "application/rss+xml";
	mime_types[".mml"] = "text/mathml";
	mime_types[".txt"] = "text/plain";
	mime_types[".jad"] = "text/vnd.sun.j2me.app-descriptor";
	mime_types[".wml"] = "text/vnd.wap.wml";
	mime_types[".htc"] = "text/x-component";
	mime_types[".avif"] = "image/avif";
	mime_types[".png"] = "image/png";
	mime_types[".svg"] = "image/svg+xml";
	mime_types[".svgz"] = "image/svg+xml";
	mime_types[".tif"] = "image/tiff";
	mime_types[".tiff"] = "image/tiff";
	mime_types[".wbmp"] = "image/vnd.wap.wbmp";
	mime_types[".webp"] = "image/webp";
	mime_types[".ico"] = "image/x-icon";
	mime_types[".jng"] = "image/x-jng";
	mime_types[".bmp"] = "image/x-ms-bmp";
	mime_types[".woff"] = "font/woff";
	mime_types[".woff2"] = "font/woff2";
	mime_types[".jar"] = "application/java-archive";
	mime_types[".war"] = "application/java-archive";
	mime_types[".ear"] = "application/java-archive";
	mime_types[".json"] = "application/json";
	mime_types[".hqx"] = "application/mac-binhex40";
	mime_types[".doc"] = "application/msword";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".ps"] = "application/postscript";
	mime_types[".eps"] = "application/postscript";
	mime_types[".ai"] = "application/postscript";
	mime_types[".rtf"] = "application/rtf";
	mime_types[".m3u8"] = "application/vnd.apple.mpegurl";
	mime_types[".kml"] = "application/vnd.google-earth.kml+xml";
	mime_types[".kmz"] = "application/vnd.google-earth.kmz";
	mime_types[".xls"] = "application/vnd.ms-excel";
	mime_types[".eot"] = "application/vnd.ms-fontobject";
	mime_types[".ppt"] = "application/vnd.ms-powerpoint";
	mime_types[".odg"] = "application/vnd.oasis.opendocument.graphics";
	mime_types[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mime_types[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types[".odt"] = "application/vnd.oasis.opendocument.text";
	mime_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types[".wmlc"] = "application/vnd.wap.wmlc";
	mime_types[".wasm"] = "application/wasm";
	mime_types[".7z"] = "application/x-7z-compressed";
	mime_types[".cco"] = "application/x-cocoa";
	mime_types[".jardiff"] = "application/x-java-archive-diff";
	mime_types[".jnlp"] = "application/x-java-jnlp-file";
	mime_types[".run"] = "application/x-makeself";
	mime_types[".pl"] = "application/x-perl";
	mime_types[".pm"] = "application/x-perl";
	mime_types[".prc"] = "application/x-pilot";
	mime_types[".pdb"] = "application/x-pilot";
	mime_types[".rar"] = "application/x-rar-compressed";
	mime_types[".rpm"] = "application/x-redhat-package-manager";
	mime_types[".sea"] = "application/x-sea";
	mime_types[".swf"] = "application/x-shockwave-flash";
	mime_types[".sit"] = "application/x-stuffit";
	mime_types[".tcl"] = "application/x-tcl";
	mime_types[".tk"] = "application/x-tcl";
	mime_types[".der"] = "application/x-x509-ca-cert";
	mime_types[".pem"] = "application/x-x509-ca-cert";
	mime_types[".crt"] = "application/x-x509-ca-cert";
	mime_types[".xpi"] = "application/x-xpinstall";
	mime_types[".xhtml"] = "application/xhtml+xml";
	mime_types[".xspf"] = "application/xspf+xml";
	mime_types[".zip"] = "application/zip";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".exe"] = "application/octet-stream";
	mime_types[".dll"] = "application/octet-stream";
	mime_types[".deb"] = "application/octet-stream";
	mime_types[".dmg"] = "application/octet-stream";
	mime_types[".iso"] = "application/octet-stream";
	mime_types[".img"] = "application/octet-stream";
	mime_types[".msi"] = "application/octet-stream";
	mime_types[".msp"] = "application/octet-stream";
	mime_types[".msm"] = "application/octet-stream";
	mime_types[".mid"] = "audio/midi";
	mime_types[".midi"] = "audio/midi";
	mime_types[".kar"] = "audio/midi";
	mime_types[".mp3"] = "audio/mpeg";
	mime_types[".ogg"] = "audio/ogg";
	mime_types[".m4a"] = "audio/x-m4a";
	mime_types[".ra"] = "audio/x-realaudio";
	mime_types[".3gpp"] = "video/3gpp";
	mime_types[".3gp"] = "video/3gpp";
	mime_types[".ts"] = "video/mp2t";
	mime_types[".mp4"] = "video/mp4";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mpg"] = "video/mpeg";
	mime_types[".mov"] = "video/quicktime";
	mime_types[".webm"] = "video/webm";
	mime_types[".flv"] = "video/x-flv";
	mime_types[".m4v"] = "video/x-m4v";
	mime_types[".mng"] = "video/x-mng";
	mime_types[".asx"] = "video/x-ms-asf";
	mime_types[".asf"] = "video/x-ms-asf";
	mime_types[".wmv"] = "video/x-ms-wmv";
	mime_types[".avi"] = "video/x-msvideo";
}

void	init_status_codes(std::map<int, STR>	&status_codes) {
	// Informational responses (100-199)
	status_codes[100] = "100 Continue";
	status_codes[101] = "101 Switching Protocols";
	status_codes[102] = "102 Processing";
	status_codes[103] = "103 Early Hints";

	// Successful responses (200-299)
	status_codes[200] = "200 OK";
	status_codes[201] = "201 Created";
	status_codes[202] = "202 Accepted";
	status_codes[203] = "203 Non-Authoritative Information";
	status_codes[204] = "204 No Content";
	status_codes[205] = "205 Reset Content";
	status_codes[206] = "206 Partial Content";
	status_codes[207] = "207 Multi-Status";
	status_codes[208] = "208 Already Reported";
	status_codes[226] = "226 IM Used";

	// Redirection messages (300-399)
	status_codes[300] = "300 Multiple Choices";
	status_codes[301] = "301 Moved Permanently";
	status_codes[302] = "302 Found";
	status_codes[303] = "303 See Other";
	status_codes[304] = "304 Not Modified";
	status_codes[305] = "305 Use Proxy";
	status_codes[307] = "307 Temporary Redirect";
	status_codes[308] = "308 Permanent Redirect";

	// Client error responses (400-499)
	status_codes[400] = "400 Bad Request";
	status_codes[401] = "401 Unauthorized";
	status_codes[402] = "402 Payment Required";
	status_codes[403] = "403 Forbidden";
	status_codes[404] = "404 Not Found";
	status_codes[405] = "405 Method Not Allowed";
	status_codes[406] = "406 Not Acceptable";
	status_codes[407] = "407 Proxy Authentication Required";
	status_codes[408] = "408 Request Timeout";
	status_codes[409] = "409 Conflict";
	status_codes[410] = "410 Gone";
	status_codes[411] = "411 Length Required";
	status_codes[412] = "412 Precondition Failed";
	status_codes[413] = "413 Payload Too Large";
	status_codes[414] = "414 URI Too Long";
	status_codes[415] = "415 Unsupported Media Type";
	status_codes[416] = "416 Range Not Satisfiable";
	status_codes[417] = "417 Expectation Failed";
	status_codes[418] = "418 I'm a teapot";
	status_codes[421] = "421 Misdirected Request";
	status_codes[422] = "422 Unprocessable Entity";
	status_codes[423] = "423 Locked";
	status_codes[424] = "424 Failed Dependency";
	status_codes[425] = "425 Too Early";
	status_codes[426] = "426 Upgrade Required";
	status_codes[428] = "428 Precondition Required";
	status_codes[429] = "429 Too Many Requests";
	status_codes[431] = "431 Request Header Fields Too Large";
	status_codes[451] = "451 Unavailable For Legal Reasons";

	// Server error responses (500-599)
	status_codes[500] = "500 Internal Server Error";
	status_codes[501] = "501 Not Implemented";
	status_codes[502] = "502 Bad Gateway";
	status_codes[503] = "503 Service Unavailable";
	status_codes[504] = "504 Gateway Timeout";
	status_codes[505] = "505 HTTP Version Not Supported";
	status_codes[506] = "506 Variant Also Negotiates";
	status_codes[507] = "507 Insufficient Storage";
	status_codes[508] = "508 Loop Detected";
	status_codes[510] = "510 Not Extended";
	status_codes[511] = "511 Network Authentication Required";
}

Response::Response(Request *request, HttpConfig *config) {
	init_mimetypes(_all_mime_types);
	init_status_codes(_all_status_codes);
	_request = request;
	_config = config;
}

STR Response::createResponse(int statusCode, const STR& contentType, const STR& body, const STR& extra) {
    std::stringstream response;
    response << "HTTP/1.1 " << _all_status_codes[statusCode] << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Access-Control-Allow-Credentials: true\r\n"
			 << extra << ((extra.empty()) ? "" : "\r\n")
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    return response.str();
}

Response::Response()
{
	init_mimetypes(_all_mime_types);
	init_status_codes(_all_status_codes);
	_request = NULL;
	_config = NULL;
}

Response::Response(const Response &obj) {
	_all_mime_types = obj._all_mime_types;
	_all_status_codes = obj._all_status_codes;
	_request = obj._request;
	_config = obj._config;
}

Response::~Response() {

}

void Response::setRequest(Request *request) {
	_request = request;
}

void Response::setConfig(HttpConfig *config) {
	_config = config;
}

bool ends_with(const STR &str, const STR &suffix)
{
	if (str.length() < suffix.length())
	{
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

STR getMimeType(const STR& path) {
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

STR	Response::getMime(STR path) {

	// std::cerr << "ending mime : " << ending << ", for path " << path << "\n";
	try
	{
		STR	ending = path.substr(path.find_last_of('.'));
		return _all_mime_types.at(ending);
	}
	catch(const std::exception& e)
	{
		return "text/plain";
	}

	return "text/plain";
}

#include <dirent.h>

// Function to decode URL-encoded strings
STR urlDecode(const STR& input) {
    STR result;
    result.reserve(input.length());

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '%' && i + 2 < input.length()) {
            // Get the two hex digits
            STR hexVal = input.substr(i + 1, 2);

            // Convert from hex to decimal
            int value = 0;
            for (size_t j = 0; j < 2; ++j) {
                value *= 16;
                char c = hexVal[j];
                if (c >= '0' && c <= '9') {
                    value += c - '0';
                } else if (c >= 'A' && c <= 'F') {
                    value += 10 + (c - 'A');
                } else if (c >= 'a' && c <= 'f') {
                    value += 10 + (c - 'a');
                }
            }

            // Append the decoded character
            result += static_cast<char>(value);
            i += 2;
        } else if (input[i] == '+') {
            result += ' ';
        } else {
            result += input[i];
        }
    }

    return result;
}

//rewrite, recheck
STR Response::handleDIR(STR path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return createErrorResponse(500, "text/plain", "Failed to read directory", NULL);
    }

    std::stringstream html;
    html << "<html><head><title>Index of " << _request->_file_path << "</title></head><body>\n";
    html << "<h1>Index of " << _request->_file_path << "</h1><hr><pre>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		STR name = entry->d_name;
        struct stat st;
        STR root_path = path + "/" + name;

        if (stat(root_path.c_str(), &st) == 0) {
			if (S_ISDIR(st.st_mode)){
				name += "/";
			}
			STR fullpath = _request->_file_path + "/" + name;

            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));	//REDO, bad funcs!

            STR displayName = urlDecode(name);

			if (displayName.length() >= 45)
				displayName = displayName.substr(0, 42) + "...";

			// std::cerr << "DEBUG Response::handleDIR: displayName is " << displayName << "\n";

			html << "<a href=\"" << fullpath << "\">"
			<< displayName << "</a>"
			<< STR(50 - displayName.length(), ' ')
			<< timeStr
			<< STR(20, ' ')
			<< st.st_size << "\n";
        }
    }

    html << "</pre><hr></body></html>";
    closedir(dir);
    return createResponse(200, "text/html", html.str(), "");
}

void	Response::selectIndexIndexes(VECTOR<STR> indexes, STR &best_match, float &match_quality, STR dir_path) {
	size_t	i = 0;

	while (i < indexes.size()) {
		if (match_quality == 1)
			break;

		// check the file exists
		if (checkFile(dir_path + "/" + indexes[i]) != NormalFile) {
			i++;
			continue;
		}

		STR index_mime = getMime(indexes[i]);

		try
		{
			if (_request->_accepted_types[index_mime] > match_quality) {
				Logger::cerrlog(Logger::DEBUG, index_mime + " is better match that" + best_match + "! Quality " + Utils::floatToString(_request->_accepted_types[index_mime]) + " is better than " + Utils::floatToString(match_quality));
				best_match = indexes[i];
				match_quality = _request->_accepted_types[index_mime];
			}
			else
				Logger::cerrlog(Logger::DEBUG, index_mime + " is not more than " + Utils::floatToString(match_quality));
		}
		catch(const std::exception& e)
		{
			Logger::cerrlog(Logger::ERROR, index_mime + " is not accepted: " + index_mime);
		}
		try
		{
			if (_request->_accepted_types["*/*"] > match_quality) {
				Logger::cerrlog(Logger::DEBUG, "*/* is the better match than " + best_match
					+ "! Quality " + Utils::floatToString(_request->_accepted_types["*/*"]) + " is better than " + Utils::floatToString(match_quality));
				best_match = indexes[i];
				match_quality = _request->_accepted_types["*/*"];
			}
			else
				Logger::cerrlog(Logger::DEBUG, "*/* is not more than " + Utils::floatToString(match_quality));
		}
		catch(const std::exception& e)
		{
			Logger::cerrlog(Logger::ERROR, "*/* is not accepted: " + index_mime);
		}

		i++;
	}
}
STR	Response::selectIndexAll(LocationConfig* location, STR dir_path) {
	STR best_match = "";
	float		match_quality = 0.0;

	/*
		Searching on best existing level for index match with best quality.

		quality is not needed - remove later
	*/
	AConfigBase* local_ref = location;
	while (local_ref) {
		if (!local_ref->_index.empty()) {
			selectIndexIndexes(local_ref->_index, best_match, match_quality, dir_path);
			if (match_quality == 1)
				break;
		}
		local_ref = local_ref->back_ref;
	}
	// if (!location->_index.empty()) {
	// 	selectIndexIndexes(location->_index, best_match, match_quality);
	// } else if (!location->back_ref->_index.empty())
	// 	selectIndexIndexes(location->back_ref->_index, best_match, match_quality);
	// else
	// 	selectIndexIndexes(location->back_ref->back_ref->_index, best_match, match_quality);

	if (best_match == "")
	{
		throw std::runtime_error("No index match");
	}
	else
		Logger::cerrlog(Logger::DEBUG, "Response::selectIndexAll: best_match is " + best_match + ", quality: " + Utils::floatToString(match_quality));
	return best_match;
}

FileType Response::checkFile(const STR& path) {
    if (path.empty()) {
		Logger::cerrlog(Logger::ERROR, "Error: Empty path provided.");
        return NotFound;
    }

    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
		Logger::cerrlog(Logger::ERROR, "Error checking file: " + path + " Reason: " + strerror(errno));
        return NotFound;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        return Directory;
    }
    return NormalFile;
}


STR	Response::handleGET(STR full_path, bool isDIR) {
	if (isDIR) {
		return handleDIR(full_path);
	}

	if (access(full_path.c_str(), R_OK) == 0) {
		std::ifstream file(full_path.c_str(), std::ios::binary);
		if (file) {
			std::stringstream content;
			content << file.rdbuf();
			return createResponse(200, getMimeType(full_path), content.str(), "");
		}
	}
	return createErrorResponse(403, "text/plain", "HANDLEGET ERROR (Forbidden)", NULL);
}

STR Response::handlePOST(STR full_path) {
	// std::cerr << "DEBUG Response::handlePOST start\n";

    // Check if directory exists to upload to
    STR dir_path = full_path.substr(0, full_path.find_last_of('/'));
    if (access(dir_path.c_str(), W_OK) != 0) {
        return createErrorResponse(403, "text/plain", "HANDLEPOST ERROR (Forbidden - Cannot write to directory)", NULL);
    }

    // Check if file already exists
    bool file_exists = (access(full_path.c_str(), F_OK) == 0);

    // Open file for writing
    std::ofstream file(full_path.c_str(), std::ios::binary);
    if (!file) {
        return createErrorResponse(500, "text/plain", "HANDLEPOST ERROR (Internal Server Error - Cannot create file)", NULL);
    }

    file << _request->_body;
    file.close();

    // Return appropriate status code (201 Created or 200 OK if updated)
    STR status_message = file_exists ? "OK - File Updated" : "Created";
    int status_code = file_exists ? 200 : 201;

    // STR status_message =  "Created";
    // int status_code = 201;

	Logger::log(Logger::INFO, "Response::handlePOST end");

    return createResponse(status_code, "text/plain", status_message, "");
}

STR Response::handleDELETE(STR full_path) {
    // Check if file exists
    if (access(full_path.c_str(), F_OK) != 0) {
        return createErrorResponse(404, "text/plain", "Not Found", NULL);
    }

    // Check if we have permission to delete
    if (access(full_path.c_str(), W_OK) != 0) {
        return createErrorResponse(403, "text/plain", "Forbidden", NULL);
    }

    // Try to delete the file
    if (remove(full_path.c_str()) != 0) {
        return createErrorResponse(500, "text/plain", "Internal Server Error - Failed to delete", NULL);
    }

    // Return success response
    return createResponse(204, "text/plain", "", "");
}

STR	regress_path(STR path) {
	if (path.find_last_of("/") == std::string::npos)
		return path;
	if (path == "/")
		return "";
	if (path.find_last_of("/") == 0) {
		return path.substr(0, path.find_last_of("/") + 1);
	} else {
		return path.substr(0, path.find_last_of("/"));
	}
	return path;
}

// test buildDirPath function
LocationConfig *Response::buildDirPath(ServerConfig *matchServer, STR &full_path, bool &isDIR) {
	LocationConfig *matchLocation = NULL;
	STR path_to_match = _request->_file_path;

	(void) isDIR;

	// search exact match first
	while (path_to_match != "" && !matchLocation) {
		MAP<STR, LocationConfig *> loc_loc = matchServer->_locations;
		while (loc_loc.size() > 0) {
			MAP<STR, LocationConfig *>::iterator it = loc_loc.begin();
			if (it->first == path_to_match) {
				matchLocation = it->second;
				break;
			}
			if (it->second->_locations.size() > 0) {
				loc_loc.insert(it->second->_locations.begin(), it->second->_locations.end());
			}
			loc_loc.erase(it);
		}

		if (!matchLocation && path_to_match != "" && path_to_match[path_to_match.length() - 1] != '/') {
			STR path_with_slash = path_to_match + "/";
			loc_loc = matchServer->_locations;
			while (loc_loc.size() > 0) {
				MAP<STR, LocationConfig *>::iterator it = loc_loc.begin();
				if (it->first == path_with_slash) {
					matchLocation = it->second;
					break;
				}
				if (it->second->_locations.size() > 0) {
					loc_loc.insert(it->second->_locations.begin(), it->second->_locations.end());
				}
				loc_loc.erase(it);
			}
		}

		if (!matchLocation) {
			Logger::cerrlog(Logger::DEBUG, "Regressed path " + path_to_match + " -> " + regress_path(path_to_match));
			path_to_match = regress_path(path_to_match);
		}
	}

	if (!matchLocation)
		return NULL;

	// add root path
	AConfigBase* local_ref = matchLocation;
	while (local_ref) {
		if (local_ref->_root != "") {
			full_path.append(local_ref->_root);
			break;
		}
		local_ref = local_ref->back_ref;
	}

	full_path.append(_request->_file_path);

	Logger::cerrlog(Logger::DEBUG, "Response::buildDirPath: dir path is " + full_path);
	return matchLocation;
}

// Original buildDirPath function
// LocationConfig	*Response::buildDirPath(ServerConfig *matchServer, STR &full_path, bool &isDIR) {
// 	LocationConfig* matchLocation = NULL;
// 	STR	path_to_match = _request->_file_path;
// 	(void) isDIR;
// 	while (path_to_match != "" && !matchLocation)
// 	{
// 		MAP<STR, LocationConfig*> loc_loc = matchServer->_locations;
// 		while (loc_loc.size() > 0) {
// 			MAP<STR, LocationConfig*>::iterator it = loc_loc.begin();
// 			if (it->first == path_to_match) {
// 				matchLocation = it->second;
// 				break;
// 			}
// 			if (it->second->_locations.size() > 0) {
// 				loc_loc.insert(it->second->_locations.begin(), it->second->_locations.end());
// 			}
// 			loc_loc.erase(it);
// 		}

// 		// if (matchLocation && (matchLocation->_proxy_pass_host == "" && !isDIR)) {
// 		// 	matchLocation = NULL;
// 		// }

// 		if (!matchLocation)
// 			std::cerr << "Regressed path " << path_to_match << " -> " << regress_path(path_to_match) << std::endl;
// 		path_to_match = regress_path(path_to_match);
// 	}

// 	path_to_match += "/";

// 	while (path_to_match != "" && !matchLocation)
// 	{
// 		MAP<STR, LocationConfig*> loc_loc = matchServer->_locations;
// 		while (loc_loc.size() > 0) {
// 			MAP<STR, LocationConfig*>::iterator it = loc_loc.begin();
// 			if (it->first == path_to_match) {
// 				matchLocation = it->second;
// 				break;
// 			}
// 			if (it->second->_locations.size() > 0) {
// 				loc_loc.insert(it->second->_locations.begin(), it->second->_locations.end());
// 			}
// 			loc_loc.erase(it);
// 		}

// 		// if (matchLocation && (matchLocation->_proxy_pass_host == "" && !isDIR)) {
// 		// 	matchLocation = NULL;
// 		// }

// 		if (!matchLocation)
// 			std::cerr << "Regressed path " << path_to_match << " -> " << regress_path(path_to_match) << std::endl;
// 		path_to_match = regress_path(path_to_match);
// 	}

// 	// std::cerr << "TEST TEST buildDirPath is matchLocation " << (matchLocation != NULL) << std::endl;
// 	if (!matchLocation)
// 		return NULL;

// 	//add root to final path first
// 	// if ((matchLocation)->_root != "") {
// 	// 	full_path.append((matchLocation)->_root);
// 	// }
// 	// else if ((matchLocation)->back_ref->_root != "") {
// 	// 	full_path.append((matchLocation)->back_ref->_root);
// 	// }
// 	// else {
// 	// 	full_path.append((matchLocation)->back_ref->back_ref->_root);
// 	// }

// 	AConfigBase* local_ref = matchLocation;
// 	while (local_ref) {
// 		if (local_ref->_root != "") {
// 			full_path.append(local_ref->_root);
// 			break;
// 		}
// 		local_ref = local_ref->back_ref;
// 	}

// 	full_path.append(_request->_file_path);

// 	std::cerr << "Response::buildDirPath: dir path is " << full_path << "\n";
// 	return matchLocation;
// }

int Response::buildIndexPath(LocationConfig *matchLocation, STR &best_file_path, STR dir_path) {
	if (best_file_path != "/" && best_file_path[best_file_path.length() - 1] != '/')
		best_file_path.append("/");

	//if request is file
	if (_request->_file_name != "") {
		best_file_path.append(_request->_file_name);
		Logger::log(Logger::INFO, "Response::buildBestPath: FILE full path is " + best_file_path);
		return 1;
	}

	//if request doesn't have file name - searching for index
	try
	{
		best_file_path.append(selectIndexAll(matchLocation, dir_path));
	}
	catch(const std::exception& e)
	{
		// return createResponse(403, "text/plain", "NO SUCH FILE FOUND (change later)", "");

		Logger::cerrlog(Logger::ERROR, "Response::buildFilePath: no index file found");
		return 0;
	}

	Logger::log(Logger::INFO, "Response::buildFilePath: index file is " + best_file_path);
	return 1;
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

STR	Response::matchMethod(STR path, bool isDIR, LocationConfig *matchLocation) {
	if (_request->_method == "GET") {
		if (!check_method_allowed("GET", matchLocation))
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
			Logger::log(Logger::INFO, "Response::matchMethod GET path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handleGET(path, isDIR));
	} else if (_request->_method == "POST") {
		if (!check_method_allowed("POST", matchLocation))
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
			Logger::log(Logger::INFO, "Response::matchMethod POST path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handlePOST(path));
	} else if (_request->_method == "DELETE") {
		if (!check_method_allowed("DELETE", matchLocation))
			return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
			Logger::log(Logger::INFO, "Response::matchMethod DELETE path" + path + " isDIR " + Utils::floatToString(isDIR));
		return (handleDELETE(path));
	} else {
		Logger::cerrlog(Logger::ERROR, "Response::matchMethod: UNUSUAL METHOD ERROR: " + _request->_method);
		return createErrorResponse(405, "text/plain", "Method Not Allowed", matchLocation);
	}
}


STR	Response::checkRedirect(LocationConfig *matchLocation) {
	AConfigBase*	back_ref = matchLocation;
	ServerConfig* tempServer;
	LocationConfig* tempLocation;

	while (back_ref && back_ref->_identify(back_ref) != HTTP) {
		switch (back_ref->_identify(back_ref))
		{
		case SERVER:
			 tempServer = dynamic_cast<ServerConfig*>(back_ref);

			if (tempServer->_return_url != "" && tempServer->_return_code == -1) {
				return createResponse(301, "text/plain", "Redirect", "Location: " + tempServer->_return_url);
			}
			if (tempServer->_return_url != "") {
				return createResponse(tempServer->_return_code, "text/plain", "Redirect", "Location: " + tempServer->_return_url);
			}
			break;
		case LOCATION:
			 tempLocation = dynamic_cast<LocationConfig*>(back_ref);

			if (tempLocation->_return_url != "" && tempLocation->_return_code == -1) {
				return createResponse(301, "text/plain", "Redirect", "Location: " + tempLocation->_return_url);
			}
			if (tempLocation->_return_url != "") {
				return createResponse(tempLocation->_return_code, "text/plain", "Redirect", "Location: " + tempLocation->_return_url);
			}
			break;
		case HTTP:
			return "";
		default:
			break;
		}

		back_ref = back_ref->back_ref;
	}
	return "";
}


STR	Response::createErrorResponse(int statusCode, const STR& contentType, const STR& body, AConfigBase *base) {
	while (base) {
		try
		{
			if (strncmp(base->_error_pages[statusCode].c_str(), "default_errors/", 15) || base->_identify(base) == HTTP) {

				if (access(base->_error_pages[statusCode].c_str(), R_OK) == 0) {
					std::ifstream file(base->_error_pages[statusCode].c_str(), std::ios::binary);
					if (file) {
						std::stringstream content;
						content << file.rdbuf();
						return createResponse(statusCode, getMimeType(base->_error_pages[statusCode]), content.str(), "");
					}
				}
			}
		}
		catch(const std::exception& e)
		{
		}
		base = base->back_ref;
	}

	return createResponse(statusCode, contentType, body, "");
}

bool	Response::checkBodySize(LocationConfig *matchLocation) {
	AConfigBase* local_ref = matchLocation;
	while (local_ref) {
		if (local_ref->_client_max_body_size > 0) {
			// std::cerr << "Response::checkBodySize: client_max_body_size is " << local_ref->_client_max_body_size << "\n";
			// std::cerr << "Response::checkBodySize: body size is " << _request->_body.length() << "\n";
			if (_request->_body.length() > (size_t)local_ref->_client_max_body_size)
				return false;
			else
				return true;
		}

		local_ref = local_ref->back_ref;
	}
	return true;
}

/*
	paths with spaces are not found
*/
STR Response::getResponse() {
	ServerConfig*	matchServer = NULL;
	LocationConfig*	matchLocation = NULL;

	bool	isDIR = false;
	STR dir_path = "";
	STR	file_path = "";

	if (!_request || !_config) {
		Logger::cerrlog(Logger::ERROR, "Response::getResponse error, no config or request");
		return "";
	}

	for (size_t i = 0; i < _config->_servers.size(); i++) {
		if (_config->_servers[i]->_listen_port != _request->_port)
			continue;
		for (size_t k = 0; k < _config->_servers[i]->_server_name.size(); k++) {
			if (_config->_servers[i]->_server_name[k] == _request->_host) {
				matchServer = _config->_servers[i];
				break;
			}
		}
	}

	//if no server matches - defauts to first one
	if (!matchServer)
		matchServer = _config->_servers[0];

	if (_request->_file_path.size() > 1 && _request->_file_path.at(_request->_file_path.size() - 1) == '/') {
		isDIR = true;

		// std::cerr << "IS DIR : " << _request->_file_path;
		_request->_file_path = _request->_file_path.substr(0, _request->_file_path.size() - 1);
		_request->_file_name += '\0';
		// std::cerr << ", new: " << _request->_file_path << "\n";
	}

	matchLocation = buildDirPath(matchServer, dir_path, isDIR);
	if (!matchLocation)
		return createErrorResponse(404, "text/plain", "Not Found", matchServer);

	// check body size
	if (!checkBodySize(matchLocation)) {
		Logger::cerrlog(Logger::ERROR, "Response::getResponse: body size is too big");
		return createErrorResponse(413, "text/plain", "Payload Too Large", matchServer);
	}

	STR temp_str = checkRedirect(matchLocation);
	if (temp_str != "")
		return temp_str;

	if (_request->_method == "POST") {
		Logger::log(Logger::INFO, "Response::getResponse POST path " + dir_path + " isDIR " + Utils::floatToString(isDIR));

		try {
			if (!matchLocation->_upload_store.empty()) {  // this part is to be tested, upload_store
				if (dir_path.find_last_of('/') != std::string::npos) {
					dir_path = matchLocation->_upload_store + "/" + dir_path.substr(dir_path.find_last_of('/') + 1);
				} else {
					dir_path = matchLocation->_upload_store + dir_path;
				}
				Logger::log(Logger::INFO, "Response::getResponse POST path " + dir_path + " isDIR " + Utils::floatToString(isDIR));
			}
		}
		catch (const std::exception& e) {
			Logger::cerrlog(Logger::ERROR, "Response::getResponse: upload_store error: " + STR(e.what()));
		}
		return (handlePOST(dir_path));
	}

	file_path = dir_path;

	//serve file if path is a file
	if (checkFile(file_path) == NormalFile) {
		return (matchMethod(file_path, false, matchLocation));
	}
	//--server file

	//if it's not a directory return error
	if (checkFile(dir_path) != Directory)
		return createErrorResponse(404, "text/plain", "Not Found", matchServer);
	//--check dir

	// REDIRECT TO 301 path with slash


	//add index file name to file_path
	buildIndexPath(matchLocation, file_path, dir_path);

	//index file exists - serve it
	if (checkFile(file_path) == NormalFile) {
		return (matchMethod(file_path, false, matchLocation));
	}

	//index file doesn't exist - create directory if autoindex is on
	if (matchLocation->_autoindex) {
		//return directory listing
		return matchMethod(dir_path, true, matchLocation);
	} else {
		//autoindex is off
		return createErrorResponse(403, "text/plain", "Forbidden", matchLocation);
	}

	return createErrorResponse(403, "text/plain", "TERRIBLE ERROR (Impossible)", matchLocation);

}
