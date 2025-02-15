// #include <iostream>

// /*
// 	pesudo maybe
// */

// void ClassName::parseConfigure(std::string pathToConfFile)
// {
// 	std::ifstream file(pathToConfFile);
// 	std::string infoLine;
// 	std::stringstream ss(infoline);
// 	if (fileNotOpen)
// 		throw FileNotOpenException();
// 	while (std::getline(file, ss))
// }
// void ClassName::startServer(???) {
// 	int serverSocket = createSocket(); // using socket(AF_INET, SOCK_STREAM, 0)
// 	if (serverSocket == -1) throw FailureSocketCreate();
// 	while (true) {
// 		int clientSocket = acceptConnect(); // listen(serversocket, 4) accept(serverSocket, NULL, NULL)
// 		if (clinetSocket == -1) throw FailureSocketAccess();
// 		parseRequest();

// 		if (cgiRequest)
// 			cgiHandler();
// 		else
// 			staticFileHandler();
// 	}
// }

// func ClassName::sendHttpResponse() {
// 	if (fileNotRead)
// 		sendHttpError(404)
// }

// func ClassName::sendHttpError() {
// 	std::endl << msg << errNum << std::endl;
// }

// int main(int argc, char **argv) {
// 	if (argc != 1) // Don't sure element argv-line
// 		error
// 	parseConfigure(pathToConfFile);
// 	try {
// 		startServer(?);
// 	} catch (const std::exception &e) {
// 		std::cerr << e.what() << std::endl;
// 	}
// }
