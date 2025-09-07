CC = gcc
CFLAGS = -Wall -pedantic -std=c90 -ansi
LDFLAGS =

# List of source files
SRCS = ../../../source/lexer.c \
       ../../../source/error_handler.c \
       ../../../source/safe_allocations.c \
       ../../../utils/string_util.c \
       ../../../utils/char_util.c \
       tokenize_nonOp_identifiers_test.c

# Output executable
TARGET = tokenize_nonOp_identifiers_test

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
