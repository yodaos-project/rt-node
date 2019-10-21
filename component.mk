#
# Component Makefile
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

LIBS := jerry-core jerry-ext jerry-port-default-minimal

CFLAGS += -D__FREERTOS__=1

COMPONENT_ADD_LDFLAGS     += -L $(COMPONENT_PATH)/deps/jerryscript/lib \
                           $(addprefix -l,$(LIBS))

COMPONENT_ADD_INCLUDEDIRS := include \
                             src \
                             src/modules \
                             deps/jerryscript/include \
                             deps/libtuv/include \
                             deps/libtuv/src \
                             deps/httpsession

COMPONENT_SRCDIRS := src \
                     src/modules \
                     deps/libtuv/src \
                     deps/libtuv/src/unix \
                     deps/httpsession
