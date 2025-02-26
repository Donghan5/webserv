#ifndef ACONFIGELEMENT_HPP
#define ACONFIGELEMENT_HPP
#pragma once

#include <string>
#include <vector>
#include <map>

class ConfigElement {
	public:
		virtual ~ConfigElement() {}
		virtual std::string toString(int indent = 0) const = 0;
};

#endif
