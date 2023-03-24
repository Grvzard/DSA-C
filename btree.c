
#include <stdlib.h>
#include <stdio.h>
#include "btree.h"

#define UNUSED(x) (void)(x)

typedef struct _btnode btnode;

struct __attribute__ ((__packed__)) _btnode {
    size_t num_keys;
    size_t is_leaf:1;
    KeyType keys[BTREE_M];
    btnode *children[BTREE_M + 1];
    btnode *parent;
};

struct __attribute__ ((__packed__)) _btree {
    btnode *root;
};

// >> internal functions
static btnode* _btnodeNew();
static void _btnodeInit(btnode *node);
static void _btnodeFree(btnode *node);
static size_t _btnodeSearchKey(btnode *node, KeyType key);
static void _leafnodeInsertKey(btnode *node, KeyType key);
static void _internelnodeInsertKey(btnode *node, KeyType key, btnode *child_left, btnode *child_right);
static void _btnodeAddKey(btnode *node, size_t pos, KeyType key);
static void _btnodeAddChild(btnode *parent, size_t child_p, btnode *child);
static void _btnodeSplit(btnode *node);
static void _btnodeSplitMembers(btnode *origin, btnode *new_left, btnode *new_right);
static btnode* _btreeMinNode(btree *tree);
static btnode* _btreeMaxNode(btree *tree);
// << internal functions


extern btree* btreeNew() {
    btree *tree = (btree *)malloc(sizeof(btree));
    tree->root = _btnodeNew();

    return tree;
}

extern void btreeFree(btree *tree) {
    _btnodeFree(tree->root);
    free(tree);
}

extern void btreeInsert(btree *tree, KeyType key) {
    btnode *node = tree->root;
    for (; !(node->is_leaf); ) {
        size_t next_p = _btnodeSearchKey(node, key);
        node = node->children[next_p];
    }
    _leafnodeInsertKey(node, key);
}

static btnode* _btnodeNew() {
    btnode* node;
    node = (btnode *)malloc(sizeof(btnode));
    _btnodeInit(node);

    return node;
}

static void _btnodeInit(btnode *node) {
    for (size_t i = 0; i < BTREE_M; i++) {
        node->keys[i] = 0;
        node->children[i] = NULL;
    }
    node->children[BTREE_M] = NULL;
    node->num_keys = 0;
    node->is_leaf = 1;
    node->parent = NULL;
}

static void _btnodeFree(btnode *node) {
    if (node->is_leaf) {
        free(node);
    } else {
        size_t nchildren = node->num_keys + 1;
        for (size_t i = 0; i < nchildren; i++) {
            _btnodeFree(node->children[i]);
        }
    }
}

static void _btnodeAddKey(btnode *node, size_t pos, KeyType key) {
    node->keys[pos] = key;
    node->num_keys += 1;
}

static void _btnodeAddChild(btnode *parent, size_t child_p, btnode *child) {
    parent->children[child_p] = child;
    child->parent = parent;
}

// a general function using binary search
static size_t ArrSearchKey(KeyType keys[], size_t len, KeyType key) {
    size_t left_p = 0,
           right_p = len,
           mid_p;
    for (; right_p > left_p; ) {
        mid_p = (left_p + right_p) / 2;
        KeyType mid_key = keys[mid_p];
        if (key < mid_key) {
            right_p = mid_p;
        } else if (key > mid_key) {
            left_p = mid_p + 1;
        } else {
            left_p = mid_p;
            right_p = mid_p;
        }
    }
    return left_p;
}

static size_t _btnodeSearchKey(btnode *node, KeyType key) {
    return ArrSearchKey(node->keys, node->num_keys, key);
}

static void _leafnodeInsertKey(btnode *node, KeyType key) {
    size_t pos = _btnodeSearchKey(node, key);

    KeyType *p_keys = node->keys;
    for (size_t i = node->num_keys; i > pos; i--) {
        p_keys[i] = p_keys[i-1];
    }
    _btnodeAddKey(node, pos, key);

    if (node->num_keys >= BTREE_M) {
        _btnodeSplit(node);
    }
}

static void _internelnodeInsertKey(btnode *node, KeyType key, btnode *child_left, btnode *child_right) {
    size_t pos = _btnodeSearchKey(node, key);

    KeyType *p_keys = node->keys;
    for (size_t i = node->num_keys; i > pos; i--) {
        p_keys[i] = p_keys[i-1];
    }
    _btnodeAddKey(node, pos, key);

    btnode **p_children = node->children;
    for (size_t i = node->num_keys; i > pos; i--) {
        p_children[i] = p_children[i-1];
    }
    _btnodeAddChild(node, pos, child_left);
    _btnodeAddChild(node, pos + 1, child_right);

    if (node->num_keys >= BTREE_M) {
        _btnodeSplit(node);
    }
}

static void _btnodeSplit(btnode *node) {
    size_t mid_p = node->num_keys / 2;
    KeyType mid_key = node->keys[mid_p];

    if (node->parent) {
        // >> internel node split
        btnode *new_left = _btnodeNew();
        btnode *new_right = _btnodeNew();

        _btnodeSplitMembers(node, new_left, new_right);
        _internelnodeInsertKey(node->parent, mid_key, new_left, new_right);

        free(node);

    } else {
        // >> root node split
        btnode *new_left = _btnodeNew();
        btnode *new_right = _btnodeNew();

        _btnodeSplitMembers(node, new_left, new_right);
        // reuse origin root node
        _btnodeInit(node);
        btnode *new_root = node;
        new_root->is_leaf = 0;
        _btnodeAddKey(new_root, 0, mid_key);
        _btnodeAddChild(new_root, 0, new_left);
        _btnodeAddChild(new_root, 1, new_right);
    }
}

static void _btnodeSplitMembers(btnode *origin, btnode *new_left, btnode *new_right) {
    size_t num_keys = origin->num_keys;
    size_t mid_p = num_keys / 2;

    for (size_t i = 0; i < num_keys; i++) {
        KeyType key = origin->keys[i];
        if (i < mid_p) {
            new_left->keys[i] = key;
            new_left->num_keys += 1;
        } else if (i > mid_p) {
            new_right->keys[i - mid_p - 1] = key;
            new_right->num_keys += 1;
        } else {
            // pass
        }
    }

    if (!(origin->is_leaf)) {
        new_left->is_leaf = 0;
        new_right->is_leaf = 0;

        for (size_t i = 0; i <= num_keys; i++) {
            btnode *child = origin->children[i];
            if (i <= mid_p) {
                _btnodeAddChild(new_left, i, child);
            } else {
                _btnodeAddChild(new_right, i - mid_p - 1, child);
            }
        }
    }
}

static btnode* _btreeMinNode(btree *tree) {
    btnode *node = tree->root;
    for (; !(node->is_leaf); ) {
        node = node->children[0];
    }
    return node;
}

static btnode* _btreeMaxNode(btree *tree) {
    btnode *node = tree->root;
    for (; !(node->is_leaf); ) {
        node = node->children[node->num_keys];
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
        if (!(node->is_leaf)) {
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

    for (int i = 0; i < 16; i++) {
        btreeInsert(tree, i);
    }
    btreePrint(tree);
    btreeInsert(tree, 16);
    btreePrint(tree);

    btreeFree(tree);
    tree = NULL;
}

extern void btreeTest2(void) {
    btree *tree = btreeNew();

    for (int i = 0; i < 200000; i++) {
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
