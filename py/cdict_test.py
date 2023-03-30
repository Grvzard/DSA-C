from random import randint
from ctypes import CDLL

import cdict


def test():
    libdict = CDLL("../build/bin/libdict")
    cdict.cdll_init(libdict)

    libdict.dictTest1()

    d = libdict.Dict_New()

    for x in range(1000):
        libdict.Dict_Set(d, x, randint(0, 10000))

    print(f'dict len: {libdict.Dict_Len(d)}')
    print(f'get(10): {libdict.Dict_Get(d, 10)}')


if __name__ == '__main__':
    test()
