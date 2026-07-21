
#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "../ctx/ctx.h"

// finds the maximum number of indices, after which one stops (might be wiser to stop earlier though, if maxIndexCnt is larger than 6 or smth).
static size_t vfasttables_indices__indexcntMAX(struct vfasttables_parseinput_s* s){
    size_t indexcntMAX = 0;
    for(size_t i = 0; i < s->incnt; i++){
        indexcntMAX = s->inlen[i] > indexcntMAX? s->inlen[i]: indexcntMAX;
    }
//    return indexcntMAX;
    return indexcntMAX > 4? 4: indexcntMAX;     // more than 4 indices is impractical.
}

// formula is 128**indexcnt.
static size_t vfasttables_indices__bitmapsize(const size_t indexcnt){
    size_t accumulator = 1;
    for(size_t i = 0; i < indexcnt; i++)
        accumulator *= vfasttables_supportedchars;
    return (accumulator/8) + (accumulator%8 != 0);
}

static bool vfasttables_indices__put(uint8_t* restrict bitmap, const size_t index){
    const size_t byteindex = index/8, bitindex = index%8;

    const bool prev = bitmap[byteindex] >> bitindex & 1;
    if(prev) return true;

    bitmap[byteindex] |= 1 << bitindex;
    return false;
}

static size_t vfasttables_indices__properindex(char* const restrict instr, const size_t inlen, size_t* const restrict indices, const size_t indexcnt){
    size_t accum = inlen;
    for(size_t i = 1; i < indexcnt; i++)
        accum = accum*vfasttables_supportedchars + vfasttables_index_get(instr, indices[i-1], inlen);

    return accum;
}

static bool vfasttables_indices__increaseindexcnt(size_t* const restrict indexcnt, size_t* restrict* const restrict indices, size_t* const restrict bitmapsize, uint8_t* restrict* const restrict bitmap, const size_t maxstrlen, const size_t indexcntMAX){
    for(size_t i = 0; i < *indexcnt - 1; i++){
        (*indices)[i] += 1;
        if((*indices)[i] < maxstrlen){
            memset(*bitmap, 0, *bitmapsize*sizeof(**bitmap));
            return false;
        }
        (*indices)[i] %= maxstrlen;
    }

    if(*indexcnt == indexcntMAX) return true;

    *indexcnt += 1;
    *indices = realloc(*indices, (*indexcnt-1)*sizeof(*indices));
    *bitmapsize = vfasttables_indices__bitmapsize(*indexcnt);
    *bitmap = realloc(*bitmap, *bitmapsize*sizeof(**bitmap));
    memset(*indices, 0, (*indexcnt-1)*sizeof(**indices));
    memset(*bitmap,  0, *bitmapsize*sizeof(**bitmap));
    return false;
}

static size_t vfasttables_indices__maxstrlen(struct vfasttables_parseinput_s* s){
    size_t max = 0;

    for(size_t i = 0; i < s->incnt; i++)
        if(max < s->inlen[i]) max = s->inlen[i];

    return max;
}

// obtains the minimum
static size_t* vfasttables_indices(struct vfasttables_parseinput_s* s, size_t* const restrict p_indexcnt){
    const size_t maxstrlen = vfasttables_indices__maxstrlen(s);

// starts at 1, progressively increases.
// number of indices taken from string is indexcnt-1; this is because the first index is always the length,
// for ease of simplicity length may range from 0 to 127, as if it were an extra character..
    size_t indexcnt = 1;
    size_t* restrict indices = calloc(indexcnt-1, sizeof(*indices));
    const size_t indexcntMAX = vfasttables_indices__indexcntMAX(s);

    size_t bitmapsize = vfasttables_indices__bitmapsize(indexcnt);
    uint8_t* restrict bitmap = calloc(bitmapsize, sizeof(*bitmap));
    while(1){
        // checks
        for(size_t i = 0; i < s->incnt; i++){
            const size_t properindex = vfasttables_indices__properindex(s->instr[i], s->inlen[i], indices, indexcnt);

            if(vfasttables_indices__put(bitmap, properindex)){
                if(vfasttables_indices__increaseindexcnt(&indexcnt, &indices, &bitmapsize, &bitmap, maxstrlen, indexcntMAX)){
                    free(bitmap);
                    free(indices);
                    return NULL;
                }
                break;
            }
        // condition to exit the loop successfully: all strings do not conflict.
            if(i == s->incnt-1){
                free(bitmap);
                *p_indexcnt = indexcnt;
                return indices;
            }
        }
    }
}
