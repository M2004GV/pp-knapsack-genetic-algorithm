#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "leitura.h"
#include "algoritmo_genetico.h"


int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr,
                "Uso: mpirun -np <P> %s <arquivo> "
                "[pop] [ger] [mut] [k_torneio] [seed] [intervalo_migracao]\n",
                argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    const char *arquivo   = argv[1];
    int    tam_pop        = argc > 2 ? atoi(argv[2]) : 100;
    int    n_ger          = argc > 3 ? atoi(argv[3]) : 200;
    double taxa_mut       = argc > 4 ? atof(argv[4]) : 0.02;
    int    k_torneio      = argc > 5 ? atoi(argv[5]) : 3;
    unsigned int semente  = argc > 6 ? (unsigned int)atoi(argv[6]) : 42u;
    int    intervalo_mig  = argc > 7 ? atoi(argv[7]) : 10;

    Mochila inst;
    if (rank == 0) {
        inst = ler_arquivo(arquivo);
    }
    distribuir_mochila(&inst, rank);

    if (rank == 0) {
        printf("=== AG-MPI para o Problema da Mochila ===\n");
        exibir_mochila(&inst);
        printf("Processos MPI : %d\n", size);
        printf("Pop/ilha      : %d  (total = %d)\n", tam_pop, tam_pop * size);
        printf("Geracoes      : %d\n", n_ger);
        printf("Mutacao       : %.3f\n", taxa_mut);
        printf("Torneio (k)   : %d\n", k_torneio);
        printf("Migracao      : a cada %d geracoes\n", intervalo_mig);
        printf("==========================================\n\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    Individuo melhor = executar_ag_mpi(&inst, tam_pop, n_ger, taxa_mut,
                                       k_torneio, semente,
                                       rank, size, intervalo_mig);

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();
    double tempo_local = t1 - t0;

    struct { int fitness; int rank; } local, global;
    local.fitness = melhor.fitness;
    local.rank    = rank;

    MPI_Reduce(&local, &global, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

    double tempo_max;
    MPI_Reduce(&tempo_local, &tempo_max, 1, MPI_DOUBLE, MPI_MAX, 0,
               MPI_COMM_WORLD);

    int *fitness_ilhas = NULL;
    if (rank == 0) fitness_ilhas = malloc(size * sizeof(int));
    MPI_Gather(&melhor.fitness, 1, MPI_INT,
               fitness_ilhas,   1, MPI_INT, 0, MPI_COMM_WORLD);

    int dono = (rank == 0) ? global.rank : -1;
    MPI_Bcast(&dono, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int n = inst.n_itens;
    Individuo solucao_final;
    solucao_final.genes       = malloc(n * sizeof(int));
    solucao_final.fitness     = 0;
    solucao_final.peso_total  = 0;
    solucao_final.valor_total = 0;

    if (dono == 0) {
        if (rank == 0) {
            memcpy(solucao_final.genes, melhor.genes, n * sizeof(int));
            solucao_final.fitness     = melhor.fitness;
            solucao_final.peso_total  = melhor.peso_total;
            solucao_final.valor_total = melhor.valor_total;
        }
    } else {
        const int TAG_SOL = 200;
        if (rank == dono) {
            int meta[3] = { melhor.fitness, melhor.peso_total, melhor.valor_total };
            MPI_Send(melhor.genes, n, MPI_INT, 0, TAG_SOL, MPI_COMM_WORLD);
            MPI_Send(meta,         3, MPI_INT, 0, TAG_SOL, MPI_COMM_WORLD);
        } else if (rank == 0) {
            int meta[3];
            MPI_Recv(solucao_final.genes, n, MPI_INT, dono, TAG_SOL,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(meta, 3, MPI_INT, dono, TAG_SOL,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            solucao_final.fitness     = meta[0];
            solucao_final.peso_total  = meta[1];
            solucao_final.valor_total = meta[2];
        }
    }

    if (rank == 0) {
        printf("Melhor fitness por ilha:\n");
        for (int i = 0; i < size; i++) {
            printf("  ilha %d (rank %d): %d\n", i, i, fitness_ilhas[i]);
        }

        printf("\n=== Resultado Final (global) ===\n");
        printf("Encontrado na ilha (rank): %d\n", global.rank);
        printf("Valor total : %d\n", solucao_final.valor_total);
        printf("Peso total  : %d / %d\n", solucao_final.peso_total,
                                          inst.capacidade);
        printf("Fitness     : %d\n", solucao_final.fitness);
        printf("Tempo (max entre processos): %.4f s\n", tempo_max);

        free(fitness_ilhas);
    }

    free(solucao_final.genes);
    free(melhor.genes);
    liberar_mochila(&inst);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
