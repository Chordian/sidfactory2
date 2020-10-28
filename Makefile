# Makefile for SIDFactoryII linux distribution
#
# Also used to build a 'linux-style' macOS binary for debugging only.
#
# There is a 'proper' macOS Makefile in /macos, which compiles and
# packages a self-contained SIDFactoryII.app, and distribution disk image.
#
# Prerequisites:
# - gnu make
# - g++ (Xcode with command line utilities on macOS)
# - git
# - sdl2
#
# Make an executable:
#   make
#
# Make a complete distribution:
#   make dist
#
# Clean build artifacts:
#   make clean
#
# Build artifacts are in /artifacts

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

# Compiler
CC=g++
CC_FLAGS=$(shell sdl2-config --cflags) -I$(SOURCE) -D_SF2_$(PLATFORM) -D_BUILD_NR=\"$(BUILD_NR)\" -std=gnu++14 -g
LINKER_FLAGS=$(shell sdl2-config --libs) -lstdc++ -flto
ifeq ($(PLATFORM),MACOS)
	LINKER_FLAGS := $(LINKER_FLAGS) -framework ApplicationServices
endif

ifneq ($(TARGET),DEBUG)
  # optimizations, don't play well with debugging
	CC_FLAGS := $(CC_FLAGS) -O2 -flto -DNDEBUG
endif

.PHONY: clean
.PHONY: ubuntu
.PHONY: dist

# Rule to compile .o from .cpp
%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

# Rule to compile .o from .c
%.o: %.c
	gcc -c $< -o $@

# Determine all .o files to be built
OBJ = $(SRC:.cpp=.o) $(SOURCE)/libraries/miniz/miniz.o

# Compile SIDFactoryII
$(EXE): $(OBJ) $(ARTIFACTS_FOLDER) \
$(ARTIFACTS_FOLDER)/drivers \
$(ARTIFACTS_FOLDER)/overlay \
$(ARTIFACTS_FOLDER)/color_schemes \
$(ARTIFACTS_FOLDER)/config/config.ini
	$(CC) $(OBJ) $(LINKER_FLAGS) -o $(EXE)

$(ARTIFACTS_FOLDER)/drivers: $(PROJECT_ROOT)/drivers
	cp -r $(PROJECT_ROOT)/drivers $(ARTIFACTS_FOLDER)

$(ARTIFACTS_FOLDER)/overlay: $(PROJECT_ROOT)/overlay
	cp -r $(PROJECT_ROOT)/overlay $(ARTIFACTS_FOLDER)

$(ARTIFACTS_FOLDER)/color_schemes: $(PROJECT_ROOT)/color_schemes
	cp -r $(PROJECT_ROOT)/color_schemes $(ARTIFACTS_FOLDER)

$(ARTIFACTS_FOLDER)/config/config.ini: $(ARTIFACTS_FOLDER)/config
	cp $(PROJECT_ROOT)/config.ini $@

$(ARTIFACTS_FOLDER)/config:
	mkdir -p $@

# Create a distribution folder with executables and resources
dist: $(EXE) $(DIST_FOLDER)
	strip $(EXE)
	mv $(EXE) $(DIST_FOLDER)
	mkdir -p ${DIST_FOLDER}/config
	mv $(ARTIFACTS_FOLDER)/drivers $(DIST_FOLDER)
	mv $(ARTIFACTS_FOLDER)/overlay $(DIST_FOLDER)
	mv $(ARTIFACTS_FOLDER)/color_schemes $(DIST_FOLDER)
	mv $(ARTIFACTS_FOLDER)/config $(DIST_FOLDER)
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
