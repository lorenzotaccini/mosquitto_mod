include ../../config.mk

PLUGIN_CPPFLAGS += -Icpp-modules/
PLUGIN_CPPFLAGS += -Ilib/xml
PLUGIN_CPPFLAGS += -Ilib/csv
PLUGIN_CPPFLAGS += -Ilib/json
PLUGIN_CPPFLAGS += -Ilib/img
#PLUGIN_CPPFLAGS += -Ilib/yaml/include/yaml-cpp

.PHONY : all binary check clean reallyclean test install uninstall

PLUGIN_NAME=mosquitto_msg_splitter

#cpp-modules= cpp-modules/process.cpp cpp-modules/yaml_loader_full.cpp #cpp-modules/wrapper.cpp
cpp-modules= cpp-modules/wrapper.cpp lib/xml/tinyxml2.cpp cpp-modules/user_functions.cpp lib/img/lodepng.cpp
all : binary

binary : ${PLUGIN_NAME}.so



#$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

%.o : %.cpp
	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) $(PLUGIN_LDFLAGS) -fPIC -c $< -o $@


# libmodules.a: $(cpp-modules)
# 	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) $(PLUGIN_LDFLAGS) -fPIC $^ -o $@

libmodules.a: $(cpp-modules:.cpp=.o)
	ar rcs $@ $^

${PLUGIN_NAME}.so : ${PLUGIN_NAME}.c libmodules.a 
	$(CROSS_COMPILE)$(CXX) $< $(PLUGIN_CPPFLAGS) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) -fPIC -shared libmodules.a -L/usr/lib/x86_64-linux-gnu -lyaml-cpp -lstdc++ -o $@

reallyclean : clean
clean:
	-rm -f cpp-modules/*.o *.o lib/*/*.o ${PLUGIN_NAME}.so *.gcda *.gcno libmodules.a

check: test
test:

install: ${PLUGIN_NAME}.so
	#$(INSTALL) -d "${DESTDIR}$(libdir)"
	#$(INSTALL) ${STRIP_OPTS} ${PLUGIN_NAME}.so "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"

uninstall :
	-rm -f "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"

