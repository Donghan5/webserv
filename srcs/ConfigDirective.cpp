#include "../includes/ConfigDirective.hpp"

ConfigDirective::ConfigDirective(const std::string& name, const std::vector<std::string>& params): name(name), parameters(params) {}

ConfigDirective::~ConfigDirective() {}

const std::string& ConfigDirective::getName() const { return name; }

const std::vector<std::string>& ConfigDirective::getParameters() const { return parameters; }

void ConfigDirective::addParameter(const std::string& param) {
parameters.push_back(param);
}

std::string ConfigDirective::toString(int indent) const {
	std::string result(indent, ' ');
	result += name;
	for (size_t i = 0; i < parameters.size(); ++i) {
		result += " " + parameters[i];
	}
	result += ";\n";
	return result;
}
