# Compilation/Linking variables
CXX = g++
INCLUDES = -Iinclude/
COMPILE_FLAGS = -std=c++17 -Wall -pthread $(INCLUDES)
LIBS = -lGL -lglfw -lGLEW  -lpthread
SRC_PATH = src
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin
BIN_NAME = FuuGBemu
CPP_SOURCES = $(shell find $(SRC_PATH) -name '*.cpp' | sort -k 1nr | cut -f2-)

# Platform specific flags
ifeq ($(OS),Windows_NT)
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
	endif
	ifeq ($(UNAME_S),Darwin)
	endif
endif

OBJECTS = $(CPP_SOURCES:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)

# Rules
.PHONY: debug release makeDirs clean

debug: makeDirs
	@echo "Building debug x86_64..."
	@$(eval export DEBUG_FLAGS = -g -DFUUGB_DEBUG)
	@$(MAKE) $(BIN_PATH)/$(BIN_NAME)

release: makeDirs
	@echo "Building release x86_64..."
	@$(MAKE) $(BIN_PATH)/$(BIN_NAME)

makeDirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

clean:
	@echo "Deleting $(BIN_NAME) symlink"
	@$(RM) $(BIN_NAME)
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(BIN_PATH)

$(BIN_PATH)/$(BIN_NAME) : $(OBJECTS)
	@echo "Linking $^ -> $@"
	@$(CXX) $(OBJECTS) -o $@ $(LIBS)
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	@echo "Compiling: $< -> $@"
	$(CXX) $(COMPILE_FLAGS) $(DEBUG_FLAGS) -MP -MMD -c $< -o $@
