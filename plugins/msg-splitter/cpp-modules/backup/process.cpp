#include "process.hpp"
#include <iostream>
#include <vector>


void process_msg(const char *clientid,const char *topic, int payload_len, const void* payload, int qos,bool retain, mosquitto_property *properties){
    std::cout<<"prova"<<std::endl;
    mosquitto_broker_publish_copy(clientid,topic,payload_len,payload,qos,retain,properties);
    //load_yaml();
}

