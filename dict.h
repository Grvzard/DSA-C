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

typedef int DictKeyType;
typedef int DictValueType;
typedef uint64_t hash_t;
// << settings

typedef struct {
    hash_t hash;
    DictKeyType key;
    DictValueType value;
} DictKeyEntry;

typedef struct {
    uint8_t dk_log2_size;

    // cpython use dk_log2_index_bytes
    // but dk_index_bytes seems more eaier to understand
    // (1 << dk_log2_index_bytes) == (dk_size * dk_index_bytes)
    // possible values: [ 1 | 2 | 4 | 8 ]
    uint8_t dk_index_bytes;

    int (*keyCmpFunc)(DictKeyType key1, DictKeyType key2);

    hash_t (*keyHashFunc)(DictKeyType key);

    /* Number of usable entries in dk_entries. */
    size_t dk_usable;

    size_t dk_nentries;  // used + dummies

    // It's called flexible array member, new in C99
    char dk_indices[];

    /* "PyDictKeyEntry dk_entries[USABLE_FRACTION(DK_SIZE(dk))];" array follows:
       see the DK_ENTRIES() macro */
} DictKeys;

typedef struct {
    /* Number of items in the dictionary */
    uint32_t used;

    // number of bytes needed for the dictkeys object
    size_t dk_size;

    DictKeys* keys;
} Dict;


// >> external API
extern Dict*
dictNew(void);
extern Dict*
dictNewPresized(size_t size);
extern DictValueType
dictGet(Dict* mp, DictKeyType key);
extern void
dictSet(Dict* mp, DictKeyType key, DictValueType value);
extern int
dictHas(Dict* mp, DictKeyType key);
extern int
dictDel(Dict* mp, DictKeyType key);
extern size_t
dictLen(Dict* mp);
extern void
dictFree(Dict* d);
#ifdef DICT_TEST
extern void
dictTest1(void);
extern void
dictTest2(void);
extern void
dictTest3(void);
extern void
dictTest4(void);
extern void
dictTest5(void);
#endif
// << external API

#endif  // _DICT_H_
