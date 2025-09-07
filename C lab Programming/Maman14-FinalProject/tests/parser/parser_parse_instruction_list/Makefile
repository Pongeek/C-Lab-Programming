CC = gcc
CFLAGS = -Wall -pedantic -std=c90 -ansi
LDFLAGS =

# Source directory
SRC_DIR = ../../../source
UTILS_DIR = ../../../utils
HEADERS_DIR = ../../../headers

# List of source files
SRCS = $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/preprocessor.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/error_handler.c \
       $(SRC_DIR)/safe_allocations.c \
       $(UTILS_DIR)/string_util.c \
       $(UTILS_DIR)/char_util.c \
       parser_parse_instruction_list_test.c

# Output executable
TARGET = parser_parse_instruction_list

# Object files
OBJS = $(SRCS:.c=.o)

# Test files
TEST_FILES = test1

# Default target
all: $(TARGET) copy_test_files

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -I$(HEADERS_DIR) -c $< -o $@

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
