CXX ?= g++
PROFILE ?= DEBUG

CXXFLAGS_DEBUG := -DDEBUG -O0 -ggdb -fno-builtin
CXXFLAGS_RELEASE := -Ofast

SRC_DIR := src
INC_DIR := include
TGT_DIR := build
OBJ_DIR := $(TGT_DIR)/obj

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

INCLUDES += -I$(INC_DIR) -Ilib/ezc/include -Ilib/quikmafs/include -Ivendor/imgui/include
CXXFLAGS += $(CXXFLAGS_$(PROFILE)) $(INCLUDES) -std=c++20 -Wall -Wextra -MMD -MP
LDFLAGS += -Lvendor/imgui/bin -l:imgui.a -limc -lX11 -lGL -lGLEW

BIN := $(TGT_DIR)/kingcraft

# Default goal
all: $(BIN)

# Remove object files and binaries
clean:
	rm -rf $(TGT_DIR)

# Rebuild the project
rebuild: clean all

# Make the binary
$(BIN): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Intermediate objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $@

.PHONY: clean all prebuild rebuild
