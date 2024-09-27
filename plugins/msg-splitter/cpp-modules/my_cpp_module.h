// my_cpp_module.h

#ifdef __cplusplus
extern "C" {
#endif

// Dichiarazioni delle funzioni wrapper per C
typedef struct MyClass MyClass;

MyClass* MyClass_new();
void MyClass_doSomething(MyClass* instance);
void MyClass_delete(MyClass* instance);

#ifdef __cplusplus
}
#endif
