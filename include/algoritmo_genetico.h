#ifndef GA_H
#define GA_H

#include "types.h"

Populacao criar_populacao(int tamanho, int n_items);
void destruir_populacao(Populacao *populacao);
void inicializar_populacao(Populacao *populacao);
void avaliar_populacao(Populacao *populacao, InstanciaMochila *instancia);

int torneio(Populacao *populacao, int k);

void crossover(Individuo *pai1, Individuo *pai2, Individuo *filho1, Individuo *filho2, int n_items);

#endif // GA_H