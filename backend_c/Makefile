# Compiler and Flags
CC=gcc
CFLAGS=-Iinclude -std=c99 -O2 -Wall

# Directories and Files
SRCDIR=src
BUILDDIR=build

# --- MODIFICATION 1: Define the target executable name here ---
TARGET=$(BUILDDIR)/disaster_alloc.exe

# List of source files
SOURCES=$(wildcard $(SRCDIR)/*.c)

# Generate object file names from source file names
OBJECTS=$(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)


# --- MODIFICATION 2: The 'all' rule now builds the correct TARGET ---
all: $(TARGET)

# Rule to link all object files into the final executable
$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Rule to compile a .c file into a .o file
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Rule to clean up build artifacts
clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
