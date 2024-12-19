CC=gcc

SRC_DIR = src
BUILD_DIR = build
C_FLAGS = -Wall -pedantic -std=gnu99

C_SOURCES = $(shell find $(SRC_DIR) -name '*.c')
EXEC = $(BUILD_DIR)/file-warden

all: $(EXEC)

$(EXEC): $(C_SOURCES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(C_SOURCES) -o $(EXEC) $(CFLAGS)

clean:
	rm -r $(BUILD_DIR)

run:
	@$(EXEC)

.phony: all clean run
