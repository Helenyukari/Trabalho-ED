#include <stdio.h>
#include <stdlib.h>
#include <float.h> 
#include <string.h>
#include <assert.h>
#include <math.h> 
#include "minHeap.h" // Inclui a definição do MinHeap
// --- DEFINIÇÕES DO USUÁRIO ADAPTADAS ---
#define tam_emb 128 
#define tam_id 100 


//Ajustado para as embeddings
typedef struct _reg {
    float embedding[tam_id];
    char person_id[tam_id];  
} treg;

//ajustado para alogar a treg e as embaddings
void *aloca_reg(float *embedding, char *id) {
    treg *reg;
    reg = malloc(sizeof(treg));
    memcpy(reg->embedding, embedding, tam_id * sizeof(float));
    strncpy(reg->person_id, id, tam_id - 1);
    return reg;
}

// Comparador para embeddings de N dimensões
int comparador(void *a, void *b, int pos) {
    float val_a = ((treg *)a)->embedding[pos];
    float val_b = ((treg *)b)->embedding[pos];

    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0; 
}

// Distância Euclidiana 
double distancia(void *a, void *b) {
    double soma = 0.0;
    // itera para cada uma das 128 dimensões do embedding
    for (int i = 0; i < tam_id; ++i) {
        // Calcula a diferença entre as coordenadas nas dimensões de ambos os embeddings
        double dif = ((treg *)a)->embedding[i] - ((treg *)b)->embedding[i];
        // Adiciona o quadrado dessa diferença à soma total
        soma += dif * dif;
    }
    return sqrt(soma); // Retorna a raiz quadrada da soma
}

// --- DEFINIÇÕES DA BIBLIOTECA DA KD-TREE (Adaptadas) ---
typedef struct _node {
    void *key; 
    struct _node *esq;
    struct _node *dir;
} tnode;

typedef struct _arv {
    tnode *raiz;
    int (*cmp)(void *, void *, int);
    double (*dist)(void *, void *);
    int k; // Adiciona o eixo para o nó
} tarv;


// --- FUNÇÕES DA BIBLIOTECA DA KD-TREE (Adaptadas) ---
void kdtree_constroi(tarv *arv, int (*cmp)(void *a, void *b, int), double (*dist)(void *, void *), int k) {
    arv->raiz = NULL;
    arv->cmp = cmp;
    arv->dist = dist;
    arv->k = k; // k será tam_id
}

