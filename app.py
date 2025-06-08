from fastapi import FastAPI, Query
from typing import List
from kdtree_wrapper import lib, Tarv, TReg
from ctypes import POINTER, c_char, c_float
from pydantic import BaseModel

app = FastAPI()

class PontoEntrada(BaseModel):
   person_id: str   
   embedding: list[float]

class EmbeddingEntrada(BaseModel):
    embedding: List[float]


@app.post("/construir-arvore")
def constroi_arvore():
    lib.kdtree_construir_global()
    return {"message": "Árvore KD inicializada com sucesso."}

@app.post("/inserir")
def inserir(ponto: PontoEntrada):
    nome_bytes = ponto.person_id.encode('utf-8')[:99]  # Trunca se necessário
    embedding_c=(c_float * 128)(*ponto.embedding) #cria array de float[128]
    novo_ponto = TReg(person_id=nome_bytes, embedding=embedding_c)
    
    lib.inserir_ponto(novo_ponto)
    return {"mensagem": f"Ponto '{ponto.person_id}' inserido com sucesso."}


@app.post("/buscar")
def buscar(dados: EmbeddingEntrada): 
    emb_array = (c_float * 128)(*dados.embedding)
    query = TReg(embedding=emb_array)
     
    arv = lib.get_tree()
    resultado_c = lib.buscar_mais_proximo(arv, query)
     
    person_id_str = resultado_c.person_id.decode('utf-8').rstrip('\x00')
    embedding_list = list(resultado_c.embedding)
    print(f"busca encontrou {person_id_str}")
    return {
        "person_id": person_id_str,
        "embedding": embedding_list
    }
