#include "../includes/EventConf.hpp"

EventConf::EventConf(): _workerConnections(0), _useMethods("") {}
EventConf::EventConf(int workerConnections, std::string useMethods): _workerConnections(workerConnections), _useMethods(useMethods) {};

EventConf::~EventConf() {} // destructor

EventConf::EventConf(const EventConf &obj) {
	*this = obj;
}

EventConf &EventConf::operator=(const EventConf &obj) {
	if (this != &obj) {
		this->_workerConnections = obj._workerConnections;
		this->_useMethods = obj._useMethods;
	}
	return (*this);
}

/*
	Setting worker_connections
		- it must be at least 1
*/
void EventConf::setWorkerConnections(int workerConnections) {
	if (workerConnections < 1) {
		throw std::invalid_argument("worker_connections must be at least 1");
	}
	this->_workerConnections = workerConnections;
}

/*
	Settings methods
		- it should be one of commands which are "select", "poll", "kqueue", "eventport", "devpoll"
*/
void EventConf::setUseMethods(std::string useMethods) {
	const std::string validMethodsArray[] = {"select", "poll", "kqueue", "eventport", "devpoll"};
	const string_set validMethods(validMethodsArray, validMethodsArray + 5);
	if (validMethods.find(useMethods) == validMethods.end()) { // verfy real argument
		std::invalid_argument("Invalid argument: " + useMethods);
	}
	this->_useMethods = useMethods;
}

int EventConf::getWorkerConnections(void) {
	return this->_workerConnections;
}

const std::string &EventConf::getUseMethods(void) const {
	return this->_useMethods;
}
