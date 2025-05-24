// min_heap.c
#include <stdio.h> 
#include <stdlib.h> 
#include "minHeap.h" 

static void troca(nodeHeap *a, nodeHeap *b) {
    nodeHeap aux = *a;
    *a = *b;
    *b = aux;
}
int pai(int i) {
    return (i - 1) / 2;
}
int filhoEsq(int i) {
    return 2 * i + 1;
}
int filhoDir(int i) {
    return 2 * i + 2;
}

theap *contruirHeap(int i) {
    theap *heap = (theap *)malloc(sizeof(theap));
    if (heap == NULL) {
        perror("Erro ao alocar heap");
        exit(EXIT_FAILURE);
    }
    heap->lista = (nodeHeap *)malloc(i * sizeof(nodeHeap));
    if (heap->lista == NULL) {
        perror("Erro ao alocar lista do Heap");
        free(heap);
        exit(EXIT_FAILURE);
    }
    heap->capacidade = i;
    heap->tam = 0;
    return heap;
}

// desce para um MIN-HEAP
void desce(theap *heap, int indice) {
    int menor = indice; 
    int esq = filhoEsq(indice);
    int dir = filhoDir(indice);

    // Verifica se o filho esquerdo existe e é MENOR que o nó atual 
    if (esq < heap->tam && heap->lista[esq].distancia < heap->lista[menor].distancia) {
        menor = esq; 
    }
    // Verifica se o filho direito existe e é MENOR 
    if (dir < heap->tam && heap->lista[dir].distancia < heap->lista[menor].distancia) {
        menor = dir; 

    }
    if (menor != indice) {
        troca(&heap->lista[indice], &heap->lista[menor]); 
        desce(heap, menor); 
    }
}

void sobe(theap *heap, int indice_no_atual) {
    int indice_pai;
    while (indice_no_atual > 0) {
        indice_pai = pai(indice_no_atual);
        if (heap->lista[indice_no_atual].distancia < heap->lista[indice_pai].distancia) {
            troca(&heap->lista[indice_no_atual], &heap->lista[indice_pai]);
            indice_no_atual = indice_pai; // Move o foco para a posição do pai
        } else {
            break; // A propriedade do heap está satisfeita
        }
    }
}


//pensando como se a heap fosse uma min heap, o nó com menor valor é o que está na raiz
void insere(theap *heap, nodeHeap node) {
    //se a heap estive cheia, conferimos se o nó pode ser trocado por algum outro nó da heap
    if (heap->tam == heap->capacidade) {
        if (node.distancia < heap->lista[heap->tam-1].distancia) {
            heap->lista[heap->tam-1] = node; // Adiciona o nó , trocando o nó com maior valor
            troca(&heap->lista[0], &heap->lista[heap->tam-1]); // Troca o nó com o menor valor
            desce(heap, 0); //ajusta
        }
    }
    else {
        heap->lista[heap->tam] = node; // Adiciona o novo nó
        heap->tam++;
        int i = heap->tam - 1;
        sobe(heap, i); // Sobe o nó para a posição correta
    }
}


// nodeHeap extrairMinHeap(MinHeap *minHeap) {
//     if (minHeap->size <= 0) {
//         fprintf(stderr, "Erro: Heap vazio ao tentar extrair.\n");
//         // Retornar um nodeHeap "vazio" ou com um indicador de erro poderia ser uma alternativa
//         // a sair do programa, dependendo da robustez desejada.
//         // Por simplicidade e consistência com o código original:
//         static nodeHeap emptyNode = {0.0, NULL}; // Cuidado com static se multithread
//         if (minHeap->size == 0) return emptyNode; // Ou tratar erro de forma mais robusta
//         // Se o código original usava exit(), pode ser mantido se for o comportamento desejado.
//         // exit(EXIT_FAILURE); // Descomente se preferir sair
//     }
//     if (minHeap->size == 1) {
//         minHeap->size--;
//         return minHeap->array[0];
//     }

//     // Armazena o elemento raiz (o menor)
//     nodeHeap root = minHeap->array[0];

//     // Substitui a raiz pelo último elemento
//     minHeap->array[0] = minHeap->array[minHeap->size - 1];
//     minHeap->size--;

//     // Reorganiza o heap (Bubble-down)
//     heapify(minHeap, 0);

//     return root;
// }

void liberar_heap(theap *heap) {
    if (heap == NULL) return;
    free(heap->lista);
    free(heap);
}