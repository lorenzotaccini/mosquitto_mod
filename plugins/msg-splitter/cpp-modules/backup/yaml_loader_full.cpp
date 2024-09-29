#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <yaml-cpp/yaml.h>

//#include <mosquitto.h>


#include "process.hpp"

class YamlLoader {
public:
    YamlLoader(const std::string& configfile_name)
        : configfile_name(configfile_name) {}

    std::vector<YAML::Node> load() {
        std::vector<YAML::Node> valid_docs;
        try {
            std::ifstream file(configfile_name);
            if (!file.is_open()) {
                throw std::runtime_error("Config file not found");
            }
            std::cout<< "Loading configuration file: "<<configfile_name.c_str();
            std::vector<YAML::Node> docs = YAML::LoadAll(file);

            for (size_t i = 0; i < docs.size(); ++i) {
                YAML::Node doc = docs[i];
                if (check_structure(doc)) {
                    std::cout<<"Successfully loaded document "<<std::to_string(i).c_str();
                    valid_docs.push_back(doc);
                } else {
                    std::cout<<"Document not loaded: "<<std::to_string(i).c_str();
                }
            }
        } catch (const std::runtime_error& e) {
            std::cout<<std::string(e.what()).c_str();
            std::exit(-1);
        } catch (const YAML::ParserException& e) {
            std::cout<<"Error in YAML file: "<<std::string(e.what()).c_str();
            return {};
        }

        return valid_docs;
    }

private:
    std::string configfile_name;

    bool check_structure(const YAML::Node& yaml_content) {
        std::map<std::string, std::regex> required_fields = {
            {"inTopic", std::regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"outTopic", std::regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"retain", std::regex(R"(^(true|false)$)", std::regex_constants::icase)},
            {"function", std::regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"parameters", std::regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"outFormat", std::regex(R"(\b(json|xml|yaml|csv)\b)")},
            {"inFormat", std::regex(R"(\b(json|xml|yaml|csv)\b)")}
        };

        std::vector<std::string> wrong_fields;
        std::cout<<"Spell checking...";

        for (const auto& [field, pattern] : required_fields) {
            if (!yaml_content[field]) {
                wrong_fields.push_back(field);
            }
            else if (yaml_content[field].IsSequence()) {
                for (auto v : yaml_content[field]) {
                    if (!std::regex_match(v.as<std::string>(), pattern)) {
                        wrong_fields.push_back(field);
                    }
                }
            } 
            else {
                if (!std::regex_match(yaml_content[field].as<std::string>(), pattern)) {
                    wrong_fields.push_back(field);
                }
            }
        }

        if (!wrong_fields.empty()) {
            std::cout<<"The following fields are wrong or missing: ";
            for (const auto& field : wrong_fields) {
                std::cout<<(" - " + field)<<std::endl;
            }
            return false;
        }

        std::cout<<"Configuration file is valid"<<std::endl;
        return true;
    }
};

int main() {
    YamlLoader loader("config.yml");
    auto documents = loader.load();

    for (const auto& doc : documents) {
        std::cout << "Loaded document: " << doc << std::endl;
    }

    return 0;
}



