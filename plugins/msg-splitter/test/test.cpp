#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

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
std::vector<void*> split_image(void* image_data, int data_size, int n, int& width, int& height, int& channels) {
    // Carica l'immagine dal buffer
    unsigned char* img = stbi_load_from_memory(static_cast<unsigned char*>(image_data), data_size, &width, &height, &channels, 0);
    if (!img) {
        std::cerr << "Errore nel caricamento dell'immagine dal buffer" << std::endl;
        return {};
    }

    // Dimensioni standard delle tile
    int base_tile_width = width / n;
    int base_tile_height = height / n;

    // Vettore che conterrà i payload delle parti dell'immagine
    std::vector<void*> tiles;

    for (int row = 0; row < n; ++row) {
        for (int col = 0; col < n; ++col) {
            // Calcola la larghezza della tile (gestisci le ultime colonne)
            int tile_width = (col == n - 1) ? width - col * base_tile_width : base_tile_width;

            // Calcola l'altezza della tile (gestisci le ultime righe)
            int tile_height = (row == n - 1) ? height - row * base_tile_height : base_tile_height;

            // Alloca memoria per la tile
            unsigned char* tile = new unsigned char[tile_width * tile_height * channels];

            // Copia i pixel della tile
            for (int y = 0; y < tile_height; ++y) {
                for (int x = 0; x < tile_width; ++x) {
                    for (int c = 0; c < channels; ++c) {
                        tile[(y * tile_width + x) * channels + c] =
                            img[((row * base_tile_height + y) * width + (col * base_tile_width + x)) * channels + c];
                    }
                }
            }

            // Aggiungi la tile alla lista come void*
            tiles.push_back(static_cast<void*>(tile));
        }
    }

    // Libera la memoria dell'immagine originale
    stbi_image_free(img);

    return tiles;
}

int main() {
    // Carica un'immagine reale da file in un buffer
    const char* filename = "dc_font_courier.png";  // Inserisci il nome del file PNG
    std::vector<unsigned char> buffer = read_file_to_buffer(filename);

    if (buffer.empty()) {
        std::cerr << "Errore: buffer vuoto, impossibile continuare." << std::endl;
        return -1;
    }

    // Simula il payload immagine (void*) da passare alla funzione
    void* payload = static_cast<void*>(buffer.data());
    int data_size = buffer.size();

    int width, height, channels;
    int n = 2;  // Dividi in 3^2 = 9 parti

    // Chiama la funzione per splittare l'immagine
    std::vector<void*> tiles = split_image(payload, data_size, n, width, height, channels);

    if (tiles.empty()) {
        std::cerr << "Errore nella divisione dell'immagine." << std::endl;
        return -1;
    }

    // Salva le immagini risultanti su disco
    for (int i = 0; i < tiles.size(); ++i) {
        unsigned char* tile_data = static_cast<unsigned char*>(tiles[i]);

        // Calcola dimensioni della singola tile
        int tile_width = (i % n == n - 1) ? width - (n - 1) * (width / n) : (width / n);
        int tile_height = (i / n == n - 1) ? height - (n - 1) * (height / n) : (height / n);

        // Crea il nome del file di output per ogni tile
        std::string output_filename = "tile_" + std::to_string(i) + ".png";

        // Salva la tile come immagine PNG
        stbi_write_png(output_filename.c_str(), tile_width, tile_height, channels, tile_data, tile_width * channels);

        // Libera la memoria della tile
        delete[] tile_data;
    }

    return 0;
}
