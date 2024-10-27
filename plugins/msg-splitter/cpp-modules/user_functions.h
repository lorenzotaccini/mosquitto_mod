#ifndef IMAGE_SPLITTER_H
#define IMAGE_SPLITTER_H

#include <vector>
#include <string>

using namespace std;

// Funzione che splitta l'immagine in n^2 parti con gestione delle righe e colonne rimanenti
std::vector<pair<int,unsigned char*>> split_image(unsigned char*, size_t, int);

#endif // IMAGE_SPLITTER_H
