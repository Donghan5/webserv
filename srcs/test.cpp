#include "../includes/ParseConf.hpp"
#include "../includes/HttpServer.hpp"

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
	try {
		std::string portStr = parser.getWebServConf().getFirstListenValue();
		std::cout << "listen: " << portStr << std::endl;
		int port = std::atoi(portStr.c_str());
		std::string root_dir = "./www";
		HttpServer server(port, root_dir);
		server.start();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
