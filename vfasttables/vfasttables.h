#ifndef vfasttables_included
#define vfasttables_included

#include "stdbool.h"
#include "stdio.h"

#include "computation/indices.h"
#include "computation/associated_values.h"

#include "output/writecode.h"

#include "parse/parse.h"

// 'vfasttables' is a tool to be used to generate hashtables quickly, using the same algorithm that 'gperf' does.
// the end result is quicker than gperf's output, as vfasttables is specifically designed for the VERB programming language's tokeniser.
// it happens that vfasttables only needs 56 bytes for its hash function's LUT of associated values, at a cost of not supporting many different characters & certain strings.
// vfasttables is also meant to work with a low number of input strings.
// vfasttables is entirely C99-compliant (the best C standard, if it wasn't for its lack of atomic operations).

// ALGORITHM DESCRIPTION:
//  read input data.
//  find minimum number of indices that uniquely identify each string (starting from 1 to the length of the longest string).
//  generate associated_values array.
//  print C code.

// generates a hashtable with a given input argument string.
void vfasttable(char* argv[], const int argc, const bool printInfo){
    vfasttables_ctx* const restrict ctx = vfasttables_parseargs(argv, argc);
    if(!ctx) return;
    struct vfasttables_parseinput_s s = vfasttables_parseinput(ctx);

    size_t indexcnt;
    size_t* indices = vfasttables_indices(&s, &indexcnt);
    indexcnt--;                                                 // done because vfasttables_indices considers 'size' to be an index while the rest of the program doesn't.
#ifdef VFASTTABLES_DEBUG
    printf("indexcnt %lu\n", indexcnt);                         // when summer ends & you still haven't had your 12 episode romcom anime IRL...
    for(size_t i = 0; i < indexcnt; i++)
        printf("indices[%lu] == %lu\n", i, indices[i]);
#endif

    if(!indices){
        puts("no indices.");
        vfasttables_parseinput_s_destroy(&s);
        vfasttables_ctx_destroy(ctx, true);
        return;
    }
    uint32_t* associated_values = vfasttables_associated_values(indices, indexcnt, &s);
    if(!associated_values){
        vfasttables_parseinput_s_destroy(&s);
        free(indices);
        vfasttables_ctx_destroy(ctx, true);
        return;
    }

    vfasttables_out_print(ctx->dst, ctx->pfix, associated_values, &s, ctx, indices, indexcnt);

    vfasttables_parseinput_s_destroy(&s);

    free(indices);
    free(associated_values);
    vfasttables_ctx_destroy(ctx, true);
}

#endif
