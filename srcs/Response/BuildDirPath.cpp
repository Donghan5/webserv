#include "Response.hpp"

STR	regress_path(STR path) {
	if (path.find_last_of("/") == STR::npos)
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

LocationConfig *Response::buildDirPath(ServerConfig *matchServer, STR &full_path, bool &isDIR) {
	LocationConfig *matchLocation = NULL;
	STR path_to_match = _request._file_path;

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
			Logger::log(Logger::DEBUG, "Regressed path " + path_to_match + " -> " + regress_path(path_to_match));
			path_to_match = regress_path(path_to_match);
		}
	}

	if (!matchLocation)
		return NULL;

	STR relative_path = "";

	// add root path
	AConfigBase* local_ref = matchLocation;
	while (local_ref) {
		if (local_ref->_root != "") {
			relative_path.append(local_ref->_root);
			break;
		}
		local_ref = local_ref->back_ref;
	}

	relative_path.append(_request._file_path);

	STR absolute_path = _request._file_path;

	// check alias
	if (matchLocation->_alias != "") {
		if (matchLocation->_alias[0] != '/')
			matchLocation->_alias = "/" + matchLocation->_alias;
		size_t pos = relative_path.find(path_to_match);
		size_t pos_abs = absolute_path.find(path_to_match);

		if (pos != STR::npos) {
			STR new_path = relative_path;
			new_path.replace(pos, relative_path.length(), matchLocation->_alias);

			relative_path = new_path;

			Logger::log(Logger::DEBUG, "Applied alias: replaced '" + path_to_match +
						"' with '" + matchLocation->_alias + "' => '" + relative_path + "'");
		} else {
			Logger::log(Logger::WARNING, "Could not apply alias: location path not found in request path");
		}

		if (pos_abs != STR::npos) {
			STR new_path = absolute_path;
			new_path.replace(pos_abs, absolute_path.length(), matchLocation->_alias);

			absolute_path = new_path;
		} else {
			Logger::log(Logger::WARNING, "Could not apply alias: location path not found in request path");
		}
	}

	// check which path exists - relative or absolute
	if (checkFile(relative_path) != NotFound) {
		Logger::log(Logger::DEBUG, "Response::buildDirPath: relative path is " + relative_path);
		full_path = relative_path;
	} else if (checkFile(absolute_path) != NotFound) {
		Logger::log(Logger::DEBUG, "Response::buildDirPath: absolute path is " + absolute_path);
		full_path = absolute_path;
	} else if (!isDIR && checkFile(regress_path(relative_path)) != NotFound) {
		Logger::log(Logger::DEBUG, "Response::buildDirPath: relative path is " + relative_path);
		full_path = relative_path;
	} else if (!isDIR && checkFile(regress_path(absolute_path)) != NotFound) {
		Logger::log(Logger::DEBUG, "Response::buildDirPath: absolute path is " + absolute_path);
		full_path = relative_path;
	} else {
		Logger::log(Logger::INFO, "No such file or directory \"" + full_path + "\" for " + _request._file_path + "!");
	}

	Logger::log(Logger::DEBUG, "Response::buildDirPath: dir path is " + full_path);
	return matchLocation;
}
