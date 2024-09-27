// my_cpp_module.cpp

#include <iostream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

class MyClass {
public:
    
    void doSomething() {
        std::cout<<"prova"<<std::endl;

        std::ifstream file("config.yml");
        std::vector<YAML::Node> docs = YAML::LoadAll(file);
        
        for (size_t i = 0; i < docs.size(); ++i) {
            YAML::Node doc = docs[i];
            std::cout<<doc<<std::endl;
        }
    }
    
};

extern "C" {
    MyClass* MyClass_new() { return new MyClass(); }
    void MyClass_doSomething(MyClass* instance) { instance->doSomething(); }
    void MyClass_delete(MyClass* instance) { delete instance; }
}
