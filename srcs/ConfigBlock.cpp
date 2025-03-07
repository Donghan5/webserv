#include "../includes/ConfigBlock.hpp"

ConfigBlock::ConfigBlock(const std::string& name, const std::vector<std::string>& params): name(name), parameters(params) {}

ConfigBlock::~ConfigBlock() {
	for (size_t i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void ConfigBlock::addDirective(const std::string& name, const std::vector<std::string>& params) {
	children.push_back(new ConfigDirective(name, params));
}

void ConfigBlock::addBlock(ConfigBlock* block) {
	children.push_back(block);
}

void ConfigBlock::addElement(ConfigElement* element) {
	children.push_back(element);
}

const std::vector<ConfigElement*>& ConfigBlock::getChildren() const {
	return children;
}

const std::string& ConfigBlock::getName() const { return name; }

const std::vector<std::string>& ConfigBlock::getParameters() const { return parameters; }

std::string ConfigBlock::toString(int indent = 0) const {
	std::string result(indent, ' ');
	result += name;
	for (size_t i = 0; i < parameters.size(); ++i) {
		result += " " + parameters[i];
	}
	result += " {\n";
	for (size_t i = 0; i < children.size(); ++i) {
		result += children[i]->toString(indent + 4);
	}
	result += std::string(indent, ' ') + "}\n";
	return result;
}
