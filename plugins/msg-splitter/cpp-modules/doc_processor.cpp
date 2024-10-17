#include "doc_processor.h"


#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <string.h>


#include "tinyxml2.h"
#include "json.hpp"
#include <yaml-cpp/yaml.h>
#include "rapidcsv.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


using namespace tinyxml2;
using json = nlohmann::json;

using namespace std;

namespace DocsElaboration {

vector<pair<int,void*>> executeChain(void* initialPayload, int payload_len, const std::vector<DocProcessor*>& processors) {
    void* current_payload = initialPayload;
    int current_len = payload_len;
    std::vector<std::pair<int, void*>> result;

    for (auto processor : processors) {
        result = processor->process(current_payload, current_len);
        // Aggiorna il payload corrente con il risultato (ad esempio, il primo elemento del vettore)
        if (!result.empty()) {
            current_payload = result[0].second;
        }
    }

    return result;
}


// Virtual class to be derived when writing user-defined classes and functions 
class DocProcessor {
    vector<string> params;
public:
    virtual ~DocProcessor() = default;

    DocProcessor(const std::vector<std::string>& params) : params(params) {
        if (!check_n_params(params.size())) {
            throw std::invalid_argument("Number of given parameters is wrong for this function.\nPlease check your plugin's configuration file.");
        }
    }

    virtual bool check_n_params(int n_given) const = 0;

    /* This function MUST be overridden and implemented by all user-defined classes with the purpose of payload processing.
       Input parameters:
        - a void* pointer to the payload to be processed
        - size in bytes of the payload

       Output parameters:
        - a std::vector of pairs in which first element is processed payload size expressed in bytes,
          and second element will be a void* type pointer to the payload itself.

       An instance to any derived class of this must be instanciated in Wrapper class along the others.
    */
    virtual std::vector<std::pair<int, void*>> process(void* payload, int payload_len) = 0;

    bool check_n_params(int n_expected, int n_given){ return n_expected == n_given ;}
};

class ImageSplit : public DocProcessor {
public:

    ImageSplit(const std::vector<std::string>& params) : DocProcessor(params) {}

    bool check_n_params(int n_given) const override {
        return n_given == 1;
    }

    std::vector<std::pair<int, void*>> process(void* payload, int payload_len) override {

        std::vector<std::pair<int, void*>> result;
        
        

        return result;
    }
};



vector<Document> normalize_input(const string& input_format, char* data) {
    vector<Document> result;

    if (input_format == "csv") {
        stringstream ss(data);
        rapidcsv::Document doc(ss);
        size_t rows = doc.GetRowCount();
        size_t cols = doc.GetColumnCount();

        for (size_t i = 0; i < rows; ++i) {
            Document row_map;
            for (size_t j = 0; j < cols; ++j) {
                row_map[doc.GetColumnName(j)] = doc.GetCell<string>(j, i); // Usa stringhe per CSV
            }
            result.push_back(row_map);
        }

    } else if (input_format == "xml") {
        XMLDocument doc;
        doc.Parse(data);
        XMLElement* root = doc.RootElement();

        Document xml_map;
        for (XMLElement* child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
            xml_map[child->Name()] = child->GetText() ? string(child->GetText()) : "";
        }
        result.push_back(xml_map);

    } else if (input_format == "json") {
        json json_data = json::parse(data);

        for (auto& [key, value] : json_data.items()) {
            Document json_map;

            if (value.is_string()) {
                json_map[key] = value.get<string>();
            } else if (value.is_number()) {
                json_map[key] = value.get<int>();
            } else if (value.is_boolean()) {
                json_map[key] = value.get<bool>();
            } else if (value.is_array()) {
                if (value[0].is_string()) {
                    vector<string> array_values;
                    for (const auto& item : value) {
                        array_values.push_back(item.get<string>());
                    }
                    json_map[key] = array_values;
                } else if (value[0].is_number()) {
                    vector<int> array_values;
                    for (const auto& item : value) {
                        array_values.push_back(item.get<int>());
                    }
                    json_map[key] = array_values;
                }
            } else {
                json_map[key] = "Unsupported type";
            }

            result.push_back(json_map);
        }

    } else if (input_format == "yaml") {
        YAML::Node yaml_data = YAML::Load(data);

        for (YAML::const_iterator it = yaml_data.begin(); it != yaml_data.end(); ++it) {
            Document yaml_map;
            if (it->second.IsScalar()) {
                yaml_map[it->first.as<string>()] = it->second.as<string>();
            } else if (it->second.IsSequence()) {
                vector<string> array_values;
                for (auto v : it->second) {
                    array_values.push_back(v.as<string>());
                }
                yaml_map[it->first.as<string>()] = array_values;
            }
            result.push_back(yaml_map);
        }

    } else if(input_format == "png"){

    }
    
    else {
        throw invalid_argument("input_format is not supported");
    }

    return result;
}

}
