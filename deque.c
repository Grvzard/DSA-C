
#include "deque.h"


static dequeNode *_nodeNew(DequeValueType val) {
    dequeNode *n = (dequeNode *)malloc(sizeof(dequeNode));
    n->value = val;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

static void _nodeFree(dequeNode *n) {
    free(n);
}

deque *dequeNew() {
    deque *d = (deque *)malloc(sizeof(deque));
    d->curr = NULL;
    d->len = 0;
    return d;
}

void dequeFree(deque *d) {
    dequeNode *n = d->curr;
    dequeNode *next;
    for (size_t i = 0; i < d->len; i++) {
        next = n->next;
        _nodeFree(n);
        n = next;
    }
    free(d);
}

void dequeNext(deque *d) {
    if (d->len > 1) {
        d->curr = d->curr->next;       
    }
}

void dequePrev(deque *d) {
    if (d->len > 1) {
        d->curr = d->curr->prev;       
    }
}

void dequePush(deque *d, DequeValueType val) {
    dequeNode *n = _nodeNew(val);
    if (d->len == 0) {
        n->prev = n;
        n->next = n;
    // } else if (d->len == 1) {
    //     d->curr->prev = n;
    //     d->curr->next = n;
    //     n->prev = d->curr;
    //     n->next = d->curr;
    } else {
        n->prev = d->curr;
        n->next = d->curr->next;
        d->curr->next->prev = n;
        d->curr->next = n;
    }
    d->curr = n;
    d->len++;
}

void dequePop(deque *d) {
    if (d->len == 0) {
        return;
    }
    dequeNode *n = d->curr;
    if (d->len == 1) {
        d->curr = NULL;
    } else {
        dequeNode *next = d->curr->next;
        dequeNode *prev = d->curr->prev;
        prev->next = next;
        next->prev = prev;
        d->curr = next;
    }
    _nodeFree(n);
    d->len--;
}

DequeValueType dequeGet(deque *d) {
    return d->curr->value;
}

size_t dequeLen(deque *d) {
    return d->len;
}

void dequePrint(deque *d) {
    dequeNode *n = d->curr;
    printf("len: %zu, elems: [ ", d->len);
    for (size_t i = 0; i < d->len; i++) {
        printf("%d ", n->value);
        n = n->next;
    }
    printf("]\n");
}

#ifdef DEQUE_TEST
void dequeTest() {
    deque *d = dequeNew();
    for (int i = 0; i < 10; i++) {
        dequePush(d, i);
    }
    dequePrint(d);
    for (int i = 0; i < 5; i++) {
        dequeNext(d);
    }
    dequePrint(d);
    for (int i = 0; i < 5; i++) {
        dequePop(d);
    }
    dequePrint(d);
    dequeFree(d);
}
#endif