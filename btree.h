
#ifndef _BTREE_H_
#define _BTREE_H_

// >> settings
#define BTREE_TEST
#define BTREE_M 400

typedef int KeyType;
// << settings

typedef struct _btree btree;

// >> external API
extern btree* btreeNew(void);
extern void btreeInsert(btree *tree, KeyType key);
extern void btreeFree(btree *tree);
#ifdef BTREE_TEST
extern void btreePrint(btree *tree);
extern void btreePrintMaxNode(btree *tree);
extern void btreeTest1(void);
extern void btreeTest2(void);
#endif
// << external API

#endif  // _BTREE_H_
