CC = gcc
CFLAGS = -Wall -pedantic -std=c90 -ansi
LDFLAGS =

SRC_DIR = ../../source
UTILS_DIR = ../../utils
HEADERS_DIR = ../../headers

SRCS = $(SRC_DIR)/lexer.c \
       $(SRC_DIR)/preprocessor.c \
       $(SRC_DIR)/parser.c \
       $(SRC_DIR)/semantic_analyzer.c \
       $(SRC_DIR)/code_generator.c \
       $(SRC_DIR)/error_handler.c \
       $(SRC_DIR)/safe_allocations.c \
       $(UTILS_DIR)/string_util.c \
       $(UTILS_DIR)/char_util.c \
       main.c

TARGET = assembler

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(HEADERS_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
