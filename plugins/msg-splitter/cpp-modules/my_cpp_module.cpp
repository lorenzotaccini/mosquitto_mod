// my_cpp_module.cpp

#include <iostream>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <yaml-cpp/yaml.h>
#include "mosquitto_broker.h"
#include "my_cpp_module.h"


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

FORMAT string_to_format(const std::string &format_str) {
    if (format_str == "json") return JSON;
    if (format_str == "xml") return XML;
    if (format_str == "yaml") return YML;
    if (format_str == "csv") return CSV;
    throw std::runtime_error("Formato non riconosciuto: " + format_str);
}



class Wrapper {
public:
    
    void publish(const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) {
        std::cout<<"prova"<<std::endl;

        mosquitto_broker_publish_copy(clientid,topic,payload_len,payload,qos,retain,properties);

        std::cout<<"fine"<<std::endl;
    }

    int load_yaml(const char *filename, YamlDocument **documents){
        
        YamlLoader loader(filename);
        std::vector<YAML::Node> yaml_docs = loader.load();

        for(auto &e: yaml_docs){
            std::cout<<e<<std::endl;
        }

        *documents = (YamlDocument*)malloc(yaml_docs.size() * sizeof(YamlDocument));
        if (*documents == NULL) {
            return -1;
        }

        std::cout<<"fine"<<std::endl;

        for (size_t i = 0; i < yaml_docs.size(); ++i) {
            YAML::Node doc = yaml_docs[i];

            // Carica in_topic e out_topic
            strncpy((*documents)[i].in_topic, doc["inTopic"].as<std::string>().c_str(), sizeof((*documents)[i].in_topic) - 1);
            (*documents)[i].in_topic[sizeof((*documents)[i].in_topic) - 1] = '\0'; // Assicura che sia null-terminato

            strncpy((*documents)[i].out_topic, doc["outTopic"].as<std::string>().c_str(), sizeof((*documents)[i].out_topic) - 1);
            (*documents)[i].out_topic[sizeof((*documents)[i].out_topic) - 1] = '\0'; // Assicura che sia null-terminato


            // Retain
            (*documents)[i].retain = doc["retain"].as<bool>();

            // Funzioni (array dinamico di stringhe)
            (*documents)[i].num_functions = doc["function"].size();
            (*documents)[i].functions = (char**)malloc((*documents)[i].num_functions * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_functions; ++j) {
                (*documents)[i].functions[j] = strdup(doc["function"][j].as<std::string>().c_str());
            }

            // Parametri (array dinamico di stringhe)
            (*documents)[i].num_parameters = doc["parameters"].size();
            (*documents)[i].parameters = (char**)malloc((*documents)[i].num_parameters * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_parameters; ++j) {
                (*documents)[i].parameters[j] = strdup(doc["parameters"][j].as<std::string>().c_str());
            }

            // Formati di input e output
            (*documents)[i].in_format = string_to_format(doc["inFormat"].as<std::string>());
            (*documents)[i].out_format = string_to_format(doc["outFormat"].as<std::string>());
        }

        // Restituisce il numero di documenti caricati
        return yaml_docs.size();
    }


    
};

extern "C" {
    Wrapper* wrapper_new() { return new Wrapper(); }
    void wrapper_publish(Wrapper* instance, const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) { 
        instance->publish(clientid,topic,payload_len,payload,qos,retain,properties); 
    }
    int wrapper_load_yaml(Wrapper* instance, const char *filename, YamlDocument **documents){ 
        return instance->load_yaml(filename, documents); 
    }
    void wrapper_free_docs_mem(YamlDocument *docs, int n_docs){int I;};
    void wrapper_delete(Wrapper* instance) { delete instance; }
}
