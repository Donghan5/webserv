#ifndef CONFIGBLOCK_HPP
#define CONFIGBLOCK_HPP
#pragma once

#include "ConfigElement.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Logger.hpp"

class ConfigBlock : public ConfigElement {
	private:
		std::string name;
		std::vector<std::string> parameters;
		std::vector<ConfigElement*> children;

	public:
		ConfigBlock(const std::string& name, const std::vector<std::string>& params = std::vector<std::string>());
		~ConfigBlock();

		void addDirective(const std::string& name, const std::vector<std::string>& params);

		void addBlock(ConfigBlock* block);

		void addElement(ConfigElement* element);
		const std::vector<ConfigElement*>& getChildren() const;

		const std::string& getName() const;
		const std::vector<std::string>& getParameters() const;

		std::string toString(int indent = 0) const;
};

#endif
