// my_cpp_module.h

#pragma once
#include "mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

// Dichiarazioni delle funzioni wrapper per C
typedef struct MyClass MyClass;

MyClass* MyClass_new();
void MyClass_publish(MyClass* instance,const char *,const char *, int, void* , int ,bool , mosquitto_property *);
void MyClass_load_yaml(MyClass* instance);
void MyClass_delete(MyClass* instance);

#ifdef __cplusplus
}
#endif
