#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Funzione per leggere un file in un buffer
std::vector<unsigned char> read_file_to_buffer(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Errore nell'apertura del file: " << filename << std::endl;
        return {};
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cout << "File letto correttamente: " << filename << std::endl;
        return buffer;
    } else {
        std::cerr << "Errore nella lettura del file: " << filename << std::endl;
        return {};
    }
}

// Funzione che splitta l'immagine in n^2 parti con gestione delle righe e colonne rimanenti
std::vector<pair<int,void*>> split_image(void* image_data, int data_size, int n) {
    int width, height, channels;
    
    // Carica l'immagine dal buffer
    unsigned char* img = stbi_load_from_memory(static_cast<unsigned char*>(image_data), data_size, &width, &height, &channels, 0);
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

            // Calcola la dimensione della tile in memoria
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

            cout<<"number of channels: "<<channels<<endl; 
            //GUARDA STB_WRITE_PNG_TO_MEM, RESTITUISCE LA DIMENSIONE
            // Aggiungi la tile alla lista con la sua dimensione in memoria
            tiles.push_back(make_pair(tile_size, static_cast<void*>(tile)));
        }
    }

    // Libera la memoria dell'immagine originale
    stbi_image_free(img);

    return tiles;
}