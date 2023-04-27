
#include "set.h"
#include <assert.h>
#include <stdio.h>


void test1(void) {
    printf("[set] test-1\n");
    set *s = setNew();
    for (int i = 0; i < 10; i++) {
        setAdd(s, i);
    }
    assert(setHas(s, 9));
    assert(!setHas(s, 11));
    setFree(s);
}

void test2(void) {
    printf("[set] test-2\n");
    set *s = setNew();

    for (int i = 0; i < 10; i++) {
        setAdd(s, i);
    }
    for (int i = 0; i < 10; i++) {
        setDel(s, i);
    }
    for (int i = 10; i < 100; i++) {
        setAdd(s, i);
    }
    assert(!setHas(s, 9));
    assert(setHas(s, 99));
    setFree(s);
}


int main(void) {
    test1();
    test2();

    printf("ok");
    return 0;
}
