PROGRAM_NAME := main

SRC_DIR := src
INC_DIR := inc
BUILD_DIR := build

SOURCE_EXTENSION := .cpp

SOURCE_FILES := $(wildcard $(SRC_DIR)/*$(SOURCE_EXTENSION))
OBJECT_FILES := $(patsubst $(SRC_DIR)/%$(SOURCE_EXTENSION),$(BUILD_DIR)/%.o,$(SOURCE_FILES))
DEPENDENCY_FILES := $(OBJECT_FILES:.o=.d)
EXEC_FILE := $(BUILD_DIR)/$(PROGRAM_NAME)

CC := c++
CFLAGS := -MMD -MP -std=c++20 -pedantic -Wall -Wextra -O3 -I$(INC_DIR)
LDFLAGS := -lsqlite3

all: $(EXEC_FILE)

$(BUILD_DIR):
	mkdir -pv $@

# check if the build directory exists
$(OBJECT_FILES): | $(BUILD_DIR)

# compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%$(SOURCE_EXTENSION)
	$(CC) -c $(CFLAGS) $< -o $@

-include $(DEPENDENCY_FILES)

# link
$(EXEC_FILE): $(OBJECT_FILES)
	$(CC) $^ $(LDFLAGS) -o $@

run: $(EXEC_FILE)
	./$(EXEC_FILE)

# remove the build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
