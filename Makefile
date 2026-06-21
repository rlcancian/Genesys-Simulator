BUILD_DIR := build/terminal-smart
TEST_BUILD_DIR := build/tests-kernel-unit

TERMINAL_BINARY := $(BUILD_DIR)/source/applications/terminal/genesys_terminal_application
PETRI_TEST_BINARY := $(TEST_BUILD_DIR)/source/tests/unit/genesys_test_petri_net

.PHONY: build run clean rebuild test test-all

build:
	rm -rf $(BUILD_DIR)
	cmake --preset terminal-smart
	cmake --build --preset terminal-smart

run:
	@test -f $(TERMINAL_BINARY) || \
		(echo "Executável não encontrado. Execute 'make build' primeiro."; exit 1)
	$(TERMINAL_BINARY)

clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR)

rebuild: clean build

# Constrói e roda especificamente o teste da Rede de Petri
test:
	@echo "==> Configurando o preset de testes..."
	cmake --preset tests-kernel-unit
	@echo "==> Compilando o teste da Rede de Petri..."
	cmake --build $(TEST_BUILD_DIR) --target genesys_test_petri_net
	@echo "==> Executando o teste..."
	$(PETRI_TEST_BINARY)

# Bônus: Roda todos os testes da suite do kernel
test-all:
	@echo "==> Configurando o preset de testes..."
	cmake --preset tests-kernel-unit
	@echo "==> Compilando todos os testes..."
	cmake --build $(TEST_BUILD_DIR)
	@echo "==> Executando testes via CTest..."
	ctest --test-dir $(TEST_BUILD_DIR) --output-on-failure
