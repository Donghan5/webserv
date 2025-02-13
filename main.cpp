#include <iostream>

/*
	pesudo maybe
*/
func startServer(???) {
	int serverSocket = createSocket();
	while (true) {
		int clientSocket = acceptConnect();
		parseRequest();

		if (cgiRequest)
			cgiHandler();
		else
			staticFileHandler();
	}
}

func sendHttpResponse() {
	if (fileNotRead)
		sendHttpError(404)
}

func sendHttpError() {
	std::endl << msg << errNum << std::endl;
}

int main(int argc, char **argv) {
	if (argc != 1) // Don't sure element argv-line
		error
	parseConfigure(pathToConfFile);
	try {
		startServer(?);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
