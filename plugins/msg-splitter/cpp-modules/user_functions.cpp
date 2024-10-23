#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "user_functions.h"


vector<pair<int,unsigned char*>> split_image(unsigned char* image_data, int data_size, int n) {
    int width, height, channels;
    
    // Carica l'immagine dal buffer
    unsigned char* img = stbi_load_from_memory(image_data, data_size, &width, &height, &channels, 0);
    if (!img) {
        cerr << "Errore nel caricamento dell'immagine dal buffer" << endl;
        return {};
    }

    // Dimensioni standard delle tile
    int base_tile_width = width / n;
    int base_tile_height = height / n;

    // Vettore che conterrà i payload delle parti dell'immagine e le dimensioni
    vector<pair<int,unsigned char*>> tiles;

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
            //tiles.push_back(make_pair(png_size, static_cast<void*>(png_buffer)));
            tiles.push_back(make_pair(png_size, png_buffer));
            // Libera la memoria della tile temporanea
            delete[] tile;
        }
    }

    // Libera la memoria dell'immagine originale
    stbi_image_free(img);

    return tiles;
}
