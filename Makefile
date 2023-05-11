CXX       := gcc
CXX_FLAGS := -std=c17 -lpthread -ggdb

BIN     := bin
SRC     := src

LIBRARIES   :=
EXECUTABLE  := main


all: $(BIN)/$(EXECUTABLE)

run: clean all

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
	$(CXX) $^ -o $@ $(LIBRARIES) $(CXX_FLAGS)

clean:
	-rm -rf $(BIN)/*
