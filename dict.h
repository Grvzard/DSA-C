// References:

// https://github.com/python/cpython/blob/main/Objects/dictobject.c
// | *

// https://github.com/python/cpython/blob/main/Include/cpython/dictobject.h
// | PyDictObject

// https://github.com/python/cpython/blob/main/Include/internal/pycore_dict.h
// | DK_ENTRIES | DK_LOG_SIZE | PyDictKeyEntry | DKIX | PyDictKeysObject

#ifndef _DICT_H_
#define _DICT_H_

#include <stdint.h>

// >> settings
#define DICT_TEST

#define DICT_LOG_MINSIZE 3

typedef int64_t DictKeyType;
typedef int64_t DictValueType;
// << settings

typedef struct _dict Dict;

// >> external API
extern Dict*
Dict_New(void);
extern Dict*
Dict_NewPresized(size_t size);
extern DictValueType
Dict_Get(Dict* mp, DictKeyType key);
extern void
Dict_Set(Dict* mp, DictKeyType key, DictValueType value);
extern int
Dict_Has(Dict* mp, DictKeyType key);
extern int
Dict_Del(Dict* mp, DictKeyType key);
extern size_t
Dict_Len(Dict* mp);
extern void
Dict_Free(Dict* d);
#ifdef DICT_TEST
extern void
dictTest1(void);
extern void
dictTest2(void);
extern void
dictTest3(void);
extern void
dictTest4(void);
#endif
// << external API

#endif  // _DICT_H_
