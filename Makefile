CC = g++
PROFILE ?= DEBUG

CCFLAGS_DEBUG = -DDEBUG -O0 -g -fno-builtin
CCFLAGS_RELEASE = -O2

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
GUI_DIR = res/vendor/imgui

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
GUI_SRCS := $(wildcard $(GUI_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp) $(wildcard $(GUI_DIR)/*.hpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
GUI_OBJS := $(patsubst $(GUI_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(GUI_SRCS))

CCFLAGS += $(CCFLAGS_$(PROFILE)) -I$(INC_DIR) -std=c++20 -Wall -Wextra -Wformat #-Werror
LDFLAGS += -lX11 -lGL -lGLEW -llac

BIN := kingcraft

# Default rule
all: $(BIN)

# Remove object files and binaries
clean:
	rm -f $(BIN) $(OBJ_DIR)/*.o

# Rebuild the project
rebuild: clean all

# Make the binary
$(BIN): $(OBJS) $(GUI_OBJS)
	$(CC) $^ -o $@ $(CCFLAGS) $(LDFLAGS)

# Intermediate objects for the main application
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $< -c -o $@ $(CCFLAGS) $(LDFLAGS)

# Intermediate objects for ImGui
$(OBJ_DIR)/%.o: $(GUI_DIR)/%.cpp
	$(CC) $< -c -o $@ $(CCFLAGS) $(LDFLAGS)

.PHONY: clean all rebuild
