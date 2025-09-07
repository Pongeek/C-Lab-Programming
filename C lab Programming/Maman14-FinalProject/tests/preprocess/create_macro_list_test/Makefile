CC = gcc
CFLAGS = -Wall -pedantic -std=c90 -ansi
LDFLAGS =

# List of source files
SRCS = ../../../source/lexer.c \
       ../../../source/preprocessor.c \
       ../../../source/error_handler.c \
       ../../../source/safe_allocations.c \
       ../../../utils/string_util.c \
       ../../../utils/char_util.c \
       create_macro_list_test.c

# Output executable
TARGET = create_macro_list_test

# Object files
OBJS = $(SRCS:.c=.o)

# Test files
TEST_FILES = test1 test2 test3 test4

# Default target
all: $(TARGET) copy_test_files

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Copy test files
copy_test_files: $(TEST_FILES)
	@echo "Copying test files..."
	@for file in $(TEST_FILES); do \
		if [ -f $$file ]; then \
			cp $$file ./; \
		else \
			echo "Warning: $$file not found"; \
		fi; \
	done

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_FILES)

.PHONY: all clean copy_test_files
