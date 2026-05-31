BUILD_DIR := build/terminal-smart

TERMINAL_BINARY := \
$(BUILD_DIR)/source/applications/terminal/genesys_terminal_application

.PHONY: build run clean rebuild

build:
	rm -rf $(BUILD_DIR)
	cmake --preset terminal-smart
	cmake --build --preset terminal-smart

run:
	@test -f $(TERMINAL_BINARY) || \
		(echo "Executável não encontrado. Execute 'make build' primeiro."; exit 1)
	$(TERMINAL_BINARY)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build