// Note: A função de inserção (_kdtree_insere) também precisará armazenar o 'axis' no nó.
// Isso simplifica a busca, pois cada nó "sabe" em qual dimensão foi dividido.
void _kdtree_insere(tnode **raiz, void *key, int (*cmp)(void *a, void *b, int), int profund, int k) {
    if (*raiz == NULL) {
        *raiz = malloc(sizeof(tnode));
        (*raiz)->key = key;
        (*raiz)->esq = NULL;
        (*raiz)->dir = NULL;
    } 
    else {
        int pos = profund % k;
        if (cmp((*raiz)->key, key, pos) < 0) { // Se o novo é maior que o nó atual na dimensão 'pos'
            _kdtree_insere(&((*raiz)->dir), key, cmp, profund + 1, k);
        } else { // Se o novo é menor ou igual
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
        free(node->key); // Libera o treg
        free(node);      // Libera o tnode
    }
}

void kdtree_destroi(tarv *arv) {
    _kdtree_destroi(arv->raiz);
}

// --- FUNÇÃO DE BUSCA DOS N VIZINHOS MAIS PRÓXIMOS (Principal Mudança) ---
void _kdtree_BuscaProximo(tarv *arv, tnode *Atual, void *key, int vizinho, MinHeap *max) {
    if (Atual == NULL) {
        return;
    }
    // Calcula a distância do nó atual para a consulta
    double distAtual = arv->dist(Atual->key, key);

    // Adiciona ao heap se ele ainda não estiver cheio, ou se for mais próximo que o mais distante no heap
    if (max->size < vizinho) {
        inserirMinHeap(max, (nodeHeap){-distAtual, Atual->key}); // Usa -dist para simular max-heap
    } else if (distAtual < -max->array[0].distance) { // Compare com o topo do heap (maior distância)
        extrairMinHeap(max); // Remove o mais distante
        inserirMinHeap(max, (nodeHeap){-distAtual, Atual->key});
    }

    // Determine qual lado buscar primeiro
    int axis = Atual->axis; // Usa o axis armazenado no nó
    tnode *next_branch;
    tnode *other_branch;

    if (comparador(key, Atual->key, axis) < 0) { // Se a consulta é "menor" na dimensão atual
        next_branch = Atual->esq;
        other_branch = Atual->dir;
    } else {
        next_branch = Atual->dir;
        other_branch = Atual->esq;
    }

    // Busca no ramo principal
    _kdtree_BuscaProximo(arv, next_branch, key, vizinho, max);

    // Verifique se o ramo oposto pode conter um vizinho mais próximo
    // Distância do ponto de consulta ao plano de divisão (ao quadrado para comparar com distância^2)
    float diff_on_axis = ((treg *)key)->embedding[axis] - ((treg *)Atual->key)->embedding[axis];
    double dist_to_plane_sq = diff_on_axis * diff_on_axis;

    // A condição de poda agora usa a distância (negativa) do elemento mais distante no heap
    // Se o heap ainda não está cheio OU a distância ao plano é menor que a maior distância atual no heap
    if (max->size < vizinho || dist_to_plane_sq < pow(-max->array[0].distance, 2)) {
        _kdtree_BuscaProximo(arv, other_branch, key, vizinho, max);
    }
}

// Função principal para busca de N vizinhos
// Retorna um array de ponteiros para treg dos N vizinhos mais próximos
treg **BuscaProximo(tarv *arv, void *key, int n) {
    if (arv->raiz == NULL) {
        return NULL;
    }

    MinHeap *max_heap = contruir_minHeap(n); // Cria um max-heap (usando min-heap com distâncias negativas)

    _kdtree_BuscaProximo(arv, arv->raiz, key, n, max_heap);

    // Extrair os resultados do heap e retornar em um array
    treg **results = malloc(max_heap->size * sizeof(treg *));
    if (results == NULL) {
        perror("Erro ao alocar array de resultados");
        freeMinHeap(max_heap);
        return NULL;
    }

    // Copia os elementos do heap para o array de resultados, invertendo as distâncias
    int count = max_heap->size;
    for (int i = count - 1; i >= 0; --i) {
        nodeHeap item = extrairMinHeap(max_heap);
        results[i] = (treg *)item.data;
    }

    freeMinHeap(max_heap);
    return results; // Lembre-se de liberar este array e seus tregs se forem cópias
}

void inserir_ponto(treg p) {
    treg *novo = aloca_reg(p.embedding, p.person_id); // Use aloca_reg
    kdtree_insere(&arvore_global, novo);
}

void kdtree_construir_global() { // Renomeado para evitar conflito com test_constroi
    arvore_global.k = tam_id; // O K agora é a dimensão do embedding
    arvore_global.dist = distancia;
    arvore_global.cmp = comparador;
    arvore_global.raiz = NULL;
}


// --- Testes (PRECISAM SER ADAPTADOS) ---
// Os testes atuais usam lat/lon. Você precisará criar testes com dados de embedding
// (mesmo que sejam aleatórios por enquanto) para verificar a lógica.

void test_constroi_adapted() {
    tarv arv;
    float emb1[tam_id] = { /* preencher com floats */ };
    float emb2[tam_id] = { /* preencher com floats */ };
    // Exemplo: emb1[0] = 2.0; emb1[1] = 3.0; ... para Dourados
    // Exemplo: emb2[0] = 1.0; emb2[1] = 1.0; ... para Campo Grande

    void *key1 = aloca_reg(emb1, "Dourados_ID");
    void *key2 = aloca_reg(emb2, "Campo_Grande_ID");

    kdtree_constroi(&arv, comparador, distancia, tam_id);

    assert(arv.raiz == NULL);
    assert(arv.k == tam_id);
    assert(arv.cmp(key1, key2, 0) == 1 || arv.cmp(key1, key2, 0) == -1); // Depende dos valores
    // ... adicione mais asserts conforme a lógica de seus dados de teste ...

    free(key1);
    free(key2);
}

void test_find_n_nearest() {
    tarv arv;
    kdtree_constroi(&arv, comparador, distancia, tam_id);

    // Crie e insira alguns embeddings de teste
    float emb_a[tam_id]; for(int i=0; i<tam_id; ++i) emb_a[i] = 10.0f;
    float emb_b[tam_id]; for(int i=0; i<tam_id; ++i) emb_b[i] = 20.0f;
    float emb_c[tam_id]; for(int i=0; i<tam_id; ++i) emb_c[i] = 1.0f; emb_c[0] = 1.0f; emb_c[1] = 10.0f;
    float emb_d[tam_id]; for(int i=0; i<tam_id; ++i) emb_d[i] = 3.0f; emb_d[0] = 3.0f; emb_d[1] = 5.0f;

    kdtree_insere(&arv, aloca_reg(emb_a, "a_id"));
    kdtree_insere(&arv, aloca_reg(emb_b, "b_id"));
    kdtree_insere(&arv, aloca_reg(emb_c, "c_id"));
    kdtree_insere(&arv, aloca_reg(emb_d, "d_id"));
    // ... insira mais dados para ter uma árvore razoável para teste

    float query_emb[tam_id]; for(int i=0; i<tam_id; ++i) query_emb[i] = 10.5f; // Ponto de consulta

    printf("\nTestando busca dos N vizinhos mais proximos...\n");
    int n = 2; // Buscar os 2 vizinhos mais próximos
    treg **nearest_neighbors = BuscaProximo(&arv, query_emb, n);

    if (nearest_neighbors != NULL) {
        printf("Os %d vizinhos mais proximos:\n", n);
        for (int i = 0; i < n; ++i) {
            if (nearest_neighbors[i] != NULL) {
                double dist = distancia(query_emb, nearest_neighbors[i]->embedding);
                printf("  ID: %s, Distancia: %.4f\n", nearest_neighbors[i]->person_id, dist);
            }
        }
        free(nearest_neighbors); // Libere o array retornado
    }

    kdtree_destroi(&arv);
}


int main(void) {
    printf("Rodando testes...\n");
    // test_constroi_adapted(); // Descomente e adapte este teste
    // test_find_n_nearest(); // Descomente e adapte este teste

    // Exemplo de uso da API global (após ter embeddings de verdade)
    kdtree_construir_global();

    float my_emb[tam_emb] = {-0.9540755748748779,
  -2.479801893234253,
  -0.36793839931488037,
  -0.6942156553268433,
  0.1172848790884018,
  -0.36969107389450073,
  1.3702824115753174,
  0.47709202766418457,
  -1.7161850929260254,
  0.12417732179164886,
  0.2880435883998871,
  -0.24127306044101715,
  1.131235957145691,
  0.5080835819244385,
  0.8487039804458618,
  1.250220537185669,
  2.1950080394744873,
  -1.0733388662338257,
  1.2392503023147583,
  -0.8738752007484436,
  -1.6702111959457397,
  -0.8751335144042969,
  -2.2556092739105225,
  -0.1050572618842125,
  0.33191102743148804,
  -0.14769169688224792,
  1.400815725326538,
  -0.4477466344833374,
  0.3556976318359375,
  0.3776315450668335,
  -0.464281290769577,
  0.2480984628200531,
  0.27185380458831787,
  -0.7629454135894775,
  0.4411030113697052,
  0.261106938123703,
  0.2280164361000061,
  0.511577844619751,
  -0.5180702209472656,
  -1.1787961721420288,
  1.5398184061050415,
  -0.0019539594650268555,
  -0.7270607948303223,
  0.8635854721069336,
  0.10341743379831314,
  -2.2359912395477295,
  -0.16349156200885773,
  0.7181979417800903,
  -0.029192261397838593,
  0.3331388831138611,
  -1.7241566181182861,
  1.7099326848983765,
  -1.0119547843933105,
  0.0850394144654274,
  -0.3558950126171112,
  0.6361727118492126,
  0.21069763600826263,
  -0.1891246885061264,
  1.808251142501831,
  -0.0516827255487442,
  -0.5510203242301941,
  0.33660560846328735,
  1.2300275564193726,
  2.204685688018799,
  0.13514728844165802,
  1.074007272720337,
  -0.3723274767398834,
  0.9072832465171814,
  0.2476714551448822,
  0.7935322523117065,
  -0.22664059698581696,
  -0.6715865135192871,
  -0.9380842447280884,
  -0.02740451693534851,
  0.7947195172309875,
  -0.19978541135787964,
  -0.7772310972213745,
  -1.104081153869629,
  -0.7851266860961914,
  -0.1962304711341858,
  -0.9133906960487366,
  0.5492947101593018,
  -3.116969347000122,
  -0.5732014775276184,
  3.060408115386963,
  -1.669589638710022,
  -0.4091629087924957,
  -0.5994044542312622,
  -2.6901018619537354,
  0.5361208915710449,
  -0.5412876605987549,
  -0.08187766373157501,
  1.0482133626937866,
  -0.7754993438720703,
  -1.6223249435424805,
  -0.14051468670368195,
  1.278308391571045,
  0.7650377154350281,
  -0.5313907861709595,
  1.215232253074646,
  -0.829127311706543,
  0.8376884460449219,
  1.235361099243164,
  0.02412550337612629,
  -0.1554570496082306,
  1.828536033630371,
  -0.3206011652946472,
  -0.8731174468994141,
  -0.7923187613487244,
  0.28730475902557373,
  0.060953058302402496,
  1.27537202835083,
  1.7244852781295776,
  -1.1123603582382202,
  -0.8848960399627686,
  -1.3329240083694458,
  0.704565167427063,
  0.17373736202716827,
  -0.7720436453819275,
  1.091594934463501,
  2.090299129486084,
  0.9140962362289429,
  1.2665961980819702,
  -0.21315577626228333,
  -0.04270055145025253,
  0.6443256735801697,
  0.31324103474617004,
  0.7933658361434937}; // Exemplo de embedding


   float friend_emb[tam_id] = {-1.3552043437957764,
  -1.3289408683776855,
  -0.4641669988632202,
  -1.1806755065917969,
  -0.08854549378156662,
  -1.6132820844650269,
  1.3428430557250977,
  0.21469831466674805,
  -2.647386312484741,
  1.249377965927124,
  -0.21004816889762878,
  0.7800650596618652,
  1.6192381381988525,
  0.16636443138122559,
  1.4454292058944702,
  1.3886278867721558,
  1.6351075172424316,
  -2.4968388080596924,
  -0.9429507255554199,
  -1.1600861549377441,
  -0.49106863141059875,
  -0.8787230253219604,
  -1.5587091445922852,
  -0.5598255395889282,
  0.057255372405052185,
  -0.9714768528938293,
  1.8261200189590454,
  0.4912168085575104,
  -0.0917915403842926,
  0.5240821838378906,
  0.11081389337778091,
  0.1782020926475525,
  1.8726452589035034,
  0.15078416466712952,
  0.19381272792816162,
  -0.5504370927810669,
  0.40156641602516174,
  -0.5840137004852295,
  -0.15664047002792358,
  -2.397036075592041,
  -0.4401240944862366,
  1.4158217906951904,
  -2.4306347370147705,
  0.38280951976776123,
  -0.4900963008403778,
  -2.6993396282196045,
  -0.05978662520647049,
  1.003877878189087,
  -0.20503546297550201,
  0.9704304933547974,
  -2.621591329574585,
  2.115513324737549,
  -1.5110118389129639,
  0.6159526109695435,
  0.8099595904350281,
  0.6276631951332092,
  0.0997534692287445,
  0.08768470585346222,
  0.7585175633430481,
  -0.19350945949554443,
  -1.020464539527893,
  -0.5978552103042603,
  1.751653790473938,
  1.7207410335540771,
  0.5013245940208435,
  0.9681766033172607,
  -1.5727900266647339,
  1.4913718700408936,
  -0.27652910351753235,
  1.7614848613739014,
  -0.3446376323699951,
  0.2936097979545593,
  -0.8927733302116394,
  -0.800968587398529,
  0.668950080871582,
  -1.0831478834152222,
  -1.246508240699768,
  -2.008660316467285,
  0.3643149733543396,
  -0.46151459217071533,
  -0.9877055287361145,
  0.4483443796634674,
  -1.8673490285873413,
  1.1486763954162598,
  2.517819404602051,
  0.14237278699874878,
  -0.7509599328041077,
  -1.5485475063323975,
  -2.1473870277404785,
  0.19734425842761993,
  -0.5445888638496399,
  -0.25469809770584106,
  1.0529016256332397,
  -0.8532118201255798,
  -0.01115380972623825,
  0.9226323366165161,
  0.47706320881843567,
  0.8005662560462952,
  -0.5123848915100098,
  0.25081750750541687,
  -0.7203763127326965,
  1.2774118185043335,
  -0.48440778255462646,
  0.7223607301712036,
  0.460582435131073,
  1.487523078918457,
  -1.4970552921295166,
  -1.38498854637146,
  -0.6785808205604553,
  1.5167882442474365,
  -1.2402184009552002,
  0.4768984317779541,
  1.3856182098388672,
  -0.06494633108377457,
  -0.4435824751853943,
  -0.5403563976287842,
  -0.6103478670120239,
  -1.2148901224136353,
  -0.5780946016311646,
  0.18587243556976318,
  2.6181752681732178,
  0.2981008291244507,
  0.72157883644104,
  -0.9039414525032043,
  -0.31305843591690063,
  0.2603487968444824,
  0.9201691150665283,
  0.8300462365150452;
    // Simulação de inserção de dados
    // float my_emb[tam_id]; for(int i=0; i<tam_id; ++i) my_emb[i] = 5.0f + (float)i/100.0f;
    // float friend_emb[tam_id]; for(int i=0; i<tam_id; ++i) friend_emb[i] = 5.1f + (float)i/100.0f;
    float random_emb[tam_id]; for(int i=0; i<tam_id; ++i) random_emb[i] = (float)rand()/(float)RAND_MAX * 10.0f;

    treg my_face_data = {.embedding = {0}, .person_id = "helen"};
    memcpy(my_face_data.embedding, my_emb, tam_id * sizeof(float));
    inserir_ponto(my_face_data);

    treg friend_face_data = {.embedding = {0}, .person_id = "Amigo_1"};
    memcpy(friend_face_data.embedding, friend_emb, tam_id * sizeof(float));
    inserir_ponto(friend_face_data);

    for (int i = 0; i < 998; ++i) { // Inserir 998 faces aleatórias
        treg rand_face_data;
        for(int j=0; j<tam_id; ++j) rand_face_data.embedding[j] = (float)rand()/(float)RAND_MAX * 10.0f;
        sprintf(rand_face_data.person_id, "Random_Person_%d", i);
        inserir_ponto(rand_face_data);
    }
    printf("1000 faces inseridas (simuladas).\n");

    // Consulta e verificação
    int n_neighbors = 5;
    treg **results = BuscaProximo(&arvore_global, my_emb, n_neighbors);

    printf("\nOs %d vizinhos mais proximos para 'Seu_Nome_Completo':\n", n_neighbors);
    if (results != NULL) {
        for (int i = 0; i < n_neighbors; ++i) {
            if (results[i] != NULL) {
                double dist = distancia(my_emb, results[i]->embedding);
                printf("  ID: %s, Distancia: %.4f\n", results[i]->person_id, dist);
            }
        }
        free(results); // Libere o array retornado
    }

    kdtree_destroi(&arvore_global); // Libera a árvore global

    printf("SUCCESS!!\n");
    return EXIT_SUCCESS;
}

// --- Implementação das Funções Auxiliares de Heap (fora do main) ---
// (coloque as funções contruir_minHeap, troca, heapify, inserirMinHeap, extrairMinHeap, freeMinHeap aqui)