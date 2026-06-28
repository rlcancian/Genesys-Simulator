# ==========================================================
# Configuração geral
# ==========================================================

# Diretórios de build
GUI_BUILD_DIR := build/gui
TERMINAL_BUILD_DIR := build/terminal-app

# Qt6 path (Ubuntu/Debian)
QT6_PATH := /usr/lib/x86_64-linux-gnu/cmake/Qt6

# Número de threads
JOBS := $(shell nproc)

# Binários
GUI_BINARY := $(GUI_BUILD_DIR)/source/applications/gui/qt/GenesysQtGUI/genesys_qt_gui_application
TERMINAL_BINARY := $(TERMINAL_BUILD_DIR)/source/applications/terminal/genesys_terminal_application

# Cache files (evita reconfiguração)
GUI_CMAKE_CACHE := $(GUI_BUILD_DIR)/CMakeCache.txt
TERMINAL_CMAKE_CACHE := $(TERMINAL_BUILD_DIR)/CMakeCache.txt

.PHONY: \
	gui run-gui gui-clean gui-reconfigure \
	terminal run-terminal terminal-clean terminal-reconfigure \
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
# CLEAN
# ==========================================================

gui-clean:
	rm -rf $(GUI_BUILD_DIR)

terminal-clean:
	rm -rf $(TERMINAL_BUILD_DIR)

clean: gui-clean terminal-clean
