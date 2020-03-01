#OBJS specifies the files to compile as part of the project
OBJS = main.cpp chip8.cpp

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w supresses all warnings
COMPILER_FLAGS = -w
DEBUG_FLAGS = -g

#LINKER_FLAGS specifies the libraties we;re linking against
LINKER_FLAGS = -lSDL2 

#OBJ_NAMe specifies the name of our executable
OBJ_NAME = Chip8-Emulator
OBJ_NAME_DEBUG = Chip8-Emulator-debug

#This is the target that compiles our executable
all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

debug: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(DEBUG_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME_DEBUG)
