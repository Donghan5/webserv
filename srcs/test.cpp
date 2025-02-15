#include "ParseConf.hpp"

/*
*
*	To test the parsing part
*
*/
int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "USAGE: ./webserver xxx.conf" << std::endl;
		return 1;
	}
	std::string name = argv[1];
	ParseConf parser(name);
	return 0;
}
