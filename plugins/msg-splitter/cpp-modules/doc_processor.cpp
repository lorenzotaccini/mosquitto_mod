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
#include "simdjson.h"
#include <yaml-cpp/yaml.h>
#include "rapidcsv.h"
#include <map>

using namespace std;
using namespace tinyxml2;
using namespace simdjson;

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
        // Parsing JSON con simdjson
        ondemand::parser parser;
        ondemand::document json_doc = parser.iterate(data);

        for (auto field : json_doc.get_object()) {
            map<string, string> json_map;
            json_map[string(field.unescaped_key())] = string(field.value().get_string());
            result.push_back(json_map);
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