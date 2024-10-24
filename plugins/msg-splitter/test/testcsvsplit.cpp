#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include "rapidcsv.h"

std::string unsignedCharPtrToString(const unsigned char* payload, size_t length) {
    return std::string(reinterpret_cast<const char*>(payload), length);
}

std::vector<std::pair<int, unsigned char*>> splitCSVColumns(const unsigned char* payload, size_t length, int n) {
    std::string csvData = unsignedCharPtrToString(payload, length);
    std::stringstream csvStream(csvData);
    rapidcsv::Document doc(csvStream, rapidcsv::LabelParams(0, 0));

    std::vector<std::string> columnNames = doc.GetColumnNames();
    size_t totalColumns = columnNames.size();
    size_t basePartSize = totalColumns / n;
    size_t remainder = totalColumns % n;

    std::vector<std::pair<int, unsigned char*>> splitCsvParts;
    size_t columnIndex = 0;
    
    for (int part = 0; part < n; ++part) {
        size_t columnsInThisPart = basePartSize + (part == n - 1 ? remainder : 0);
        std::ostringstream partStream;

        for (size_t i = 0; i < columnsInThisPart; ++i) {
            if (i > 0) {
                partStream << ",";
            }
            partStream << columnNames[columnIndex + i];
        }
        partStream << "\n";

        for (size_t rowIndex = 0; rowIndex < doc.GetRowCount(); ++rowIndex) {
            for (size_t i = 0; i < columnsInThisPart; ++i) {
                if (i > 0) {
                    partStream << ",";
                }
                partStream << doc.GetCell<std::string>(columnNames[columnIndex + i], rowIndex);
            }
            partStream << "\n";
        }

        std::string partData = partStream.str();
        size_t partSize = partData.size();
        unsigned char* partPayload = new unsigned char[partSize];
        std::memcpy(partPayload, partData.c_str(), partSize);

        splitCsvParts.push_back({static_cast<int>(partSize), partPayload});
        columnIndex += columnsInThisPart;
    }

    return splitCsvParts;
}

int main() {
    std::string sampleCsv = "Name,Age,Occupation,Location,Salary\nAlice,30,Engineer,NY,5000\nBob,25,Designer,LA,4500";
    unsigned char* inputPayload = reinterpret_cast<unsigned char*>(const_cast<char*>(sampleCsv.c_str()));
    size_t inputLength = sampleCsv.size();

    int n = 3;
    std::vector<std::pair<int, unsigned char*>> splitParts = splitCSVColumns(inputPayload, inputLength, n);

    for (size_t i = 0; i < splitParts.size(); ++i) {
        std::cout << "Parte " << i + 1 << ":\n";
        std::string resultCsv(reinterpret_cast<char*>(splitParts[i].second), splitParts[i].first);
        std::cout << resultCsv << std::endl;
        delete[] splitParts[i].second;
    }

    return 0;
}
