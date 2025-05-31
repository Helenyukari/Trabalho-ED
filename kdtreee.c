#include <stdio.h>
#include <stdlib.h>
#include <float.h> 
#include <string.h>
#include <math.h> 
#include "my_lib/heap.h"

/*Definições desenvolvedor usuario*/
#define tam_emb 128 
#define tam_id 100 

//Ajustado para as embeddings + ID
typedef struct _reg {
    float embedding[tam_emb];
    char person_id[tam_id];  
} treg;

//ajustado para alocar a treg e as embaddings
void *aloca_reg(float *embedding, char *id) {
    treg *reg;
    reg = malloc(sizeof(treg));
    memcpy(reg->embedding, embedding, tam_emb * sizeof(float));
    strncpy(reg->person_id, id, tam_id - 1);
    return reg;
}

// Comparador para embeddings
int comparador(void *a, void *b, int pos) {
    float va = ((treg *)a)->embedding[pos]; 
    float vb = ((treg *)b)->embedding[pos];
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

// Distância Euclidiana 
double distancia(void *a, void *b) {
    double soma = 0.0;
    for (int i = 0; i < tam_emb; ++i) {
        double dif = ((treg *)a)->embedding[i] - ((treg *)b)->embedding[i];
        soma += dif * dif;
    }
    return sqrt(soma); 
}

/*Definições desenvolvedor da biblioteca*/
typedef struct _node {
    void *key; 
    struct _node *esq;
    struct _node *dir;
} tnode;

typedef struct _arv {
    tnode *raiz;
    int (*cmp)(void *, void *, int);
    double (*dist)(void *, void *);
    int k; 

} tarv;

/*funções desenvolvedor da biblioteca*/

void kdtree_constroi(tarv *arv, int (*cmp)(void *a, void *b, int), double (*dist)(void *, void *), int k) {
    arv->raiz = NULL;
    arv->cmp = cmp;
    arv->dist = dist;
    arv->k = k; 
}

void _kdtree_insere(tnode **raiz, void *key, int (*cmp)(void *a, void *b, int), int profund, int k) {
    if (*raiz == NULL) {
        *raiz = malloc(sizeof(tnode));
        (*raiz)->key = key;
        (*raiz)->esq = NULL;
        (*raiz)->dir = NULL;
    } 
    else {
        int pos = profund % k;
        if (cmp((*raiz)->key, key, pos) < 0) {
            _kdtree_insere(&((*raiz)->dir), key, cmp, profund + 1, k);
        } else { 
            _kdtree_insere(&((*raiz)->esq), key, cmp, profund + 1, k);
        }
    }
}

void kdtree_insere(tarv *arv, void *key) {
    _kdtree_insere(&(arv->raiz), key, arv->cmp, 0, arv->k);
}

void _kdtree_destroi(tnode *node) {
    if (node != NULL) {
        _kdtree_destroi(node->esq);
        _kdtree_destroi(node->dir);
        free(node->key); 
        free(node);      
    }
}

void kdtree_destroi(tarv *arv) {
    _kdtree_destroi(arv->raiz);
}

//função de busca modificada para salvar os n-vizinho na heap
void _kdtree_busca(tarv *arv, tnode **atual, void *key, int profund, theap *heap) {
    if (*atual != NULL) {
        double dist_atual = arv->dist((*atual)->key, key);

        // Cria o nó da heap com distância e ponteiro para o nó da árvore
        nodeHeap item;
        item.distancia = dist_atual;
        item.node = *atual;


        insere(heap, item);

        int pos = profund % arv->k;
        int comp = arv->cmp(key, (*atual)->key, pos);

        tnode **lado_principal;
        tnode **lado_oposto;

        if (comp < 0) {
            lado_principal = &((*atual)->esq);
            lado_oposto = &((*atual)->dir);
        } else {
            lado_principal = &((*atual)->dir);
            lado_oposto = &((*atual)->esq);
        }

        _kdtree_busca(arv, lado_principal, key, profund + 1, heap);

        float diferenca = ((treg *)key)->embedding[pos] - ((treg *)(*atual)->key)->embedding[pos];
        if ((diferenca * diferenca) < heap->lista[0].distancia || heap->tam < heap->capacidade) {
            _kdtree_busca(arv, lado_oposto, key, profund + 1, heap);
        }
    }
}

void kdtree_busca(tarv *arv, treg *query, int N) {
    theap *heap = construirHeap(N);

    _kdtree_busca(arv, &(arv->raiz), query, 0, heap);

    printf("\nTop %d vizinhos mais proximos:\n", N);
    for (int i = 0; i < heap->tam; ++i) {
        treg *vizinho = (treg *)(heap->lista[i].node);
        printf("%d. %s (distancia: %.4f)\n", i + 1, vizinho->person_id, heap->lista[i].distancia);
    }

    liberar_heap(heap);
}

treg buscar_mais_proximo(tarv *arv, treg query) {
    theap *heap = construirHeap(1);
    _kdtree_busca(arv, &(arv->raiz), &query, 0, heap);
    treg resultado = *((treg *)(heap->lista[0].node));
    liberar_heap(heap);
    return resultado;
}

tarv arvore_global;

tarv* get_tree() {
    return &arvore_global;
}

void inserir_ponto(treg p) {
    treg *novo = aloca_reg(p.embedding, p.person_id);
    kdtree_insere(&arvore_global, novo);
}

void kdtree_construir_global() { 
    arvore_global.k = tam_emb; // aqui salva o tamanho da dimensão
    arvore_global.dist = distancia;
    arvore_global.cmp = comparador;
    arvore_global.raiz = NULL;
}


int main(void) {

    return EXIT_SUCCESS;
}