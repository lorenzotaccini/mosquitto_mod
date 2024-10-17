#pragma once

#include <vector>
#include <string>
#include <variant>
#include <map>

using namespace std;

using FieldValue = std::variant<std::string, int, bool, std::vector<std::string>, std::vector<int>>;
using Document = std::map<std::string, FieldValue>;

namespace DocsElaboration {
    vector<Document> normalize_input(const string&, char*);
    class DocProcessor {};
    class ImageSplit : public DocProcessor{};
    vector<DocProcessor*> create_processors(vector<pair<string,vector<string>>>);
};

