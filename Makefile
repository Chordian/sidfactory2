PLATFORM=LINUX

APP_NAME=SIDFactoryII
BUILD_NR= $(shell git show --no-patch --format='%cs').$(shell git rev-parse --short HEAD)
ARTIFACTS_FOLDER=artifacts
DIST_FOLDER=$(ARTIFACTS_FOLDER)/$(APP_NAME)_$(PLATFORM)_$(BUILD_NR)

# SF2 sources
PROJECT_ROOT=./SIDFactoryII
SOURCE=$(PROJECT_ROOT)/source
SRC_TMP=$(PROJECT_ROOT)/main.cpp $(shell find $(SOURCE) -name "*.cpp")
SRC=$(patsubst %miniz_tester.cpp,,$(SRC_TMP))

# Artifacts
EXE=$(ARTIFACTS_FOLDER)/$(APP_NAME)

# The compiler (gcc, g++, c++,clang++)
ifeq ($(PLATFORM),MACOS)
	CC=MACOSX_DEPLOYMENT_TARGET=10.9 gcc
else
	CC=g++
endif

CC_FLAGS= $(shell sdl2-config --cflags) \
  -I $(SOURCE) \
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
OBJ = $(SRC:.cpp=.o) $(SOURCE)/libraries/miniz/miniz.o

# Compile SIDFactoryII
$(EXE): $(OBJ) $(ARTIFACTS_FOLDER)
	$(CC) $(OBJ) $(LINKER_FLAGS) -o $(EXE)
	strip $(EXE)
	
# Create a distribution folder with executables and resources
dist: $(EXE) $(DIST_FOLDER)
	mv $(EXE) $(DIST_FOLDER)
	cp -r $(PROJECT_ROOT)/drivers $(DIST_FOLDER)
	cp -r $(PROJECT_ROOT)/overlay $(DIST_FOLDER)
	cp -r $(PROJECT_ROOT)/color_schemes $(DIST_FOLDER)
	cp -r $(PROJECT_ROOT)/config $(DIST_FOLDER)
	cp -r $(PROJECT_ROOT)/music $(DIST_FOLDER)
	cp -r dist/documentation $(DIST_FOLDER)
	cp $(PROJECT_ROOT)/COPYING $(DIST_FOLDER)

$(ARTIFACTS_FOLDER):
	mkdir -p $@

$(DIST_FOLDER):
	mkdir -p $@

clean:
	rm ${OBJ} || true
	rm -rf $(ARTIFACTS_FOLDER) || true

# Compile with the Ubuntu image on Docker

BUILD_IMAGE_UBUNTU=sidfactory2/build-ubuntu
TMP_CONTAINER=sf2_build_tmp

ubuntu:
	docker container rm $(TMP_CONTAINER) || true
	docker build -t $(BUILD_IMAGE_UBUNTU) .
	docker run --name $(TMP_CONTAINER) $(BUILD_IMAGE_UBUNTU)
	docker cp $(TMP_CONTAINER):/home/$(ARTIFACTS_FOLDER) .
	docker container rm $(TMP_CONTAINER)
