#include "help.h"
#include "../ctx/ctx.h"

// stdlib includes.
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

// parses command-line input.
// input:
//  argc, argv.
// output:
//  vfasttables ctx that holds all relevant info:
//      src file
//      dst file
//      any other options
static vfasttables_ctx* vfasttables_parseargs(char* argv[], const int argc){
    if(argc == 1){
        puts("ERROR: no input files.");
        return NULL;
    }

    if(vfasttables_check_arg(argv[1]))
        return NULL;

    vfasttables_options_e opts = VFASTTABLES_OPT_NONE;

    char* const srcPath = argv[argc-1];
    char* dstPath = NULL;
    char* pfix = NULL;
    char* enumname = NULL;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "--prefixEnum") == 0) opts |= VFASTTABLES_OPT_PREFIX_ENUM;

        else if(strcmp(argv[i], "-o") == 0){
            if(argc - 2 < i){
                puts("ERROR: no input files.");
                return NULL;
            }
            else dstPath = argv[++i];
        }

        else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--Prefix") == 0){
            if(argc - 2 < i){
                puts("ERROR: prefix specified but none present.");
                return NULL;
            }
            else pfix = argv[++i];
        }

        else if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--Enum") == 0){
            if(argc - 2 < i){
                puts("ERROR: enum name specified but none present.");
                return NULL;
            }
            else enumname = argv[++i];
        }
    }

    if(dstPath == NULL)
        dstPath = "out.c";

    vfasttables_ctx* const restrict ctx = vfasttables_ctx_init(NULL, srcPath, dstPath, pfix, enumname, opts);
    return ctx;
}

// expects input to be correctly formatted
static struct vfasttables_parseinput_s vfasttables_parseinput(vfasttables_ctx* ctx){
    char**  instr = NULL;
    char**  intok = NULL;
    size_t* inlen = NULL;
    size_t  incnt = 0;

    while(!feof(ctx->src)){
        const size_t inpos = incnt++;
        instr = realloc(instr, incnt*sizeof(*instr));
        intok = realloc(intok, incnt*sizeof(*intok));
        inlen = realloc(inlen, incnt*sizeof(*inlen));

        int err = fscanf(ctx->src, "%lu", &inlen[inpos]);

        if(err < 0){                // why does it error here? I hate anything file manipulation related in C.
            incnt--;
            break;
        }

        fseek(ctx->src, 1, SEEK_CUR);

        instr[inpos] = malloc(inlen[inpos] + 1);
        fread(instr[inpos], sizeof(char), inlen[inpos], ctx->src);
        instr[inpos][inlen[inpos]] = '\0';
        fseek(ctx->src, 1, SEEK_CUR);

        size_t toklen;
        fscanf(ctx->src, "%lu", &toklen);
        fseek(ctx->src, 1, SEEK_CUR);

        intok[inpos] = malloc(toklen+1);
        fread(intok[inpos], sizeof(char), toklen, ctx->src);
        intok[inpos][toklen] = '\0';
        fseek(ctx->src, 1, SEEK_CUR);
#ifdef VFASTTABLES_DEBUG
        printf("%lu %s %lu %s\n", inlen[inpos], instr[inpos], toklen, intok[inpos]);
#endif
    }

    fclose(ctx->src);
    ctx->src = NULL;
// like functional languages? get ready for a whole lot of this!
// tbf it doesn't even look that ugly & it's perfectly readable.
// perhaps the 'default' or 'conditionless' value could be specified with a noop 'default' keyword.
    const uint32_t mask =
        incnt <= 256?   255:
        incnt <= 65536? 65535:
                        UINT32_MAX;

    return (struct vfasttables_parseinput_s){instr, intok, inlen, incnt, mask};
}
