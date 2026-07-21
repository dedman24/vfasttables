#ifndef vfasttables_context_included
#define vfasttables_context_included

#include "stdio.h"
#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"

// definitions.
#define vfasttables_supportedchars 128      // all 128 ascii characters are supported (even those from 00h to 1Fh)
#define VFASTTABLES_VERSION_STRING "1.0.0"

typedef enum{                               // add ctx options here.
    VFASTTABLES_OPT_NONE,
    VFASTTABLES_OPT_PREFIX_ENUM        = 1, // specifies that enum should be prefixed.
} vfasttables_options_e;

typedef struct{
    FILE* restrict src;                     // src file.
    FILE* restrict dst;                     // dst file.
    char* restrict pfix;                    // prefix to add to all functions, 'vfasttables' as default prefix.
    char* restrict enumname;                // explicit name of enum _hash should return.
    vfasttables_options_e options;
} vfasttables_ctx;

vfasttables_ctx* vfasttables_ctx_init(vfasttables_ctx* restrict ctx, char* const restrict srcPath, char* const restrict dstPath, char* restrict pfix, char* restrict enumname, const vfasttables_options_e options){
    if(!ctx){
        ctx = malloc(sizeof(*ctx));
        if(!ctx) return NULL;
    }

    ctx->src = fopen(srcPath, "r");
    if(!ctx->src){ free(ctx); return NULL; }

    ctx->dst = fopen(dstPath, "w");
    if(!ctx->dst){ free(ctx); return NULL; }
// default prefix.
    if(!pfix) pfix = "vfasttables";
    ctx->pfix = pfix;
// default enum name.
    if(!enumname){
        enumname = malloc(strlen(ctx->pfix) + strlen("_enum") + 1);
        sprintf(enumname, "%s_enum", ctx->pfix);
    }
    else{
        char* restrict enumnamecopy = malloc(strlen(enumname));
        memcpy(enumnamecopy, enumname, strlen(enumname) + 1);
        enumname = enumnamecopy;
    }
    ctx->enumname = enumname;

    ctx->options = options;

    return ctx;
}

void vfasttables_ctx_destroy(vfasttables_ctx* const restrict ctx, const bool freeIt){
    if(ctx->src) fclose(ctx->src);
    if(ctx->dst) fclose(ctx->dst);
    free(ctx->enumname);
    if(freeIt) free(ctx);
}

struct vfasttables_parseinput_s{
    char**  instr;
    char**  intok;
    size_t* inlen;
    size_t  incnt;
    uint32_t mask;
};

void vfasttables_parseinput_s_destroy(struct vfasttables_parseinput_s* s){
    for(size_t i = 0; i < s->incnt; i++){
        free(s->instr[i]);
        free(s->intok[i]);
    }
    free(s->instr);
    free(s->intok);
    free(s->inlen);
}

static size_t vfasttables_index_get(char* const restrict str, const size_t index, const size_t slen){
    return str[index % slen];
}

#endif
