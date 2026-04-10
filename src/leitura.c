#include <stdio.h>
#include <stdlib.h>
#include "../include/leitura.h"

Mochila ler_arquivo(const char *caminho) {
    Mochila m;

    FILE *fp = fopen(caminho, "r");
    if (fp == NULL) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'\n", caminho);
        exit(1);
    }

    // lê cabeçalho: número de itens e capacidade
    fscanf(fp, "%d %d", &m.n_itens, &m.capacidade);

    // aloca memória dinamicamente para os itens
    m.itens = (Item *)malloc(m.n_itens * sizeof(Item));
    if (m.itens == NULL) {
        printf("Erro: falha ao alocar memoria\n");
        fclose(fp);
        exit(1);
    }

    // lê cada item
    for (int i = 0; i < m.n_itens; i++) {
        fscanf(fp, "%d %d", &m.itens[i].valor, &m.itens[i].peso);
    }

    fclose(fp);
    return m;
}

void exibir_mochila(Mochila *m) {
    printf("=== Instancia carregada ===\n");
    printf("Numero de itens : %d\n", m->n_itens);
    printf("Capacidade      : %d\n\n", m->capacidade);

    printf("%-6s %-8s %-8s\n", "Item", "Valor", "Peso");
    printf("----------------------\n");
    for (int i = 0; i < m->n_itens; i++) {
        printf("%-6d %-8d %-8d\n", i + 1, m->itens[i].valor, m->itens[i].peso);
    }
}

void liberar_mochila(Mochila *m) {
    free(m->itens);
    m->itens = NULL;
}