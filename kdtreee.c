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
    float valor_a = ((treg *)a)->embedding[pos]; 
    float valor_b = ((treg *)b)->embedding[pos];
    if (valor_a < valor_b) return -1;
    if (valor_a > valor_b) return 1;
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

        // _kdtree_busca(arv, lado_principal, key, profund + 1, heap);

        // float diferenca = ((treg *)key)->embedding[pos] - ((treg *)(*atual)->key)->embedding[pos];
        // if (heap->tam < heap->capacidade || (diferenca * diferenca) < heap->lista[0].distancia) {
        //     _kdtree_busca(arv, lado_oposto, key, profund + 1, heap);
        // }

        // Em _kdtree_busca:
        float diferenca_axial = ((treg *)key)->embedding[pos] - ((treg *)(*atual)->key)->embedding[pos];
        double dist_max_na_heap_atual = heap->lista[0].distancia;
        if (heap->tam < heap->capacidade || (diferenca_axial * diferenca_axial) < (dist_max_na_heap_atual * dist_max_na_heap_atual)) {
            _kdtree_busca(arv, lado_oposto, key, profund + 1, heap);
    }
    }
}
void kdtree_busca(tarv *arv, treg *query, int N) {
    theap *heap = construirHeap(N);
    _kdtree_busca(arv, &(arv->raiz), query, 0, heap);

//     // Em kdtree_busca, antes do loop de printf:
//     if (heap->tam > 0) {
//     qsort(heap->lista, heap->tam, sizeof(nodeHeap), comparador_heap); // Usa o seu comparador_heap
//     }
// // ... (o seu loop de printf continua igual)

    printf("\nTop %d vizinhos mais proximos:\n", N);
    for (int i = 0; i < heap->tam; ++i) {
        tnode* no_da_arvore = (tnode*)(heap->lista[i].node);
        treg *vizinho = (treg *)(no_da_arvore->key); 
        printf("%d. %s (distancia: %.4f)\n", i + 1, vizinho->person_id, heap->lista[i].distancia);
    }

    liberar_heap(heap);
}

treg buscar_mais_proximo(tarv *arv, treg query) {
    theap *heap = construirHeap(1);
    _kdtree_busca(arv, &(arv->raiz), &query, 0, heap);
    
    tnode* no_da_arvore = (tnode*)(heap->lista[0].node);
    treg resultado = *((treg *)(no_da_arvore->key)); 
    
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

// Função de comparação para qsort, para ordenar a saída da heap
int comparador_heap(const void *a, const void *b) {
    nodeHeap *nA = (nodeHeap *)a;
    nodeHeap *nB = (nodeHeap *)b;
    if (nA->distancia < nB->distancia) return -1;
    if (nA->distancia > nB->distancia) return 1;
    return 0;
}

// Função auxiliar recursiva para contar nós
int _kdtree_contar_nos_recursivo(tnode *node) {
    if (node == NULL) {
        return 0;
    }
    return 1 + _kdtree_contar_nos_recursivo(node->esq) + _kdtree_contar_nos_recursivo(node->dir);
}

// Função pública para ser chamada pela API (via wrapper)
int kdtree_obter_numero_de_pontos(tarv *arv) {
    if (arv == NULL || arv->raiz == NULL) {
        return 0;
    }
    return _kdtree_contar_nos_recursivo(arv->raiz);
}

// int main(void) {
//     // Para este teste, garanta que no topo do arquivo você tem: #define tam_emb 2
//     // if (tam_emb != 3) {
//     //     printf("ERRO: Para este teste, defina tam_emb como 2 no topo do arquivo.\n");
//     //     return 1;
//     // }

//     // 1. Inicializa a árvore global
//     kdtree_construir_global();
//     printf("Arvore de 3 dimesoes construida.\n");

//     // 2. Cria 5 pontos de dados
//     treg p1, p2, p3, p4, p5;
//     strcpy(p1.person_id, "Ana");   p1.embedding[0] = 5.0f; p1.embedding[1] = 5.0f; p1.embedding[2] = 2.0f;
//     strcpy(p2.person_id, "Bia");   p2.embedding[0] = 4.0f; p2.embedding[1] = 4.0f; p2.embedding[2] = 3.0f;
//     strcpy(p3.person_id, "Caio");  p3.embedding[0] = 7.0f; p3.embedding[1] = 5.0f; p3.embedding[2] = 4.0f;
//     strcpy(p4.person_id, "Duda");  p4.embedding[0] = 1.0f; p4.embedding[1] = 2.0f; p4.embedding[2] = 5.0f;
//     strcpy(p5.person_id, "Leo");   p5.embedding[0] = 9.0f; p5.embedding[1] = 9.0f; p5.embedding[2] = 6.0f;

//     // 3. Insere os pontos na árvore
//     printf("Inserindo 5 pessoas na arvore...\n\n");
//     inserir_ponto(p1);
//     inserir_ponto(p2);
//     inserir_ponto(p3);
//     inserir_ponto(p4);
//     inserir_ponto(p5);
    
//     // 4. Prepara a struct de consulta
//     treg consulta;
//     strcpy(consulta.person_id, "Ponto de Consulta");
//     consulta.embedding[0] = 5.0f;
//     consulta.embedding[1] = 5.0f;
//     consulta.embedding[2] = 5.0f;
    
//     // =================================================================
//     // PARTE 1: TESTE PARA buscar_mais_proximo (N=1)
//     // =================================================================
//     printf("--- Testando buscar_mais_proximo (N=1) ---\n");
//     treg resultado_unico = buscar_mais_proximo(&arvore_global, consulta);
//     printf("Mais proximo de 'Consulta {5,5,5}' foi '%s {%.1f, %.1f, %.1f}'\n", 
//            resultado_unico.person_id, resultado_unico.embedding[0], resultado_unico.embedding[1], resultado_unico.embedding[2]);
    
//     // =================================================================
//     // PARTE 2: TESTE PARA kdtree_busca (N=3)
//     // =================================================================
//     int N = 3;
//     printf("\n--- Testando kdtree_busca (N=%d) ---\n", N);
    
//     // Adicionamos o qsort dentro da kdtree_busca para uma saída ordenada
//     // (Presumindo que você adicionou a sugestão da resposta anterior)
//     kdtree_busca(&arvore_global, &consulta, N); 

//     // 5. Libera memória
//     kdtree_destroi(&arvore_global);
//     printf("\nÁrvore destruída. Fim do programa.\n");
    
//     return EXIT_SUCCESS;
// }