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

LDFLAGS = -lX11 -lGL -lGLEW -llac
CFLAGS = -I$(DDIR) -L/usr/lib/ -Wall -Wextra -Wformat -ansi -pedantic -std=c++14 $($(BUILD)_FLAGS) #-Werror
BIN := KingCraft

# Default rule
all: $(BIN)

# Make the binary
$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

# Intermediate objects
$(ODIR)/%.o: $(SRCS)
	$(CC) -c $< $(CFLAGS) -o $@

# Remove object files and binaries
clean:
	rm -f $(BIN) $(ODIR)/*.o

rebuild: clean all

.PHONY: clean all rebuild
