CC := cc
CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

VISUALSSS := visualSSS
CLI_OBJ := $(BUILD_DIR)/cli_parse.o
MAIN_OBJ := $(BUILD_DIR)/main.o
GF257_OBJ := $(BUILD_DIR)/gf257.o
PERMUTATION_TABLE_OBJ := $(BUILD_DIR)/permutation_table.o
BMP_IO_OBJ := $(BUILD_DIR)/bmp_io.o
BMP_METADATA_OBJ := $(BUILD_DIR)/bmp_metadata.o

GF257_TEST := $(BUILD_DIR)/test_gf257
PERMUTATION_TABLE_TEST := $(BUILD_DIR)/test_permutation_table
BMP_TEST := $(BUILD_DIR)/test_bmp

.PHONY: all visualSSS test test-cli clean

all: visualSSS test

visualSSS: $(MAIN_OBJ) $(CLI_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test: $(GF257_TEST) $(PERMUTATION_TABLE_TEST) $(BMP_TEST)
	./$(GF257_TEST)
	./$(PERMUTATION_TABLE_TEST)
	./$(BMP_TEST)

test-cli: visualSSS
	./$(TEST_DIR)/test_cli.sh ./$(VISUALSSS)

$(MAIN_OBJ): $(SRC_DIR)/main.c include/cli.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CLI_OBJ): $(SRC_DIR)/cli/parse.c include/cli.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(GF257_TEST): $(TEST_DIR)/test_gf257.c $(GF257_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(GF257_OBJ): $(SRC_DIR)/gf257.c include/gf257.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(PERMUTATION_TABLE_TEST): $(TEST_DIR)/test_permutation_table.c $(PERMUTATION_TABLE_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(PERMUTATION_TABLE_OBJ): $(SRC_DIR)/permutation_table.c include/permutation_table.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_IO_OBJ): $(SRC_DIR)/bmp/bmp_io.c include/bmp.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_METADATA_OBJ): $(SRC_DIR)/bmp/bmp_metadata.c include/bmp.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_TEST): $(TEST_DIR)/test_bmp.c $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(VISUALSSS)
