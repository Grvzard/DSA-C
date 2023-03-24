
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "dict.h"

typedef int32_t ix_t;
typedef size_t hash_t;

#define DKIX_EMPTY (-1)
#define DKIX_DUMMY (-2)

#define USABLE_FRACTION(n) (((n) << 1)/3)
#define GROWTH_RATE(dict)  ((dict)->used*3)

#define DK_LOG_SIZE(dk)  ((dk)->dk_log2_size)
#define DK_SIZE(dk)      ((size_t)1<<DK_LOG_SIZE(dk))
#define DK_MASK(dk)      (DK_SIZE(dk)-1)

#define PERTURB_SHIFT 5

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

    /* Number of usable entries in dk_entries. */
    size_t dk_usable;

    size_t dk_nentries;  // used + dummies

    // It's called flexible array member, new in C99
    char dk_indices[];

    /* "PyDictKeyEntry dk_entries[USABLE_FRACTION(DK_SIZE(dk))];" array follows:
       see the DK_ENTRIES() macro */
} DictKeys;

struct _dict {
    /* Number of items in the dictionary */
    uint32_t used;

    // number of bytes needed for the dictkeys object
    size_t dk_size;

    DictKeys* keys;

};


// >> internal functions
static inline DictKeyEntry*
DK_ENTRIES(DictKeys* dk) {
    int8_t* indices = (int8_t*)(dk->dk_indices);
    size_t index = dk->dk_index_bytes * DK_SIZE(dk);
    return (DictKeyEntry*)(&indices[index]);
}
static inline hash_t
Key_Hash(DictKeyType key) {
    return (hash_t)key;
}
static void
_Dict_Resize(Dict* mp);
static DictKeys*
_DictKeys_New(uint8_t log2_size);
static int
_DictKeys_Get(DictKeys* dk, DictKeyType key, DictValueType* value);
static int
_DictKeys_Set(DictKeys* dk, DictKeyType key, DictValueType value);
static void
_DictKeys_Free(DictKeys* dk);
static ix_t
_DictKeys_Lookup(DictKeys* dk, DictKeyType key, hash_t hash);
static ix_t
_DictKeys_GetIndex(const DictKeys* dk, size_t i);
static void
_DictKeys_SetIndex(DictKeys* dk, size_t i, ix_t ix);
static size_t
_DictKeys_FindEmptySlot(DictKeys* dk, hash_t hash);
static void
_DictKeys_BuildIndices(DictKeys* dk, DictKeyEntry* newentries, size_t nentries);
static size_t
_DictKeys_GetHashPosition(DictKeys* dk, hash_t hash, ix_t index);
// << internal functions


extern Dict*
Dict_New(void) {
    Dict* mp = (Dict*)malloc(sizeof(Dict));
    mp->used = 0;
    mp->keys = _DictKeys_New(DICT_LOG_MINSIZE);
    return mp;
}

extern DictValueType
Dict_Get(Dict* mp, DictKeyType key) {
    DictValueType value;
    assert(_DictKeys_Get(mp->keys, key, &value) == 0);
    return value;
}

extern void
Dict_Set(Dict* mp, DictKeyType key, DictValueType value) {
    if (mp->keys->dk_usable <= 0) {
        _Dict_Resize(mp);
    }
    int8_t ret = _DictKeys_Set(mp->keys, key, value);
    assert(ret == 0 || ret == 1);
    mp->used += ret;
}

extern int
Dict_Has(Dict* mp, DictKeyType key) {
    DictKeys* dk = mp->keys;
    ix_t ix = _DictKeys_Lookup(dk, key, Key_Hash(key));
    return (ix >= 0);
}

extern int
Dict_Del(Dict* mp, DictKeyType key) {
    DictKeys* dk = mp->keys;
    ix_t ix = _DictKeys_Lookup(dk, key, Key_Hash(key));
    if (ix >= 0) {
        // Delete Key
        size_t hashpos = _DictKeys_GetHashPosition(dk, Key_Hash(key), ix);
        _DictKeys_SetIndex(dk, hashpos, DKIX_DUMMY);
        mp->used--;
    } else {
        // do nothing
    }
    return 0;
}

extern size_t
Dict_Len(Dict* mp) {
    return mp->used;
}

extern void
Dict_Free(Dict* d) {
    _DictKeys_Free(d->keys);
    free(d);
}

