#include "../includes/ParseConf.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Logger.hpp"

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
	Logger::init();
	std::string name = argv[1];
	try {
		ParseConf parser(name);
		const WebServConf &webconf = parser.getWebServConf();

		HttpServer server(webconf);
		server.start();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
