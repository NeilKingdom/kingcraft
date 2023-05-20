CC = g++
BUILD ?= DEBUG

DEBUG_FLAGS = -DDEBUG -O0 -g -fno-builtin
RELEASE_FLAGS = -O2 

DDIR = include
ODIR = obj
SDIR = src

DEPS = $(wildcard $(DDIR)/*.h)
SRCS = $(wildcard $(SDIR)/*.cpp)
OBJS = $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(SRCS))

LDFLAGS = -lX11 -lGL -lGLEW
CFLAGS = -I$(DDIR) -Werror -Wall -Wextra -Wformat -ansi -pedantic -std=c++14 $($(BUILD)_FLAGS) 
BIN := KingCraft

# Default rule
all: $(BIN)

# Make the binary
$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) 

# Intermediate objects
$(OBJS): $(SRCS)
	$(CC) -c $< $(CFLAGS)

# Remove object files and binaries
clean: 
	rm -f $(BIN) $(ODIR)/*.o

.PHONY: clean all
