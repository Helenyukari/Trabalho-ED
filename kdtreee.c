#include <stdio.h>
#include <stdlib.h>
#include <float.h> 
#include <string.h>
#include <math.h> 
#include "heap.h"

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

//busca mais proximo com poda
void _kdtree_busca(tarv *arv, tnode *atual, void *key, int profund, tnode **melhor_no, double *melhor_dist) {
    if (atual == NULL) return;

    double dist_atual = arv->dist(atual->key, key);
    if (dist_atual < *melhor_dist) {
        *melhor_dist = dist_atual;
        *melhor_no = atual;
    }

    int pos = profund % arv->k;
    int comp = arv->cmp(key, atual->key, pos);

    tnode *lado_principal = (comp < 0) ? atual->esq : atual->dir;
    tnode *lado_oposto    = (comp < 0) ? atual->dir : atual->esq;

    _kdtree_busca(arv, lado_principal, key, profund + 1, melhor_no, melhor_dist);

    // Verifica se ainda pode haver algo mais próximo no outro lado
    double diferenca = ((treg *)key)->embedding[pos] - ((treg *)atual->key)->embedding[pos];
    if (fabs(diferenca) < *melhor_dist) {
        _kdtree_busca(arv, lado_oposto, key, profund + 1, melhor_no, melhor_dist);
    }
}
treg buscar_mais_proximo(tarv *arv, treg query) {
    tnode *melhor_no = NULL;
    double melhor_dist = INFINITY;

    _kdtree_busca(arv, arv->raiz, &query, 0, &melhor_no, &melhor_dist);

    if (melhor_no != NULL)
        return *((treg *)melhor_no->key);

    treg vazio;
    strcpy(vazio.person_id, "NAO_ENCONTRADO");
    return vazio;
}

//BUSCA com a HEAP
// void _kdtree_busca(tarv *arv, tnode **atual, void *key, int profund, theap *heap) {
//     if (*atual == NULL) return;
    
//     //recebe o nô atual e calcula a distancia dos demais nôs
//     double dist_atual = arv->dist((*atual)->key, key);

//     // Insere na heap (max-heap) a distância já calculada
//     nodeHeap item = { dist_atual, *atual };
//      insere(heap, item);

//     int pos = profund % arv->k;
//     int comp = arv->cmp(key, (*atual)->key, pos);

//     tnode **lado_principal = (comp < 0)
//         ? &((*atual)->esq) 
//         : &((*atual)->dir);
//     tnode **lado_oposto = (comp < 0)
//         ? &((*atual)->dir)
//         : &((*atual)->esq);

//     // Desce primeiro no ramo que parece m
//     _kdtree_busca(arv, lado_principal, key, profund + 1, heap);
    
//     // Obtém a “pior” (maior) distância da heap cheia, ou INFINITY se ainda não estiver cheia
//     double maxDist = (heap->tam == heap->capacidade)
//         ? heap->lista[0].distancia
//         : INFINITY;

//     // Se a diferença absoluta na dimensão de corte for menor que maxDist,
//     // ainda pode haver pontos melhores no ramo oposto
//     double diferenca = ((treg *)key)->embedding[pos]
//                      - ((treg *)(*atual)->key)->embedding[pos];
//     if (fabs(diferenca) < maxDist) {
//         _kdtree_busca(arv, lado_oposto, key, profund + 1, heap);
//     }
// }    

// void kdtree_busca(tarv *arv, treg *query, int N) {
//     theap *heap = construirHeap(N);

//     _kdtree_busca(arv, &(arv->raiz), query, 0, heap);

//     printf("\nTop %d vizinhos mais proximos:\n", N);
//     for (int i = 0; i < heap->tam; ++i) {
//         treg *vizinho = (treg *)(heap->lista[i].node);
//         printf("%d. %s (distancia: %.4f)\n", i + 1, vizinho->person_id, heap->lista[i].distancia);
//     }
//     liberar_heap(heap);
// }

// treg buscar_mais_proximo(tarv *arv, treg query) {
//     theap *heap = construirHeap(1);
//     _kdtree_busca(arv, &(arv->raiz), &query, 0, heap);
//     treg resultado = *((treg *)(heap->lista[0].node));
//     liberar_heap(heap);
//     return resultado;
// }

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

