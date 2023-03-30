from ctypes import *

__all__ = 'cdll_init',


class cdict(Structure):
    _fields_ = [
        ("used", c_uint),
        ("dk_size", c_size_t),
        ("keys", c_void_p),
    ]


def cdll_init(cdll) -> CDLL:
    cdll.Dict_New.restype = POINTER(cdict)
    cdll.Dict_NewPresized.restype = POINTER(cdict)
    return cdll
