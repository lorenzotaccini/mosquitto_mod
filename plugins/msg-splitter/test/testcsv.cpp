#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>  // Per memcpy
#include "rapidcsv.h"  // Include the rapidcsv library

// Funzione per convertire void* in std::string
std::string voidPtrToString(void* payload, size_t length) {
    return std::string(static_cast<char*>(payload), length);
}

// Funzione per normalizzare il CSV e restituire un pair <void*, size_t>
std::pair<void*, size_t> processCSVPayload(void* payload, size_t length, const std::string& format) {
    if (format != "csv") {
        std::cerr << "Formato non supportato!" << std::endl;
        return {nullptr, 0};
    }

    // Convertire il payload in una stringa
    std::string csvData = voidPtrToString(payload, length);

    // Caricare il CSV da una stringa usando rapidcsv
    std::stringstream csvStream(csvData);
    rapidcsv::Document doc(csvStream, rapidcsv::LabelParams(0, -1)); // Se la prima riga è header

    // Esempio di elaborazione: ottenere la prima colonna (indice 0)
    std::vector<std::string> colData = doc.GetColumn<std::string>(2);

    // Esegui qualsiasi elaborazione necessaria sul CSV (esempio: splitting righe, modifiche colonne)
    // ...

    // Ricostruire il CSV dopo l'elaborazione
    std::ostringstream newCsvStream;
    for (size_t i = 0; i < colData.size(); ++i) {
        newCsvStream << colData[i];
        if (i != colData.size() - 1) {
            newCsvStream << "\n";  // Aggiunge un newline tra le righe
        }
    }

    // Convertire di nuovo la stringa elaborata in un payload void*
    std::string newCsv = newCsvStream.str();
    size_t newSize = newCsv.size();
    char* newPayload = new char[newSize];
    std::memcpy(newPayload, newCsv.c_str(), newSize);

    // Restituire il nuovo payload insieme alla sua dimensione
    return {static_cast<void*>(newPayload), newSize};
}

int main() {
    // Esempio di utilizzo della funzione
    std::string sampleCsv = "Name,Age,Occupation\nAlice,30,Engineer\nBob,25,Designer";
    void* payload = static_cast<void*>(const_cast<char*>(sampleCsv.c_str()));
    size_t length = sampleCsv.size();

    std::pair<void*, size_t> result = processCSVPayload(payload, length, "csv");

    // Stampare il risultato
    if (result.first) {
        std::string resultCsv(static_cast<char*>(result.first), result.second);
        std::cout << "CSV elaborato:\n" << resultCsv << std::endl;

        // Deallocare la memoria del nuovo payload
        delete[] static_cast<char*>(result.first);
    }

    return 0;
}
