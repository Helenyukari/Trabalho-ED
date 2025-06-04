import ctypes
from typing import List, Optional
from fastapi import FastAPI, HTTPException, Body
from pydantic import BaseModel, Field

# --- 1. Importar do seu wrapper ---
try:
    from kdtree_wrapper import lib, C_TReg, TAM_EMB, TAM_ID, C_Tarv 
    print("DEBUG app.py: kdtree_wrapper e seus componentes importados com sucesso.")
    if lib is None:
        print("ERRO CRÍTICO app.py: 'lib' foi importado como None do wrapper. A biblioteca C não carregou.")
except ImportError as e_imp:
    print(f"ERRO FATAL em app.py (ImportError): {e_imp}")
    exit()
except AttributeError as e_attr:
    print(f"ERRO FATAL em app.py (AttributeError): Um atributo esperado não foi encontrado em 'kdtree_wrapper.py'.")
    print(f"Verifique se kdtree_wrapper.py define lib, C_TReg, TAM_EMB, TAM_ID. Detalhe: {e_attr}")
    exit()

# --- 2. Modelos Pydantic ---
class PontoEntrada(BaseModel):
    embedding: List[float] = Field(..., min_length=TAM_EMB, max_length=TAM_EMB)
    id_ponto: str = Field(..., min_length=1, max_length=TAM_ID - 1) # Garante que id_ponto cabe

class PontoSaida(BaseModel):
    embedding_preview: List[float]
    id_ponto: str
    distancia: Optional[float] = None

class MensagemResposta(BaseModel):
    mensagem: str

# Antes da definição de 'app = FastAPI()'
class StatusArvoreResposta(BaseModel):
    construida: bool
    numero_de_pontos: int
    k_dimensao: int

# --- 3. Aplicação FastAPI ---
app = FastAPI(
    title="API de Busca por Similaridade com K-d Tree",
    description="Uma API que utiliza uma árvore k-d implementada em C para buscar embeddings de vetores.",
    version="1.0.2"
)

arvore_construida_estado = {"status": False}

# --- 4. Endpoints da API ---
@app.get("/", response_model=MensagemResposta, summary="Verifica se a API está online")
async def root():
    if lib is None:
        return {"mensagem": "API da Árvore K-d está online, MAS A BIBLIOTECA C NÃO FOI CARREGADA."}
    return {"mensagem": "API da Árvore K-d está online. Use /docs para ver a documentação."}

@app.post("/construir-arvore", response_model=MensagemResposta, summary="Constrói/Reinicializa a árvore k-d")
def construir_arvore():
    global arvore_construida_estado
    if lib is None:
        raise HTTPException(status_code=503, detail="Serviço indisponível: Biblioteca C não carregada.")
    try:
        lib.kdtree_construir_global()
        arvore_construida_estado["status"] = True
        print("INFO app.py: Árvore KD inicializada com sucesso.")
        return {"mensagem": "Árvore KD inicializada com sucesso."}
    except AttributeError:
        raise HTTPException(status_code=500, detail="Erro interno: Função 'kdtree_construir_global' não encontrada.")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Erro interno ao construir a árvore: {e}")

@app.post("/inserir", response_model=MensagemResposta, summary="Insere um ponto na árvore")
def inserir(ponto: PontoEntrada):
    if lib is None or C_TReg is None:
        raise HTTPException(status_code=503, detail="Serviço indisponível: Componentes da biblioteca C não carregados.")
    if not arvore_construida_estado["status"]:
        raise HTTPException(status_code=400, detail="A árvore não foi inicializada. Chame /construir-arvore primeiro.")

    try:
        vetor_c = (ctypes.c_float * TAM_EMB)(*ponto.embedding)
        
        # Prepara os bytes para o ID. O Pydantic já validou o tamanho da string id_ponto.
        # A codificação para UTF-8 pode aumentar o número de bytes se houver caracteres especiais.
        # No entanto, o campo c_char * TAM_ID em ctypes, ao receber bytes, fará uma cópia
        # similar a strncpy, truncando se necessário e geralmente tratando da terminação nula
        # se a string de bytes for mais curta que o buffer.
        # Para garantir, podemos truncar os bytes codificados para caber, deixando espaço para o nulo.
        id_bytes_encoded = ponto.id_ponto.encode('utf-8')
        id_bytes_for_c = id_bytes_encoded[:TAM_ID -1] # Garante que não excede e deixa espaço para \0

        # Cria a instância da struct C passando os argumentos para o construtor
        reg_c = C_TReg(embedding=vetor_c, id=id_bytes_for_c)

        lib.inserir_ponto(reg_c)
        print(f"INFO app.py: Ponto '{ponto.id_ponto}' inserido.")
        return {"mensagem": f"Embedding '{ponto.id_ponto}' inserido com sucesso."}
    except AttributeError:
        raise HTTPException(status_code=500, detail="Erro interno: Função 'inserir_ponto' não encontrada.")
    except Exception as e:
        print(f"ERRO app.py: Exceção ao inserir o ponto - {e}") # Para depuração no servidor
        raise HTTPException(status_code=500, detail=f"Erro interno ao inserir o ponto: {e}")

