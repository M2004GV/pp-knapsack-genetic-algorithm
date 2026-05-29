#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "leitura.h"

Mochila ler_arquivo(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", caminho);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    Mochila m;
    if (fscanf(f, "%d %d", &m.n_itens, &m.capacidade) != 2) {
        fprintf(stderr, "Erro: cabecalho invalido em %s\n", caminho);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    m.itens = malloc(m.n_itens * sizeof(Item));
    if (!m.itens) {
        fprintf(stderr, "Erro de alocacao de memoria\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    for (int i = 0; i < m.n_itens; i++) {
        if (fscanf(f, "%d %d", &m.itens[i].valor, &m.itens[i].peso) != 2) {
            fprintf(stderr, "Erro: item %d invalido em %s\n", i, caminho);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }
    fclose(f);
    return m;
}

void distribuir_mochila(Mochila *m, int rank) {
    MPI_Bcast(&m->n_itens,    1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m->capacidade, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        m->itens = malloc(m->n_itens * sizeof(Item));
        if (!m->itens) {
            fprintf(stderr, "Erro de alocacao no rank %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    MPI_Bcast(m->itens, 2 * m->n_itens, MPI_INT, 0, MPI_COMM_WORLD);
}

void exibir_mochila(Mochila *m) {
    printf("Itens: %d | Capacidade: %d\n", m->n_itens, m->capacidade);
}

void liberar_mochila(Mochila *m) {
    free(m->itens);
    m->itens = NULL;
}
