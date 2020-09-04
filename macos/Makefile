# Makefile for macOS version

# Sources
BUILD_NR=$(shell date +"%Y%m%d").$(shell git rev-parse --short HEAD)
PROJECT_ROOT=../SIDFactoryII
SOURCE=$(PROJECT_ROOT)/source
SRC=$(PROJECT_ROOT)/main.cpp $(shell find $(SOURCE) -name "*.cpp") 
DOCS=../dist/documentation
SF2_CONVERTER=../../sf2converter/macos/artifacts/SF2Converter

# Artifacts
APP_NAME=SIDFactoryII
ARTIFACTS_FOLDER=artifacts
EXE=$(ARTIFACTS_FOLDER)/$(APP_NAME)
APP=$(ARTIFACTS_FOLDER)/$(APP_NAME).app
DMG=$(ARTIFACTS_FOLDER)/$(APP_NAME)_macOS_$(BUILD_NR).dmg
DMG_TMP=$(ARTIFACTS_FOLDER)/tmp.dmg
DMG_CONTENTS=$(ARTIFACTS_FOLDER)/dmg
DMG_TITLE=$(APP_NAME)
DMG_SIZE=16384
ICONSET=$(ARTIFACTS_FOLDER)/$(APP_NAME).iconset
ICONS=$(ARTIFACTS_FOLDER)/$(APP_NAME).icns

# The compiler (gcc, g++, c++,clang++)
CC=gcc

CC_FLAGS= \
  -I $(SOURCE) \
	-I ./App/Contents/Frameworks/SDL2.framework/Headers \
	-D_THREAD_SAFE \
	-std=gnu++14 \
	-stdlib=libc++ \
	-O2 \
	-flto \
	-mmacosx-version-min=10.7 \
	-Wno-bitwise-op-parentheses 

LINKER_FLAGS=\
	-framework SDL2 \
	-framework ApplicationServices \
	-F ./App/Contents/Frameworks \
	-lstdc++ \
	-flto \
	-Wl,-rpath,@executable_path/../Frameworks

.PHONY: clean 
.PHONY: tmp 
.PHONY: dmg
.PHONY: app

# Rule to compile .o from .cpp
%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

# Determine all .o files to be built
OBJ = $(SRC:.cpp=.o)

# Compile executable
$(EXE): $(OBJ) $(ARTIFACTS_FOLDER)
	$(CC) $(LINKER_FLAGS) -o $(EXE) $(OBJ)
	strip $(EXE)
	 
app: $(APP)

# ----------------------------------------------------

# Step 1: "make tmp" to create a temporary DMG
tmp: $(DMG_TMP)

# Step 2: Mount tmp.dmg, lay it out manually, unmount

# Step 3: "make dmg" to finalize the DMG
dmg: $(DMG)

# ---------------------------------------------------

# Package executable and dependencies in macOS App
$(APP): $(EXE) $(ICONS)
	rm -rf $(APP)
	mkdir -p $(APP)
	cp -r App/ ${APP}
	mkdir -p ${APP}/Contents/MacOS
	mkdir -p ${APP}/Contents/Resources
	cp $(EXE) $(APP)/Contents/MacOS
	cp -r $(PROJECT_ROOT)/drivers $(APP)/Contents/Resources
	cp -r $(PROJECT_ROOT)/overlay $(APP)/Contents/Resources
	cp -r $(PROJECT_ROOT)/color_schemes $(APP)/Contents/Resources
	cp -r $(PROJECT_ROOT)/config $(APP)/Contents/Resources
	cp $(ICONS) $(APP)/Contents/Resources
	sed -i 's/{VERSION}/$(BUILD_NR)/g' $(APP)/Contents/Info.plist


# Make final distribution DMG
$(DMG):
	DMG=$(DMG) \
  APP_NAME=$(APP_NAME) \
	DMG_TITLE=$(DMG_TITLE) \
	DMG_TMP=$(DMG_TMP) \
	./make_dmg.sh

# Collect the contents of the DMG
$(DMG_CONTENTS): $(APP) $(ARTIFACTS_FOLDER)/keys.html
	rm -rf $(DMG_CONTENTS)
	mkdir -p $(DMG_CONTENTS)/$(APP_NAME)
	cp -r $(APP) $(DMG_CONTENTS)
	cp -r $(DOCS)/ $(DMG_CONTENTS)/$(APP_NAME)/documentation
	cp $(ARTIFACTS_FOLDER)/*.html $(DMG_CONTENTS)/$(APP_NAME)/documentation
	cp -r Install.txt $(DMG_CONTENTS)
	cp -r $(PROJECT_ROOT)/music/ ${DMG_CONTENTS}/$(APP_NAME)/music
	cp -r $(PROJECT_ROOT)/drivers ${DMG_CONTENTS}/$(APP_NAME)/drivers
	cp $(SF2_CONVERTER) $(DMG_CONTENTS)/$(APP_NAME)
	ln -s /Applications $(DMG_CONTENTS)/Applications

# Make a temporary DMG. This needs to be layed out nicely by hand before building the final DMG.
$(DMG_TMP): $(DMG_CONTENTS)
	rm -f $(DMG_TMP)
	hdiutil create -srcfolder "$<" -volname "$(DMG_TITLE)" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size $(DMG_SIZE)k $@

# https://developer.apple.com/library/archive/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Optimizing/Optimizing.html
$(ICONS): icon.svg $(ARTIFACTS_FOLDER)
	mkdir -p $(ICONSET)
	rsvg-convert -w 16 -h 16 -f png -o $(ICONSET)/icon_16x16.png $<
	rsvg-convert -w 32 -h 32 -f png -o $(ICONSET)/icon_32x32.png $<
	rsvg-convert -w 128 -h 128 -f png -o $(ICONSET)/icon_128x128.png $<
	rsvg-convert -w 256 -h 256 -f png -o $(ICONSET)/icon_256x256.png $<
	rsvg-convert -w 512 -h 512 -f png -o $(ICONSET)/icon_512x512.png $<
	rsvg-convert -w 1024 -h 1024 -f png -o $(ICONSET)/icon_512x512@2x.png $< 
	cp $(ICONSET)/icon_32x32.png $(ICONSET)/icon_16x16@2x.png
	cp $(ICONSET)/icon_256x256.png $(ICONSET)/icon_128x128@2x.png
	cp $(ICONSET)/icon_512x512.png $(ICONSET)/icon_256x256@2x.png
	iconutil -c icns $(ICONSET) $@

# Create html from markdown documentation
$(ARTIFACTS_FOLDER)/%.html: %.md
	pandoc $< -c github.css --toc --standalone --self-contained --metadata title="$(APP_NAME) Keys (macOS)" -o $@

$(ARTIFACTS_FOLDER):
	mkdir -p $@

clean:
	rm ${OBJ} || true
	rm -rf $(ARTIFACTS_FOLDER)
