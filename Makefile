CC = g++
PROFILE ?= DEBUG

CCFLAGS_DEBUG = -DDEBUG -O0 -ggdb -fno-builtin
CCFLAGS_RELEASE = -Ofast

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
DEPS := $(wildcard $(INC_DIR)/*.hpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

CCFLAGS += $(CCFLAGS_$(PROFILE)) -I$(INC_DIR) -I./res/vendor/imgui -std=c++20 -Wall -Wextra
LDFLAGS += -L./ -lX11 -lGL -lGLEW -llac -limc -l:imgui.a

BIN := kingcraft

# Default goal
all: $(BIN)

# Remove object files and binaries
clean:
	rm -f $(BIN) $(OBJ_DIR)/*.o

# Rebuild the project
rebuild: clean all

# Make the binary
$(BIN): $(OBJS) ./imgui.a
	$(CC) $^ -o $@ $(CCFLAGS) $(LDFLAGS)

# Intermediate objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $< -c -o $@ $(CCFLAGS) $(LDFLAGS)

.PHONY: clean all rebuild
