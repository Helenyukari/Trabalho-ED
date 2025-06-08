import ctypes
from ctypes import Structure, POINTER, c_double, c_int, c_char, c_float, c_void_p

# Define a estrutura TReg (embedding + nome)
class TReg(Structure):
    _fields_ = [
        ("embedding", c_float * 128),
        ("person_id", c_char * 100)
    ]

# Define a estrutura TNode
class TNode(Structure):
    pass

TNode._fields_ = [
    ("key", c_void_p),
    ("esq", POINTER(TNode)),
    ("dir", POINTER(TNode))
]

# Define a árvore Tarv
class Tarv(Structure):
    _fields_ = [
        ("raiz", POINTER(TNode)),
        ("cmp", c_void_p),
        ("dist", c_void_p),
        ("k", c_int)
    ]

# Carrega a biblioteca compilada 
try:
    lib = ctypes.CDLL("./kdtreee.dll") 
except OSError as e:
    print(f"Erro ao carregar a biblioteca: {e}")
    exit()


# Define as assinaturas das funções C que serão chamadas
lib.kdtree_construir_global.argtypes = []
lib.kdtree_construir_global.restype = None

lib.get_tree.argtypes = []
lib.get_tree.restype = POINTER(Tarv)

lib.inserir_ponto.argtypes = [TReg]
lib.inserir_ponto.restype = None

lib.buscar_mais_proximo.argtypes = [POINTER(Tarv), TReg]
lib.buscar_mais_proximo.restype = TReg
