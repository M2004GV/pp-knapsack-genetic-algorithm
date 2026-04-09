#include <stdio.h>
#include "types.h"
#include "algoritmo_genetico.h"

#include "leitura.c"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.knap>\n", argv[0]);
        return 1;
    }

    Mochila m = ler_arquivo(argv[1]);
    exibir_mochila(&m);
    liberar_mochila(&m);

    return 0;
}