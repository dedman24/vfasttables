
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

    printf("%lu %lu\n", byteindex, bitindex);

    const bool prev = bitmap[byteindex] >> bitindex & 1;
    if(prev) return true;

    bitmap[byteindex] |= 1 << bitindex;
    return false;
}

static size_t vfasttables_indices__properindex(char* const restrict instr, const size_t inlen, size_t* const restrict indices, const size_t indexcnt){
    size_t accum = 0;
    for(size_t i = 0; i < indexcnt; i++)
        accum = accum*vfasttables_supportedchars + vfasttables_index_get(instr, indices[i], inlen);

    return accum;
}

static void vfasttables_indices__increaseindexcnt(size_t* const restrict indexcnt, size_t* restrict* const restrict indices, size_t* const restrict bitmapsize, uint8_t* restrict* const restrict bitmap){
    *indexcnt += 1;
    *indices = realloc(*indices, *indexcnt*sizeof(*indices));
    *bitmapsize = vfasttables_indices__bitmapsize(*indexcnt);
    *bitmap = realloc(*bitmap, *bitmapsize*sizeof(*bitmap));
    memset(*indices, 0, *indexcnt);
    memset(*bitmap,  0, *bitmapsize);
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
    size_t indexcnt = 1;
    size_t* restrict indices = calloc(indexcnt, sizeof(*indices));
    const size_t indexcntMAX = vfasttables_indices__indexcntMAX(s);

    size_t bitmapsize = vfasttables_indices__bitmapsize(indexcnt);
    uint8_t* restrict bitmap = calloc(bitmapsize, sizeof(*bitmap));
    while(1){
        // checks 
        for(size_t i = 0; i < s->incnt; i++){
            const size_t properindex = vfasttables_indices__properindex(s->instr[i], s->inlen[i], indices, indexcnt);  
            if(vfasttables_indices__put(bitmap, properindex)){
                if(indexcnt == indexcntMAX){
                    free(bitmap);
                    free(indices);
                    return NULL;
                }

                vfasttables_indices__increaseindexcnt(&indexcnt, &indices, &bitmapsize, &bitmap);
                break;
            }
        // condition to exit the loop successfully: all strings do not conflict.
            if(i == s->incnt-1){
                free(bitmap);
                *p_indexcnt = indexcnt;
                return indices;
            } 
        }
        
    // it cannot; it increments all indices in a cascadeful manner.
        size_t carry = 0;
        for(size_t i = 0; i < indexcnt; i++){
            indices[i] += 1;
            if(indices[i] < maxstrlen) break;
            indices[i] %= maxstrlen;
        }
    }
}
