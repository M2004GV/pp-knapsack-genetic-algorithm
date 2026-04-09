#ifndef TYPES_H
#define TYPES_H

typedef struct {
    int valor;
    int peso;
} Item;

typedef struct {
    int n_items;
    int capacidade;
    Item *items;
} InstanciaMochila;

typedef struct {
    int *genes;
    int fitness;
    int peso_total;
    int valor_total;
} Individuo;

typedef struct {
    Individuo *individuos;
    int tamanho;
} Populacao;



#endif // TYPES_H