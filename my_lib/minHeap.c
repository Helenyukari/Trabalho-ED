// min_heap.c
#include <stdio.h>  // Para perror, fprintf, stderr
#include <stdlib.h> // Para malloc, free, exit, EXIT_FAILURE
#include "minHeap.h" // Inclui as definições e protótipos do nosso heap

// Função auxiliar para trocar dois nodeHeap
// Pode ser 'static' pois só é usada dentro deste arquivo.
static void troca(nodeHeap *a, nodeHeap *b) {
    nodeHeap aux = *a;
    *a = *b;
    *b = aux;
}

// --- Funções Auxiliares de Heap (Implementação Básica) ---
MinHeap *contruir_minHeap(int capacity) {
    MinHeap *minHeap = (MinHeap *)malloc(sizeof(MinHeap));
    if (minHeap == NULL) {
        perror("Erro ao alocar MinHeap");
        exit(EXIT_FAILURE);
    }
    minHeap->array = (nodeHeap *)malloc(capacity * sizeof(nodeHeap));
    if (minHeap->array == NULL) {
        perror("Erro ao alocar array do Heap");
        free(minHeap);
        exit(EXIT_FAILURE);
    }
    minHeap->capacity = capacity;
    minHeap->size = 0;
    return minHeap;
}

void heapify(MinHeap *minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left].distance < minHeap->array[smallest].distance) {
        smallest = left;
    }
    if (right < minHeap->size && minHeap->array[right].distance < minHeap->array[smallest].distance) {
        smallest = right;
    }

    if (smallest != idx) {
        troca(&minHeap->array[idx], &minHeap->array[smallest]);
        heapify(minHeap, smallest);
    }
}

void inserirMinHeap(MinHeap *minHeap, nodeHeap item) {
    // A lógica complexa de quando o heap está cheio foi simplificada aqui
    // e a decisão de substituir é feita na função _kdtree_BuscaProximo
    // Esta função de inserção apenas insere se houver capacidade ou
    // se o heap está cheio, assume que a substituição já foi decidida antes de chamar.
    // No entanto, a versão original tinha uma lógica para substituir o topo.
    // Vamos manter a lógica original de 'subir o elemento' se ele for inserido.

    if (minHeap->size == minHeap->capacity) {
        // Se o heap está cheio, a lógica de decidir se este 'item' deve substituir o topo
        // (max->array[0] no contexto do max-heap simulado)
        // é feita ANTES de chamar inserirMinHeap na sua função _kdtree_BuscaProximo.
        // Ex: extrairMinHeap() é chamado primeiro, depois inserirMinHeap().
        // Portanto, se chegamos aqui com o heap cheio, é um erro de lógica ou
        // a capacidade é 0. Por segurança, podemos imprimir um aviso ou simplesmente retornar.
        // Para o KNN, é comum remover o "pior" e inserir o novo se o novo for melhor.
        // Sua lógica de inserirMinHeap tinha um tratamento para isso, que recrio aqui.
        if (item.distance > minHeap->array[0].distance) { // Simula max-heap: nova distância negativa é "maior"
            minHeap->array[0] = item; // Substitui o topo (que seria o mais distante no max-heap)
            heapify(minHeap, 0); // Reconstroi o heap a partir do topo
        }
        // Se não for melhor, não faz nada.
        return;
    }

    // Se há espaço, insere no final e sobe.
    int i = minHeap->size;
    minHeap->array[i] = item;
    minHeap->size++;

    // Sobe o elemento para manter a propriedade do heap (Bubble-up)
    // O pai de 'i' é '(i-1)/2'
    while (i != 0 && minHeap->array[(i - 1) / 2].distance > minHeap->array[i].distance) {
        troca(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}


nodeHeap extrairMinHeap(MinHeap *minHeap) {
    if (minHeap->size <= 0) {
        fprintf(stderr, "Erro: Heap vazio ao tentar extrair.\n");
        // Retornar um nodeHeap "vazio" ou com um indicador de erro poderia ser uma alternativa
        // a sair do programa, dependendo da robustez desejada.
        // Por simplicidade e consistência com o código original:
        static nodeHeap emptyNode = {0.0, NULL}; // Cuidado com static se multithread
        if (minHeap->size == 0) return emptyNode; // Ou tratar erro de forma mais robusta
        // Se o código original usava exit(), pode ser mantido se for o comportamento desejado.
        // exit(EXIT_FAILURE); // Descomente se preferir sair
    }
    if (minHeap->size == 1) {
        minHeap->size--;
        return minHeap->array[0];
    }

    // Armazena o elemento raiz (o menor)
    nodeHeap root = minHeap->array[0];

    // Substitui a raiz pelo último elemento
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    minHeap->size--;

    // Reorganiza o heap (Bubble-down)
    heapify(minHeap, 0);

    return root;
}

void freeMinHeap(MinHeap *minHeap) {
    if (minHeap == NULL) return;
    free(minHeap->array);
    free(minHeap);
}