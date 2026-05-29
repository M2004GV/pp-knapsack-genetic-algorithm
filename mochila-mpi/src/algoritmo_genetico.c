#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "algoritmo_genetico.h"
#include "rng.h"
#include "types.h"


Populacao criar_populacao(int tamanho, int n_itens) {
    Populacao pop;
    pop.tamanho = tamanho;
    pop.individuos = malloc(tamanho * sizeof(Individuo));

    for (int i = 0; i < tamanho; i++) {
        pop.individuos[i].genes       = malloc(n_itens * sizeof(int));
        pop.individuos[i].fitness     = 0;
        pop.individuos[i].peso_total  = 0;
        pop.individuos[i].valor_total = 0;
    }
    return pop;
}

void destruir_populacao(Populacao *pop) {
    for (int i = 0; i < pop->tamanho; i++) {
        free(pop->individuos[i].genes);
    }
    free(pop->individuos);
    pop->individuos = NULL;
}

void copiar_individuo(const Individuo *origem, Individuo *destino, int n_itens) {
    memcpy(destino->genes, origem->genes, n_itens * sizeof(int));
    destino->fitness     = origem->fitness;
    destino->peso_total  = origem->peso_total;
    destino->valor_total = origem->valor_total;
}

#ifndef REPARO_GULOSO
#define REPARO_GULOSO 0
#endif

void reparar_individuo(Individuo *ind, Mochila *inst) {
    ind->peso_total  = 0;
    ind->valor_total = 0;
    for (int i = 0; i < inst->n_itens; i++) {
        if (ind->genes[i]) {
            ind->peso_total  += inst->itens[i].peso;
            ind->valor_total += inst->itens[i].valor;
        }
    }

#if REPARO_GULOSO == 0
    for (int i = inst->n_itens - 1;
         i >= 0 && ind->peso_total > inst->capacidade; i--) {
        if (ind->genes[i]) {
            ind->genes[i]     = 0;
            ind->peso_total  -= inst->itens[i].peso;
            ind->valor_total -= inst->itens[i].valor;
        }
    }
#else
    while (ind->peso_total > inst->capacidade) {
        int pior = -1;
        double pior_razao = 1e18;
        for (int i = 0; i < inst->n_itens; i++) {
            if (ind->genes[i]) {
                double razao = (double)inst->itens[i].valor
                             / (double)inst->itens[i].peso;
                if (razao < pior_razao) {
                    pior_razao = razao;
                    pior = i;
                }
            }
        }
        if (pior < 0) break;
        ind->genes[pior]  = 0;
        ind->peso_total  -= inst->itens[pior].peso;
        ind->valor_total -= inst->itens[pior].valor;
    }
    int folga = inst->capacidade - ind->peso_total;
    for (int i = 0; i < inst->n_itens; i++) {
        if (!ind->genes[i] && inst->itens[i].peso <= folga) {
            ind->genes[i]     = 1;
            ind->peso_total  += inst->itens[i].peso;
            ind->valor_total += inst->itens[i].valor;
            folga            -= inst->itens[i].peso;
        }
    }
#endif

    ind->fitness = ind->valor_total;
}

void inicializar_populacao(Populacao *pop, Mochila *inst, unsigned int *seed) {
    int n = inst->n_itens;
    for (int i = 0; i < pop->tamanho; i++) {
        for (int j = 0; j < n; j++) {
            pop->individuos[i].genes[j] = rand_int_ts(0, 1, seed);
        }
        reparar_individuo(&pop->individuos[i], inst);
    }
}

void avaliar_populacao(Populacao *pop, Mochila *inst) {
    for (int i = 0; i < pop->tamanho; i++) {
        reparar_individuo(&pop->individuos[i], inst);
    }
}

int torneio(Populacao *pop, int k, unsigned int *seed) {
    int melhor = rand_int_ts(0, pop->tamanho - 1, seed);
    for (int i = 1; i < k; i++) {
        int cand = rand_int_ts(0, pop->tamanho - 1, seed);
        if (pop->individuos[cand].fitness > pop->individuos[melhor].fitness) {
            melhor = cand;
        }
    }
    return melhor;
}

void crossover(const Individuo *pai1, const Individuo *pai2,
               Individuo *filho1, Individuo *filho2,
               int n_itens, unsigned int *seed) {
    int ponto = rand_int_ts(1, n_itens - 1, seed);

    for (int i = 0; i < ponto; i++) {
        filho1->genes[i] = pai1->genes[i];
        filho2->genes[i] = pai2->genes[i];
    }
    for (int i = ponto; i < n_itens; i++) {
        filho1->genes[i] = pai2->genes[i];
        filho2->genes[i] = pai1->genes[i];
    }
}

