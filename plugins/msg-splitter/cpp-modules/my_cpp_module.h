// my_cpp_module.h

#pragma once
#include "mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dichiarazioni delle funzioni wrapper per C
typedef struct Wrapper Wrapper;

typedef struct YamlContent;

Wrapper* Wrapper_new();
void Wrapper_publish(Wrapper* instance,const char *,const char *, int, void* , int ,bool , mosquitto_property *);
void Wrapper_load_yaml(Wrapper* instance);
void Wrapper_delete(Wrapper* instance);

#ifdef __cplusplus
}
#endif
