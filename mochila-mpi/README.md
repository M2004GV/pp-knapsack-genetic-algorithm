# Mochila MPI — Algoritmo Genético Paralelo

Trabalho da Unidade 2 — Disciplina de Programação Paralela
Departamento de Ciência da Computação — UERN

Versão **MPI** (memória distribuída) do algoritmo genético para o
Problema da Mochila Binária, evoluído a partir da versão sequencial/OpenMP
da Unidade 1.

---

## Modelo de paralelização: Ilhas (Island Model)

Cada processo MPI é uma **ilha** com sua própria população, que evolui de
forma independente. Periodicamente, as ilhas trocam o seu melhor indivíduo
com a ilha vizinha numa **topologia em anel** (migração). O programa suporta
um número **indefinido** de processos — basta variar o `-np` do `mpirun`.

```
   ilha 0  ──►  ilha 1  ──►  ilha 2  ──►  ...  ──►  ilha N-1
     ▲                                                  │
     └──────────────────────────────────────────────────┘
                     (migração em anel)
```

### Por que ilhas, e não dividir a população?

Em MPI cada processo tem memória própria. Manter uma população por processo
evita comunicação a cada operação genética (que seria custosa pela rede) e
explora regiões diferentes do espaço de busca em paralelo. A comunicação
fica restrita a três momentos: distribuir a instância (início), migrar os
melhores (periódico) e coletar o resultado (fim).

---

## Rotinas MPI utilizadas

| Rotina | Onde | Para quê |
|--------|------|----------|
| `MPI_Init` / `MPI_Finalize` | main | abre/fecha o ambiente MPI |
| `MPI_Comm_rank` / `MPI_Comm_size` | main | identifica o processo e o total |
| `MPI_Bcast` | leitura | distribui a instância do rank 0 para todos |
| `MPI_Sendrecv` | migração | troca anti-deadlock do melhor entre vizinhos |
| `MPI_Reduce` + `MPI_MAXLOC` | main | acha o melhor fitness global e quem o tem |
| `MPI_Gather` | main | coleta o melhor de cada ilha (resumo) |
| `MPI_Send` / `MPI_Recv` | main | dono do melhor envia a solução ao rank 0 |
| `MPI_Barrier` / `MPI_Wtime` | main | cronometragem do trecho paralelo |

---

## Compilação

```bash
make                              # versão padrão (reparo da Unidade 1)
make CFLAGS_EXTRA=-DREPARO_GULOSO=1   # versão com reparo guloso (melhor qualidade)
```

## Execução

```bash
mpirun -np <P> ./mochila_mpi <arquivo> [pop] [ger] [mut] [k_torneio] [seed] [migracao]
```

Exemplo:

```bash
mpirun -np 4 ./mochila_mpi data/large_scale/knapPI_3_1000_1000_1 200 1000 0.02 3 42 50
```

Parâmetros configuráveis (na ordem):
1. **arquivo** — instância de entrada (obrigatório)
2. **pop** — tamanho da população por ilha (padrão 100)
3. **ger** — número máximo de gerações (padrão 200)
4. **mut** — taxa de mutação (padrão 0.02)
5. **k_torneio** — tamanho do torneio (padrão 3)
6. **seed** — semente base (padrão 42)
7. **migracao** — intervalo de migração em gerações; 0 desativa (padrão 10)

> O número de processos vem do ambiente MPI (`-np`), como pede o enunciado.

## Benchmark de speedup

```bash
./benchmark.sh data/large_scale/knapPI_3_5000_1000_1 800 1000
```

Mede o tempo com 1, 2, 4 e 8 processos e calcula o speedup.
**Execute numa máquina com vários núcleos físicos** — com 1 núcleo, os
processos competem pela mesma CPU e o tempo não cai.

---

## Estrutura

```
mochila-mpi/
├── Makefile
├── benchmark.sh
├── include/
│   ├── types.h               # structs (Item, Mochila, Individuo, Populacao)
│   ├── rng.h                 # gerador aleatório por processo (xorshift32)
│   ├── leitura.h
│   └── algoritmo_genetico.h
├── src/
│   ├── leitura.c             # leitura + MPI_Bcast da instância
│   ├── algoritmo_genetico.c  # AG + migração em anel
│   └── main.c                # orquestração MPI
└── data/
    ├── large_scale/          # instâncias
    └── large_scale-optimum/  # valores ótimos de referência
```
