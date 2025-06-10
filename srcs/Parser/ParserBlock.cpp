#include "Parser/Parser.hpp"
#include "Utils.hpp"

// create block from line (http, server, location)
AConfigBase	*Parser::CreateBlock(STR line, int start) {
	AConfigBase *block;
	VECTOR<STR>	tokens;
	STR			trimmed_line;
	STR			block_name;
	STR			block_body;
	int	close_brace = line.find('}', start);

	trimmed_line = line.substr(start, close_brace - start);
	block_name = trimmed_line.substr(0, trimmed_line.find('{'));
	tokens = Utils::split(block_name, ' ', 1);

	block = NULL;
	if (tokens[0] == "http") {
		HttpConfig *conf = new HttpConfig();
		block = conf;
	} else if (tokens[0] == "server") {
		ServerConfig *conf = new ServerConfig();
		block = conf;
	} else if (tokens[0] == "location") {
		LocationConfig *conf = new LocationConfig();
		conf->_path = tokens[1];
		block = conf;
	}

	return block;
}

// add block to parent block (http, server, location)
AConfigBase	*Parser::AddBlock(AConfigBase *prev_block, STR line, int start) {
	AConfigBase	*child = NULL;
	ConfigBlock parent_type = ERROR;

	parent_type = prev_block->_identify(prev_block);
	if (parent_type == HTTP) {
		HttpConfig* httpConf = dynamic_cast<HttpConfig*>(prev_block);
		ServerConfig* serverConf;

		child = CreateBlock(line, start);
		if (!child || !httpConf || httpConf->_identify(child) != SERVER) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		serverConf = dynamic_cast<ServerConfig*>(child);
		httpConf->_servers.push_back(serverConf);
		child->back_ref = prev_block;
	} else if (parent_type == SERVER) {
		ServerConfig* serverConf = dynamic_cast<ServerConfig*>(prev_block);
		LocationConfig* location;

		child = CreateBlock(line, start);
		if (!child || !serverConf || serverConf->_identify(child) != LOCATION) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		location = dynamic_cast<LocationConfig*>(child);

		if (!ParserUtils::check_location_path_duplicate(location->_path, serverConf->_locations)) {
			child->_self_destruct();

			Logger::log(Logger::ERROR, "Location has duplicate " + location->_path);

			return NULL;
		}

		serverConf->_locations[location->_path] = location;
		// location->back_ref = serverConf;
		child->back_ref = prev_block;
	} else if (parent_type == LOCATION) {
		LocationConfig* locParent = dynamic_cast<LocationConfig*>(prev_block);
		LocationConfig* locChild;

		// this is changed, adding verication dup server name
		ServerConfig* serverConf = NULL;

		child = CreateBlock(line, start);
		if (!child || !locParent || locParent->_identify(child) != LOCATION) {
			if (child)
				child->_self_destruct();
			return NULL;
		}
		locChild = dynamic_cast<LocationConfig*>(child);

		// find server
		AConfigBase* temp = prev_block;
		while (temp && temp->_identify(temp) != HTTP) {
			if (temp->_identify(temp) == SERVER) {
				serverConf = dynamic_cast<ServerConfig*>(temp);
				break;
			}
			temp = temp->back_ref;
		}

		if (!serverConf || serverConf->_identify(serverConf) != SERVER) {
			if (child)
				child->_self_destruct();
			Logger::log(Logger::ERROR, "Location server not found");
			return NULL;
		}

		// new thing, check dup server name
		if (!ParserUtils::check_location_path_duplicate(locChild->_path, serverConf->_locations)) {
			child->_self_destruct();

			Logger::log(Logger::ERROR, "Location has duplicate " + locChild->_path);

			return NULL;
		}

		locParent->_locations[locChild->_path] = locChild;
		// locChild->back_ref = locParent;
		child->back_ref = prev_block;
	} else {
		std::stringstream ss;
		ss << "Block type error: " << static_cast<int>(parent_type);
		Logger::log(Logger::ERROR, ss.str());
		return NULL;
	}
	return child;
}
