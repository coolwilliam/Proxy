CPATH=$(shell pwd)


DIR_INC +=-I$(CPATH)/include
DIR_INC += -I/usr/include
#DIR_INC += -I/usr/local/include
DIR_INC += -I/usr/local/include/luajit-2.1
DIR_INC += -I$(SF_ROOT)/include
DIR_INC += -I$(SF_ROOT)/include/server_frame_tools
DIR_INC += -I$(SF_ROOT)/include/server_frame_network
DIR_INC += -I$(SF_ROOT)/include/server_frame_business

DIR_SRC =$(CPATH)/source
DIR_OBJ =$(CPATH)/obj

LIB_S += -L./lib
LIB_S += -L/usr/local/lib
LIB_S += -L$(SF_ROOT)/lib

### dynamic link libraries
LIB_S += -lpthread
LIB_S += -lboost_system
LIB_S += -lboost_thread
LIB_S += -lboost_timer
LIB_S += -lboost_filesystem
LIB_S += -lrt
LIB_S += -lluajit-5.1
LIB_S += -lcurl
LIB_S += -lm
LIB_S += -ldl
LIB_S += -lserver_frame_tools
LIB_S += -lserver_frame_network
LIB_S += -lserver_frame_business

### static link libraries


SRC = $(wildcard $(DIR_SRC)/*.cpp)
SRC += $(wildcard $(CPATH)/*.cpp)
OBJ = $(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

MACROS:= -DBOOST_SPIRIT_THREADSAFE -DBOOST_ALL_DYN_LINK

TARGET = iptv_server 
CXX = g++
CFLAGS := $(DIR_INC) -Wall -fexceptions $(MACROS) -Wl,-z,defs
LD_FLAGS := -Wall -fexceptions $(MACROS) -Wl,-z,defs


all: $(DIR_OBJ) $(TARGET)
	
$(DIR_OBJ):
	mkdir -p $(DIR_OBJ)

$(TARGET): $(OBJ)
	$(CXX) $(LD_FLAGS) $(OBJ) $(LIB_S) $(LIB_SQL) -o $@
	
	
$(DIR_OBJ)/%.o : $(DIR_SRC)/%.cpp 
	@echo $(DIR_OBJ)
	$(CXX) $(CFLAGS) -o $@ -c $<
	
$(DIR_OBJ)/%.o : $(CPATH)/%.cpp
	@echo $(DIR_OBJ)
	$(CXX) $(CFLAGS) -o $@ -c $<
 
clean:
	rm -f $(DIR_OBJ)/*.o 
	rm -f $(TARGET)
	
rebuild : clean all

debug : CFLAGS+=-g
debug : LD_FLAGS+=-g
debug : rebuild 

release: CFLAGS+=-O2
release: LD_FLAGS+=-O2
release: rebuild
