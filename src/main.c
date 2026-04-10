// main.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/types.h"
#include "../include/algoritmo_genetico.h"
#include "../include/leitura.h"


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.knap>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    Mochila m = ler_arquivo(argv[1]);
    exibir_mochila(&m);

    int    tam_populacao = 100;
    int    n_geracoes    = 200;
    int    k_torneio     = 3;
    double taxa_mutacao  = 0.02;

    executar_algoritmo_genetico(&m, tam_populacao, n_geracoes, taxa_mutacao, k_torneio);

    liberar_mochila(&m);
    return 0;
}