
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dlist.h"

typedef struct _dlistnode node;

struct _dlistnode {
    node *prev, *next;
    dlistValType value;
};

struct _dlist {
    node *head, *tail;
    unsigned long len;
};

// >> internal functions
static node* nodeNew(dlistValType value);
static void nodeFree(node *n);
static dlistValType nodeGetValue(node *n);
// << internal functions


extern dlist* dlistNew(void) {
    dlist *list;
    list = malloc(sizeof(dlist));
    if (list == NULL)
        return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;

    return list;
}

extern unsigned long dlistLen(dlist *list) {
    return list->len;
}

extern dlist* dlistLPush(dlist *list, dlistValType value) {
    node *n = nodeNew(value);
    if (n == NULL)
        return NULL;

    if (list->len == 0) {
        list->head = n;
        list->tail = n;
        list->len += 1;
    } else {
        node *old_head = list->head;
        old_head->prev = n;
        n->next = old_head;
        list->head = n;
        list->len += 1;
    }

    return list;
}

extern dlist* dlistRPush(dlist *list, dlistValType value) {
    node *n = nodeNew(value);
    if (n == NULL)
        return NULL;

    if (list->len == 0) {
        list->head = n;
        list->tail = n;
        list->len += 1;
    } else {
        node *old_tail = list->tail;
        old_tail->next = n;
        n->prev = old_tail;
        list->tail = n;
        list->len += 1;
    }

    return list;
}

extern dlistValType dlistLPop(dlist *list) {
    assert(list->len > 0);

    node *old_head = list->head;
    dlistValType value = old_head->value;

    if (list->len == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = old_head->next;
        list->head->prev = NULL;
    }
    nodeFree(old_head);
    list->len -= 1;

    return value;
}

extern dlistValType dlistRPop(dlist *list) {
    assert(list->len > 0);

    node *old_tail = list->tail;
    dlistValType value = old_tail->value;

    if (list->len == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = old_tail->prev;
        list->tail->next = NULL;
    }
    nodeFree(old_tail);
    list->len -= 1;

    return value;
}

static node* nodeNew(dlistValType value) {
    node *n;
    n = malloc(sizeof(node));
    n->prev = NULL;
    n->next = NULL;
    n->value = value;

    return n;
}

static void nodeFree(node *n) {
    free(n);
}

static dlistValType nodeGetValue(node *n) {
    return n->value;
}

#ifdef DLIST_TEST
extern void dlistTest1(void) {
    dlist *list = dlistNew();

    dlistLPush(list, 0);
    dlistRPush(list, 0);
    assert(0 == dlistLPop(list));
    assert(0 == dlistLPop(list));
    dlistRPush(list, 0);
    dlistLPush(list, 0);
    assert(0 == dlistRPop(list));
    assert(0 == dlistRPop(list));
    assert(0 == dlistLen(list));

    printf("test1 ok\n");
}
#endif  // DLIST_TEST
