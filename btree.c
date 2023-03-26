// References:
// https://github.com/google/btree/blob/master/btree.go

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "btree.h"

#define UNUSED(x) (void)(x)

typedef struct _btnode btnode;

struct __attribute__ ((__packed__)) _btnode {
    size_t num_keys;
    size_t num_children;
    btreeKeyType *keys;
    btnode **children;
};

struct __attribute__ ((__packed__)) _btree {
    size_t degree;
    size_t length;
    size_t num_nodes;
    size_t node_bytes;  // size of a node
    btnode *root;
};

// >> internal functions
static inline size_t _MinKeys(btree *t) {
    return t->degree - 1;
}
static inline size_t MaxKeys(btree *t) {
    return t->degree*2 - 1;
}
static inline int isLeaf(btnode *n) {
    return (n->num_children == 0);
}
static btnode* _btreeNewNode(btree *t);
static void _btnodeInit(btnode *node);
static void _btreeFreeNode(btree *t, btnode *node);
static void _btreeFreeNodeR(btree *t, btnode *node);
static void _btnodeInsertKeyAt(btree *t, btnode *n, size_t p, btreeKeyType key);
static void _btnodeInsertChildAt(btree *t, btnode *n, size_t p, btnode *child);
static size_t ArrSearchKey(btreeKeyType keys[], size_t len, btreeKeyType key, int *found);
static size_t _btnodeSearchKey(btnode *node, btreeKeyType key, int *found);
static int _btnodeInsert(btree *t, btnode *node, btreeKeyType key);
static btreeKeyType _btnodeSplitToRight(btree *t, btnode *node_l, btnode *node_r);
static int _btnodeMaybeSplitChild(btree *t, btnode *node, size_t p);
static btnode* _btreeMinNode(btree *tree);
static btnode* _btreeMaxNode(btree *tree);
// << internal functions


extern btree* btreeNew(void) {
    btree *tree = (btree *)malloc(sizeof(btree));
    tree->degree = ((BTREE_M / 2) + (BTREE_M&1));
    tree->length = 0;
    tree->num_nodes = 0;
    tree->node_bytes = sizeof(btnode)
                       + sizeof(btreeKeyType) * MaxKeys(tree)
                       + sizeof(btnode*) * (MaxKeys(tree) + 1);
    tree->root = _btreeNewNode(tree);

    return tree;
}

extern void btreeFree(btree *tree) {
    assert(tree != NULL);
    _btreeFreeNodeR(tree, tree->root);
    free(tree);
}

// returns 1 if inserted a key or 0 if the key already exists
extern int btreeInsert(btree *tree, btreeKeyType key) {
    assert(tree != NULL);

    btnode *root = tree->root;
    if (root == NULL) {
        root = _btreeNewNode(tree);
        _btnodeInsertKeyAt(tree, root, 0, key);
        tree->length++;
        return 1;
    }
    if (root->num_keys >= MaxKeys(tree)) {
        btnode *new_right = _btreeNewNode(tree);
        btreeKeyType mid_key = _btnodeSplitToRight(tree, root, new_right);
        btnode *new_root = _btreeNewNode(tree);
        _btnodeInsertKeyAt(tree, new_root, 0, mid_key);
        _btnodeInsertChildAt(tree, new_root, 0, root);
        _btnodeInsertChildAt(tree, new_root, 1, new_right);
        tree->root = new_root;
    }

    int inserted = _btnodeInsert(tree, tree->root, key);
    if (inserted) {
        tree->length++;
    }

    return inserted;
}

static btnode* _btreeNewNode(btree *t) {
    btnode* node;
    node = (btnode *)malloc(t->node_bytes);

    size_t _keys_offset = sizeof(btnode);
    size_t _children_offset = _keys_offset + sizeof(btreeKeyType) * MaxKeys(t);
    char *node_base = (char*)node;
    node->keys = (btreeKeyType*)(&node_base[_keys_offset]);
    node->children = (btnode**)(&node_base[_children_offset]);
    _btnodeInit(node);

    t->num_nodes++; 

    return node;
}

static void _btnodeInit(btnode *node) {
    node->num_keys = 0;
    node->num_children = 0;
}

static void _btreeFreeNode(btree *t, btnode *node) {
    free(node);
    t->num_nodes--;
}

static void _btreeFreeNodeR(btree *t, btnode *node) {
    if (isLeaf(node)) {
        _btreeFreeNode(t, node);
    } else {
        for (size_t i = 0; i < node->num_children; i++) {
            _btreeFreeNodeR(t, node->children[i]);
        }
    }
}

static void _btnodeInsertKeyAt(btree *t, btnode *n, size_t p, btreeKeyType key) {
    UNUSED(t);
    assert(p <= n->num_keys);
    if (p < n->num_keys) {
        memcpy(&n->keys[p+1], &n->keys[p], (n->num_keys - p) * sizeof(n->keys[0]));
    }
    n->keys[p] = key;
    n->num_keys++;
}

static void _btnodeInsertChildAt(btree *t, btnode *n, size_t p, btnode *child) {
    UNUSED(t);
    assert(p <= n->num_children);
    if (p < n->num_children) {
        memcpy(&n->children[p+1], &n->children[p], (n->num_children - p) * sizeof(n->children[0]));
    }
    n->children[p] = child;
    n->num_children++;
}

