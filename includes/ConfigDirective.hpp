#ifndef CONFIGDIRECTIVE_HPP
#define CONFIGDIRECTIVE_HPP
#pragma once

#include <string>
#include <vector>
#include <map>
#include "ConfigElement.hpp"

class ConfigDirective : public ConfigElement {
	private:
		std::string name;
		std::vector<std::string> parameters;

		ConfigDirective();

	public:
		ConfigDirective(const std::string& name, const std::vector<std::string>& params);
		~ConfigDirective();

		const std::string& getName() const;
		const std::vector<std::string>& getParameters() const;

		void addParameter(const std::string& param);
		std::string toString(int indent = 0) const;
};

#endif