static void
_Dict_Resize(Dict* mp) {
    DictKeys* oldkeys = mp->keys;
    size_t nentries = mp->used;

    uint8_t new_log2_size = DICT_LOG_MINSIZE;
    for (; ((size_t)1 << new_log2_size) < GROWTH_RATE(mp);) {
        new_log2_size++;
    }

    mp->keys = _DictKeys_New(new_log2_size);
    // if (mp->keys == NULL) {
    //     mp->keys = oldkeys;
    //     return;
    // }
    assert(mp->keys != NULL);

    assert(mp->keys->dk_usable >= nentries);

    DictKeyEntry* old_entries = DK_ENTRIES(oldkeys);
    DictKeyEntry* new_entries = DK_ENTRIES(mp->keys);
    if (oldkeys->dk_nentries == nentries) {
        memcpy(new_entries, old_entries, nentries * sizeof(DictKeyEntry));
        _DictKeys_BuildIndices(mp->keys, new_entries, nentries);
    } else {
        DictKeyEntry* ep = old_entries;
        for (ix_t ix = 0; ix < nentries; ix++, ep++) {
            for (; _DictKeys_Lookup(oldkeys, ep->key, ep->hash) < 0;) {
                ep++;
            }
            size_t i = _DictKeys_FindEmptySlot(mp->keys, ep->hash);
            _DictKeys_SetIndex(mp->keys, i, ix);
        }
    }

    _DictKeys_Free(oldkeys);

    mp->keys->dk_usable -= nentries;
    mp->keys->dk_nentries = nentries;
}

static void
_DictKeys_BuildIndices(DictKeys* dk, DictKeyEntry* ep, size_t nentries) {
    size_t mask = DK_MASK(dk);
    for (ix_t ix = 0; ix < nentries; ix++, ep++) {
        size_t i = _DictKeys_FindEmptySlot(dk, ep->hash);
        _DictKeys_SetIndex(dk, i, ix);
    }
}

static int
_DictKeys_Get(DictKeys* dk, DictKeyType key, DictValueType* value) {
    int err = 0;
    ix_t ix = _DictKeys_Lookup(dk, key, Key_Hash(key));
    if (ix >= 0) {
        *value = DK_ENTRIES(dk)[ix].value;
    } else {
        err = -1;
    }
    return err;
}

static int
_DictKeys_Set(DictKeys* dk, DictKeyType key, DictValueType value) {
    hash_t hash = Key_Hash(key);
    int ret = 0;
    assert(dk->dk_usable > 0);
    ix_t ix = _DictKeys_Lookup(dk, key, hash);
    DictKeyEntry* ep;
    if (ix < 0) {
        // Insert Key
        size_t i = _DictKeys_FindEmptySlot(dk, hash);
        _DictKeys_SetIndex(dk, i, dk->dk_nentries);
        ep = &DK_ENTRIES(dk)[dk->dk_nentries];
        dk->dk_usable--;
        dk->dk_nentries++;
    } else {
        // Update Key
        ep = &DK_ENTRIES(dk)[ix];
    }
    ep->key = key;
    ep->hash = hash;
    ep->value = value;
    return (ix < 0);
}

static DictKeys*
_DictKeys_New(uint8_t log2_size) {
    DictKeys* dk;
    uint8_t index_bytes;
    if (log2_size < 8) {
        index_bytes = 1;
    } else if (log2_size < 16) {
        index_bytes = 2;
    } else if (log2_size >= 32) {
        index_bytes = 8;
    } else {
        // 16 <= log2_size < 32
        index_bytes = 4;
    }
    size_t dk_size = (size_t)1 << log2_size;
    size_t entry_bytes = sizeof(DictKeyEntry);
    size_t usable = USABLE_FRACTION(dk_size);
    size_t total_bytes = sizeof(DictKeys)
                         + dk_size * index_bytes
                         + usable * entry_bytes;
    dk = (DictKeys*)malloc(total_bytes);
    assert(dk != NULL);
    dk->dk_log2_size = log2_size;
    dk->dk_index_bytes = index_bytes;
    dk->dk_usable = usable;
    dk->dk_nentries = 0;
    // for (int i = 0; i < size; i++) {
    //     dk->dk_indices[i] = -1;
    // }
    memset(&dk->dk_indices[0], 0xff, dk_size * index_bytes);
    memset(DK_ENTRIES(dk), 0, usable * entry_bytes);
    return dk;
}

