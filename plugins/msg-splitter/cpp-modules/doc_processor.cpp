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

namespace DocsElaboration {

vector<DocProcessor*> create_processors(vector<pair<string,vector<string>>> proc_info){
   vector<DocProcessor*> v1;
   return v1;
}

vector<pair<int,void*>> executeChain(void* initialPayload, int payload_len, const std::vector<DocProcessor*>& processors) {
    void* current_payload = initialPayload;
    int current_len = payload_len;
    std::vector<std::pair<int, void*>> result;

    for (auto processor : processors) {
        result = processor->process(current_payload, current_len); //wrong, only for testing purposes with one processor
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
        else {
            this->params = params;
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
    vector<string> params;
public:

    ImageSplit(const std::vector<std::string>& params) : DocProcessor(params) {}

    bool check_n_params(int n_given) const override {
        cout<<"vector of params for this derived class is:";
        for(auto e: this->params){
            cout<<e<<endl;
        }
        return n_given == 1;
    }

    std::vector<std::pair<int, void*>> process(void* payload, int payload_len) override {

        std::vector<std::pair<int, void*>> result;
        
        
        int width, height, channels, n = stoi((this->params)[0]);
    
        // Carica l'immagine dal buffer
        unsigned char* img = stbi_load_from_memory(static_cast<unsigned char*>(payload), payload_len, &width, &height, &channels, 0);
        if (!img) {
            std::cerr << "Errore nel caricamento dell'immagine dal buffer" << std::endl;
            return {};
        }

        // Dimensioni standard delle tile
        int base_tile_width = width / n;
        int base_tile_height = height / n;

        // Vettore che conterrà i payload delle parti dell'immagine e le dimensioni
        std::vector<pair<int,void*>> tiles;

        for (int row = 0; row < n; ++row) {
            for (int col = 0; col < n; ++col) {
                // Calcola la larghezza della tile (gestisci le ultime colonne)
                int tile_width = (col == n - 1) ? width - col * base_tile_width : base_tile_width;

                // Calcola l'altezza della tile (gestisci le ultime righe)
                int tile_height = (row == n - 1) ? height - row * base_tile_height : base_tile_height;

                // Calcola la dimensione della tile in memoria (approssimativa per ora)
                int tile_size = tile_width * tile_height * channels;

                // Alloca memoria per la tile
                unsigned char* tile = new unsigned char[tile_size];

                // Copia i pixel della tile
                for (int y = 0; y < tile_height; ++y) {
                    for (int x = 0; x < tile_width; ++x) {
                        for (int c = 0; c < channels; ++c) {
                            tile[(y * tile_width + x) * channels + c] =
                                img[((row * base_tile_height + y) * width + (col * base_tile_width + x)) * channels + c];
                        }
                    }
                }

                // Utilizza stb_write_png_to_mem per ottenere la dimensione corretta
                unsigned char* png_buffer = nullptr;
                int png_size = 0;

                // Scrivi l'immagine PNG in memoria
                png_buffer = stbi_write_png_to_mem(tile, tile_width * channels, tile_width, tile_height, channels, &png_size);

                cout<<"tile size is: "<<png_size<<endl;

                // Aggiungi la tile alla lista con la sua dimensione corretta (come PNG)
                tiles.push_back(make_pair(png_size, static_cast<void*>(png_buffer)));

                // Libera la memoria della tile temporanea
                delete[] tile;
            }
        }

        // Libera la memoria dell'immagine originale
        stbi_image_free(img);

        return tiles;

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
