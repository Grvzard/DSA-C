#ifndef _DEQUE_H_
#define _DEQUE_H_

#include <stdlib.h>
#include <stdio.h>

typedef int DequeValueType;
#define DEQUE_TEST

struct _deque_node {
    DequeValueType value;

    struct _deque_node *prev;
    struct _deque_node *next;
};

typedef struct _deque_node dequeNode;

typedef struct {
    size_t len;
    struct _deque_node *curr;
} deque;


deque *dequeNew();
void dequeFree(deque *d);
void dequeNext(deque *d);
void dequePrev(deque *d);
void dequePush(deque *d, DequeValueType value);
void dequePop(deque *d);
DequeValueType dequeGet(deque *d);
size_t dequeLen(deque *d);
void dequePrint(deque *d);
#ifdef DEQUE_TEST
void dequeTest();
#endif  // DEQUE_TEST


#endif  // _DEQUE_H_
