PLATFORM=LINUX

# SF2 sources
PROJECT_ROOT=./SIDFactoryII
SOURCE=$(PROJECT_ROOT)/source
SRC=$(PROJECT_ROOT)/main.cpp $(shell find $(SOURCE) -name "*.cpp")

SRC_SF2C=\
  ./SF2Converter/main.cpp \
	$(shell find ./SF2Converter/converter -name "*.cpp") \
	$(shell find $(SOURCE) -name "*.cpp")
SRC_SF2C_C=../SF2Converter/libraries/miniz_v115_r4/miniz.c

# Artifacts
APP_NAME=SIDFactoryII
ARTIFACTS_FOLDER=artifacts
EXE=$(ARTIFACTS_FOLDER)/$(APP_NAME)

EXE_SF2C=$(ARTIFACTS_FOLDER)/SF2Converter

BUILD_IMAGE_UBUNTU=sidfactory2/build-ubuntu
TMP_CONTAINER=sf2_build_tmp

# The compiler (gcc, g++, c++,clang++)
ifeq ($(PLATFORM),MACOS)
	CC=MACOSX_DEPLOYMENT_TARGET=10.9 gcc
else
	CC=g++
endif

CC_FLAGS= $(shell sdl2-config --cflags) \
  -I $(SOURCE) \
	-I ../SF2Converter \
	-D _SF2_$(PLATFORM) \
	-O2 \
	-std=gnu++14 \
	-flto

ifeq ($(PLATFORM),MACOS)
	LINKER_FLAGS= $(shell sdl2-config --libs) -lstdc++ -flto \
	-framework ApplicationServices 
else
	LINKER_FLAGS= $(shell sdl2-config --libs) -lstdc++ -flto
endif

.PHONY: clean
.PHONY: ubuntu
.PHONY: dist

# Rule to compile .o from .cpp
%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

# Rule to compile .o from .c
%.o: %.c
	$(CC) -c $< -o $@

# Determine all .o files to be built
OBJ = $(SRC:.cpp=.o)
OBJ_SF2C = $(SRC_SF2C:.cpp=.o) $(SRC_SF2C_C:.c=.o)

# Compile executable
$(EXE): $(OBJ) $(ARTIFACTS_FOLDER)
	$(CC) $(OBJ) $(LINKER_FLAGS) -o $(EXE)
	strip $(EXE)
	
# Compile SF2Converter
$(EXE_SF2C) : $(OBJ_SF2C) $(ARTIFACTS_FOLDER)
	$(CC) $(OBJ_SF2C) $(LINKER_FLAGS) -o $(EXE_SF2C)
	strip $(EXE_SF2C)

dist: $(EXE) $(EXE_SF2C)
	cp -r $(PROJECT_ROOT)/drivers $(ARTIFACTS_FOLDER)
	cp -r $(PROJECT_ROOT)/overlay $(ARTIFACTS_FOLDER)
	cp -r $(PROJECT_ROOT)/color_schemes $(ARTIFACTS_FOLDER)
	cp -r $(PROJECT_ROOT)/config $(ARTIFACTS_FOLDER)
	cp -r $(PROJECT_ROOT)/music $(ARTIFACTS_FOLDER)

$(ARTIFACTS_FOLDER):
	mkdir -p $@

clean:
	rm ${OBJ} || true
	rm -rf $(ARTIFACTS_FOLDER) || true

# Compile with the Ubuntu image on Docker

ubuntu:
	docker container rm $(TMP_CONTAINER) || true
	docker build -t $(BUILD_IMAGE_UBUNTU) .
	docker run --name $(TMP_CONTAINER) $(BUILD_IMAGE_UBUNTU)
	docker cp $(TMP_CONTAINER):/home/$(ARTIFACTS_FOLDER) .
	docker container rm $(TMP_CONTAINER)
