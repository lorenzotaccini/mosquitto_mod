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


using namespace std;
using namespace tinyxml2;
using json = nlohmann::json;

using namespace std;


namespace DocProcessor {

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

    } else {
        throw invalid_argument("input_format is not supported");
    }

    return result;
}

}