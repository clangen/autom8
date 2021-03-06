# may be one of the following: drawin, linux
BUILD_TARGET := linux

ifeq ($(BUILD_TARGET), darwin)
	# mac: WARNING! using "-undefined suppress -flat_namespace" will cause problems
	# when using node.js with ffi. specifically: memory allocation issues and random
	# segfaults. why? who knows...
	C := clang
	CXX := clang++
	DEFAULT_INCLUDES :=
	DEFAULT_LIBRARIES := -L./bin/osx/lib -L/usr/local/lib
	LOCAL_INCLUDES := -I./3rdparty/include -I./libautom8/include
	CFLAGS := $(DEFAULT_INCLUDES) $(LOCAL_INCLUDES) -g
	CXXFLAGS := $(CFLAGS) -fexceptions
	LIBRARY_FLAGS := $(DEFAULT_LIBRARIES) -lpthread -lssl -lcrypto -lboost_system-mt -lboost_regex-mt -lboost_date_time-mt -lboost_filesystem-mt -lboost_thread-mt
	LD_FLAGS := -dynamiclib -o libautom8.dylib
else ifeq ($(BUILD_TARGET), linux)
	C := clang
	CXX := clang++
	LLVMCONFIG := /usr/bin/llvm-config
	C_DEFAULT_INCLUDES := -I/usr/include/i386-linux-gnu/c++/4.8 -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(shell $(LLVMCONFIG) --cflags)
	CXX_DEFAULT_INCLUDES := -I/usr/include/i386-linux-gnu/c++/4.8 -I$(shell $(LLVMCONFIG) --src-root)/tools/clang/include -I$(shell $(LLVMCONFIG) --obj-root)/tools/clang/include $(shell $(LLVMCONFIG) --cxxflags)
	LOCAL_INCLUDES := -I./3rdparty/include -I./libautom8/include
	CFLAGS := $(C_DEFAULT_INCLUDES) $(LOCAL_INCLUDES) -Wno-extra-tokens -g
	CXXFLAGS := $(CXX_DEFAULT_INCLUDES) $(LOCAL_INCLUDES) -Wno-extra-tokens -fexceptions -g
	LIBRARY_FLAGS := -lpthread -lssl -lcrypto -lboost_system -lboost_regex -lboost_date_time -lboost_filesystem -lboost_thread
	LD_FLAGS := -shared -o libautom8.so
endif

C_SOURCES = \
	3rdparty/src/sqlite/sqlite3.c

CXX_SOURCES = \
	3rdparty/src/lib_json/json_reader.cpp \
	3rdparty/src/lib_json/json_writer.cpp \
	3rdparty/src/lib_json/json_value.cpp \
	3rdparty/src/base64/base64.cpp \
	libautom8/src/util/utility.cpp \
	libautom8/src/util/json.cpp \
	libautom8/src/util/ssl_certificate.cpp \
	libautom8/src/util/debug.cpp \
	libautom8/src/util/preferences.cpp \
	libautom8/src/db/db.cpp \
	libautom8/src/message/common_messages.cpp \
	libautom8/src/message/message.cpp \
	libautom8/src/message/request.cpp \
	libautom8/src/message/request_handler_factory.cpp \
	libautom8/src/message/request_handler_registrar.cpp \
	libautom8/src/message/response.cpp \
	libautom8/src/message/requests/get_device_list.cpp \
	libautom8/src/message/requests/get_security_alert_count.cpp \
	libautom8/src/message/requests/send_device_command.cpp \
	libautom8/src/net/client.cpp \
	libautom8/src/net/server.cpp \
	libautom8/src/net/session.cpp \
	libautom8/src/device/device_base.cpp \
	libautom8/src/device/device_model.cpp \
	libautom8/src/device/device_system.cpp \
	libautom8/src/device/simple_device.cpp \
	libautom8/src/device/null_device_system.cpp \
	libautom8/src/device/x10/x10_appliance.cpp \
	libautom8/src/device/x10/x10_device.cpp \
	libautom8/src/device/x10/x10_device_factory.cpp \
	libautom8/src/device/x10/x10_lamp.cpp \
	libautom8/src/device/x10/x10_security_sensor.cpp \
	libautom8/src/device/x10/mochad/mochad_controller.cpp \
	libautom8/src/device/x10/mochad/mochad_device_system.cpp \
	libautom8/src/autom8.cpp

CXX_OBJECTS = $(CXX_SOURCES:%.cpp=%.o)
C_OBJECTS = $(C_SOURCES:%.c=%.o)

all: $(C_OBJECTS) $(CXX_OBJECTS)
	sh bin/gather_static_libraries
	$(CXX) $(LD_FLAGS) $(C_OBJECTS) $(CXX_OBJECTS) $(LIBRARY_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(C) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(CXX_OBJECTS) $(C_OBJECTS) *~
	-rm -f autom8_cli/autom8_cli
	-rm -f libautom8.so
	-rm -f libautom8.dylib
