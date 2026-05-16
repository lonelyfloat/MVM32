PLATFORM=PLATFORM_DESKTOP
LDFLAGS=

CC=gcc
CFLAGS = -Os -std=c99 -Wall 
CPPFLAGS = -Os -std=c++20 -fPIC

LIBLOC =
DYNLIBLOC = 
PROJECT_NAME = game
HOT_RELOAD_NAME = game

GAME_LIB_FILES = $(shell find src/game -name *.c)
FRAMEWORK_FILES = $(shell find src/framework -maxdepth 1 -name *.c)
FRAMEWORK_FILES += $(shell find src/framework/components -name *.c)
LDFLAGS +=  -lGL -lm -lpthread -ldl -lrt -lX11 -lraylib

RAYLIB_PATH = external/raylib/src

IMGUI_TARGETS = objs/imgui.o objs/imgui_demo.o objs/imgui_draw.o objs/imgui_tables.o objs/imgui_widgets.o objs/dcimgui.o
IMGUI_DIR = external/imgui
DCIMGUI_DIR = external/dear_bindings

# Add imgui deps
FILES += $(IMGUI_TARGETS)
FRAMEWORK_FILES += $(IMGUI_TARGETS)
FRAMEWORK_FILES += $(shell find src/framework/ui -name *.c)
LDFLAGS += -lstdc++ 

ifeq ($(PLATFORM),PLATFORM_WEB)
CC=emcc
CFLAGS+= -s USE_GLFW=3  -DPLATFORM_WEB --shell-file external/emscripten_template.html --preload-file assets
PROJECT_NAME=index.html
endif


.PHONY: all framework reload-lib reload-program clean clean_external external

objs/%.o: $(IMGUI_DIR)/%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

objs/%.o: $(DCIMGUI_DIR)/%.cpp
	$(CC) $(CPPFLAGS) -c -o  $@ $< -Iexternal/imgui

# debug mode
all: clean
	$(CC) $(FRAMEWORK_FILES) $(GAME_LIB_FILES) src/main.c -o build/$(PROJECT_NAME) $(CFLAGS) -Iexternal -Isrc -Isrc/framework -Lobjs $(LDFLAGS)

framework:
	$(CC) -shared -o build/libframework.so $(FRAMEWORK_FILES) -Iexternal -Isrc -Isrc/framework $(LIBLOC) -Lobjs $(CFLAGS) $(LDFLAGS) -fPIC

reload-lib: 
	$(CC) -shared -o build/$(PROJECT_NAME).so $(GAME_LIB_FILES) -Iexternal -Isrc -Isrc/framework -Lobjs -Wl,-rpath=build -Lbuild $(CFLAGS) $(LDFLAGS) -lframework -fPIC

reload-program: framework reload-lib
	$(CC) src/main.c -o build/$(PROJECT_NAME)_debug -Iexternal -Isrc -Isrc/framework $(CFLAGS) -Wl,-rpath=build -Lbuild -Lobjs $(LDFLAGS) -lframework -DDYNAMIC_LIB=./build/$(PROJECT_NAME).so 

external: $(IMGUI_TARGETS)
	$(MAKE) -C $(RAYLIB_PATH) PLATFORM=$(PLATFORM)
	mv $(RAYLIB_PATH)/libraylib.a objs
	$(MAKE) -C $(RAYLIB_PATH) clean

clean_external:
	rm -rf objs
	mkdir objs

clean:
	rm -rf build
	mkdir build
