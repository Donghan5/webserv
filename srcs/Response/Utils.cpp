#include "Response.hpp"

bool	Response::checkBodySize(LocationConfig *matchLocation) {
	AConfigBase* local_ref = matchLocation;
	while (local_ref) {
		if (local_ref->_client_max_body_size > 0) {
			// std::cerr << "Response::checkBodySize: client_max_body_size is " << local_ref->_client_max_body_size << "\n";
			// std::cerr << "Response::checkBodySize: body size is " << _request._body.length() << "\n";
			if (_request._body.length() > (size_t)local_ref->_client_max_body_size)
				return false;
			else
				return true;
		}

		local_ref = local_ref->back_ref;
	}
	return true;
}

bool Response::ends_with(const STR &str, const STR &suffix)
{
	if (str.length() < suffix.length())
	{
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

FileType Response::checkFile(const STR& path) {
	if (path.empty()) {
		Logger::log(Logger::DEBUG, "File " + path + " not found: Empty path provided.");
        return NotFound;
    }

    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
		Logger::log(Logger::DEBUG, "File " + path + " not found. Reason: " + strerror(errno));
        return NotFound;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        return Directory;
    }
    return NormalFile;
}

STR Response::urlDecode(const STR& input) { // convert from hex to decimal
    STR result;
    result.reserve(input.length());

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '%' && i + 2 < input.length()) {
            STR hexVal = input.substr(i + 1, 2);
            
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
