#include "Response.hpp"

STR Response::GenerateListing(STR path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
		Logger::log(Logger::ERROR, "Failed to open directory: " + path + " Reason: " + strerror(errno));
        return createErrorResponse(500, "text/plain", "Failed to read directory", _config);
    }

    std::stringstream html;
    html << "<html><head><title>Index of " << _request._file_path << "</title></head><body>\n";
    html << "<h1>Index of " << _request._file_path << "</h1><hr><pre>\n";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		STR name = entry->d_name;
        struct stat st;
        STR root_path = path + "/" + name;

        if (stat(root_path.c_str(), &st) == 0) {
			if (S_ISDIR(st.st_mode)){
				name += "/";
			}
			STR fullpath = _request._file_path + "/" + name;

            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));	//REDO, bad funcs!

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

			html << "<a href=\"" << fullpath << "\">"
			<< displayName << "</a>"
			<< ' '
			<< timeStr
			<< ' '
			<< st.st_size << "\n";
        }
    }

    html << "</pre><hr></body></html>";
    closedir(dir);
    return createResponse(200, "text/html", html.str(), "");
}