static ix_t
_DictKeys_Lookup(DictKeys* dk, DictKeyType key, hash_t hash) {
    ix_t ix;
    DictKeyEntry* ep0 = DK_ENTRIES(dk);
    size_t perturb = (size_t)hash;
    size_t size = DK_SIZE(dk);
    // size_t i = hash % size;
    size_t mask = DK_MASK(dk);
    size_t i = (size_t)hash & mask;
    for (; ; ) {
        ix_t ix = _DictKeys_GetIndex(dk, i);
        if (ix >= 0) {
            DictKeyEntry* ep = &ep0[ix];
            if (ep->key == key) {
                return ix;
            }
        } else {
            return ix;
        }
        perturb >>= PERTURB_SHIFT;
        i = (i * 5 + perturb + 1) % size;
    }
}

/* Internal function to find slot for an item from its hash
   when it is known that the key is not present in the dict. */
static size_t
_DictKeys_FindEmptySlot(DictKeys* dk, hash_t hash) {
    assert(dk != NULL);
    const size_t mask = DK_MASK(dk);
    size_t i = hash & mask;
    for (size_t perturb = hash; _DictKeys_GetIndex(dk, i) >= 0;) {
        perturb >>= PERTURB_SHIFT;
        i = (i * 5 + perturb + 1) & mask;
    }
    return i;
}

static size_t
_DictKeys_GetHashPosition(DictKeys* dk, hash_t hash, ix_t index) {
    size_t mask = DK_MASK(dk);
    size_t perturb = (size_t)hash;
    size_t i = (size_t)hash & mask;
    for (; _DictKeys_GetIndex(dk, i) != index;) {
        perturb >>= PERTURB_SHIFT;
        i = mask & (i * 5 + perturb + 1);
    }
    return i;
}

static void
_DictKeys_Free(DictKeys* dk) {
    free(dk);
}

static inline ix_t
_DictKeys_GetIndex(const DictKeys* dk, size_t i) {
    ix_t ix;
    uint8_t index_bytes = dk->dk_index_bytes;
    if (index_bytes == 1) {
        int8_t* indices = (int8_t*)(dk->dk_indices);
        ix = indices[i];
    } else if (index_bytes == 2) {
        int16_t* indices = (int16_t*)(dk->dk_indices);
        ix = indices[i];
    } else if (index_bytes == 8) {
        int64_t* indices = (int64_t*)(dk->dk_indices);
        ix = indices[i];
    } else {
        int32_t* indices = (int32_t*)(dk->dk_indices);
        ix = indices[i];
    }
    return ix;
}

static inline void
_DictKeys_SetIndex(DictKeys* dk, size_t i, ix_t ix) {
    uint8_t index_bytes = dk->dk_index_bytes;
    if (index_bytes == 1) {
        int8_t* indices = (int8_t*)(dk->dk_indices);
        assert(ix <= 0x7f);
        indices[i] = (char)ix;
    } else if (index_bytes == 2) {
        int16_t* indices = (int16_t*)(dk->dk_indices);
        assert(ix <= 0x7fff);
        indices[i] = (int16_t)ix;
    } else if (index_bytes == 8) {
        int64_t* indices = (int64_t*)(dk->dk_indices);
        indices[i] = ix;
    } else {
        int32_t* indices = (int32_t*)(dk->dk_indices);
        assert(ix <= 0x7fffffff);
        indices[i] = (int32_t)ix;
    }
}

#ifdef DICT_TEST
extern void
dictTest1(void) {
    Dict *d = Dict_New();

    if (!Dict_Has(d, 22)) {
        Dict_Set(d, 22, 0);
    }
    if (!Dict_Has(d, 22)) {
        Dict_Set(d, 22, -1);
    }
    int val = (int)Dict_Get(d, 22);
    printf("%d\n", val);
    assert(val == 0);

    Dict_Free(d);
    d = NULL;
}

extern void
dictTest2(void) {
    Dict *d = Dict_New();

    for (int i = 0; i < 70; i++) {
        if (!Dict_Has(d, i)) {
            Dict_Set(d, i, i);
        }
    }
    for (int i = 0; i < 30; i++) {
        if (Dict_Has(d, i)) {
            Dict_Del(d, i);
        }
    }

    // printf("val1: %d\n", Dict_Get(d, 12));
    printf("val2: %d\n", Dict_Get(d, 69));
    printf("len: %d\n", Dict_Len(d));
    assert(Dict_Get(d, 69) == 69);
    assert(Dict_Len(d) == 40);

    Dict_Free(d);
    d = NULL;
}

extern void
dictTest3(void) {
    printf("%d\n", sizeof(DictKeys));
}
#endif  // DICT_TEST
