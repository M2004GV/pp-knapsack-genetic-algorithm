#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "algoritmo_genetico.h"


Populacao criar_populacao(int tamanho, int n_itens) {
    Populacao populacao;
   
    populacao.tamanho = tamanho;
    populacao.individuos = (Individuo *)malloc(tamanho * sizeof(Individuo));

    if(!populacao.individuos) {
        fprintf(stderr, "Erro ao alocar memória para a população.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < tamanho; i++){
        populacao.individuos[i].genes = (char *)malloc(n_itens * sizeof(char));
        if(!populacao.individuos[i].genes) {
            fprintf(stderr, "Erro ao alocar memória para os genes do indivíduo %d.\n", i);
            exit(EXIT_FAILURE);
        }
        memset(populacao.individuos[i].genes, 0, n_itens * sizeof(char)); 
        populacao.individuos[i].fitness = 0;
        populacao.individuos[i].peso_total = 0;
        populacao.individuos[i].valor_total = 0;
    }
    return populacao;
}

void destruir_populacao(Populacao *populacao) {

    if(!populacao || !populacao->individuos) {        return;     }

    for(int i = 0; i < populacao->tamanho; i++) {
        free(populacao->individuos[i].genes);
        populacao->individuos[i].genes = NULL;
    }

    free(populacao->individuos);
    populacao->individuos = NULL;
    populacao->tamanho = 0;
}

void inicializar_populacao(Populacao *populacao){}

void avaliar_populacao(Populacao *populacao, Mochila *instancia){

    
    for(int i = 0; i < populacao->tamanho; i++){
        int peso_total = 0;
        int valor_total = 0;

        for(int j = 0; j < instancia->n_itens; j++){
            if(populacao->individuos[i].genes[j]){
                peso_total += instancia->itens[j].peso;
                valor_total += instancia->itens[j].valor;
            }
        }

        populacao->individuos[i].peso_total = peso_total;
        populacao->individuos[i].valor_total = valor_total;

        if (peso_total > instancia->capacidade) {
            populacao->individuos[i].fitness = 0;
        } else {
            populacao->individuos[i].fitness = valor_total;
        }

    }
}

int torneio(Populacao *populacao, int k){

    int melhor_idx = rand_int(0, populacao->tamanho - 1);

    for( int i = 1; i < k; i++){
        int comp =  rand_int(0, populacao->tamanho - 1);

        if(populacao->individuos[comp].fitness > populacao->individuos[melhor_idx].fitness) melhor_idx = comp;
        
    }
    return melhor_idx;
}
void crossover(Individuo *pai1, Individuo *pai2, Individuo *filho1, Individuo *filho2, int n_itens){

    int ponto_corte = rand_int(1, n_itens - 1);

    for(int i = 0; i < n_itens; i++){
        if(i < ponto_corte){
            filho1->genes[i] = pai1->genes[i];
            filho2->genes[i] = pai2->genes[i];
        }else{
            filho1->genes[i] = pai2->genes[i];
            filho2->genes[i] = pai1->genes[i];
        }
    }
}
