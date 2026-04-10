#ifndef GA_H
#define GA_H

#include "types.h"

Populacao criar_populacao(int tamanho, int n_itens);
void destruir_populacao(Populacao *populacao);
void inicializar_populacao(Populacao *populacao, int n_itens);
void avaliar_populacao(Populacao *populacao, Mochila *instancia);

int torneio(Populacao *populacao, int k);

void crossover(Individuo *pai1, Individuo *pai2, Individuo *filho1, Individuo *filho2, int n_itens);
void mutacao(Individuo *individuo, double taxa_mutacao, int n_itens);

Individuo melhor_individuo(Populacao *populacao, int n_itens);
void executar_algoritmo_genetico(Mochila *instancia, int tamanho_populacao, int n_geracoes, double taxa_mutacao, int k_torneio);

// algoritmo_genetico.h — altere apenas estas duas linhas
static inline int rand_int(int min, int max) { return min + rand() % (max - min + 1); }
static inline double rand_double() { return (double)rand() / RAND_MAX; }

#endif // GA_H

