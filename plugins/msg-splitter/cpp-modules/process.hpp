#include "mosquitto_broker.h"

#ifndef FRED_H
    #define FRED_H 

    #ifdef __cplusplus
    extern "C" {
    #endif
    void process_msg(const char *,const char *, int, const void* , int ,bool , mosquitto_property *);
    void load_yaml(void);
    #ifdef __cplusplus
    }
    #endif

#endif