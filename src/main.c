// main.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/types.h"
#include "../include/algoritmo_genetico.h"
#include "../include/leitura.h"

static void exibir_uso(const char *programa) {
    fprintf(stderr,
            "Uso: %s <arquivo_instancia> [tam_pop] [geracoes] [taxa_mutacao] [k_torneio] [semente]\n"
            "Exemplo: %s data/large_scale/knapPI_3_100_1000_1 200 500 0.02 3 42\n",
            programa,
            programa);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        exibir_uso(argv[0]);
        return EXIT_FAILURE;
    }

    int tam_populacao = (argc > 2) ? atoi(argv[2]) : 100;
    int n_geracoes = (argc > 3) ? atoi(argv[3]) : 200;
    double taxa_mutacao = (argc > 4) ? atof(argv[4]) : 0.02;
    int k_torneio = (argc > 5) ? atoi(argv[5]) : 3;
    unsigned int semente = (argc > 6) ? (unsigned int)strtoul(argv[6], NULL, 10) : (unsigned int)time(NULL);

    if (tam_populacao <= 0 || n_geracoes <= 0 || taxa_mutacao < 0.0 || taxa_mutacao > 1.0 || k_torneio <= 0) {
        fprintf(stderr, "Erro: parametros invalidos.\n");
        exibir_uso(argv[0]);
        return EXIT_FAILURE;
    }

    srand(semente);

    Mochila m = ler_arquivo(argv[1]);
    exibir_mochila(&m);
    printf("\nSemente utilizada : %u\n\n", semente);

    executar_algoritmo_genetico(&m, tam_populacao, n_geracoes, taxa_mutacao, k_torneio);

    liberar_mochila(&m);
    return EXIT_SUCCESS;
}
