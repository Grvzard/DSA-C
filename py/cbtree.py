from ctypes import *

__all__ = 'cdll_init',


class cbtree(Structure):
    _fields_ = [
        ("degree", c_size_t),
        ("length", c_size_t),
        ("num_nodes", c_size_t),
        ("node_bytes", c_size_t),
        ("root", c_void_p),
    ]


def cdll_init(cdll) -> CDLL:
    cdll.btreeNew.restype = POINTER(cbtree)
    return cdll
