import requests
import json

#arquivo teste
BASE_URL = "http://localhost:8000"
JSON_FILE = "embeddings.json"

def construir_arvore():
    r = requests.post(f"{BASE_URL}/construir-arvore")
    print("[✓] Árvore KD reinicializada.")

def inserir_um_ponto(pessoa):
    dados = {
        "person_id": pessoa["id"],
        "embedding": pessoa["embedding"]
    }
    r = requests.post(f"{BASE_URL}/inserir", json=dados)
    print(f"[+] Inserido: {pessoa['id']}")

def buscar(pessoa):
    r = requests.post(f"{BASE_URL}/buscar", json={"embedding": pessoa["embedding"]})
    resultado = r.json()
    print(f"[🔍] Esperado: {pessoa['id']}, Encontrado: {resultado['person_id']}")
    return resultado['person_id'] == pessoa['id']

if __name__ == "__main__":
    with open(JSON_FILE, "r", encoding="utf-8") as f:
        pessoas = json.load(f)

    # 1. Recria a árvore KD
    construir_arvore()

    # 2. Insere os 3 primeiros pontos para teste rápido
    for p in pessoas[:100]:
        inserir_um_ponto(p)

    # 3. Faz busca com o primeiro ponto
    print("\n[TESTE DE BUSCA]")
    sucesso = buscar(pessoas[10])
    print("🎯 Resultado:", "SUCESSO!" if sucesso else "FALHOU!")
