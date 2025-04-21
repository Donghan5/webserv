#ifndef PARSER_HPP
# define PARSER_HPP
# include "HttpConfig.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"
# include <iostream>
# include <fstream>
# include <limits.h>

enum ElemType {
    BLOCK,
	BLOCK_END,
	DIRECTIVE,
	BAD_TYPE
};

struct Directive {
	STR					name;
	VECTOR<STR>	values;
};


struct Block {
	STR						name;
	VECTOR<Directive>	directives;
};


class Parser {
	private:
		HttpConfig		*_config;
		std::ifstream	_file;
		STR				_filepath;
		STR				_full_config;
		// int				_position;

		ElemType	DetectNextType(STR line, int position, int &block_size);
		bool		ValidateConfig(STR full_config);

	public:
		Parser();
		Parser(STR file);
		Parser(const Parser &obj);
		Parser &operator=(const Parser &obj);
		~Parser();

		HttpConfig	*Parse();
		static long long	verifyClientMaxBodySize(std::string client_max_body_size_str);
		static bool			verifyAutoIndex(std::string autoindex_str);
		static int 			verifyPort(std::string port_str);
};

#endif
