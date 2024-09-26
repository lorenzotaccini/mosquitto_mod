#include <lib/logging_mosq.h>

#ifndef FRED_H
    #define FRED_H 

    #ifdef __cplusplus
    extern "C" {
    #endif
    void process_msg(int);
    void load_yaml(void);
    #ifdef __cplusplus
    }
    #endif

#endif