/* doubly linked list */
#ifndef _DLIST_H_
#define _DLIST_H_

// >> settings
#define DLIST_TEST
typedef int dlistValType;
// << settings

typedef struct _dlist dlist;

// >> external API
extern dlist* dlistNew(void);
extern unsigned long dlistLen(dlist *list);
extern dlist* dlistLPush(dlist *list, dlistValType value);
extern dlist* dlistRPush(dlist *list, dlistValType value);
extern dlistValType dlistLPop(dlist *list);
extern dlistValType dlistRPop(dlist *list);
#ifdef DLIST_TEST
extern void dlistTest1(void);
#endif  // DLIST_TEST
// << external API

#endif  // _DLIST_H_
