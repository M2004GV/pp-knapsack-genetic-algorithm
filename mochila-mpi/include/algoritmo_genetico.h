#ifndef GA_H
#define GA_H

#include "types.h"

Populacao criar_populacao(int tamanho, int n_itens);
void destruir_populacao(Populacao *populacao);
void copiar_individuo(const Individuo *origem, Individuo *destino, int n_itens);

void reparar_individuo(Individuo *individuo, Mochila *instancia);
void inicializar_populacao(Populacao *populacao, Mochila *instancia,
                           unsigned int *seed);
void avaliar_populacao(Populacao *populacao, Mochila *instancia);

int  torneio(Populacao *populacao, int k, unsigned int *seed);

void crossover(const Individuo *pai1, const Individuo *pai2,
               Individuo *filho1, Individuo *filho2,
               int n_itens, unsigned int *seed);
void mutacao(Individuo *individuo, double taxa_mutacao,
             int n_itens, unsigned int *seed);

Individuo melhor_individuo(Populacao *populacao, int n_itens);

Individuo executar_ag_mpi(Mochila *inst, int tam_pop, int n_geracoes,
                          double taxa_mut, int k_torneio,
                          unsigned int semente_base,
                          int rank, int size, int intervalo_migracao);

#endif // GA_H
