#include <stdlib.h>
#include <stdio.h> 

//Precisa ser reestruturado !em desenvolvimento
typedef struct {
    double distancia;
    void *node;
} nodeHeap;

typedef struct {
    nodeHeap *lista;
    int capacidade;
    int tam;
} theap;

void troca(nodeHeap *a, nodeHeap *b);
int pai(int i);
int filhoEsq(int i);
int filhoDir(int i);
theap *construirHeap(int i);
void desce(theap *heap, int i);
void sobe(theap *heap, int i);
void insere(theap *heap, nodeHeap node);
void liberar_heap(theap *heap);

