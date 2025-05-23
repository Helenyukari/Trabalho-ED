#include <stdlib.h>
// --- ESTRUTURA PARA O HEAP ---
// Representa um vizinho com sua distância
// Nota: O campo 'data' é um ponteiro void para manter a generalidade.
// O código que usa o heap será responsável por saber o tipo real de 'data'.

typedef struct {
    double distance;
    void *data; 
} nodeHeap;

// Estrutura do Min-Heap
typedef struct {
    nodeHeap *array;
    int capacity;
    int size;
} MinHeap;

// --- Funções do Min-Heap (protótipos) ---
MinHeap *contruir_minHeap(int capacity);
void inserirMinHeap(MinHeap *minHeap, nodeHeap item);
nodeHeap extrairMinHeap(MinHeap *minHeap);
void freeMinHeap(MinHeap *minHeap);
void heapify(MinHeap *minHeap, int idx); // Função auxiliar para manter a propriedade do heap
