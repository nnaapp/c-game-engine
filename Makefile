all:
# Directory prefixes
SRC := src
INCLUDE := include
LIB := lib
BIN := bin

# Absolute path directories
ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
SRC_DIR := $(ROOT_DIR)$(SRC)
INCLUDE_DIR := $(ROOT_DIR)$(INCLUDE)
LIB_DIR := $(ROOT_DIR)$(LIB)
BUILD_DIR := $(ROOT_DIR)$(BIN)

# Executables and object files
EXEC := cgame editor
OBJ.cgame := $(SRC)/cgame.o $(SRC)/ecs.o $(SRC)/arena.o $(SRC)/console.o $(SRC)/event.o $(INCLUDE)/cJSON.o
OBJ.editor := $(SRC)/editorimgui.o $(SRC)/arena.o $(SRC)/serialeditor.o $(SRC)/windows_utils.o

# Dependencies
-include $(OBJ.cgame:%.o=$(BUILD_DIR)/%.d)
-include $(OBJ.editor:%.o=$(BUILD_DIR)/%.d)

# Shell, compiler, and linker
SHELL := /bin/bash
COMPILER := gcc

CC.gcc := /usr/bin/x86_64-w64-mingw32-gcc-win32
LD.gcc := /usr/bin/x86_64-w64-mingw32-g++-win32
CC.clang := /usr/bin/llvm-mingw-x86_64/bin/x86_64-w64-mingw32-clang
LD.clang := /usr/bin/llvm-mingw-x86_64/bin/x86_64-w64-mingw32-clang++

CC := $(CC.$(COMPILER))
LD := $(LD.$(COMPILER))

CCFLAGS.gcc := -Wall -m64 -g -fmessage-length=0
CCFLAGS.clang := -Wall -m64 -g 
CCFLAGS := $(CCFLAGS.$(COMPILER))

LDFLAGS.gcc := -g -Wall -m64
# static arg because libunwind.dll causes issues, and clang is mainly here for asan testing and such
LDFLAGS.clang := -g -Wall -m64 -static 
LDLIBS.cgame := -L $(LIB_DIR) -lraylib -lwinmm -lkernel32 -lopengl32 -lgdi32
LDLIBS.editor := -L $(LIB_DIR) -lraylib -limgui -lwinmm -lkernel32 -lopengl32 -lgdi32
LDFLAGS := $(LDFLAGS.$(COMPILER))

SAN := none
ifeq ($(COMPILER),clang)
ifeq ($(SAN),undefined)
	CCFLAGS += -fsanitize=undefined
	LDFLAGS += -fsanitize=undefined
else ifeq ($(SAN),address)
	CCFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
else ifeq ($(SAN),memory)
	CCFLAGS += -fsanitize=memory
	LDFLAGS += -fsanitize=memory
endif
endif

COMPILE = $(CC) -c -o $@ -MD -MP $(CCFLAGS) $<
LINK = $(LD) $(LDFLAGS) -o $@ $^

# Compile EXEC executable
all: checkdirs $(EXEC)

cgame: checkdirs $(BUILD_DIR)/cgame.exe

editor: checkdirs $(BUILD_DIR)/editor.exe

$(BUILD_DIR)/cgame.exe: $(BUILD_DIR)/% : $(addprefix $(BUILD_DIR)/,$(OBJ.cgame))
	$(strip $(LINK) $(LDLIBS.cgame))

$(BUILD_DIR)/editor.exe: $(BUILD_DIR)/% : $(addprefix $(BUILD_DIR)/,$(OBJ.editor))
	$(strip $(LINK) $(LDLIBS.editor))

$(BUILD_DIR)/$(SRC)/%.o: $(SRC_DIR)/%.c
	$(strip $(COMPILE))

$(BUILD_DIR)/$(INCLUDE)/%.o: $(INCLUDE_DIR)/%.c
	echo $<
	$(strip $(COMPILE))

# Verify directories exist
checkdirs: | $(BUILD_DIR) $(BUILD_DIR)/$(SRC) $(BUILD_DIR)/$(INCLUDE) $(SRC_DIR) $(INCLUDE_DIR)

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/$(SRC):
	mkdir $@

$(BUILD_DIR)/$(INCLUDE):
	mkdir $@

$(SRC_DIR):
	mkdir $@

$(LIB_DIR):
	mkdir $@

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)/src $(BUILD_DIR)/include $(BUILD_DIR)/*.exe

# Rebuild from scratch
rebuild: clean all

.PHONY: clean all checkdirs cgame editor