@app.post("/buscar-mais-proximo", response_model=PontoSaida, summary="Busca o vizinho mais próximo")
def buscar(query: PontoEntrada = Body(...)):
    if lib is None or C_TReg is None:
        raise HTTPException(status_code=503, detail="Serviço indisponível: Componentes da biblioteca C não carregados.")
    if not arvore_construida_estado["status"]:
        raise HTTPException(status_code=400, detail="A árvore não foi inicializada. Chame /construir-arvore primeiro.")

    try:
        vetor_query_c = (ctypes.c_float * TAM_EMB)(*query.embedding)
        id_query_bytes_encoded = query.id_ponto.encode('utf-8')
        id_query_bytes_for_c = id_query_bytes_encoded[:TAM_ID-1]

        consulta_c = C_TReg(embedding=vetor_query_c, id=id_query_bytes_for_c)
        
        ptr_arvore = lib.get_tree()
        if not ptr_arvore:
            raise HTTPException(status_code=500, detail="Não foi possível obter a referência para a árvore C.")
        
        resultado_c = lib.buscar_mais_proximo(ptr_arvore, consulta_c)

        id_resultado = resultado_c.id.decode('utf-8', errors='ignore').strip('\x00').strip()
        embedding_preview = [float(f) for f in resultado_c.embedding[:8]]
        
        dist = lib.distancia(ctypes.byref(consulta_c), ctypes.byref(resultado_c))
        print(f"INFO app.py: Busca por '{query.id_ponto}', vizinho '{id_resultado}', dist {dist:.4f}")

        return PontoSaida(
            embedding_preview=embedding_preview,
            id_ponto=id_resultado,
            distancia=dist
        )
    except AttributeError as e_attr:
        raise HTTPException(status_code=500, detail=f"Erro interno: Função C não encontrada ou problema de atributo: {e_attr}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Erro interno ao buscar o vizinho: {e}")

# Na seção de Endpoints da API

@app.get("/status-arvore", response_model=StatusArvoreResposta, summary="Obtém o estado atual da árvore k-d")
def obter_status_arvore():
    if lib is None: # Verifica se a biblioteca C foi carregada
        raise HTTPException(status_code=503, detail="Serviço indisponível: Biblioteca C não carregada.")

    num_pontos = 0
    k_val = 0 # Dimensionalidade da árvore

    # Acessa o estado da árvore (se foi construída ou não)
    # arvore_construida_estado é o dict global que você tem no app.py
    if arvore_construida_estado["status"]: 
        try:
            ptr_arvore = lib.get_tree() # Retorna POINTER(C_Tarv)
            if ptr_arvore and ptr_arvore.contents: # Verifica se o ponteiro e o conteúdo são válidos
                num_pontos = lib.kdtree_obter_numero_de_pontos(ptr_arvore)
                k_val = ptr_arvore.contents.k # Acessa o campo 'k' da struct C_Tarv
            else:
                # Se get_tree() falhar ou a árvore não estiver realmente lá no C
                print("AVISO app.py: get_tree() retornou None ou ponteiro inválido no endpoint /status-arvore.")
                # num_pontos e k_val permanecem 0, construida será False se ptr_arvore for None
        except AttributeError as e_attr:
            print(f"ERRO app.py: AttributeError ao obter status da árvore - {e_attr}")
            # Não lança exceção HTTP, apenas loga e retorna o que temos (num_pontos=0, k_val=0)
        except Exception as e:
            print(f"ERRO app.py: Exceção ao obter status da árvore - {e}")

    return StatusArvoreResposta(
        construida=arvore_construida_estado["status"],
        numero_de_pontos=num_pontos,
        k_dimensao=k_val if arvore_construida_estado["status"] and ptr_arvore else 0
    )
# --- Para executar a API com `python app.py` ---
if __name__ == "__main__":
    import uvicorn
    print("INFO app.py: Iniciando servidor Uvicorn...")
    uvicorn.run("app:app", host="0.0.0.0", port=8000, reload=True)
