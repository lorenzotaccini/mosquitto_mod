// my_cpp_module.cpp

#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <variant>
#include <queue>

#include <yaml-cpp/yaml.h>

#include "mosquitto_broker.h"
#include "wrapper.h"
//#include "doc_processor.h"
#include "user_functions.h"



#include "tinyxml2.h"
#include "json.hpp"
#include <yaml-cpp/yaml.h>
#include "rapidcsv.h"


using namespace tinyxml2;
using json = nlohmann::json;

using namespace std;

using TextDocument = std::vector<std::map<std::string, std::string>>;
using RawDocument = pair<size_t, unsigned char*>;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//CLASS FOR LOADING CONFIGURATION FILE
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
            cout<< "Loading configuration file: "<<configfile_name.c_str()<<endl;
            vector<YAML::Node> docs = YAML::LoadAll(file);

            for (size_t i = 0; i < docs.size(); ++i) {
                YAML::Node doc = docs[i];
                if (check_structure(doc)) {
                    cout<<"Successfully loaded document "<<to_string(i).c_str()<<endl;
                    valid_docs.push_back(doc);
                } else {
                    cout<<"Document not loaded: "<<to_string(i).c_str()<<endl;
                }
            }
        } catch (const YAML::ParserException& e) {  // Cattura prima l'eccezione derivata
            cout<<"Error in YAML file: "<<string(e.what()).c_str()<<endl;
            return {};
        } catch (const runtime_error& e) {  // Cattura successivamente quella più generale
            cout<<string(e.what()).c_str();
            exit(-1);
}

        return valid_docs;
    }

private:
    string configfile_name;

