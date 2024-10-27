// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "user_functions.h"


// vector<pair<int,unsigned char*>> split_image(unsigned char* image_data, int data_size, int n) {
//     int width, height, channels;
    
//     // Carica l'immagine dal buffer
//     unsigned char* img = stbi_load_from_memory(image_data, data_size, &width, &height, &channels, 0);
//     if (!img) {
//         cerr << "Errore nel caricamento dell'immagine dal buffer" << endl;
//         return {};
//     }

//     // Dimensioni standard delle tile
//     int base_tile_width = width / n;
//     int base_tile_height = height / n;

//     // Vettore che conterrà i payload delle parti dell'immagine e le dimensioni
//     vector<pair<int,unsigned char*>> tiles;

//     for (int row = 0; row < n; ++row) {
//         for (int col = 0; col < n; ++col) {
//             // Calcola la larghezza della tile (gestisci le ultime colonne)
//             int tile_width = (col == n - 1) ? width - col * base_tile_width : base_tile_width;

//             // Calcola l'altezza della tile (gestisci le ultime righe)
//             int tile_height = (row == n - 1) ? height - row * base_tile_height : base_tile_height;

//             // Calcola la dimensione della tile in memoria (approssimativa per ora)
//             int tile_size = tile_width * tile_height * channels;

//             // Alloca memoria per la tile
//             unsigned char* tile = new unsigned char[tile_size];

//             // Copia i pixel della tile
//             for (int y = 0; y < tile_height; ++y) {
//                 for (int x = 0; x < tile_width; ++x) {
//                     for (int c = 0; c < channels; ++c) {
//                         tile[(y * tile_width + x) * channels + c] =
//                             img[((row * base_tile_height + y) * width + (col * base_tile_width + x)) * channels + c];
//                     }
//                 }
//             }

//             // Utilizza stb_write_png_to_mem per ottenere la dimensione corretta
//             unsigned char* png_buffer = nullptr;
//             int png_size = 0;

//             // Scrivi l'immagine PNG in memoria
//             png_buffer = stbi_write_png_to_mem(tile, tile_width * channels, tile_width, tile_height, channels, &png_size);

//             //cout<<"tile size is: "<<png_size<<endl;

//             // Aggiungi la tile alla lista con la sua dimensione corretta (come PNG)
//             //tiles.push_back(make_pair(png_size, static_cast<void*>(png_buffer)));
//             tiles.push_back(make_pair(png_size, png_buffer));
//             // Libera la memoria della tile temporanea
//             delete[] tile;
//         }
//     }

//     // Libera la memoria dell'immagine originale
//     stbi_image_free(img);

//     return tiles;
// }


#include <utility>
#include <cstring>
#include <chrono>
#include "lodepng.h"

using namespace std;

vector<pair<int, unsigned char*>> split_image(unsigned char* image_data, size_t data_size, int n) {
    cout<<"data size is "<<data_size<<endl;
    // Step 1: Decode the PNG image from memory using LodePNG
    vector<unsigned char> decoded_pixels;
    unsigned width, height;
    unsigned error = lodepng::decode(decoded_pixels, width, height, image_data, data_size, LCT_RGBA, 8);

    if (error) {
        cerr << "LodePNG decoding error: " << lodepng_error_text(error) << endl;
        return {};
    }

    // Calculate base tile dimensions
    int base_tile_width = width / n;
    int base_tile_height = height / n;

    // Prepare the output vector to store tiles
    vector<pair<int, unsigned char*>> tiles;

    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col) {
            // Calculate exact tile width and height (adjust for edges)
            int tile_width = (col == n - 1) ? width - col * base_tile_width : base_tile_width;
            int tile_height = (row == n - 1) ? height - row * base_tile_height : base_tile_height;

            // Allocate memory for the tile's raw RGBA data
            vector<unsigned char> tile_data(tile_width * tile_height * 4);

            // Use memcpy to extract the tile from the main image
            for (int y = 0; y < tile_height; ++y) {
                const unsigned char* src_row = &decoded_pixels[((row * base_tile_height + y) * width + col * base_tile_width) * 4];
                unsigned char* dest_row = &tile_data[y * tile_width * 4];
                memcpy(dest_row, src_row, tile_width * 4);
            }

            // Encode the tile using LodePNG
            vector<unsigned char> png_buffer;
            error = lodepng::encode(png_buffer, tile_data, tile_width, tile_height, LCT_RGBA, 8);
            if (error) {
                cerr << "LodePNG encoding error: " << lodepng_error_text(error) << endl;
                continue;
            }

            // Allocate and copy the encoded tile buffer to ensure separate storage
            unsigned char* buffer_copy = new unsigned char[png_buffer.size()];
            memcpy(buffer_copy, png_buffer.data(), png_buffer.size());

            // Store tile with its size
            tiles.emplace_back(png_buffer.size(), buffer_copy);
        }
    }

    return tiles;
}
