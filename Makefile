# ==========================================================
# Configuração geral
# ==========================================================

# Diretórios de build
GUI_BUILD_DIR := build/gui
TERMINAL_BUILD_DIR := build/terminal-app
UNIT_TEST_BUILD_DIR := build/tests-kernel-unit

# Qt6 path (Ubuntu/Debian)
QT6_PATH := /usr/lib/x86_64-linux-gnu/cmake/Qt6

# Número de threads
JOBS := $(shell nproc)

# Pacote opcional de testes unitários. Ex.: make run-unit-tests PACKAGE=tools
PACKAGE ?=
UNIT_TEST_TARGET := $(if $(PACKAGE),genesys_$(PACKAGE)_unit_tests,genesys_kernel_unit_tests)
UNIT_TEST_CTEST_FILTER := $(if $(PACKAGE),-R "^$(PACKAGE)\.",)
INTEGRATION_TEST_TARGET := $(if $(PACKAGE),genesys_$(PACKAGE)_integration_tests,genesys_integration_tests)
INTEGRATION_TEST_CTEST_FILTER := $(if $(PACKAGE),-R "^integration\.$(PACKAGE)\.",-L integration)

# Binários
GUI_BINARY := $(GUI_BUILD_DIR)/source/applications/gui/qt/GenesysQtGUI/genesys_qt_gui_application
TERMINAL_BINARY := $(TERMINAL_BUILD_DIR)/source/applications/terminal/genesys_terminal_application

# Cache files (evita reconfiguração)
GUI_CMAKE_CACHE := $(GUI_BUILD_DIR)/CMakeCache.txt
TERMINAL_CMAKE_CACHE := $(TERMINAL_BUILD_DIR)/CMakeCache.txt
UNIT_TEST_CMAKE_CACHE := $(UNIT_TEST_BUILD_DIR)/CMakeCache.txt

EXAMPLES_BUILD_DIR := build/examples
EXAMPLES_BINARY := $(EXAMPLES_BUILD_DIR)/examples/genesys_examples_analysis_tools
EXAMPLES_CMAKE_CACHE := $(EXAMPLES_BUILD_DIR)/CMakeCache.txt

.PHONY: \
	gui run-gui gui-clean gui-reconfigure \
	terminal run-terminal terminal-clean terminal-reconfigure \
	unit-tests unit-tests-configure run-unit-tests test tests unit-tests-clean unit-tests-reconfigure \
	integration-tests integration-tests-configure run-integration-tests integration-tests-clean integration-tests-reconfigure \
	examples run-examples examples-clean \
	clean

# ==========================================================
# GUI
# ==========================================================

# Configura GUI apenas se necessário
$(GUI_CMAKE_CACHE):
	cmake -S . -B $(GUI_BUILD_DIR) -G Ninja \
		-DGENESYS_BUILD_GUI_APPLICATION=ON \
		-DGENESYS_BUILD_WEB_APPLICATION=ON \
		-DGENESYS_BUILD_TESTS=OFF \
		-DCMAKE_PREFIX_PATH=$(QT6_PATH)

# Build incremental GUI
gui: $(GUI_CMAKE_CACHE)
	cmake --build $(GUI_BUILD_DIR) \
		--target genesys_gui \
		-j$(JOBS)

# Compila e executa GUI
run-gui: gui
	$(GUI_BINARY)

# Força reconfiguração GUI
gui-reconfigure:
	rm -f $(GUI_CMAKE_CACHE)
	$(MAKE) gui


# ==========================================================
# TERMINAL
# ==========================================================

# Configura terminal apenas se necessário
$(TERMINAL_CMAKE_CACHE):
	cmake --preset terminal-app

# Build incremental terminal
terminal: $(TERMINAL_CMAKE_CACHE)
	cmake --build --preset terminal-app -j$(JOBS)

# Compila e executa terminal
run-terminal: terminal
	$(TERMINAL_BINARY)

# Força reconfiguração terminal
terminal-reconfigure:
	rm -f $(TERMINAL_CMAKE_CACHE)
	$(MAKE) terminal


# ==========================================================
# UNIT TESTS
# ==========================================================

# Configura testes unitários e limpa cache de outro checkout quando necessário
unit-tests-configure:
	@if [ -f $(UNIT_TEST_CMAKE_CACHE) ] && \
		! grep -q '^CMAKE_HOME_DIRECTORY:INTERNAL=$(CURDIR)$$' $(UNIT_TEST_CMAKE_CACHE); then \
		echo "Cache CMake de testes aponta para outro diretório. Recriando $(UNIT_TEST_BUILD_DIR)..."; \
		rm -rf $(UNIT_TEST_BUILD_DIR); \
	fi
	cmake --preset tests-kernel-unit

# Build incremental de toda a bateria de testes unitários
unit-tests: unit-tests-configure
	cmake --build $(UNIT_TEST_BUILD_DIR) \
		--target $(UNIT_TEST_TARGET) \
		-j$(JOBS)

# Compila e executa os testes unitários descobertos pelo GoogleTest/CTest.
# Use PACKAGE=tools para restringir ao pacote de tools.
run-unit-tests: unit-tests
	ctest --preset tests-kernel-unit $(UNIT_TEST_CTEST_FILTER) --output-on-failure

test tests: run-unit-tests

# Força reconfiguração dos testes unitários
unit-tests-reconfigure:
	rm -rf $(UNIT_TEST_BUILD_DIR)
	$(MAKE) unit-tests


# ==========================================================
# INTEGRATION TESTS
# ==========================================================

# Configura testes de integração no mesmo preset de testes.
integration-tests-configure: unit-tests-configure

# Build incremental da bateria de testes de integração.
# Use PACKAGE=tools para restringir ao pacote de tools.
integration-tests: integration-tests-configure
	cmake --build $(UNIT_TEST_BUILD_DIR) \
		--target $(INTEGRATION_TEST_TARGET) \
		-j$(JOBS)

# Compila e executa os testes de integração.
# Use PACKAGE=tools para restringir ao pacote de tools.
run-integration-tests: integration-tests
	ctest --preset tests-kernel-unit $(INTEGRATION_TEST_CTEST_FILTER) --output-on-failure

# Força reconfiguração dos testes de integração
integration-tests-reconfigure:
	rm -rf $(UNIT_TEST_BUILD_DIR)
	$(MAKE) integration-tests


# ==========================================================
# EXAMPLES
# ==========================================================

$(EXAMPLES_CMAKE_CACHE):
	cmake --preset examples

examples: $(EXAMPLES_CMAKE_CACHE)
	cmake --build --preset examples -j$(JOBS)

# Compila e executa o exemplo de ferramentas de análise.
# Passa o caminho do arquivo de dados a partir da raiz do projeto.
run-examples: examples
	$(EXAMPLES_BINARY)

examples-clean:
	rm -rf $(EXAMPLES_BUILD_DIR)


# ==========================================================
# CLEAN
# ==========================================================

gui-clean:
	rm -rf $(GUI_BUILD_DIR)

terminal-clean:
	rm -rf $(TERMINAL_BUILD_DIR)

unit-tests-clean:
	rm -rf $(UNIT_TEST_BUILD_DIR)

integration-tests-clean: unit-tests-clean

clean: gui-clean terminal-clean unit-tests-clean examples-clean