bool check_structure(const YAML::Node& yaml_content) {
    // Definiamo i campi obbligatori e i relativi pattern
    map<string, regex> required_fields = {
        {"inTopic", regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
        {"outTopic", regex(R"(^([a-zA-Z0-9_\-#]+/?)*[a-zA-Z0-9_\-#]+$)")},
        {"retain", regex(R"(^(true|false|True|False)$)")},
        {"format", regex(R"(\b(json|xml|yaml|csv|png)\b)")},
    };

    regex r_function = regex(R"(^[a-zA-Z0-9_\-]+$)"); // Parametri delle funzioni
    vector<string> wrong_fields;

    cout << "Spell checking..." << endl;

    // Verifichiamo ogni campo obbligatorio
    for (const auto& [field, pattern] : required_fields) {
        if (!yaml_content[field]) {
            wrong_fields.push_back(field);  // Campo mancante
            continue;
        }

        if (yaml_content[field].IsSequence()) {
            // Gestiamo i campi che sono sequenze (come `outTopic`)
            for (auto v : yaml_content[field]) {
                if (!regex_match(v.as<string>(), pattern)) {
                    wrong_fields.push_back(field);
                }
            }
        } else {
            // Verifica scalare (singolo valore)
            if (!regex_match(yaml_content[field].as<string>(), pattern)) {
                wrong_fields.push_back(field);
            }
        }
    }

    // Gestiamo la sezione 'functions', che può essere una sequenza di mappe
    if (yaml_content["functions"]) {
        if (!yaml_content["functions"].IsSequence()) {
            wrong_fields.push_back("functions");
        } else {
            for (const auto& func : yaml_content["functions"]) {
                if (!func.IsMap()) {
                    wrong_fields.push_back("functions");
                } else {
                    for (auto it = func.begin(); it != func.end(); ++it) {
                        string function_name = it->first.as<string>();
                        if (!regex_match(function_name, r_function)) {
                            wrong_fields.push_back("functions");
                        }

                        YAML::Node params = it->second;
                        if (params.IsSequence()) {
                            // Controlla i parametri della funzione
                            for (const auto& param : params) {
                                cout<<param<<endl;
                                if (!regex_match(param.as<string>(), r_function)) {
                                    wrong_fields.push_back("functions");
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        wrong_fields.push_back("functions");
    }

    // Se ci sono campi errati o mancanti, segnaliamo
    if (!wrong_fields.empty()) {
        cout << "The following fields are wrong or missing: " << endl;
        for (const auto& field : wrong_fields) {
            cout << " - " << field << endl;
        }
        return false;
    }

    cout << "Configuration file is valid" << endl;
    return true;
}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//DOCUMENT NORMALIZATION/PROCESSING FUNCTIONS

// Virtual class to be derived when writing user-defined classes and functions 
class DocProcessor {
    vector<string> params;
public:
    virtual ~DocProcessor() = default;

    DocProcessor(const vector<string>& params) : params(params) {}

    virtual bool check_n_params(int n_given) const = 0;

    /* This function MUST be overridden and implemented by all user-defined classes with the purpose of payload processing.
       Input parameters:
        - a void* pointer to the payload to be processed
        - size in bytes of the payload

       Output parameters:
        - a vector of pairs in which first element is processed payload size expressed in bytes,
          and second element will be a void* type pointer to the payload itself.

       An instance to any derived class of this must be instanciated in Wrapper class along the others.
    */
    virtual vector<pair<int, unsigned char*>> process(unsigned char* payload, int payload_len) = 0;

};


class ImageSplit : public DocProcessor {
    vector<string> params;
public:

    ImageSplit(const vector<string>& params) : DocProcessor(params) {
        if (!check_n_params(params.size())) {
            throw invalid_argument("Number of given parameters is wrong for this function.\nPlease check your plugin's configuration file.");
        }
    }

    bool check_n_params(int n_given) const override {
        cout<<"image split has "<<n_given<<" params during init"<<endl;
        return n_given == 1;
    }

    vector<pair<int, unsigned char*>> process(unsigned char* payload, int payload_len) override {
        cout<<"qui"<<endl;
        return split_image(payload,payload_len, 2);
    }
};

class CsvRowsSplit : public DocProcessor{
   vector<string> params;
public:

    CsvRowsSplit(const vector<string>& params) : DocProcessor(params) {
        if (!check_n_params(params.size())) {
            throw invalid_argument("Number of given parameters is wrong for this function.\nPlease check your plugin's configuration file.");
        }
    }

    bool check_n_params(int n_given) const override {
        cout<<"image split has "<<n_given<<" params during init"<<endl;
        return n_given == 1;
    }

    vector<pair<int, unsigned char*>> process(unsigned char* payload, int payload_len) override {
        cout<<"qui"<<endl;
        return split_image(payload,payload_len, 2);
    }
};

class CsvColsSplit : public DocProcessor{
   vector<string> params;
public:

    CsvColsSplit(const vector<string>& params) : DocProcessor(params) {
        if (!check_n_params(params.size())) {
            throw invalid_argument("Number of given parameters is wrong for this function.\nPlease check your plugin's configuration file.");
        }
    }

    bool check_n_params(int n_given) const override {
        cout<<"image split has "<<n_given<<" params during init"<<endl;
        return n_given == 1;
    }

    vector<pair<int, unsigned char*>> process(unsigned char* payload, int payload_len) override {
        cout<<"qui"<<endl;
        return split_image(payload,payload_len, 2);
    }
};

class CsvExtractCols : public DocProcessor{
   vector<string> params;
public:

    CsvExtractCols(const vector<string>& params) : DocProcessor(params), params(params) {
        if (!check_n_params(params.size())) {
            throw invalid_argument("Number of given parameters is wrong for this function.\nPlease check your plugin's configuration file.");
        }
    }

    bool check_n_params(int n_given) const override {
        cout<<"extract csv cols has "<<n_given<<" params during init"<<endl;
        return n_given > 0;
    }

    vector<pair<int, unsigned char*>> process(unsigned char* payload, int payload_len) override {

        std::stringstream csvStream(string(reinterpret_cast<const char*>(payload), payload_len));
        rapidcsv::Document doc(csvStream, rapidcsv::LabelParams(0, -1));  // Con intestazioni

        std::ostringstream outputCsvStream;

        std::vector<std::string> columnNames = doc.GetColumnNames();

        // Scrivi le intestazioni delle colonne scelte nel nuovo CSV
        cout<<"params are "<<params.size()<<" first is"<<params[0]<<endl;
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                outputCsvStream << ",";
            }
            outputCsvStream << columnNames[stoi(params[i])];  // Nome colonna
        }
        outputCsvStream << "\n";  // Fine delle intestazioni

        // Iterare sulle righe del CSV originale
        for (size_t rowIndex = 0; rowIndex < doc.GetRowCount(); ++rowIndex) {
            for (size_t i = 0; i < params.size(); ++i) {
                if (i > 0) {
                    outputCsvStream << ",";  // Separatore di colonne
                }
                // Aggiungere i valori delle colonne selezionate
                outputCsvStream << doc.GetCell<std::string>(columnNames[stoi(params[i])], rowIndex);
            }
            outputCsvStream << "\n";  // Fine della riga
        }

        // Convertire il CSV generato in una stringa
        std::string outputCsvData = outputCsvStream.str();

        // Alloca memoria per il nuovo payload unsigned char*
        size_t dataSize = outputCsvData.size();
        unsigned char* outputPayload = new unsigned char[dataSize];
        std::memcpy(outputPayload, outputCsvData.c_str(), dataSize);

        cout<<outputPayload<<endl;

        return {pair<size_t, unsigned char*>(dataSize, outputPayload)};
    }
};

DocProcessor* create_processor(pair<string,vector<string>> info) {
    DocProcessor* res_proc;

    if(info.first == "imagesplit"){
        auto *f = new ImageSplit(info.second);
        res_proc = f;
    }
    if(info.first == "splitrows"){
        auto *f = new CsvRowsSplit(info.second);
        res_proc = f;
    }
    if(info.first == "splitcols"){
        auto *f = new CsvColsSplit(info.second);
        res_proc = f;
    }
    if(info.first == "extractcols"){
        auto *f = new CsvExtractCols(info.second);
        res_proc = f;
    }
    cout<<"creato processore "<<info.first<<endl;
    return res_proc;
}

struct topics_info {
    string format;
    vector<string> output_topics;
    vector<DocProcessor*> functions;
};

vector<pair<int,unsigned char*>> executeChain(void* payload, int payload_len, topics_info &t_i) {
    cout<<"initial size is: "<<payload_len<<endl;
    return t_i.functions[0]->process(static_cast<unsigned char*>(payload),payload_len);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//MAIN CLASS OF PLUGIN, HANDLING DATA STRUCTURES FOR MESSAGES AND PLUGIN'S CONFIGURATION, PUBLISHING FEATURES, ECC

class Wrapper {
public:
    
    Wrapper(const string& configfile_name): yaml_loader(configfile_name) {
        cout << "Wrapper initialized with config file: " << configfile_name << endl;
        yaml_content = yaml_loader.load();

        //mapping info obtained from yaml configuration document
        for(auto &d: yaml_content){
            topics_info t_i;

            vector<string> o_t_vec;
            if(d["outTopic"].IsSequence()){
                o_t_vec = d["outTopic"].as<vector<string>>();
            } else{
                o_t_vec.push_back(d["outTopic"].as<string>());
            }

            t_i.output_topics = o_t_vec;

            vector<pair<string,vector<string>>> f_vec;  // Contiene le coppie (funzione, parametri)
            vector<string> params;

            if (d["functions"].IsSequence()) {
                // Itera su ogni funzione
                for (const auto& f : d["functions"]) {
                    if (f.IsMap()) {  // Ogni funzione è una mappa con un solo elemento
                        for (auto it = f.begin(); it != f.end(); ++it) {
                            string function_name = it->first.as<string>();  // Nome della funzione
                            YAML::Node param_list = it->second;  // Lista dei parametri

                            if (param_list.IsSequence()) {
                                // Itera sui parametri e li aggiunge al vettore
                                for (const auto& p : param_list) {
                                    params.push_back(p.as<string>());
                                }
                            }

                            // Aggiungi la coppia (funzione, parametri) a f_vec
                            f_vec.push_back(make_pair(function_name, params));

                            // Pulisci il vettore dei parametri per la prossima funzione
                            params.clear();
                        }
                    } else {
                        cout << "Error: Each function must be a map with a function name and its parameters." << endl;
                    }
                }
            } else {
                cout << "Error: 'functions' must be listed as a sequence in the configuration document." << endl;
            }


            for(auto f: f_vec){
                t_i.functions.push_back(create_processor(f));
            }

            t_i.format = d["format"].as<string>();

            if(d["inTopic"].IsSequence()){
                for(auto i_t: d["inTopic"]){
                    topics_map[i_t.as<string>()] = t_i;
                }
            }
            else{
                topics_map[d["inTopic"].as<string>()] = t_i;
            }
        }

    }


    void process_msg(void* payload, int payload_len, const topics_info& t_info ){
        ;;
    }


    //TODO check on payloadlen type, is it okay to cast from uint_32t to int?
    void publish(const char *clientid, const char *topic, int payload_len, void* payload, int qos, bool retain, mosquitto_property *properties) {
        int cont;
        if(topics_map.find(topic) != topics_map.end()){ //plugin has to manage this message

            v_res = executeChain(payload,payload_len,topics_map[topic]);

            for(auto &o_t: topics_map[topic].output_topics){ //iterate on output topics
                cont = 0;
                for(auto i: this->v_res){
                    cout<<i.first<<endl;
                    mosquitto_broker_publish_copy(clientid,(o_t+"/"+to_string(cont)).c_str(),i.first,i.second,qos,retain,properties);
                    cout<<"modded and published on topic "<<(o_t+"/"+to_string(cont)).c_str()<<endl;
                    cont++;
                }
            }
            v_res.clear();
        } else{
            cout<<"messages on topic "<<topic<<" are not managed by the plugin and therefore published normally"<<endl;
        }

    }

private:
    YamlLoader yaml_loader;
    vector<YAML::Node> yaml_content;

    /*  Map in which each input topic (key) is associated with a topics_info object (value).
        When a message is coming on a certain input topic, the module is assuming its format based on the "format"
        field content of this topic in the configuration .yml file. */
    map<string,topics_info> topics_map;

    vector<pair<int,unsigned char*>> v_res;

    //other functions from user

    vector<DocProcessor*> processors;

};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//WRAPPER FUNCTIONS

extern "C" {
    Wrapper* wrapper_new(const char *configfile_name) { return new Wrapper(configfile_name); }
    void wrapper_publish(Wrapper* instance, const char *clientid,const char *topic, int payload_len, void* payload, int qos,bool retain, mosquitto_property *properties) { 
        instance->publish(clientid,topic,payload_len,payload,qos,retain,properties); 
    }

    void wrapper_delete(Wrapper* instance) { delete instance; }
}
