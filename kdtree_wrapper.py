"""
Módulo Wrapper para a biblioteca C da Árvore K-d.

Este ficheiro usa a biblioteca `ctypes` do Python para carregar a
biblioteca partilhada C (`.so` ou `.dll`) e definir as estruturas
e assinaturas de função necessárias para interagir com a k-d tree
a partir do Python.
"""
import ctypes
import os
from ctypes import (Structure, POINTER, c_double, c_int, c_float,
                    c_char, c_void_p, CFUNCTYPE, CDLL)

# --- 1. Constantes (devem corresponder ao código C) ---
TAM_EMB = 128  # Dimensionalidade real dos embeddings
TAM_ID = 100   # Tamanho do buffer de char para o ID

# --- 2. Carregamento da Biblioteca Partilhada C ---
lib = None # Inicializa lib como None
try:
    if os.name == 'nt':  # Se for Windows
        lib_name = 'libkdtree.dll'
    else:  # Para Linux, macOS, etc.
        lib_name = 'libkdtree.so'
    
    # Procura a biblioteca na mesma pasta que este script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    lib_path = os.path.join(script_dir, lib_name)
    
    lib = CDLL(lib_path)
    print(f"INFO kdtree_wrapper.py: Biblioteca C '{lib_name}' carregada com sucesso.")

except OSError as e:
    print(f"ERRO kdtree_wrapper.py: Não foi possível carregar a biblioteca C '{lib_name}'.")
    print(f"Causa: {e}")
    print("Verifique se compilou a biblioteca C (64-bit se Python for 64-bit) e se ela está na mesma pasta.")
    # lib continua None, o app.py deve verificar isso.
except Exception as e_geral:
    print(f"ERRO kdtree_wrapper.py: Uma exceção inesperada ocorreu ao carregar a biblioteca: {e_geral}")
    lib = None


# --- 3. Definição das Estruturas C em Python ---
# Usamos "C_" no início para evitar conflitos com nomes de modelos Pydantic no app.py
class C_TReg(Structure):
    """ Mapeamento da struct 'treg' do C. """
    _fields_ = [("embedding", c_float * TAM_EMB),
                ("id", c_char * TAM_ID)]

class C_TNode(Structure):
    """ Mapeamento da struct 'tnode' do C. """
    pass

C_TNode._fields_ = [
    ("key", c_void_p), 
    ("esq", POINTER(C_TNode)),
    ("dir", POINTER(C_TNode))
]

class C_Tarv(Structure):
    """ Mapeamento da struct 'tarv' (árvore) do C. """
    _fields_ = [
        ("raiz", POINTER(C_TNode)),
        ("cmp", CFUNCTYPE(c_int, c_void_p, c_void_p, c_int)), # Ponteiro para função de comparação
        ("dist", CFUNCTYPE(c_double, c_void_p, c_void_p)),   # Ponteiro para função de distância
        ("k", c_int)                                         # Dimensionalidade da árvore
    ]

# --- 4. Definição das Assinaturas das Funções C ---
# Só tentamos definir as assinaturas se a biblioteca foi carregada com sucesso.
if lib:
    try:
        lib.kdtree_obter_numero_de_pontos.argtypes = [POINTER(C_Tarv)] # C_Tarv é a sua struct da árvore
        lib.kdtree_obter_numero_de_pontos.restype = c_int
        print("INFO kdtree_wrapper.py: Assinatura de kdtree_obter_numero_de_pontos definida.")
    except AttributeError as e:
        print(f"ERRO kdtree_wrapper.py: Falha ao definir assinatura para kdtree_obter_numero_de_pontos - {e}")
        # Pode definir lib = None aqui se esta função for crítica para o wrapper
    try:
        # void kdtree_construir_global();
        lib.kdtree_construir_global.argtypes = []
        lib.kdtree_construir_global.restype = None
        
        # void inserir_ponto(treg p);
        lib.inserir_ponto.argtypes = [C_TReg]
        lib.inserir_ponto.restype = None

        # tarv* get_tree();
        lib.get_tree.restype = POINTER(C_Tarv)
        lib.get_tree.argtypes = []

        # treg buscar_mais_proximo(tarv *arv, treg query);
        lib.buscar_mais_proximo.argtypes = [POINTER(C_Tarv), C_TReg]
        lib.buscar_mais_proximo.restype = C_TReg

        # double distancia(void *a, void *b);
        lib.distancia.argtypes = [c_void_p, c_void_p] # Em C: void* (para treg*), void* (para treg*)
        lib.distancia.restype = c_double
        
        # (Aqui entrariam as definições para a busca k-NN quando a função C estiver pronta para retornar dados)

        print("INFO kdtree_wrapper.py: Assinaturas das funções C definidas com sucesso.")
    except AttributeError as e_attr:
        print(f"ERRO kdtree_wrapper.py (AttributeError): Uma função C não foi encontrada na DLL ou o nome está incorreto.")
        print(f"Detalhe: {e_attr}")
        print("Verifique se todas as funções C (kdtree_construir_global, inserir_ponto, etc.) estão corretas e exportadas na DLL.")
        lib = None # Marca a lib como inválida se houver erro ao definir assinaturas
    except Exception as e_sig:
        print(f"ERRO kdtree_wrapper.py: Exceção ao definir assinaturas de função: {e_sig}")
        lib = None
else:
    print("AVISO kdtree_wrapper.py: 'lib' é None. As assinaturas das funções C não serão definidas.")

print("INFO kdtree_wrapper.py: Ficheiro carregado.")