void mutacao(Individuo *ind, double taxa, int n_itens, unsigned int *seed) {
    for (int i = 0; i < n_itens; i++) {
        if (rand_double_ts(seed) < taxa) {
            ind->genes[i] ^= 1;
        }
    }
}

Individuo melhor_individuo(Populacao *pop, int n_itens) {
    int idx = 0;
    for (int i = 1; i < pop->tamanho; i++) {
        if (pop->individuos[i].fitness > pop->individuos[idx].fitness) {
            idx = i;
        }
    }
    Individuo melhor;
    melhor.genes = malloc(n_itens * sizeof(int));
    copiar_individuo(&pop->individuos[idx], &melhor, n_itens);
    return melhor;
}

static int indice_pior(Populacao *pop) {
    int idx = 0;
    for (int i = 1; i < pop->tamanho; i++) {
        if (pop->individuos[i].fitness < pop->individuos[idx].fitness) {
            idx = i;
        }
    }
    return idx;
}

static void migrar(Populacao *pop, Mochila *inst,
                   const Individuo *melhor_local,
                   int rank, int size, int *buffer_genes) {
    int n = inst->n_itens;
    int destino = (rank + 1) % size;
    int origem  = (rank - 1 + size) % size;
    const int TAG_MIG = 100;

    MPI_Sendrecv(melhor_local->genes, n, MPI_INT, destino, TAG_MIG,
                 buffer_genes,        n, MPI_INT, origem,  TAG_MIG,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    Individuo imigrante;
    imigrante.genes = buffer_genes;
    reparar_individuo(&imigrante, inst);

    int pior = indice_pior(pop);
    if (imigrante.fitness > pop->individuos[pior].fitness) {
        copiar_individuo(&imigrante, &pop->individuos[pior], n);
    }
}

Individuo executar_ag_mpi(Mochila *inst, int tam_pop, int n_geracoes,
                          double taxa_mut, int k_torneio,
                          unsigned int semente_base,
                          int rank, int size, int intervalo_migracao) {
    int n = inst->n_itens;

    unsigned int seed = semente_base
                      ^ ((unsigned int)(rank + 1) * 2654435761u);
    if (seed == 0) seed = 1;

    Populacao pop      = criar_populacao(tam_pop, n);
    Populacao nova_pop = criar_populacao(tam_pop, n);

    inicializar_populacao(&pop, inst, &seed);
    avaliar_populacao(&pop, inst);

    int *buffer_genes = malloc(n * sizeof(int));

    Individuo melhor_global;
    melhor_global.genes = malloc(n * sizeof(int));
    {
        Individuo tmp = melhor_individuo(&pop, n);
        copiar_individuo(&tmp, &melhor_global, n);
        free(tmp.genes);
    }

    for (int g = 1; g <= n_geracoes; g++) {

        for (int i = 0; i < tam_pop / 2; i++) {
            int p1 = torneio(&pop, k_torneio, &seed);
            int p2 = torneio(&pop, k_torneio, &seed);

            crossover(&pop.individuos[p1], &pop.individuos[p2],
                      &nova_pop.individuos[2 * i],
                      &nova_pop.individuos[2 * i + 1],
                      n, &seed);

            mutacao(&nova_pop.individuos[2 * i],     taxa_mut, n, &seed);
            mutacao(&nova_pop.individuos[2 * i + 1], taxa_mut, n, &seed);

            reparar_individuo(&nova_pop.individuos[2 * i],     inst);
            reparar_individuo(&nova_pop.individuos[2 * i + 1], inst);
        }

        if (tam_pop % 2 != 0) {
            int p = torneio(&pop, k_torneio, &seed);
            copiar_individuo(&pop.individuos[p],
                             &nova_pop.individuos[tam_pop - 1], n);
        }

        Populacao tmp = pop;
        pop      = nova_pop;
        nova_pop = tmp;

        copiar_individuo(&melhor_global, &pop.individuos[0], n);

        if (intervalo_migracao > 0 && size > 1 &&
            g % intervalo_migracao == 0) {
            migrar(&pop, inst, &melhor_global, rank, size, buffer_genes);
        }

        Individuo melhor_ger = melhor_individuo(&pop, n);
        if (melhor_ger.fitness > melhor_global.fitness) {
            copiar_individuo(&melhor_ger, &melhor_global, n);
        }
        free(melhor_ger.genes);
    }

    free(buffer_genes);
    destruir_populacao(&pop);
    destruir_populacao(&nova_pop);

    return melhor_global;
}
