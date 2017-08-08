LIBIEC_HOME=libiec61850

PROJECT_BINARY_NAME = main
PROJECT_SOURCES += main.cpp

INCLUDES += -I.

include $(LIBIEC_HOME)/make/target_system.mk
include $(LIBIEC_HOME)/make/stack_includes.mk

all:	$(PROJECT_BINARY_NAME)

include $(LIBIEC_HOME)/make/common_targets.mk

CFLAGS += -Wall

$(PROJECT_BINARY_NAME):	$(PROJECT_SOURCES) $(LIB_NAME)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(PROJECT_BINARY_NAME) $(PROJECT_SOURCES) $(INCLUDES) $(LIB_NAME) $(LDLIBS) -lSDL2main -lSDL2 -lm -lGLEW -lGL

clean:
	rm -f $(PROJECT_BINARY_NAME)
