import os
import json
from deepface import DeepFace
import traceback # Para ver erros detalhados

# \--- Configurações ---

# ATENÇÃO: Altere este caminho para o local onde você descomprimiu a pasta lfw\_funneled

CAMINHO\_LFW\_FUNNELED = r"C:\\caminho\\para\\sua\\lfw\_funneled"

# Exemplo para Windows: r"D:\\Datasets\\lfw\_funneled"

# Exemplo para Linux/macOS: "/home/user/datasets/lfw\_funneled"

FICHEIRO\_OUTPUT\_JSONL = "lfw\_embeddings.jsonl"
MODELO\_USADO = "VGG-Face" \# Ou "Facenet", "ArcFace", "Dlib", "SFace", etc.
DETECTOR\_BACKEND = "opencv" \# Ou "ssd", "mtcnn", "retinaface", etc.
DIMENSAO\_EMBEDDING = 128 \# Certifique-se que o modelo escolhido gera esta dimensão ou ajuste

# Verifica se o caminho LFW existe

if not os.path.isdir(CAMINHO\_LFW\_FUNNELED):
print(f"ERRO: O caminho para o dataset LFW '{CAMINHO\_LFW\_FUNNELED}' não foi encontrado.")
print("Por favor, ajuste a variável CAMINHO\_LFW\_FUNNELED no script.")
exit()

def extrair\_embeddings():
print(f"A extrair embeddings do dataset LFW em: {CAMINHO\_LFW\_FUNNELED}")
print(f"Usando modelo: {MODELO\_USADO}, detector: {DETECTOR\_BACKEND}")