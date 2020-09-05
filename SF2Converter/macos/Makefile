# Makefile for macOS version

# Sources
PROJECT_ROOT=..
SRC=$(shell find $(PROJECT_ROOT) -name "*.cpp") 
MAIN=$(PROJECT_ROOT)/main.cpp

# Artifacts
ARTIFACTS_FOLDER=artifacts
EXE=$(ARTIFACTS_FOLDER)/SF2Converter

# The compiler (gcc, g++, c++,clang++)
CC=gcc
CC_FLAGS= \
	-I . \
	-I ../../sidfactory2/SIDFactoryII/source \
	-I /usr/local/include/SDL2 \
	-F /Library/Frameworks \
	-std=c++1z \
	-stdlib=libc++ \
	-Ofast \
	-Wno-delete-incomplete \
	-Wno-braced-scalar-init \
	-mmacosx-version-min=10.14 

LINKER_FLAGS=$(CC_FLAGS) \
	-framework SDL2 \
	-lstdc++ 

# Rule to compile .o from .cpp
%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

# Determine all .o files to be built
OBJ=$(SRC:.cpp=.o)
SF2_OBJ=$(shell find ../../sidfactory2/SIDFactoryII/source -name "*.o")

# Compile executable
$(EXE): $(MAIN) $(OBJ) $(ARTIFACTS_FOLDER)
	$(CC) $(LINKER_FLAGS) -o $(EXE) $(OBJ) $(SF2_OBJ)

$(ARTIFACTS_FOLDER):
	mkdir -p $@

clean:
	rm ${OBJ} || true

