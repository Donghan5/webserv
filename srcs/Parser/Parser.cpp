#include "Parser/Parser.hpp"
#include "Logger.hpp"

Parser::Parser() {
	_config = new HttpConfig();
}

Parser::Parser(STR file) {
	_config = new HttpConfig();

	_filepath = file;
}

Parser::Parser(const Parser &obj) {
	_config = obj._config;
	_filepath = obj._filepath;
}

Parser &Parser::operator=(const Parser &obj) {
	(void)obj;
	return (*this);
}

Parser::~Parser() {
	if (_config)
		delete _config;
}

// ---- Main parsing function ----
HttpConfig *Parser::Parse() {
	STR			line;
	VECTOR<STR>	tokens;
	STR			full_config;

	if (_filepath == "")
		_file.open("config.conf");
	else
		_file.open(_filepath.c_str());
	if (!_file.is_open()) {
		Logger::log(Logger::ERROR, "Could not open file " + _filepath);
		return (NULL);
	}

	STR temp_line;
	while (getline(_file, temp_line)) {
		full_config += temp_line;
    }
	_file.close();

	if (!ValidateConfig(full_config)) {
		Logger::log(Logger::ERROR, "Config structural integrity is violated");
		return NULL;
	}

	int	size = 1;

	HttpConfig *base = new HttpConfig;
	base->back_ref = NULL;
	AConfigBase	*currentBlock = base;

	bool skipped_block1 = -1;
	bool skipped_block2 = -1;
	MAP<int, int>	directives_per_block;
	int depth = 0;
	for (size_t i = 0; i < full_config.size(); i += size)
	{

		switch (DetectNextType(full_config, i, size))
		{
		case DIRECTIVE:
			directives_per_block[depth]++;
			Logger::log(Logger::DEBUG, "DIRECTIVE (" + full_config.substr(i, 50) + ")");
			if (!ParserFiller::FillDirective(currentBlock, full_config, i)) {
				base->_self_destruct();

				Logger::log(Logger::DEBUG, "CHECK - FillDirective failed");
				return NULL;
			}
			break;
		case BLOCK:
			depth++;
			directives_per_block[depth] = 0;
			Logger::log(Logger::DEBUG, "BLOCK (" + full_config.substr(i, 50) + ")");
			//if event/http, we already have default one
			if (!strncmp(full_config.c_str() + i, "http", 4) || !strncmp(full_config.c_str() + i, "events", 6)) {
				skipped_block1 = depth;
				continue;
			}

			currentBlock = AddBlock(currentBlock, full_config, i);
			if (!currentBlock) {
				base->_self_destruct();

				return NULL;
			}
			Logger::log(Logger::DEBUG, "--BLOCK");
			break;
		case BLOCK_END:
			Logger::log(Logger::DEBUG, "BLOCK_END (" + full_config.substr(i, 50) + ")");
			if (skipped_block1 == depth)
				skipped_block1 = -1;
			else if (skipped_block2 == depth)
				skipped_block2 = -1;
			else
				currentBlock = currentBlock->back_ref;
			if (directives_per_block[depth] == 0) {
				base->_self_destruct();

				Logger::log(Logger::ERROR, "Block doesn't have it's own directives!");
				return NULL;
			}
			depth--;
			break;
		default:
			size = -1;
			break;
		}
		if (size == -1)
			break;
	}

	if (base->_identify(currentBlock) != HTTP || currentBlock != base || depth != 0) {
		base->_self_destruct();
		return NULL;
	}

	if (!ParserUtils::minimum_value_check(base)) {
		base->_self_destruct();
		return NULL;
	}

	return (base);
}
