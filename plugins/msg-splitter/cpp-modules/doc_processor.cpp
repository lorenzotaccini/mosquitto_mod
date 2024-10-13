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



using namespace std;
using namespace tinyxml2;
using json = nlohmann::json;

using namespace std;

//TODO is it really important the parsing performance of docs libraries? supposingly small files because IoT network??
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

    } else if(input_format == "png"){
        std::vector<void*> split_image_from_buffer(void* image_buffer, int buffer_size, int n, int& width, int& height, int& channels) {
            // Carica l'immagine dal buffer
            unsigned char* img = stbi_load_from_memory(static_cast<unsigned char*>(image_buffer), buffer_size, &width, &height, &channels, 0);

            if (!img) {
                std::cerr << "Errore nel caricamento dell'immagine dal buffer" << std::endl;
                return {};
            }

            // Calcola dimensioni del tile
            int tile_width = width / n;
            int tile_height = height / n;

            std::vector<void*> tiles;
            for (int row = 0; row < n; ++row) {
                for (int col = 0; col < n; ++col) {
                    // Alloca memoria per il tile
                    unsigned char* tile = new unsigned char[tile_width * tile_height * channels];

                    // Copia i pixel del tile
                    for (int y = 0; y < tile_height; ++y) {
                        for (int x = 0; x < tile_width; ++x) {
                            for (int c = 0; c < channels; ++c) {
                                tile[(y * tile_width + x) * channels + c] =
                                    img[((row * tile_height + y) * width + (col * tile_width + x)) * channels + c];
                            }
                        }
                    }

                    // Aggiungi il tile alla lista
                    tiles.push_back(static_cast<void*>(tile));
                }
            }

            // Libera l'immagine originale
            stbi_image_free(img);

            return tiles;
        }



    }
    
    else {
        throw invalid_argument("input_format is not supported");
    }

    return result;
}

}
