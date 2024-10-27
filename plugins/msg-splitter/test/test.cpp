#include <iostream>
#include <chrono>

int main() {
    // Memorizza l'ora di inizio
    auto start = std::chrono::high_resolution_clock::now();

    // Codice di cui vuoi misurare il tempo di esecuzione
    for (int i = 0; i < 1000000; ++i) {
        // Operazioni di esempio
    }

    // Memorizza l'ora di fine
    auto end = std::chrono::high_resolution_clock::now();

    // Calcola la differenza in microsecondi
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Tempo di esecuzione: " << duration << " microsecondi" << std::endl;

    return 0;
}