// a general function using binary search
static size_t ArrSearchKey(btreeKeyType keys[], size_t len, btreeKeyType key, int *found) {
    if (found != NULL) *found = 0;
    size_t left_p = 0,
           right_p = len,
           mid_p;
    for (; right_p > left_p; ) {
        mid_p = (left_p + right_p) / 2;
        btreeKeyType mid_key = keys[mid_p];
        if (key < mid_key) {
            right_p = mid_p;
        } else if (key > mid_key) {
            left_p = mid_p + 1;
        } else {
            left_p = mid_p;
            right_p = mid_p;
            if (found != NULL) *found = 1;
        }
    }
    return left_p;
}

static size_t _btnodeSearchKey(btnode *node, btreeKeyType key, int *found) {
    return ArrSearchKey(node->keys, node->num_keys, key, found);
}

// returns 1 if inserted a key or 0 if the key already exists
static int _btnodeInsert(btree *t, btnode *node, btreeKeyType key) {
    UNUSED(t);

    int found = 0;
    size_t pos = _btnodeSearchKey(node, key, &found);
    if (found) {
        // reset value
        return 0;
    } else if (isLeaf(node)) {
        _btnodeInsertKeyAt(t, node, pos, key);
        return 1;
    }
    if (_btnodeMaybeSplitChild(t, node, pos)) {
        btreeKeyType mid_key = node->keys[pos];
        if (key < mid_key) {
            // no change
        } else if (key > mid_key) {
            pos += 1;
        } else {
            // the mid key of origin child node is what we want
            // reset value
            return 0;
        }
    }
    return _btnodeInsert(t, node->children[pos], key);
}

static btreeKeyType _btnodeSplitToRight(btree *t, btnode *node_l, btnode *node_r) {
    UNUSED(t);

    size_t nkeys = node_l->num_keys;  // nkeys == MaxKeys
    size_t mid_p = nkeys / 2;
    btreeKeyType mid_key = node_l->keys[mid_p];

    memcpy(node_r->keys, &node_l->keys[mid_p + 1], (nkeys - mid_p - 1) * sizeof(node_l->keys[0]));
    node_l->num_keys = mid_p;
    node_r->num_keys = nkeys - mid_p - 1;

    if (!isLeaf(node_l)) {
        memcpy(node_r->children,
               &node_l->children[mid_p + 2],
               (node_l->num_children - mid_p - 2) * sizeof(node_l->children[0]));
        node_l->num_children = mid_p + 1;
        node_r->num_children = node_l->num_children - mid_p - 2;
    }

    return mid_key;
}

// Returns whether or not a split occurred.
static int _btnodeMaybeSplitChild(btree *t, btnode *node, size_t p) {
    btnode *child = node->children[p];
    if (child->num_keys < MaxKeys(t)) {
        return 0;
    } else {
        btnode *new_child_right = _btreeNewNode(t);
        btreeKeyType mid_key = _btnodeSplitToRight(t, child, new_child_right);
        _btnodeInsertKeyAt(t, node, p, mid_key);
        _btnodeInsertChildAt(t, node, p + 1, new_child_right);
        return 1;
    }
}

static btnode* _btreeMinNode(btree *tree) {
    btnode *node = tree->root;
    for (; !isLeaf(node); ) {
        node = node->children[0];
    }
    return node;
}

static btnode* _btreeMaxNode(btree *tree) {
    btnode *node = tree->root;
    for (; !isLeaf(node); ) {
        node = node->children[node->num_children - 1];
    }
    return node;
}

#ifdef BTREE_TEST
static void _btnodePrint(btnode *node) {
    printf("[ ");
    for (size_t i = 0; i < node->num_keys; i++) {
        printf("%d ", node->keys[i]);
    }
    printf("]");
}

extern void btreePrint(btree *tree) {
    btnode *nodes_list[100000];
    size_t left_p = 0;
    size_t right_p = 0;

    btnode *node = tree->root;
    nodes_list[0] = node;
    right_p++;
    for (; left_p < right_p; left_p++) {
        node = nodes_list[left_p];
        if (!isLeaf(node)) {
            for (size_t i = 0; i < node->num_keys + 1; i++) {
                nodes_list[right_p] = node->children[i];
                right_p++;
            }
        }
    }

    printf("---%d node(s)---\n", right_p);
    for (size_t i = 0; i < right_p; i++) {
        _btnodePrint(nodes_list[i]);
        printf("\n");
    }
}

extern void btreeTest1(void) {
    btree *tree = btreeNew();

    for (int i = 1; i < MaxKeys(tree); i++) {
        btreeInsert(tree, i);
    }
    btreeInsert(tree, 0);
    btreePrint(tree);
    btreeInsert(tree, MaxKeys(tree) + 1);
    btreeInsert(tree, MaxKeys(tree) + 2);
    btreePrint(tree);

    btreeFree(tree);
    tree = NULL;
}

extern void btreeTest2(void) {
    btree *tree = btreeNew();

    for (int i = 0; i < 2000; i++) {
        btreeInsert(tree, i);
    }
    _btnodePrint(_btreeMinNode(tree));
    printf("\n");
    _btnodePrint(_btreeMaxNode(tree));
    printf("\n");

    btreeFree(tree);
    tree = NULL;
}
#endif  // BTREE_TEST
