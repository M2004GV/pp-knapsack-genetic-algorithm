#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "algoritmo_genetico.h"
#include "rng.h"
#include "types.h"

Populacao criar_populacao(int tamanho, int n_itens) {
    Populacao pop;
    pop.tamanho = tamanho;
    pop.individuos = malloc(tamanho * sizeof(Individuo));

    for (int i = 0; i < tamanho; i++) {
        pop.individuos[i].genes      = malloc(n_itens * sizeof(int));
        pop.individuos[i].fitness    = 0;
        pop.individuos[i].peso_total = 0;
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

void reparar_individuo(Individuo *ind, Mochila *inst) {
    ind->peso_total  = 0;
    ind->valor_total = 0;
    for (int i = 0; i < inst->n_itens; i++) {
        if (ind->genes[i]) {
            ind->peso_total  += inst->itens[i].peso;
            ind->valor_total += inst->itens[i].valor;
        }
    }

    for (int i = inst->n_itens - 1;
         i >= 0 && ind->peso_total > inst->capacidade; i--) {
        if (ind->genes[i]) {
            ind->genes[i]     = 0;
            ind->peso_total  -= inst->itens[i].peso;
            ind->valor_total -= inst->itens[i].valor;
        }
    }

    ind->fitness = ind->valor_total;
}

void inicializar_populacao(Populacao *pop, Mochila *inst) {
    int n   = inst->n_itens;
    int tam = pop->tamanho;

    #pragma omp parallel
    {
        unsigned int seed = (unsigned int)(time(NULL))
                          ^ (unsigned int)(omp_get_thread_num() * 2654435761u);

        #pragma omp for schedule(static)
        for (int i = 0; i < tam; i++) {
            for (int j = 0; j < n; j++) {
                pop->individuos[i].genes[j] = rand_int_ts(0, 1, &seed);
            }
            reparar_individuo(&pop->individuos[i], inst);
        }
    }
}

void avaliar_populacao(Populacao *pop, Mochila *inst) {
    int tam = pop->tamanho;

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < tam; i++) {
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
    int idx_melhor = 0;

    #pragma omp parallel
    {
        int idx_local = 0;

        #pragma omp for nowait
        for (int i = 1; i < pop->tamanho; i++) {
            if (pop->individuos[i].fitness > pop->individuos[idx_local].fitness) {
                idx_local = i;
            }
        }

        #pragma omp critical
        {
            if (pop->individuos[idx_local].fitness >
                pop->individuos[idx_melhor].fitness) {
                idx_melhor = idx_local;
            }
        }
    }

    Individuo melhor;
    melhor.genes = malloc(n_itens * sizeof(int));
    copiar_individuo(&pop->individuos[idx_melhor], &melhor, n_itens);
    return melhor;
}

void executar_algoritmo_genetico(Mochila *inst, int tam_pop,
                                 int n_geracoes, double taxa_mut,
                                 int k_torneio) {
    int n = inst->n_itens;

    Populacao pop      = criar_populacao(tam_pop, n);
    Populacao nova_pop = criar_populacao(tam_pop, n);

    inicializar_populacao(&pop, inst);
    avaliar_populacao(&pop, inst);

    Individuo melhor_global;
    melhor_global.genes = malloc(n * sizeof(int));
    {
        Individuo tmp = melhor_individuo(&pop, n);
        copiar_individuo(&tmp, &melhor_global, n);
        free(tmp.genes);
    }

    printf("Gen %4d | Melhor fitness: %d\n", 0, melhor_global.fitness);

    for (int g = 1; g <= n_geracoes; g++) {

        #pragma omp parallel
        {
            unsigned int seed = (unsigned int)(g * 1000003u)
                              ^ (unsigned int)(omp_get_thread_num() * 2654435761u);

            #pragma omp for schedule(static)
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

            #pragma omp single
            if (tam_pop % 2 != 0) {
                unsigned int s2 = seed;
                int p = torneio(&pop, k_torneio, &s2);
                copiar_individuo(&pop.individuos[p],
                                 &nova_pop.individuos[tam_pop - 1], n);
            }
        }

        Populacao tmp = pop;
        pop      = nova_pop;
        nova_pop = tmp;

        copiar_individuo(&melhor_global, &pop.individuos[0], n);

        Individuo melhor_ger = melhor_individuo(&pop, n);
        if (melhor_ger.fitness > melhor_global.fitness) {
            copiar_individuo(&melhor_ger, &melhor_global, n);
        }
        free(melhor_ger.genes);

        if (g % 50 == 0 || g == n_geracoes) {
            printf("Gen %4d | Melhor fitness: %d\n", g, melhor_global.fitness);
        }
    }

    printf("\n=== Resultado Final ===\n");
    printf("Valor total : %d\n", melhor_global.valor_total);
    printf("Peso total  : %d / %d\n", melhor_global.peso_total, inst->capacidade);
    printf("Fitness     : %d\n", melhor_global.fitness);

    free(melhor_global.genes);
    destruir_populacao(&pop);
    destruir_populacao(&nova_pop);
}