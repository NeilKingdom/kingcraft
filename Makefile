CC = g++
PROFILE ?= RELEASE

CCFLAGS_DEBUG = -DDEBUG -O0 -ggdb -fno-builtin
CCFLAGS_RELEASE = -Ofast

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

IMGUI = res/vendor/imgui

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

CCFLAGS += $(CCFLAGS_$(PROFILE)) -I$(INC_DIR) -I$(IMGUI)/include -std=c++20 -Wall -Wextra
LDFLAGS += -L$(IMGUI)/bin -l:imgui.a -llac -limc -lX11 -lGL -lGLEW

BIN := kingcraft

# Default goal
all: $(BIN)

# Remove object files and binaries
clean:
	rm -f $(BIN) $(OBJ_DIR)/*.o

# Rebuild the project
rebuild: clean all

# Make the binary
$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(CCFLAGS) $(LDFLAGS)

# Intermediate objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $< -c -o $@ $(CCFLAGS) $(LDFLAGS)

.PHONY: clean all rebuild imgui
