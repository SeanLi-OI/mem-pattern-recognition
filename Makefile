SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

CXX := g++

EXE := $(BIN_DIR)/mpr
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CPPFLAGS = -Iinclude -std=c++17 -DDEBUG -g -O0
else
    CPPFLAGS=  -Iinclude -std=c++17 -O3
endif
CFLAGS   :=
LDFLAGS  :=
LDLIBS   :=

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@


clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)