include(CMakePrintHelpers)

set(mosquitto_SOURCE_DIR "../..")
set(CPP_MODULES_DIR "cpp-modules")

include_directories(
	${mosquitto_SOURCE_DIR} ${mosquitto_SOURCE_DIR}/include
	${OPENSSL_INCLUDE_DIR} ${STDBOOL_H_PATH} ${STDINT_H_PATH}
	${CPP_MODULES_DIR}
)

link_directories(${mosquitto_SOURCE_DIR})
cmake_print_variables(mosquitto_SOURCE_DIR)

add_library(mosquitto_msg_splitter MODULE mosquitto_msg_splitter.c)

add_library(cpp_modules ${CPP_MODULES_DIR}/process.cpp ${CPP_MODULES_DIR}/yaml_loader_full.cpp)
target_compile_options(cpp_modules PUBLIC -fPIC)
target_link_libraries(mosquitto_msg_splitter cpp_modules)

set_target_properties(mosquitto_msg_splitter PROPERTIES
	POSITION_INDEPENDENT_CODE 1
)
set_target_properties(mosquitto_msg_splitter PROPERTIES PREFIX "")
if(WIN32)
	target_link_libraries(mosquitto_msg_splitter mosquitto cpp_modules)
endif(WIN32)

set(CMAKE_INSTALL_PREFIX ".")

install(TARGETS mosquitto_msg_splitter RUNTIME DESTINATION .. LIBRARY DESTINATION ..)
