CC=gcc
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
BIN = file-warden

CFLAGS = -Wall -pedantic -std=gnu99 -I$(INCLUDE_DIR)
SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJS      = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
LIBS = $(shell pkg-config --cflags --libs libnotify)
EXEC = $(BUILD_DIR)/file-warden

all: $(EXEC)

$(EXEC): $(SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(SOURCES) -o $(EXEC) $(CFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -r $(BUILD_DIR)

.phony: all clean
