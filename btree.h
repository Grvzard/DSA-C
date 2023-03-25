
#ifndef _BTREE_H_
#define _BTREE_H_

// >> settings
#define BTREE_TEST
#define BTREE_M 400

typedef int btreeKeyType;
// << settings

// #define BTREE_MinKeys ((BTREE_M / 2) + (BTREE_M&1) - 1)
typedef struct _btree btree;

// >> external API
extern btree* btreeNew(void);
extern int btreeInsert(btree *tree, btreeKeyType key);
// extern int btreeDel(btree *tree, btreeKeyType key);
extern void btreeFree(btree *tree);
#ifdef BTREE_TEST
extern void btreePrint(btree *tree);
extern void btreeTest1(void);
extern void btreeTest2(void);
// extern void btreeTest3(void);
#endif
// << external API

#endif  // _BTREE_H_
