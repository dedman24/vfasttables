
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "../ctx/ctx.h"

void vfasttables_print_version(){
    puts(
        "vfasttables " VFASTTABLES_VERSION_STRING ".\n"
        "Copyright (C) 2026 0xded.\n"
        "This is free software; see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
    );
}

void vfasttables_print_help(char* const restrict command){
    fprintf(stdout,
        "vfasttables version " VFASTTABLES_VERSION_STRING ".\n"
        "Usage: %s [options] source > destination\n"
        "Options:\n"
        "   --help                  prints out handy little guide to the program.\n"
        "   --version               prints out vfasttables version.\n"
        "   -p pfix                 specifies final prefix, by default 'vfasttables' if omitted.\n"
        "   --Prefix pfix           longer version of -p.\n"
        "   -e enumname             specifies final enum name, by default 'pfix'_enum if omitted.\n"
        "   --Enum enumname         longer version of -e.\n"
        "   --cardinality num       specifies how large the set of values hash function maps to should be, by default however many elements to process if omitted.\n"
        "   -j num                  specifies what the increment of associated_values should be, by default 1 if omitted.\n"
        "   --prefixEnum            I forgot.\n\n"
        "for bug reporting, contact me on my email or report an issue on the github repo.",
        command
    );
}

bool vfasttables_check_arg(char* const restrict command, char* const restrict arg){
    if(strcmp(arg, "--help") == 0){
        vfasttables_print_help(command);
        return true;
    }

    if(strcmp(arg, "--version") == 0){
        vfasttables_print_version();
        return true;
    }

    return false;
}
