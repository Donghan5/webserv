#include <iostream>

/*
	pesudo maybe
*/
void ClassName::startServer(???) {
	int serverSocket = createSocket();
	if (serverSocket == -1) throw FailureSocketCreate();
	while (true) {
		int clientSocket = acceptConnect();
		if (clinetSocket == -1) throw FailureSocketAccess();
		parseRequest();

		if (cgiRequest)
			cgiHandler();
		else
			staticFileHandler();
	}
}

func ClassName::sendHttpResponse() {
	if (fileNotRead)
		sendHttpError(404)
}

func ClassName::sendHttpError() {
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
