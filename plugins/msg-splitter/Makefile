include ../../config.mk

PLUGIN_CPPFLAGS += -Icpp-modules/


.PHONY : all binary check clean reallyclean test install uninstall

PLUGIN_NAME=mosquitto_msg_splitter

#cpp-modules= cpp-modules/process.cpp cpp-modules/yaml_loader_full.cpp #cpp-modules/wrapper.cpp
cpp-modules= cpp-modules/my_cpp_module.cpp
all : binary

binary : ${PLUGIN_NAME}.so



$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

%.o : %.cpp
	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) $(PLUGIN_LDFLAGS) -fPIC -c $< -o $@


# libmodules.a: $(cpp-modules)
# 	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) $(PLUGIN_LDFLAGS) -fPIC $^ -o $@

libmodules.a: $(cpp-modules:.cpp=.o)
	ar rcs $@ $^

${PLUGIN_NAME}.so : ${PLUGIN_NAME}.c libmodules.a 
	$(CROSS_COMPILE)$(CXX) $< $(PLUGIN_CPPFLAGS) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS)  -fPIC -shared libmodules.a -lstdc++ -o $@

reallyclean : clean
clean:
	-rm -f *.o ${PLUGIN_NAME}.so *.gcda *.gcno libmodules.a

check: test
test:

install: ${PLUGIN_NAME}.so
	# Don't install, these are examples only.
	#$(INSTALL) -d "${DESTDIR}$(libdir)"
	#$(INSTALL) ${STRIP_OPTS} ${PLUGIN_NAME}.so "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"

uninstall :
	-rm -f "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"


# include ../../config.mk

# PLUGIN_CPPFLAGS += -Icpp-modules/

# .PHONY : all binary check clean reallyclean test install uninstall

# PLUGIN_NAME=mosquitto_msg_splitter

# cpp-modules=cpp-modules/process.cpp cpp-modules/yaml_loader_full.cpp

# all : binary

# binary : ${PLUGIN_NAME}.so

# %.o: %.cpp
# 	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) -fPIC -c $< -o $@

# libmodules.a: $(cpp-modules:.cpp=.o)
# 	ar rcs libmodules.a $^

# ${PLUGIN_NAME}.so : ${PLUGIN_NAME}.c libmodules.a 
# 	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) -fpermissive $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) -L. -lmodules -fPIC -shared $< -o $@

# reallyclean : clean
# clean:
# 	-rm -f *.o ${PLUGIN_NAME}.so *.gcda *.gcno libmodules.a

# check: test
# test:

# install: ${PLUGIN_NAME}.so
# 	# Don't install, these are examples only.
# 	#$(INSTALL) -d "${DESTDIR}$(libdir)"
# 	#$(INSTALL) ${STRIP_OPTS} ${PLUGIN_NAME}.so "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"

# uninstall :
# 	-rm -f "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"