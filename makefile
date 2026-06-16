SRC_DIR := src
BIN_DIR := bin

CXXFLAGS := -std=c++20 -Iinclude

SFML := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
EXE_FILES := $(BIN_DIR)/main.exe

all: $(EXE_FILES)

$(BIN_DIR)/main.exe: $(CPP_FILES)
	g++ $(CXXFLAGS) -mwindows $(CPP_FILES) -o $@ $(SFML)

clean:
	rm -f $(BIN_DIR)/*.exe

.PHONY: all clean
