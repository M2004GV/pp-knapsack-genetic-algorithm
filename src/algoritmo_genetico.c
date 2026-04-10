#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/algoritmo_genetico.h"




Populacao criar_populacao(int tamanho, int n_itens) {
    Populacao populacao;
   
    populacao.tamanho = tamanho;
    populacao.individuos = (Individuo *)malloc(tamanho * sizeof(Individuo));

    if(!populacao.individuos) {
        fprintf(stderr, "Erro ao alocar memória para a população.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < tamanho; i++){
        populacao.individuos[i].genes = (int *)malloc(n_itens * sizeof(int));
        if(!populacao.individuos[i].genes) {
            fprintf(stderr, "Erro ao alocar memória para os genes do indivíduo %d.\n", i);
            exit(EXIT_FAILURE);
        }
        memset(populacao.individuos[i].genes, 0, n_itens * sizeof(int)); 
        populacao.individuos[i].fitness = 0;
        populacao.individuos[i].peso_total = 0;
        populacao.individuos[i].valor_total = 0;
    }
    return populacao;
}

void destruir_populacao(Populacao *populacao) {

    if(!populacao || !populacao->individuos) return; 

    for(int i = 0; i < populacao->tamanho; i++) {
        free(populacao->individuos[i].genes);
        populacao->individuos[i].genes = NULL;
    }

    free(populacao->individuos);
    populacao->individuos = NULL;
    populacao->tamanho = 0;
}

void inicializar_populacao(Populacao *populacao, int n_itens){
    for(int i = 0; i < populacao->tamanho; i++){
        for(int j = 0; j < n_itens; j++){
            populacao->individuos[i].genes[j] = rand() % 2;
        }
    }
}

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

void mutacao(Individuo *individuo, double taxa_mutacao, int n_itens){
    for(int i = 0; i < n_itens; i++){
        if((double)rand() / RAND_MAX < taxa_mutacao){
            individuo->genes[i] = 1 - individuo->genes[i];
        }
    }

}

Individuo melhor_individuo(Populacao *populacao, int n_itens){
    int melhor = 0;
    for(int i = 1; i < populacao->tamanho; i++){
        if(populacao->individuos[i].fitness > populacao->individuos[melhor].fitness){
            melhor = i;
        }
    }

    Individuo copia;
    copia.fitness = populacao->individuos[melhor].fitness;
    copia.peso_total = populacao->individuos[melhor].peso_total;
    copia.valor_total = populacao->individuos[melhor].valor_total;
    copia.genes = (int *)malloc(n_itens * sizeof(int));
    memcpy(copia.genes, populacao->individuos[melhor].genes, n_itens * sizeof(int));

    return copia;
}

void executar_algoritmo_genetico(Mochila *instancia, int tamanho_populacao, int n_geracoes, double taxa_mutacao, int k_torneio){
    int n = instancia->n_itens;

    // garante tamanho par para o crossover trabalhar em pares
    if (tamanho_populacao % 2 != 0) {
        tamanho_populacao++;
    }

    // cria, inicializa e avalia a população inicial
    Populacao pop = criar_populacao(tamanho_populacao, n);
    inicializar_populacao(&pop, n);
    avaliar_populacao(&pop, instancia);

    // guarda o melhor encontrado ao longo de todas as gerações
    Individuo melhor_geral = melhor_individuo(&pop, n);

    printf("=== Algoritmo Genetico ===\n");
    printf("Populacao : %d | Geracoes : %d | Torneio k=%d | Mutacao : %.1f%%\n\n",
           tamanho_populacao, n_geracoes, k_torneio, taxa_mutacao * 100.0);

    for (int g = 0; g < n_geracoes; g++) {

        Populacao nova_pop = criar_populacao(tamanho_populacao, n);

        // preenche nova geração par a par
        for (int i = 0; i < tamanho_populacao; i += 2) {
            int idx1 = torneio(&pop, k_torneio);
            int idx2 = torneio(&pop, k_torneio);

            Individuo *pai1   = &pop.individuos[idx1];
            Individuo *pai2   = &pop.individuos[idx2];
            Individuo *filho1 = &nova_pop.individuos[i];
            Individuo *filho2 = &nova_pop.individuos[i + 1];

            crossover(pai1, pai2, filho1, filho2, n);
            mutacao(filho1, taxa_mutacao, n);
            mutacao(filho2, taxa_mutacao, n);
        }

        avaliar_populacao(&nova_pop, instancia);

        // atualiza melhor geral se a geração produziu solução superior
        Individuo melhor_geracao = melhor_individuo(&nova_pop, n);
        if (melhor_geracao.fitness > melhor_geral.fitness) {
            free(melhor_geral.genes);
            melhor_geral = melhor_geracao;
        } else {
            free(melhor_geracao.genes);
        }

        // substitui população antiga pela nova
        destruir_populacao(&pop);
        pop = nova_pop;

        // exibe progresso a cada 10 gerações e na primeira
        if (g == 0 || (g + 1) % 10 == 0) {
            printf("Geracao %4d | Melhor fitness: %d | Peso: %d / %d\n",
                   g + 1,
                   melhor_geral.fitness,
                   melhor_geral.peso_total,
                   instancia->capacidade);
        }
    }

    // exibe resultado final
    printf("\n=== Resultado Final ===\n");
    printf("Melhor valor encontrado : %d\n", melhor_geral.fitness);
    printf("Peso total utilizado    : %d / %d\n", melhor_geral.peso_total, instancia->capacidade);
    printf("Itens selecionados      : ");
    for (int i = 0; i < n; i++) {
        if (melhor_geral.genes[i]) {
            printf("%d ", i + 1);
        }
    }
    printf("\n");

    free(melhor_geral.genes);
    destruir_populacao(&pop);
}