// my_cpp_module.h

#pragma once
#include "mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dichiarazioni delle funzioni wrapper per C
typedef struct Wrapper Wrapper;

typedef enum {XML,JSON,CSV,YML} FORMAT;


Wrapper* wrapper_new();
void wrapper_publish(Wrapper* instance,const char *,const char *, int, void* , int ,bool , mosquitto_property *);
void wrapper_delete(Wrapper* instance);

#ifdef __cplusplus
}
#endif
