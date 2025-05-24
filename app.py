from fastapi import FastAPI, Query
from kdtree_wrapper import lib, Tarv, TReg
from ctypes import POINTER, c_char, c_float
from pydantic import BaseModel, conlist


app = FastAPI()

class PontoEntrada(BaseModel):
    embedding: conlist(float, min_items=128, max_items=128)
    id: str

@app.post("/construir-arvore")
def constroi_arvore():
    lib.kdtree_construir()
    return {"mensagem": "Árvore KD inicializada com sucesso."}

@app.post("/inserir")
def inserir(ponto: PontoEntrada):
    vetor = (c_float * 128)(*ponto.embedding);
    id_bytes = ponto.id.encode('utf-8')[:99] 
    reg = TReg(embedding=vetor, id=id_bytes)
    lib.inserir_ponto(reg)
    return {"mensagem": f"Embedding '{ponto.id}' inserido com sucesso."}

@app.get("/buscar")
def buscar(lat: float = Query(...), lon: float = Query(...)):
    vetor = (c_float * 128)(*query.embedding)
    id_bytes = query.id.encode("utf-8")[:99]
    consulta = TReg(embedding=vetor, id=id_bytes)
    arv = lib.get_tree()
    resultado = lib.buscar_mais_proximo(arv, consulta)

    return {
        "id": resultado.id.decode("utf-8").rstrip("\x00"),
        "embedding": list(resultado.embedding)[:5] + ["..."]  # Mostra só os 5 primeiros valores
    }