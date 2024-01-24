CC = g++
PROFILE ?= DEBUG

CCFLAGS_DEBUG = -DDEBUG -O0 -g -fno-builtin
CCFLAGS_RELEASE = -O2

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

CCFLAGS += $(CCFLAGS_$(PROFILE)) -I$(INC_DIR) -Wall -Wextra -Wformat -std=c++20 #-Werror
LDFLAGS += -lX11 -lXi -lGL -lGLEW -lglfw -llac -lgui 

BIN := kingcraft

# Default rule
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
	$(CC) $< -c -o $@ $(CCFLAGS)

.PHONY: clean all rebuild
