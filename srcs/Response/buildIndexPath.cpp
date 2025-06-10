#include "Response.hpp"

void	Response::searchIndex(VECTOR<STR> indexes, STR &best_match, float &match_quality, STR dir_path) {
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
			if (_request._accepted_types[index_mime] > match_quality) {
				Logger::log(Logger::DEBUG, index_mime + " is better match that" + best_match + "! Quality " + Utils::floatToString(_request._accepted_types[index_mime]) + " is better than " + Utils::floatToString(match_quality));
				best_match = indexes[i];
				match_quality = _request._accepted_types[index_mime];
			}
			else
				Logger::log(Logger::DEBUG, index_mime + " is not more than " + Utils::floatToString(match_quality));
		}
		catch(const std::exception& e)
		{
			Logger::log(Logger::DEBUG, index_mime + " is not accepted: " + index_mime);
		}
		try
		{
			if (_request._accepted_types["*/*"] > match_quality) {
				Logger::log(Logger::DEBUG, "*/* is the better match than " + best_match
					+ "! Quality " + Utils::floatToString(_request._accepted_types["*/*"]) + " is better than " + Utils::floatToString(match_quality));
				best_match = indexes[i];
				match_quality = _request._accepted_types["*/*"];
			}
			else
				Logger::log(Logger::DEBUG, "*/* is not more than " + Utils::floatToString(match_quality));
		}
		catch(const std::exception& e)
		{
			Logger::log(Logger::DEBUG, "*/* is not accepted: " + index_mime);
		}

		i++;
	}
}

STR	Response::buildIndex(LocationConfig* location, STR dir_path) {
	STR 		best_match = "";
	float		match_quality = 0.0;

	/*
		Searching on best existing level for index match with best quality.

		quality is not needed - remove later
	*/
	AConfigBase* local_ref = location;
	while (local_ref) {
		if (!local_ref->_index.empty()) {
			searchIndex(local_ref->_index, best_match, match_quality, dir_path);
			if (match_quality == 1)
				break;
		}
		local_ref = local_ref->back_ref;
	}

	if (best_match == "")
		throw std::runtime_error("No index match");
	else
		Logger::log(Logger::DEBUG, "Response::selectIndexAll: best_match is " + best_match + ", quality: " + Utils::floatToString(match_quality));
	return best_match;
}


int Response::buildIndexPath(LocationConfig *matchLocation, STR &best_file_path, STR dir_path) {
	if (best_file_path != "/" && best_file_path[best_file_path.length() - 1] != '/')
		best_file_path.append("/");

	//if request doesn't have file name - searching for index
	try
	{
		best_file_path.append(buildIndex(matchLocation, dir_path));
		Logger::log(Logger::DEBUG, "Response::buildFilePath: AFT best_file_path is " + best_file_path);
	}
	catch(const std::exception& e)
	{
		// return createResponse(403, "text/plain", "NO SUCH FILE FOUND (change later)", "");

		Logger::log(Logger::DEBUG, "Response::buildFilePath: no index file found");
		return 0;
	}

	Logger::log(Logger::DEBUG, "Response::buildFilePath: index file is " + best_file_path);
	return 1;
}
