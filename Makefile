CC := cc
CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

VISUALSSS := visualSSS
CLI_OBJ := $(BUILD_DIR)/cli_parse.o
MAIN_OBJ := $(BUILD_DIR)/main.o
APP_OBJ := $(BUILD_DIR)/app_visualsss.o
GF257_OBJ := $(BUILD_DIR)/gf257.o
PERMUTATION_TABLE_OBJ := $(BUILD_DIR)/permutation_table.o
BMP_IO_OBJ := $(BUILD_DIR)/bmp_io.o
BMP_METADATA_OBJ := $(BUILD_DIR)/bmp_metadata.o
LSB_OBJ := $(BUILD_DIR)/lsb.o
STEGO_CAPACITY_OBJ := $(BUILD_DIR)/stego_capacity.o
LAGRANGE_OBJ := $(BUILD_DIR)/lagrange.o
SHAMIR_OBJ := $(BUILD_DIR)/shamir.o

GF257_TEST := $(BUILD_DIR)/test_gf257
PERMUTATION_TABLE_TEST := $(BUILD_DIR)/test_permutation_table
BMP_TEST := $(BUILD_DIR)/test_bmp
LSB_TEST := $(BUILD_DIR)/test_lsb
LAGRANGE_TEST := $(BUILD_DIR)/test_lagrange
SHAMIR_TEST := $(BUILD_DIR)/test_shamir
E2E_TEST := $(BUILD_DIR)/test_e2e
DEMO_LSB_PRUEBA := $(BUILD_DIR)/demo_lsb_prueba

.PHONY: all visualSSS test test-cli demo-lsb-prueba clean

all: visualSSS test

visualSSS: $(MAIN_OBJ) $(CLI_OBJ) $(APP_OBJ) $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) $(LSB_OBJ) $(STEGO_CAPACITY_OBJ) $(SHAMIR_OBJ) $(LAGRANGE_OBJ) $(GF257_OBJ) $(PERMUTATION_TABLE_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test: $(GF257_TEST) $(PERMUTATION_TABLE_TEST) $(BMP_TEST) $(LSB_TEST) $(LAGRANGE_TEST) $(SHAMIR_TEST) $(E2E_TEST) test-cli
	./$(GF257_TEST)
	./$(PERMUTATION_TABLE_TEST)
	./$(BMP_TEST)
	./$(LSB_TEST)
	./$(LAGRANGE_TEST)
	./$(SHAMIR_TEST)
	./$(E2E_TEST)

test-cli: visualSSS
	./$(TEST_DIR)/test_cli.sh ./$(VISUALSSS)

demo-lsb-prueba: $(DEMO_LSB_PRUEBA)
	./$(DEMO_LSB_PRUEBA)

$(MAIN_OBJ): $(SRC_DIR)/main.c include/app.h include/cli.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CLI_OBJ): $(SRC_DIR)/cli/parse.c include/cli.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(APP_OBJ): $(SRC_DIR)/app/visualsss.c include/app.h include/cli.h include/bmp.h include/stego.h include/shamir.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(GF257_TEST): $(TEST_DIR)/test_gf257.c $(GF257_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(GF257_OBJ): $(SRC_DIR)/crypto/gf257.c include/gf257.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(PERMUTATION_TABLE_TEST): $(TEST_DIR)/test_permutation_table.c $(PERMUTATION_TABLE_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(PERMUTATION_TABLE_OBJ): $(SRC_DIR)/crypto/permutation_table.c include/permutation_table.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LAGRANGE_OBJ): $(SRC_DIR)/crypto/lagrange.c include/lagrange.h include/gf257.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(SHAMIR_OBJ): $(SRC_DIR)/crypto/shamir.c include/shamir.h include/lagrange.h include/gf257.h include/permutation_table.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_IO_OBJ): $(SRC_DIR)/bmp/bmp_io.c include/bmp.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_METADATA_OBJ): $(SRC_DIR)/bmp/bmp_metadata.c include/bmp.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BMP_TEST): $(TEST_DIR)/test_bmp.c $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(LSB_OBJ): $(SRC_DIR)/stego/lsb.c include/stego.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(STEGO_CAPACITY_OBJ): $(SRC_DIR)/stego/stego_capacity.c include/stego.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LSB_TEST): $(TEST_DIR)/test_lsb.c $(LSB_OBJ) $(STEGO_CAPACITY_OBJ) $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(LAGRANGE_TEST): $(TEST_DIR)/test_lagrange.c $(LAGRANGE_OBJ) $(GF257_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(SHAMIR_TEST): $(TEST_DIR)/test_shamir.c $(SHAMIR_OBJ) $(LAGRANGE_OBJ) $(GF257_OBJ) $(PERMUTATION_TABLE_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(E2E_TEST): $(TEST_DIR)/test_e2e.c $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) visualSSS | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_DIR)/test_e2e.c $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) -o $@

$(DEMO_LSB_PRUEBA): $(TEST_DIR)/demo_lsb_prueba.c $(LSB_OBJ) $(STEGO_CAPACITY_OBJ) $(BMP_IO_OBJ) $(BMP_METADATA_OBJ) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(VISUALSSS)
