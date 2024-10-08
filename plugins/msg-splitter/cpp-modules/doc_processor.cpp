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
#include <map>

using namespace std;
using namespace tinyxml2;
using json = nlohmann::json;

namespace DocProcessor {


// Funzione per normalizzare l'input nei diversi formati
vector<map<string, string>> normalize_input(const string& input_format, const string& data) {
    vector<map<string, string>> result;

    if (input_format == "csv") {
        // Parsing CSV con rapidcsv
        stringstream ss(data);
        rapidcsv::Document doc(ss);
        size_t rows = doc.GetRowCount();
        size_t cols = doc.GetColumnCount();

        for (size_t i = 0; i < rows; ++i) {
            map<string, string> row_map;
            for (size_t j = 0; j < cols; ++j) {
                row_map[doc.GetColumnName(j)] = doc.GetCell<string>(j, i);
            }
            result.push_back(row_map);
        }

    } else if (input_format == "xml") {
        // Parsing XML con tinyxml2
        XMLDocument doc;
        doc.Parse(data.c_str());
        XMLElement* root = doc.RootElement();

        map<string, string> xml_map;
        for (XMLElement* child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
            xml_map[child->Name()] = child->GetText() ? child->GetText() : "";
        }
        result.push_back(xml_map);

} else if (input_format == "json") {
    // Parsing JSON con nlohmann/json
    json json_data = json::parse(data); // Parsing della stringa JSON

    // Conversione in una struttura dati
    for (auto& [key, value] : json_data.items()) {
        std::map<std::string, std::string> json_map;
        
        // Assicurati di gestire i tipi di dati appropriati
        if (value.is_string()) {
            json_map[key] = value.get<std::string>();
        } else if (value.is_number()) {
            json_map[key] = std::to_string(value.get<double>());
        } else if (value.is_boolean()) {
            json_map[key] = value.get<bool>() ? "true" : "false";
        } else if (value.is_array()) {
            // Se l'array è presente, gestiscilo come necessario
            std::vector<std::string> array_values;
            for (const auto& item : value) {
                if (item.is_string()) {
                    array_values.push_back(item.get<std::string>());
                }
            }
            // Puoi decidere come vuoi gestire l'array
            json_map[key] = "Array with " + std::to_string(array_values.size()) + " elements.";
        } else {
            // Altri tipi di dati (oggetti, null, ecc.)
            json_map[key] = "Unsupported type";
        }
        
        result.push_back(json_map); // Aggiungi la mappa al risultato
    }

    } else if (input_format == "yaml") {
        // Parsing YAML con yaml-cpp
        YAML::Node yaml_data = YAML::Load(data);

        for (YAML::const_iterator it = yaml_data.begin(); it != yaml_data.end(); ++it) {
            map<string, string> yaml_map;
            yaml_map[it->first.as<string>()] = it->second.as<string>();
            result.push_back(yaml_map);
        }

    } else {
        throw invalid_argument("input_format is not supported");
    }

    return result;
}

}