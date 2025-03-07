#ifndef CONFIGELEMENT_HPP
#define CONFIGELEMENT_HPP
#pragma once

#include <string>

class ConfigElement {
	public:
		virtual ~ConfigElement() {}
		virtual std::string toString(int indent = 0) const = 0;
};

#endif
