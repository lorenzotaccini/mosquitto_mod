// my_cpp_module.h

#pragma once
#include "mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Wrapper Wrapper;

Wrapper* wrapper_new(const char*);
void wrapper_publish(Wrapper* instance,const char *,const char *, int, void* , int ,bool , mosquitto_property *);
void wrapper_delete(Wrapper* instance);

#ifdef __cplusplus
}
#endif
