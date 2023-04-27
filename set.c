
#include "set.h"
#include <string.h>


static bool _setLookup(set *s, SetKeyType key, size_t *pos);
static void _slotsResize(set *s);
static size_t _slotsSize(set *s);
static size_t _calcMinSize(size_t used);


set *setNew(void) {
    set *s;

    s = malloc(sizeof(set));
    if (s == NULL)
        return NULL;

    s->fill = 0;
    s->used = 0;
    s->slots = NULL;
    _slotsResize(s);

    return s;
}

void setFree(set *s) {
    if (s == NULL) {
        return;
    }

    if (s->slots != NULL) {
        free(s->slots);
    }
    free(s);
}

bool setAdd(set* s, SetKeyType key) {
    if (s->fill >= _slotsSize(s)) {
        _slotsResize(s);
    }
    size_t pos;
    if (_setLookup(s, key, &pos)) {
        return false;
    }

    s->slots[pos].status = 0;
    s->slots[pos].key = key;
    ++s->fill;
    ++s->used;
    return true;
}

bool setDel(set* s, SetKeyType key) {
    size_t pos;
    if (_setLookup(s, key, &pos)) {
        s->slots[pos].status = SET_SLOT_DUMMY;
        --s->used;
        return true;
    }
    return false;
}

bool setHas(set* s, SetKeyType key) {
    return _setLookup(s, key, NULL);
}

static bool _setLookup(set *s, SetKeyType key, size_t *pos) {
    size_t hash = (size_t)key;

    size_t perturb = hash;
    size_t i = hash & s->mask;

    while (s->slots[i].status >= (char)0) {
        if (s->slots[i].key == key) {
            if (pos) *pos = i;
            return true;
        }
        perturb >>= PERTURB_SHIFT;
        i = (i * 5 + 1 + perturb) & s->mask;
    }
    if (pos) *pos = i;
    return false;
}

static size_t _calcMinSize(size_t used) {
    size_t i = SET_MIN_SIZE;
    while (i <= used) i <<= 1;
    return i;
}

static void _slotsResize(set *s) {
    size_t old_mask = s->mask;
    setentry *old_slots = s->slots;

    size_t size = _calcMinSize(s->used);
    s->mask = size - 1;
    s->slots = malloc(size * sizeof(setentry));
    memset(s->slots, 0xff, size * sizeof(setentry));

    if (old_slots == NULL) {
        return;
    }

    for (setentry *entry = old_slots; entry <= old_slots + old_mask; entry++) {
        if (entry->status >= 0) {
            size_t pos;
            _setLookup(s, entry->key, &pos);
            s->slots[pos] = *entry;
        }
    }

    free(old_slots);
    s->fill = s->used;
}

static size_t _slotsSize(set *s) {
    return s->mask + 1;
}
