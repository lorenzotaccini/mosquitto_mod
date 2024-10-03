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
    

    Wrapper(const string& configfile_name)
        : yaml_loader(configfile_name) {
        cout << "Wrapper initialized with config file: " << configfile_name << endl;
        yaml_content = yaml_loader.load();
    }

    void publish(const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) {

        //TODO filter based on input topic, but also: modify function to get message's input topic
        for(auto &d: yaml_content){
            if(d["outTopic"].IsSequence()){
                for(auto &o_t: d["outTopic"].as<vector<string>>()){
                    cout<<o_t<<endl;
                    mosquitto_broker_publish_copy(clientid,o_t.c_str(),payload_len,payload,qos,retain,properties);
                }
            } else {
                cout<<d["outTopic"].as<string>()<<endl;
                mosquitto_broker_publish_copy(clientid,d["outTopic"].as<string>().c_str(),payload_len,payload,qos,retain,properties);
            }
        }
    }


private:
    YamlLoader yaml_loader;
    vector<YAML::Node> yaml_content;

};

extern "C" {
    Wrapper* wrapper_new(const char *configfile_name) { return new Wrapper(configfile_name); }
    void wrapper_publish(Wrapper* instance, const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) { 
        instance->publish(clientid,topic,payload_len,payload,qos,retain,properties); 
    }

    void wrapper_delete(Wrapper* instance) { delete instance; }
}
