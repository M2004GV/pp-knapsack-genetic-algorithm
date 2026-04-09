#ifndef GA_H
#define GA_H

#include "types.h"

Populacao criar_populacao(int tamanho, int n_itens);
void destruir_populacao(Populacao *populacao);
void inicializar_populacao(Populacao *populacao);
void avaliar_populacao(Populacao *populacao, Mochila *instancia);

int torneio(Populacao *populacao, int k);

void crossover(Individuo *pai1, Individuo *pai2, Individuo *filho1, Individuo *filho2, int n_itens);

#endif // GA_H