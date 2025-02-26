#ifndef CONFIGDIRECTIVE_HPP
#define CONFIGDIRECTIVE_HPP
#pragma once

#include <string>
#include <vector>
#include <map>

class ConfigDirective : public ConfigElement {
	private:
		std::string name;
		std::vector<std::string> parameters;

	public:
		ConfigDirective(const std::string& name, const std::vector<std::string>& params)
			: name(name), parameters(params) {}

		const std::string& getName() const { return name; }
		const std::vector<std::string>& getParameters() const { return parameters; }

		void addParameter(const std::string& param) {
			parameters.push_back(param);
		}

		std::string toString(int indent = 0) const {
			std::string result(indent, ' ');
			result += name;
			for (size_t i = 0; i < parameters.size(); ++i) {
				result += " " + parameters[i];
			}
			result += ";\n";
			return result;
		}
};

#endif
