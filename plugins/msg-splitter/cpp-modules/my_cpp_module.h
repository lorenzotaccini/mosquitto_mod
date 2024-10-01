// my_cpp_module.h

#pragma once
#include "mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dichiarazioni delle funzioni wrapper per C
typedef struct Wrapper Wrapper;

typedef enum {XML,JSON,CSV,YML} FORMAT;

typedef struct
{
    char **in_topic;
    int num_in_topics;
    char **out_topic;
    int num_out_topics;
    bool retain;
    char **functions;
    int num_functions;
    char **parameters;
    int num_parameters;
    FORMAT in_format;
    FORMAT out_format;

} YamlDocument;


Wrapper* wrapper_new();
void wrapper_publish(Wrapper* instance,const char *,const char *, int, void* , int ,bool , mosquitto_property *);
int wrapper_load_yaml(Wrapper* instance, const char *filename, YamlDocument **yaml_docs);
void wrapper_free_docs_mem(YamlDocument *docs, int n_docs);
void wrapper_delete(Wrapper* instance);

#ifdef __cplusplus
}
#endif
