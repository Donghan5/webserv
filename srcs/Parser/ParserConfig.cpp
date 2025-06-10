#include "Parser/Parser.hpp"

ElemType	Parser::DetectNextType(STR line, int position, int &block_size) {
	int	semicol = line.find(';', position);
	int	open_brace = line.find('{', position);
	int	close_brace = line.find('}', position);
	static int depth;

	block_size = 0;

	VECTOR<STR> last_char_check = Utils::split(line.c_str() + position, ' ', 1);
	if (last_char_check.size() == 1 &&
		last_char_check[0].size() == 1 &&
		last_char_check[0][0] == '}' &&
		depth != 1) {
			return BAD_TYPE;
	}

	if (semicol != CHAR_NOT_FOUND) {
		if (open_brace != CHAR_NOT_FOUND && close_brace != CHAR_NOT_FOUND) {
			if (semicol < open_brace && semicol < close_brace) {
				if (ParserUtils::isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else if (open_brace != CHAR_NOT_FOUND) {
			if (semicol < open_brace) {
				if (ParserUtils::isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else if (close_brace != CHAR_NOT_FOUND) {
			if (semicol < close_brace) {
				if (ParserUtils::isDirectiveOk(line, position, semicol)) {
					block_size = semicol - position + 1;
					return DIRECTIVE;
				}
				else
					return BAD_TYPE;
			}
		} else {	//no open and no close braces
			if (ParserUtils::isDirectiveOk(line, position, semicol)) {
				block_size = semicol - position + 1;
				return DIRECTIVE;
			}
			else
				return BAD_TYPE;
		}
	}
	if (open_brace != CHAR_NOT_FOUND && close_brace != CHAR_NOT_FOUND) {
		if (close_brace < open_brace && depth <= 0)
			return BAD_TYPE;
		if (close_brace < open_brace) {
			if (ParserUtils::isBlockEndOk(line, position)) {
				block_size = close_brace - position + 1;
				depth--;
				return BLOCK_END;
			}
		} else if (ParserUtils::isBlockOk(line, position, close_brace)) {
			block_size = open_brace - position + 1;
			depth++;
			return BLOCK;
		}
	} else if (close_brace != CHAR_NOT_FOUND) {
		if (depth <= 0)
			return BAD_TYPE;
		if (ParserUtils::isBlockEndOk(line, position)) {
			block_size = close_brace - position + 1;
			depth--;
			return BLOCK_END;
		}
	}
	return BAD_TYPE;
}

// checking the configuration file
bool	Parser::ValidateConfig(STR full_config) {
	int	size = 1;

	for (size_t i = 0; i < full_config.size(); i += size)
	{
		switch (DetectNextType(full_config, i, size))
		{
		case DIRECTIVE:
			// std::cerr << (full_config.c_str() + i) << "\nTYPE: DIRECTIVE\n\n\n\n";
			break;
		case BLOCK:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BLOCK\n\n";
			break;
		case BLOCK_END:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BLOCK_END\n\n";
			break;
		default:
			// std::cerr << (full_config.c_str() + i) << "\n\n TYPE: BAD\n\n";
			size = -1;
			break;
		}
		if (size == -1)
			break;
	}
	if (size == -1)
		return false;
	return true;
}
