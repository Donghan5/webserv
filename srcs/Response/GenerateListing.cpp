#include "Response.hpp"
#include "Logger.hpp"

STR Response::GenerateListing(STR path, bool isRedirect) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
		Logger::log(Logger::ERROR, "GET " + path + " 500 Internal Server Error - Cannot read directory: " + STR(strerror(errno)));
        return createErrorResponse(500, "text/plain", "Failed to read directory", _config);
    }

    std::stringstream html;
    html << "<!DOCTYPE html><html><head><title>Index of " << _request._file_path << "</title></head><body>\n";
    html << "<h1>Index of " << _request._file_path << "</h1><hr><table>\n";
	html << "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		STR name = entry->d_name;
		if (name == ".") continue;

        struct stat st;
        STR root_path = path + "/" + name;

        if (stat(root_path.c_str(), &st) == 0) {
			if (S_ISDIR(st.st_mode)){
				name += "/";
			}
			STR fullpath = _request._file_path + "/" + name;

            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M", localtime(&st.st_mtime));

            STR displayName = urlDecode(name);

			// if (displayName.length() >= 45)
			// 	displayName = displayName.substr(0, 42) + "...";

			// std::cerr << "DEBUG Response::handleDIR: displayName is " << displayName << "\n";

			// html << "<a href=\"" << fullpath << "\">"
			// << displayName << "</a>"
			// << STR(50 - displayName.length(), ' ')
			// << timeStr
			// << STR(20, ' ')
			// << st.st_size << "\n";

			html << "<tr><td>\n";
			html << "<a href=\"" << fullpath << "\">"
			<< displayName << "</a></td>"
			<< ' '
			<< "<td>" << st.st_size << "</td>"
			<< ' '
			<< "<td>" << timeStr << "</td>\n";
        }
    }

    html << "</table><hr></body></html>";
    closedir(dir);
	if (isRedirect) {
		Logger::log(Logger::INFO, "GET " + path + " 301 Moved Permanently - Directory listing generated");
		return createResponse(301, "text/html", html.str(), "Location: " + _request._file_path + "/\r\n");
	}
    return createResponse(200, "text/html", html.str(), "");
}
