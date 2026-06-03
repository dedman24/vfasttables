
#include "stddef.h"
#include "stdint.h"
#include "string.h"

#include "hash.h"
#include "../ctx/ctx.h"

// returns freqency of all string elements.
static unsigned int* vfasttables_associated_values__frequency(size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s){
    unsigned int* const restrict frequency = calloc(vfasttables_supportedchars, sizeof(*frequency));

    for(size_t i = 0; i < s->incnt; i++){
        for(size_t j = 0; j < indexcnt; j++)
            frequency[vfasttables_index_get(s->instr[i], indices[j], s->inlen[i])]++;
    }

    return frequency;
}

// finds characters not shared by str0index and str1index
static char* vfasttables_associated_values__unsharedchars(const size_t str0index, const size_t str1index, size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s, size_t* unsharedcharcnt){
    char* unsharedchars = NULL;
    *unsharedcharcnt = 0;

    for(size_t i = 0; i < indexcnt; i++){
        const char c0 = vfasttables_index_get(s->instr[str0index], indices[i], s->inlen[str0index]);
        for(size_t j = i; j < indexcnt; j++){
            const char c1 = vfasttables_index_get(s->instr[str1index], indices[j], s->inlen[str1index]);
            
            if(c0 != c1){
                unsharedchars = realloc(unsharedchars, ++(*unsharedcharcnt));
                unsharedchars[*unsharedcharcnt - 1] = c0;
            }
        }
    }
// GUARANTEED to be not null.
    return unsharedchars;
}

static void vfasttables_associated_values__increment_based_on_lowest_frequency(uint32_t* const restrict associated_values, unsigned int* const restrict frequency, char* const restrict unsharedchars, const size_t sizeunsharedchars, const uint32_t mask){
    char lowfreq = unsharedchars[0];

    for(size_t i = 1; i < sizeunsharedchars; i++){
        if(frequency[lowfreq] > frequency[unsharedchars[i]]){
            lowfreq = unsharedchars[i];
        }
    }

    associated_values[lowfreq] = (associated_values[lowfreq] + 1) & mask;
}

// finds associated values.
static uint32_t* vfasttables_associated_values(size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s){
    uint32_t* const restrict associated_values = calloc(vfasttables_supportedchars, sizeof(*associated_values));
// table of where each entry places.
    size_t*  const restrict table = malloc(s->incnt*sizeof(*table));
    memset(table, -1, s->incnt*sizeof(*table));
// how often each character is present.
    unsigned int* const restrict frequency = vfasttables_associated_values__frequency(indices, indexcnt, s);
// has the threat of running endlessly, so we stop it at some ''reasonable'' point.
    for(uint32_t INTERNAL__sanityctr = 0; INTERNAL__sanityctr < 8192*s->incnt; INTERNAL__sanityctr++){
        for(size_t i = 0; i < s->incnt; i++){
            const uint32_t hash = vfasttables_hash(s->instr[i], s->inlen[i], s->incnt, associated_values, indices, indexcnt, s->mask);
// DEBUG.
//            printf("vfasttables_associated_values (sanityctr %u), (input %lu), (hash %u), (table %lu)\n", INTERNAL__sanityctr, i, hash, table[hash]);

            if(table[hash] != -1){      // if this condition succeeds, hash function is NOT perfect yet.
                size_t unsharedcharcnt;
                char* const restrict unsharedchars = vfasttables_associated_values__unsharedchars(i, table[hash], indices, indexcnt, s, &unsharedcharcnt);
                
                vfasttables_associated_values__increment_based_on_lowest_frequency(associated_values, frequency, unsharedchars, unsharedcharcnt, s->mask);
                free(unsharedchars);
                memset(table, -1, s->incnt*sizeof(*table));

                break;                  // repeats whole loop-
            }
            else table[hash] = i;       // string that made it this way.

            if(i == s->incnt-1){        // success! no hashes conflict; we have a perfect minimal hash function.
                free(frequency);
                free(table);
                return associated_values;
            }
        }
    }

    puts("ERROR: could not compute associated values array.");
    
    free(frequency);
    free(table);
    free(associated_values);
    return NULL;
}
