
#include "stddef.h"
#include "stdint.h"
#include "string.h"
#include "stdbool.h"

#include "sys/random.h"

#include "hash.h"
#include "../ctx/ctx.h"

// returns freqency of all string elements.
static unsigned int* vfasttables_associated_values__frequency(size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s){
    unsigned int* const restrict frequency = calloc(vfasttables_supportedchars, sizeof(*frequency));

    for(size_t i = 0; i < s->incnt; i++){
        frequency[s->inlen[i]]++;
        for(size_t j = 0; j < indexcnt; j++)
            frequency[vfasttables_index_get(s->instr[i], indices[j], s->inlen[i])]++;
    }

    return frequency;
}

// finds characters not shared by str0index and str1index
static size_t vfasttables_associated_values__unsharedchars(const size_t str0index, const size_t str1index, size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s, char* const restrict unsharedchars){
    size_t unsharedcharcnt = 0;

    if(s->inlen[str0index] != s->inlen[str1index]){
        unsharedchars[unsharedcharcnt++] = s->inlen[str0index];
//        unsharedchars[unsharedcharcnt++] = s->inlen[str1index];         // somehow performs WORSE with this on!
    }

    for(size_t i = 0; i < indexcnt; i++){
        const char c0 = vfasttables_index_get(s->instr[str0index], indices[i], s->inlen[str0index]);
        for(size_t j = i; j < indexcnt; j++){
            const char c1 = vfasttables_index_get(s->instr[str1index], indices[j], s->inlen[str1index]);

            if(c0 != c1)
                unsharedchars[unsharedcharcnt++] = c0;
        }
    }
// 0 only if duplicate entries are present.
    return unsharedcharcnt;
}

// increments associated values field based on which one has the lowest frequency.
static void vfasttables_associated_values__increment_based_on_lowest_frequency(
    const uint32_t hash,
    size_t* const restrict table,
    uint32_t* const restrict associated_values, unsigned int* const restrict frequency,
    char* const restrict unsharedchars, const size_t sizeunsharedchars,
    const size_t cardinality, const uint32_t increment, const uint32_t mask
){
    size_t lowfreq = unsharedchars[0];

    for(size_t i = 1; i < sizeunsharedchars; i++){
        if(frequency[lowfreq] > frequency[unsharedchars[i]])
            lowfreq = unsharedchars[i];
    }
// datum affects exactly one entry (that being ours); we can increment associated_values by how much it'd take to go to a free entry.
    if(frequency[lowfreq] == 1){
    // by how much do we have to increment the associated value so that the sum == closest_free?
    // (x0 + ... + xn) + w % mask == closest_free (mod cardinality).
    // w == closest_free - hash.
        size_t closest_free = 0;
        for(; closest_free < cardinality; closest_free++)
            if(table[closest_free] == -1) break;

        associated_values[lowfreq] = associated_values[lowfreq] + (closest_free - hash & mask) & mask;
    }
    else{
        associated_values[lowfreq] = associated_values[lowfreq] + increment & mask;
    }
}

static void vfasttables_associated_values__random_permute(struct vfasttables_parseinput_s* const restrict s){
    for(size_t i = 0; i < s->incnt; i++){
        size_t idx0; getrandom(&idx0, sizeof(idx0), 0); idx0 %= s->incnt;
        size_t idx1; getrandom(&idx1, sizeof(idx1), 0); idx1 %= s->incnt;

        char* const t_str = s->instr[idx0];
        char* const t_tok = s->intok[idx0];
        const size_t t_len = s->inlen[idx0];

        s->instr[idx0] = s->instr[idx1]; s->instr[idx1] = t_str;
        s->intok[idx0] = s->intok[idx1]; s->intok[idx1] = t_tok;
        s->inlen[idx0] = s->inlen[idx1]; s->inlen[idx1] = t_len;
    }
}

// computes associated values for only a subset of the input, that being the first n entries.
static bool vfasttables_associated_values__n(
    const size_t toprocess,
    size_t* const restrict indices, const size_t indexcnt,
    uint32_t* const restrict associated_values, size_t* const restrict table, unsigned int* const restrict frequency,
    struct vfasttables_parseinput_s* const restrict s
){
    char* const restrict unsharedchars = alloca(2*(indexcnt + 1));      // maximum number of differing characters in a conflict.

    // has the threat of running endlessly, so we stop it at some ''reasonable'' point.
    const size_t sanityctr__max = toprocess < 30? 65536: 8192;
    for(size_t INTERNAL__sanityctr = 0; INTERNAL__sanityctr < sanityctr__max*toprocess; INTERNAL__sanityctr++){
        for(size_t i = 0; i < toprocess; i++){
            const uint32_t hash = vfasttables_hash(s->instr[i], s->inlen[i], s->cardinality, associated_values, indices, indexcnt, s->mask);

            if(table[hash] != -1 && table[hash] != i){      // if this condition succeeds, hash function is NOT perfect yet.
                const size_t unsharedcharcnt = vfasttables_associated_values__unsharedchars(i, table[hash], indices, indexcnt, s, unsharedchars);

                vfasttables_associated_values__increment_based_on_lowest_frequency(hash, table, associated_values, frequency, unsharedchars, unsharedcharcnt, s->cardinality, s->increment, s->mask);
                memset(table, -1, s->cardinality*sizeof(*table));
                break;                  // repeats whole loop.
            }
            else table[hash] = i;       // string that made it this way.

            if(i == toprocess-1)        // success! no hashes conflict; we have a perfect minimal hash function.
                return true;
        }
    }
    return false;
}

// finds associated values.
static uint32_t* vfasttables_associated_values(size_t* const restrict indices, const size_t indexcnt, struct vfasttables_parseinput_s* const restrict s){
    uint32_t* const restrict associated_values = malloc(vfasttables_supportedchars*sizeof(*associated_values));
    getrandom(associated_values, vfasttables_supportedchars*sizeof(*associated_values), 0);

    for(size_t i = 0; i < vfasttables_supportedchars; i++) associated_values[i] &= s->mask;
// table of where each entry places.
    size_t* const restrict table = malloc(s->cardinality*sizeof(*table));
    memset(table, -1, s->cardinality*sizeof(*table));
// how often each character is present.
    unsigned int* const restrict frequency = vfasttables_associated_values__frequency(indices, indexcnt, s);
    vfasttables_associated_values__random_permute(s);     // found to help.

    for(size_t i = 1; i < s->incnt; i++){
        if(!vfasttables_associated_values__n(i, indices, indexcnt, associated_values, table, frequency, s)){
            fprintf(stderr, "ERROR: could not compute associated values array at entry %lu due to string %s.\n", i, s->instr[i]);
            break;
        }
        else if(i == s->incnt - 1){                       // full assignment of all values to perfect hash has been found.
            free(frequency);
            free(table);
            return associated_values;
        }
    }

    free(frequency);
    free(table);
    free(associated_values);
    return NULL;
}
