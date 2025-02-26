#include "../includes/ConfigParser.hpp"

void ConfigParser::skipWhitespace(void) {
	while (pos < input.length() && (std::isspace(input[pos]) || input[pos] == '#')) {
		if (input[pos] == '#') {
			while (pos < input.length() && input[pos] != '\n') {
				pos++;
			}
		}
		pos++;
	}
}

std::string ConfigParser::readUntil(const std::string &delimiters) {
	std::string result;
	while (pos < input.length() && delimiters.find(input[pos]) == std::string::npos) {
		if (input[pos] == '"' || input[pos] == '\'') {
			char quote = input[pos++];
			while (pos < input.length() && input[pos] != quote) {
				result += input[pos++];
			}
			if (pos < input.length()) pos++;
		} else if (input[pos] == '\\') {
			pos++;
			if (pos < input.length()) {
				result += input[pos++];
			}
		} else {
			result += input[pos++];
		}
	}
	return result;
}

ConfigDirective *ConfigParser::parseDirective(void) {
	skipWhitespace();
	std::string name = readUntil(" \t\n{;");

	std::vector<std::string> parameters;
	skipWhitespace();

	while (pos < input.length() && input[pos] != ';' && input[pos] != '{') {
		std::string param = readUntil(" \t\n{;");
		if (!param.empty()) {
			parameters.push_back(param);
		}
		skipWhitespace();
	}

	if (pos < input.length() && input[pos] == ';') {
		pos++;
		return new ConfigDirective(name, parameters);
	}

	return NULL;
}

ConfigBlock* ConfigParser::parseBlock(void) {
	skipWhitespace();
	std::string name = readUntil(" \t\n{");

	std::vector<std::string> parameters;
	skipWhitespace();

	while (pos < input.length() && input[pos] != '{') {
		std::string param = readUntil(" \t\n{");
		if (!param.empty()) {
			parameters.push_back(param);
		}
		skipWhitespace();
	}

	if (pos < input.length() && input[pos] == '{') {
		pos++;

		// create server block
		if (name == "server") {
			ServerConf* serverConf = new ServerConf();

			while (pos < input.length() && input[pos] != '}') {
				ConfigElement* element = parseElement();
				if (element) {
					ConfigDirective* directive = dynamic_cast<ConfigDirective*>(element);
					ConfigBlock* subBlock = dynamic_cast<ConfigBlock*>(element);

					if (directive) {
						std::string directiveName = directive->getName();
						std::vector<std::string> params = directive->getParameters();

						if (!params.empty()) {
							serverConf->setData(directiveName, params[0]);
						}
					}
					else if (subBlock && subBlock->getName() == "location") {
						LocationConf locationConf;

						std::vector<std::string> params = subBlock->getParameters();
						if (!params.empty()) {
							locationConf.setPath(params[0]);
						}

						// `location` 내부 설정 추가
						const std::vector<ConfigElement*>& children = subBlock->getChildren();
						for (size_t i = 0; i < children.size(); ++i) {
							ConfigDirective* subDirective = dynamic_cast<ConfigDirective*>(children[i]);
							if (subDirective) {
								std::vector<std::string> subParams = subDirective->getParameters();
								if (!subParams.empty()) {
									locationConf.setData(subDirective->getName(), subParams[0]);
								}
							}
						}

						serverConf->addLocation(locationConf);
					}
				}
				skipWhitespace();
			}

			if (pos < input.length() && input[pos] == '}') {
				pos++;
				return serverConf;  // return ServerConf
			}
		}

		// generate LocationConf
		else if (name == "location") {
			LocationConf* locationConf = new LocationConf();

			if (!parameters.empty()) {
				locationConf->setPath(parameters[0]);
			}

			while (pos < input.length() && input[pos] != '}') {
				ConfigElement* element = parseElement();
				if (element) {
					ConfigDirective* directive = dynamic_cast<ConfigDirective*>(element);
					if (directive) {
						std::vector<std::string> params = directive->getParameters();
						if (!params.empty()) {
							locationConf->setData(directive->getName(), params[0]);
						}
					}
				}
				skipWhitespace();
			}

			if (pos < input.length() && input[pos] == '}') {
				pos++;
				return locationConf;  // return locationConf
			}
		}

		// basic config
		ConfigBlock* block = new ConfigBlock(name, parameters);
		while (pos < input.length() && input[pos] != '}') {
			ConfigElement* element = parseElement();
			if (element) {
				block->addElement(element);
			}
			skipWhitespace();
		}

		if (pos < input.length() && input[pos] == '}') {
			pos++;
			return block;
		}
	}

	throw std::runtime_error("Expected block structure");
}

ConfigElement *parseElement(void) {
	skipWhitespace();
	if (pos >= input.length()) return NULL;

	size_t savedPos = pos;
	ConfigElement* directive = parseDirective();
	if (directive) return directive;

	pos = savedPos;
	return parseBlock();
}

ConfigBlock *ConfigParser::parseString(cosnt std;:string &configString) {
	input = configString;
	pos = 0;

	ConfigBlock *rootBlock = new ConfigBlock("root");

	try {
		while (pos < input.length()) {
			ConfigElement* element = parseElement();
			if (element) {
				rootBlock->addElement(element);
			}
			skipWhitespace();
		}
	} catch (const std::exception& e) {
		delete rootBlock;
		throw std::runtime_error("Parse error at position " + SSTR(pos) + ": " + e.what());
	}

	return rootBlock;
}

ConfigBlock *ConfigParser::parseFile(const std::string &filename) {
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Unable to open file: " + filename);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return parseString(buffer.str());
}

