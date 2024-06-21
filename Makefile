
# Define the compiler
CC = gcc

# Define the compiler flags
CFLAGS = `pkg-config --cflags glib-2.0 libcurl libxml-2.0`
LDFLAGS = `pkg-config --libs glib-2.0 libcurl libxml-2.0`

# Define the target executable
TARGET = webcrawl

# Define the source file
SRC = webcrawl.c

# Define the object file
OBJ = $(SRC:.c=.o)

# Default target to build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Rule to build the object file
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
