CC=gcc

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

C_FLAGS = -Wall -pedantic -std=gnu99 -I$(INCLUDE_DIR)

C_SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(C_SOURCES:$(SRC_DIR)%.c=$(BUILD_DIR)%.o)
LIBS = $(shell pkg-config --cflags --libs libnotify)

EXEC = $(BUILD_DIR)/file-warden

all: $(EXEC)

$(EXEC): $(C_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(C_SOURCES) -o $(EXEC) $(C_FLAGS) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(C_FLAGS)

clean:
	rm -r $(BUILD_DIR)

run:
	@$(EXEC)

.phony: all clean run
