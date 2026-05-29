CC      = mpicc
CFLAGS  = -Wall -Wextra -O2 -Iinclude $(CFLAGS_EXTRA)
SRC     = $(wildcard src/*.c)
TARGET  = mochila_mpi

INSTANCE ?= data/large_scale/knapPI_3_1000_1000_1
NP       ?= 4
POP      ?= 100
GEN      ?= 500
MUT      ?= 0.02
TOUR     ?= 3
SEED     ?= 42
MIG      ?= 10

.PHONY: all run bench clean help

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Execução padrão com NP processos
run: $(TARGET)
	mpirun -np $(NP) ./$(TARGET) $(INSTANCE) $(POP) $(GEN) $(MUT) $(TOUR) $(SEED) $(MIG)

# Benchmark de speedup: 1, 2, 4 e 8 processos
bench: $(TARGET)
	@echo "=== Benchmark MPI (instancia: $(INSTANCE)) ==="
	@for p in 1 2 4 8; do \
		echo "--- $$p processo(s) ---"; \
		mpirun --oversubscribe -np $$p ./$(TARGET) $(INSTANCE) $(POP) $(GEN) $(MUT) $(TOUR) $(SEED) $(MIG) | grep -E "Fitness|Tempo"; \
	done

clean:
	rm -f $(TARGET)

help:
	@echo "Alvos:"
	@echo "  make            -> compila com mpicc"
	@echo "  make run NP=4   -> executa com 4 processos"
	@echo "  make bench      -> mede tempo com 1/2/4/8 processos"
	@echo "  make clean      -> remove o executavel"
	@echo ""
	@echo "Variaveis: INSTANCE, NP, POP, GEN, MUT, TOUR, SEED, MIG"
