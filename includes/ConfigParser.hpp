
#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

class ConfigParser {
private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.length() && (std::isspace(input[pos]) || input[pos] == '#')) {
            if (input[pos] == '#') {
                while (pos < input.length() && input[pos] != '\n') {
                    pos++;
                }
            }
            pos++;
        }
    }

    std::string readUntil(const std::string& delimiters) {
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

    ConfigDirective* parseDirective() {
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

    ConfigBlock* parseBlock() {
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

    ConfigElement* parseElement() {
        skipWhitespace();
        if (pos >= input.length()) return NULL;

        size_t savedPos = pos;
        ConfigElement* directive = parseDirective();
        if (directive) return directive;

        pos = savedPos;
        return parseBlock();
    }

public:
    Config* parseString(const std::string& configString) {
        input = configString;
        pos = 0;
        Config* config = new Config();

        try {
            while (pos < input.length()) {
                ConfigElement* element = parseElement();
                if (element) {
                    config->addElement(element);
                }
                skipWhitespace();
            }
        } catch (const std::exception& e) {
            delete config;
            throw std::runtime_error("Parse error at position " + SSTR(pos) + ": " + e.what());
        }

        return config;
    }

    Config* parseFile(const std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return parseString(buffer.str());
    }
};

#endif // CONFIGPARSER_HPP