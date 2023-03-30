from ctypes import CDLL

import cbtree


def test():
    libbtree = cbtree.cdll_init(CDLL("../build/bin/libbtree"))
    
    t = libbtree.btreeNew()

    print(libbtree.btreeSet(t, 12) and 'ok' or 'set failed')
    libbtree.btreePrint(t)

    print(libbtree.btreeDel(t, 1) and 'ok' or 'del failed')
    print(libbtree.btreeDel(t, 12) and 'ok' or 'del failed')


if __name__ == '__main__':
    test()
