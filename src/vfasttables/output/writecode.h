
#include "stdio.h"
#include "stdint.h"
#include "../ctx/ctx.h"
#include "../computation/hash.h"

static void vfasttables_out_printenum(FILE* out, char* const restrict enumname, const bool prefixenum, char* const restrict pfix, uint32_t* const restrict associated_values, struct vfasttables_parseinput_s* s, size_t* const restrict indices, const size_t indexcnt){
    fputs(
        "#include \"string.h\"\n"
        "#include \"stddef.h\"\n"
        "#include \"stdint.h\"\n\n"
        "typedef enum{\n",
        out
    );

    for(size_t i = 0; i < s->incnt; i++){
        const uint32_t hash = vfasttables_hash(s->instr[i], s->inlen[i], s->incnt, associated_values, indices, indexcnt, s->mask);
        if(prefixenum)
            fprintf(out, 
                "    %s_%s = %u,\n",
                pfix, 
                s->intok[i], 
                hash
            );
        else
            fprintf(out, 
                "    %s = %u,\n", 
                s->intok[i], hash
            );
    }

    fprintf(out, "    %s_NONE = %u\n", pfix, s->incnt);

    fprintf(out, "} %s;\n", enumname);
}

void vfasttables_out_printhash(FILE* out, uint32_t* const restrict associated_values, const uint32_t incnt, char* const restrict enumname, char* const restrict associated_type, char* const restrict pfix, size_t* const restrict indices, const size_t indexcnt){
    fprintf(out, 
        "// hash function, generated with vfasttables version " VFASTTABLES_VERSION_STRING ".\n"
        "%s %s_hash(char* str, const size_t len){\n"
        "// associated values generated through hash function.\n"
        "    const %s associated_values[] = {\n", 
        enumname, pfix, associated_type
    );

    for(int i = 0; i < vfasttables_supportedchars/16; i++){
        fputs("        ", out);
        for(int j = 0; j < 16; j++){
            fprintf(out, "%u", associated_values[i*16 + j]);
            if(i != (vfasttables_supportedchars/16) - 1 || j != 15) fputc(',', out);
        }
        fputs("\n", out);
    }

    fputs(         
        "    };\n"
        "    const size_t indices[] = {\n",
        out
    );

    for(int i = 0; i < indexcnt; i++){
        fputs("        ", out);
        if(i == indexcnt-1) fprintf(out, "%lu ", indices[i]);
        else fprintf(out, "%lu, ", indices[i]);
    }

    fprintf(out, 
        "\n"
        "    };\n"
        "    %s hash = len;\n"
        "    for(size_t i = 0; i < sizeof(indices)/sizeof(*indices); i++){\n"
        "        hash += str[associated_values[indices[i]] %% len];\n"
        "    }\n"
        "    hash %%= %d;\n"
        "    return hash;\n"
        "}\n\n", 
        associated_type, incnt
    );
}

void vfasttables_out_printtable(FILE* out, struct vfasttables_parseinput_s* s){
    fputs("    char* table[] = {\n", out);

    for(size_t i = 0; i < s->incnt; i++){
        fputs("        \"", out);
        fputs(s->instr[i], out);
        
        if(i == s->incnt - 1) fputs("\"\n    };\n", out);
        else fputs("\",\n", out);
    }
}

void vfasttables_out_printsearch(FILE* out, char* const restrict pfix, char* const restrict enumname, struct vfasttables_parseinput_s* s){
// we want this function to return a condition code ('no such token exists') or the token itself.
// we can do this by having a special token value greater than all standard valid token values the hash function can physically return.
    fprintf(out,
        "%s %s_search(char* const restrict str, const size_t len){\n"
        "    const %s index = %s_hash(str, len);\n",
        enumname, pfix, enumname, pfix
    );

    vfasttables_out_printtable(out, s);

    fprintf(out,
        "    return strncmp(table[index], str, len) == 0? index: %s_NONE;\n"
        "}\n",
        pfix
    );
}

void vfasttables_out_print(FILE* out, char* const restrict pfix, uint32_t* const restrict associated_values, struct vfasttables_parseinput_s* s, vfasttables_ctx* ctx, size_t* const restrict indices, const size_t indexcnt){
    char* const restrict associated_type = 
        s->incnt <= 256?   "uint8_t" :
        s->incnt <= 65536? "uint16_t":
                           "uint32_t";

    vfasttables_out_printenum(out, ctx->enumname, (ctx->options & VFASTTABLES_OPT_PREFIX_ENUM) != 0, ctx->pfix, associated_values, s, indices, indexcnt);
    vfasttables_out_printhash(out, associated_values, s->incnt, ctx->enumname, associated_type, pfix, indices, indexcnt);
    vfasttables_out_printsearch(out, pfix, ctx->enumname, s);
}

