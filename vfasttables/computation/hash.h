#ifndef vfasttables_hash_included
#define vfasttables_hash_included

#include "stddef.h"
#include "stdint.h"

#include "../ctx/ctx.h"

// hash function for vfasttables.
uint32_t vfasttables_hash(char* const restrict s, const size_t slen, const size_t strcnt, uint32_t* const restrict associated_values, size_t* const restrict indices, size_t indexcnt, const uint32_t mask){
    uint32_t hash = associated_values[slen];

    for(size_t i = 0; i < indexcnt; i++)
        hash += associated_values[vfasttables_index_get(s, indices[i], slen)];

    return (hash & mask) % strcnt;
}


#endif
