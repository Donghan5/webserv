#ifndef EVENTCONF_HPP
#define EVENTCONF_HPP
#pragma once

#include <iostream>
#include <set>

typedef std::set<std::string> string_set;

class EventConf {
	private:
		int _workerConnections;
		std::string _useMethods;

	public:
		EventConf();
		EventConf(int workerConnections, std::string useMethods);
		EventConf(const EventConf &obj);
		~EventConf();

		EventConf &operator=(const EventConf &obj);
		void setWorkerConnections(int workerConnections);
		void setUseMethods(std::string useMethods);
		int getWorkerConnections(void);
		const std::string &getUseMethods(void) const;
};

#endif
