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
    vector<DocProcessor*> create_processors(vector<pair<string,vector<string>>>);
    vector<pair<int,void*>> executeChain(void* initialPayload, int payload_len, const std::vector<DocProcessor*>& processors);


    // Virtual class to be derived when writing user-defined classes and functions 
    class DocProcessor {
        vector<string> params;
    public:
        virtual ~DocProcessor() = default;

        DocProcessor(const std::vector<std::string>& params) : params(params) {}

        virtual bool check_n_params(int n_given) const = 0;

        /* This function MUST be overridden and implemented by all user-defined classes with the purpose of payload processing.
        Input parameters:
            - a void* pointer to the payload to be processed
            - size in bytes of the payload

        Output parameters:
            - a std::vector of pairs in which first element is processed payload size expressed in bytes,
            and second element will be a void* type pointer to the payload itself.

        An instance to any derived class of this must be instanciated in Wrapper class along the others.
        */
        virtual std::vector<std::pair<int, void*>> process(void* payload, int payload_len) = 0;

    };

    class ImageSplit : public DocProcessor {
        vector<string> params;
    public:

        ImageSplit(const std::vector<std::string>& params) : DocProcessor(params) {}

    };
};

