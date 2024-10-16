#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

// Prototipo della funzione split_image
std::vector<std::pair<int, void*>> split_image(void* image_data, int data_size, int n);

int main() {
    // Nome del file immagine da caricare (deve essere nella stessa cartella del .cpp)
    const char* filename = "image.png";

    // Carica il file immagine in un buffer
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file immagine: " << filename << std::endl;
        return -1;
    }

    // Determina la dimensione del file e leggi i dati
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(file_size);
    if (!file.read(buffer.data(), file_size)) {
        std::cerr << "Errore nella lettura del file immagine." << std::endl;
        return -1;
    }
    file.close();

    // Converte il buffer in un void* e passa alla funzione split_image
    void* image_data = static_cast<void*>(buffer.data());
    int n = 3;  // Dividere l'immagine in n^2 parti (3x3 in questo esempio)
    
    std::vector<std::pair<int, void*>> tiles = split_image(image_data, file_size, n);

    // Salva le tiles su disco per controllo
    for (size_t i = 0; i < tiles.size(); ++i) {
        // Crea un nome di file unico per ogni tile
        std::string tile_filename = "tile_" + std::to_string(i) + ".png";
        
        // Scrive il buffer della tile su disco
        std::ofstream out_file(tile_filename, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "Errore nell'apertura del file per scrivere la tile: " << tile_filename << std::endl;
            continue;
        }
        
        // Scrive il contenuto della tile su disco
        out_file.write(static_cast<char*>(tiles[i].second), tiles[i].first);
        out_file.close();

        std::cout << "Tile " << i << " salvata come " << tile_filename << std::endl;
    }

    // Libera la memoria delle tiles
    for (size_t i = 0; i < tiles.size(); ++i) {
        free(tiles[i].second);  // Libera la memoria allocata per ogni PNG
    }

    return 0;
}

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
