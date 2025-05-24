#include <stdlib.h>
// --- ESTRUTURA PARA O HEAP ---
// Representa um vizinho com sua distância
// Nota: O campo 'data' é um ponteiro void para manter a generalidade.

typedef struct {
    double distancia;
    void *dados; // Ponteiro que guarda treg
} nodeHeap;

typedef struct {
    nodeHeap *lista;
    int capacidade;
    int tam;
} theap;

//funçãos da heap modificadas para comparar distancias das embeddings
void troca(nodeHeap *a, nodeHeap *b); 
int pai(int i); 
int filhoEsq(int i); 
int filhoDir(int i); 
theap *contruirHeap(int i);
void desce(theap *heap, int indice); 
void sobe(theap *heap, int indice_no_atual); 
void insere(theap *heap, nodeHeap i);
// nodeHeap extrai(theap *heap);  necessario?
void liberar_heap(theap *heap);
