CC = g++
PROFILE ?= RELEASE

CCFLAGS_DEBUG = -DDEBUG -O0 -ggdb -fno-builtin
CCFLAGS_RELEASE = -Ofast

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
GUI_DIR = res/vendor/imgui

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp) $(wildcard $(GUI_DIR)/*.hpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
GUI_SRCS := $(wildcard $(GUI_DIR)/*.cpp)
GUI_OBJS := $(patsubst $(GUI_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(GUI_SRCS))

CCFLAGS += $(CCFLAGS_$(PROFILE)) -I$(INC_DIR) -I$(GUI_DIR) -std=c++20 -Wall -Wextra -Wformat #-Werror
LDFLAGS += -lX11 -lGL -lGLEW -llac -limc

BIN := kingcraft

# Default goal
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
