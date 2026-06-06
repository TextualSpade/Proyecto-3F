SRC_DIR := src
BIN_DIR := bin

# En SFML 3 añadimos el estándar de C++ moderno
CXXFLAGS := -std=c++20 -Iinclude

# Librerías de SFML 3 (y box2d si lo usas)
SFML := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d

CPP_FILES := $(SRC_DIR)/main.cpp
EXE_FILES := $(BIN_DIR)/main.exe

all: $(EXE_FILES)

$(BIN_DIR)/main.exe: $(SRC_DIR)/main.cpp
	g++ $(CXXFLAGS) -mconsole $< -o $@ $(SFML)

clean:
	rm -f $(BIN_DIR)/*.exe

.PHONY: all clean