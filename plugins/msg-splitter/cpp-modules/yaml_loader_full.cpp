// #include <iostream>
// #include <fstream>
// #include <yaml-cpp/yaml.h>

// int main() {
//     // Carica il file YAML
//     YAML::Node config = YAML::LoadFile("config.yaml");

//     // Accesso a un valore specifico
//     if (config["name"]) {
//         std::string name = config["name"].as<std::string>();
//         std::cout << "Name: " << name << std::endl;
//     }

//     // Accesso a un array
//     if (config["items"]) {
//         for (const auto& item : config["items"]) {
//             std::cout << "Item: " << item.as<std::string>() << std::endl;
//         }
//     }

//     return 0;
// }

#include <iostream>
#include <fstream>
#include <regex>
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

// Logger di esempio, puoi sostituirlo con una libreria come spdlog
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl;
#define LOG_WARNING(msg) std::cerr << "[WARNING] " << msg << std::endl;
#define LOG_CRITICAL(msg) std::cerr << "[CRITICAL] " << msg << std::endl;
#define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl;

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

            LOG_INFO("Loading configuration file: " + configfile_name);
            std::vector<YAML::Node> docs = YAML::LoadAll(file);

            for (size_t i = 0; i < docs.size(); ++i) {
                YAML::Node doc = docs[i];
                if (check_structure(doc)) {
                    LOG_INFO("Successfully loaded document " + std::to_string(i));
                    valid_docs.push_back(doc);
                } else {
                    LOG_WARNING("Document " + std::to_string(i) + " not loaded");
                }
            }
        } catch (const std::runtime_error& e) {
            LOG_CRITICAL(e.what());
            std::exit(-1);
        } catch (const YAML::ParserException& e) {
            LOG_WARNING("Error in YAML file: " + std::string(e.what()));
            return {};
        }

        return valid_docs;
    }

private:
    std::string configfile_name;

    bool check_structure(const YAML::Node& yaml_content) {
        std::map<std::string, std::regex> required_fields = {
            {"broker", std::regex(R"(^(((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4})$|^localhost$)")},
            {"port", std::regex(R"(^(?:[1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)")},
            {"inTopic", std::regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"outTopic", std::regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"retain", std::regex(R"(^(true|false)$)", std::regex_constants::icase)},
            {"function", std::regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"parameters", std::regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"outFormat", std::regex(R"(\b(json|xml|yaml|csv)\b)")},
            {"inFormat", std::regex(R"(\b(json|xml|yaml|csv)\b)")}
        };

        std::vector<std::string> wrong_fields;
        LOG_DEBUG("Spell checking...");

        for (const auto& [field, pattern] : required_fields) {
            if (!yaml_content[field]) {
                wrong_fields.push_back(field);
                continue;
            }

            if (yaml_content[field].IsSequence()) {
                for (auto v : yaml_content[field]) {
                    if (!std::regex_match(v.as<std::string>(), pattern)) {
                        wrong_fields.push_back(field);
                    }
                }
            } else {
                if (!std::regex_match(yaml_content[field].as<std::string>(), pattern)) {
                    wrong_fields.push_back(field);
                }
            }
        }

        if (!wrong_fields.empty()) {
            LOG_WARNING("The following fields are wrong or missing: ");
            for (const auto& field : wrong_fields) {
                LOG_WARNING(" - " + field);
            }
            return false;
        }

        LOG_DEBUG("Configuration file is valid");
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
