// References:


#ifndef _SET_H_
#define _SET_H_

#include <stdlib.h>
#include <stdbool.h>

typedef int SetKeyType;

#define SET_MIN_SIZE 8

#define SET_SLOT_EMPTY (-1)
#define SET_SLOT_DUMMY (-2)
#define PERTURB_SHIFT 5

typedef struct {
    SetKeyType key;
    char status;
} setentry;

typedef struct {
    size_t fill;  // Number active and dummy entries
    size_t used;  // Number active entries
    size_t mask;
    setentry *slots;
} set;


set *setNew(void);
void setFree(set* s);
bool setAdd(set* s, SetKeyType key);
bool setDel(set* s, SetKeyType key);
bool setHas(set* s, SetKeyType key);


#endif  // _SET_H_
