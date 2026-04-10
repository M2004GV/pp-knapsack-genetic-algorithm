CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = $(wildcard src/*.c)
TARGET = mochila

INSTANCE ?= data/large_scale/knapPI_3_100_1000_1
POP ?= 100
GEN ?= 200
MUT ?= 0.02
TOUR ?= 3
SEED ?= 42

.PHONY: all run clean help

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(INSTANCE) $(POP) $(GEN) $(MUT) $(TOUR) $(SEED)

clean:
	rm -f $(TARGET)

help:
	@echo "Targets disponiveis:"
	@echo "  make           -> compila o projeto"
	@echo "  make run       -> compila e executa com parametros padrao"
	@echo "  make clean     -> remove o executavel"
	@echo ""
	@echo "Voce pode sobrescrever variaveis na linha de comando:"
	@echo "  make run INSTANCE=data/large_scale/knapPI_3_500_1000_1 POP=200 GEN=500 MUT=0.03 TOUR=5 SEED=123"
