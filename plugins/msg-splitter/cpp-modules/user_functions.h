#ifndef IMAGE_SPLITTER_H
#define IMAGE_SPLITTER_H

#include <vector>
#include <string>

using namespace std;

// Funzione per leggere un file in un buffer
std::vector<unsigned char> read_file_to_buffer(const std::string& filename);

// Funzione che splitta l'immagine in n^2 parti con gestione delle righe e colonne rimanenti
std::vector<pair<int,unsigned char*>> split_image(unsigned char*, int, int);

#endif // IMAGE_SPLITTER_H
