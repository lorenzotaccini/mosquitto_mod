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

using namespace std;


class YamlLoader {
public:
    YamlLoader(const string& configfile_name)
        : configfile_name(configfile_name) {}

    vector<YAML::Node> load() {
        vector<YAML::Node> valid_docs;
        try {
            ifstream file(configfile_name);
            if (!file.is_open()) {
                throw runtime_error("Config file not found");
            }
            cout<< "Loading configuration file: "<<configfile_name.c_str();
            vector<YAML::Node> docs = YAML::LoadAll(file);

            for (size_t i = 0; i < docs.size(); ++i) {
                YAML::Node doc = docs[i];
                if (check_structure(doc)) {
                    cout<<"Successfully loaded document "<<to_string(i).c_str();
                    valid_docs.push_back(doc);
                } else {
                    cout<<"Document not loaded: "<<to_string(i).c_str();
                }
            }
        } catch (const runtime_error& e) {
            cout<<string(e.what()).c_str();
            exit(-1);
        } catch (const YAML::ParserException& e) {
            cout<<"Error in YAML file: "<<string(e.what()).c_str();
            return {};
        }

        return valid_docs;
    }

private:
    string configfile_name;

    bool check_structure(const YAML::Node& yaml_content) {
        map<string, regex> required_fields = {
            {"inTopic", regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"outTopic", regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
            {"retain", regex(R"(^(true|false)$)", regex_constants::icase)},
            {"function", regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"parameters", regex(R"(^([a-zA-Z0-9_\-])+$)")},
            {"outFormat", regex(R"(\b(json|xml|yaml|csv)\b)")},
            {"inFormat", regex(R"(\b(json|xml|yaml|csv)\b)")}
        };

        vector<string> wrong_fields;
        cout<<"Spell checking...";

        for (const auto& [field, pattern] : required_fields) {
            if (!yaml_content[field]) {
                wrong_fields.push_back(field);
                continue;
            }

            if (yaml_content[field].IsSequence()) {
                for (auto v : yaml_content[field]) {
                    if (!regex_match(v.as<string>(), pattern)) {
                        wrong_fields.push_back(field);
                    }
                }
            } else {
                if (!regex_match(yaml_content[field].as<string>(), pattern)) {
                    wrong_fields.push_back(field);
                }
            }
        }

        if (!wrong_fields.empty()) {
            cout<<"The following fields are wrong or missing: ";
            for (const auto& field : wrong_fields) {
                cout<<(" - " + field)<<endl;
            }
            return false;
        }

        cout<<"Configuration file is valid"<<endl;
        return true;
    }
};

FORMAT string_to_format(const string &format_str) {
    if (format_str == "json") return JSON;
    if (format_str == "xml") return XML;
    if (format_str == "yaml") return YML;
    if (format_str == "csv") return CSV;
    throw runtime_error("Formato non riconosciuto: " + format_str);
}



class Wrapper {
public:
    
    void publish(const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) {
        mosquitto_broker_publish_copy(clientid,topic,payload_len,payload,qos,retain,properties);
    }

    int load_yaml(const char *filename, YamlDocument **documents){
        
        YamlLoader loader(filename);
        vector<YAML::Node> yaml_docs = loader.load();

        for(auto &e: yaml_docs){
            cout<<e<<endl;
        }

        *documents = (YamlDocument*)mosquitto_calloc(1,yaml_docs.size() * sizeof(YamlDocument));
        if (*documents == NULL) {
            return -1;
        }

        for (size_t i = 0; i < yaml_docs.size(); ++i) {
            YAML::Node doc = yaml_docs[i];
            vector<string> v_tmp;

            // Carica in_topic e out_topic
            if (doc["inTopic"].IsSequence()){
                v_tmp = doc["inTopic"].as<vector<string>>();
            }
            else{
                v_tmp.clear();
                v_tmp.push_back(doc["inTopic"].as<string>());
            }
            
            //TODO reduce code repetition, solve for parameters and functions

            (*documents)[i].num_in_topics = v_tmp.size();
            (*documents)[i].in_topic = (char**)mosquitto_calloc(1,(*documents)[i].num_in_topics * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_in_topics; ++j) {
                (*documents)[i].in_topic[j] = strdup(v_tmp[j].c_str());
                (*documents)[i].in_topic[j][sizeof((*documents)[i].in_topic[j]) - 1] = '\0'; // Assicura che sia null-terminato
            }
            //strncpy((*documents)[i].in_topic, doc["inTopic"].as<string>().c_str(), sizeof((*documents)[i].in_topic) - 1);


            if (doc["outTopic"].IsSequence()){
                v_tmp = doc["outTopic"].as<vector<string>>();
            }
            else{
                v_tmp.clear();
                v_tmp.push_back(doc["outTopic"].as<string>());
            }

            (*documents)[i].num_out_topics = v_tmp.size();
            (*documents)[i].out_topic = (char**)mosquitto_calloc(1,(*documents)[i].num_out_topics * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_out_topics; ++j) {
                (*documents)[i].out_topic[j] = strdup(v_tmp[j].c_str());
                (*documents)[i].out_topic[j][sizeof((*documents)[i].out_topic[j]) - 1] = '\0'; // Assicura che sia null-terminato
            }
            //strncpy((*documents)[i].out_topic, doc["outTopic"].as<string>().c_str(), sizeof((*documents)[i].out_topic) - 1);

            cout<<"here"<<endl;

            // Retain
            (*documents)[i].retain = doc["retain"].as<bool>();

            cout<<"here2"<<endl;



            // Funzioni (array dinamico di stringhe)
            (*documents)[i].num_functions = doc["function"].size();
            (*documents)[i].functions = (char**)mosquitto_calloc(1,(*documents)[i].num_functions * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_functions; ++j) {
                (*documents)[i].functions[j] = strdup(doc["function"][j].as<string>().c_str());
            }

            // Parametri (array dinamico di stringhe)
            (*documents)[i].num_parameters = doc["parameters"].size();
            (*documents)[i].parameters = (char**)mosquitto_calloc(1,(*documents)[i].num_parameters * sizeof(char*));
            for (int j = 0; j < (*documents)[i].num_parameters; ++j) {
                (*documents)[i].parameters[j] = strdup(doc["parameters"][j].as<string>().c_str());
            }

            cout<<"here3"<<endl;

            // Formati di input e output
            (*documents)[i].in_format = string_to_format(doc["inFormat"].as<string>());
            (*documents)[i].out_format = string_to_format(doc["outFormat"].as<string>());

            cout<<"here4"<<endl;
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
