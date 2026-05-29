#ifndef LEITURA_H
#define LEITURA_H

#include "types.h"

Mochila ler_arquivo(const char *caminho);
void distribuir_mochila(Mochila *m, int rank);
void exibir_mochila(Mochila *m);
void liberar_mochila(Mochila *m);

#endif // LEITURA_H
