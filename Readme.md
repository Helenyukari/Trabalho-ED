
# KD-Tree com Heap para Reconhecimento Facial

Este projeto implementa uma estrutura de árvore KD otimizada para buscar os **N vizinhos mais próximos** com auxílio de uma **Max Heap**, utilizando vetores de embeddings faciais gerados pelo modelo **FaceNet**.

---

## ✨ Funcionalidades

- ✅ Inserção de embeddings (float[128] + ID) na KD-Tree
- ✅ Busca do vizinho mais próximo
- ✅ Busca dos **N vizinhos mais próximos** usando Heap
- ✅ Implementação própria de Heap (Max Heap)
- 🔄Geração de embeddings com **FaceNet (MTCNN + Keras)** ou link do professor 
- 🔄 Exportação de embeddings em `.json`
- 🔄 Suporte para integração com API (em desenvolvimento)

---

## 📂 Estrutura do Projeto

```
📁 Trabalho/
├── kdtreee.c              # Código principal com KDTree + Heap
├── my_lib/
│   └── heap.h/.c          # Implementação da Heap
├── embeddings.json        # Arquivo de saída com os embeddings gerados
├── facenet_keras.h5       # Modelo FaceNet (baixar separadamente)
└── gerar_embeddings.py    # Script para gerar os embeddings a partir do LFW
```

---

## ⚙️ Requisitos

### Para executar o C:
- GCC (MinGW no Windows)
- Estrutura compatível com `float[128]` e strings `char[100]`

### Para gerar embeddings:
```bash
pip install mtcnn keras numpy opencv-python sklearn
```

---

## 🧠 Como gerar os embeddings (Python)

1. Baixe o dataset LFW:
   https://www.kaggle.com/datasets/atulanandjha/lfwpeople

2. Baixe o modelo `facenet_keras.h5`:  
   https://github.com/nyoki-mtl/keras-facenet

3. Execute o script:
```bash
python gerar_embeddings.py
```

> O script detecta rostos com MTCNN, extrai embeddings com FaceNet e salva tudo em `embeddings.json`.

---

## 💡 Como compilar e rodar (C)

### No Windows (MinGW):
```bash
gcc -Wall -Wextra -g3 kdtreee.c my_lib/heap.c -o kdtreee.exe
.\kdtreee.exe
```

> Certifique-se que `heap.h` está incluso corretamente no `kdtreee.c`

---

## 🧪 Casos de Teste (main)

No arquivo `main()`:

- Insere 5 registros com embeddings 2D (para visualização mais fácil)
- Executa:
  - `buscar_mais_proximo()` (N = 1)
  - `kdtree_busca()` (N = 3)

---

## 🧠 Sobre a Estrutura

### KD-Tree
Árvore binária particionada por dimensões:
- Usa `embedding[pos]` como critério de divisão.
- `kdtree_insere()` insere os registros recursivamente.
- `kdtree_busca()` usa **recursão + heap** para encontrar os N vizinhos.

### Heap
- Implementada como Max Heap (maior distância no topo).
- Permite manter apenas os **N menores vizinhos**.
- Operações: `insere()`, `sobe()`, `desce()`, `liberar_heap()`

---

## 📦 Embeddings JSON

Formato:
```json
[
  {
    "id": "George_Bush",
    "embedding": [0.0113, -0.0045, ... (128 floats) ]
  },
  ...
]
```

---

## 🚧 A Fazer

- [ ] Integrar KDTree com API REST via FastAPI
- [ ] Suporte a inserção via JSON dinâmico
- [ ] Benchmark para escalabilidade com 1000+ registros

---

## 🧑‍💻 Autora

Desenvolvido por **Helen** para o projeto de Reconhecimento Facial com Estruturas de Dados.

---
