#include <stdio.h> 
#include <stdlib.h> 
#include "heap.h" 

void troca(nodeHeap *a, nodeHeap *b) {
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

theap *construirHeap(int i) {
    theap *heap = malloc(sizeof(theap));
    
    if (heap == NULL) {
        return NULL; 
    }
    heap->lista = malloc(i * sizeof(nodeHeap));
    if (heap->lista == NULL) {
        free(heap); 
        return NULL;
    }

    heap->capacidade = i;
    heap->tam = 0;
    
    return heap;
}

void desce(theap *heap, int i) {
    int maior = i;
    int esq = filhoEsq(i);
    int dir = filhoDir(i);

    if (esq < heap->tam && heap->lista[esq].distancia > heap->lista[maior].distancia) {
        maior = esq;
    }
    if (dir < heap->tam && heap->lista[dir].distancia > heap->lista[maior].distancia) {
        maior = dir;
    }

    if (maior != i) {
        troca(&heap->lista[i], &heap->lista[maior]);
        desce(heap, maior);
    }
}

void sobe(theap *heap, int i) {
    int p;
    while (i > 0) {
        p = pai(i);
        if (heap->lista[i].distancia < heap->lista[p].distancia) {
            troca(&heap->lista[i], &heap->lista[p]);
            i = p; 
        }
    }
}

void insere(theap *heap, nodeHeap node) {
    if (heap->tam < heap->capacidade) {
        heap->lista[heap->tam] = node;
        sobe(heap, heap->tam);
        heap->tam++;
    } else if (node.distancia < heap->lista[0].distancia) {
        heap->lista[0] = node;
        desce(heap, 0);
    }
}

void liberar_heap(theap *heap) {
    if (heap == NULL) return;
    free(heap->lista);
    free(heap);
}

