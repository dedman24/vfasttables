#include "../ctx/ctx.h"

// stdlib includes.
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

void vfasttables_print_version(){
    puts(
        "vfasttables " VFASTTABLES_VERSION_STRING ".\n"
        "Copyright (C) 2026 0xded.\n"
        "This is free software; see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
    );
}

void vfasttables_print_help(){
    puts(
        "vfasttables version " VFASTTABLES_VERSION_STRING ".\n"
        "Usage: vfasttables [options] file\n"
        "Options:\n"
        "   --help                  prints out handy little guide to the program.\n"
        "   --version               prints out vfasttables version.\n"
        "   -o file                 specifies output file.\n"
        "   -p pfix                 specifies final prefix, by default 'vfasttables' if omitted.\n"
        "   --Prefix pfix           longer version of -p.\n"
        "   -e enumname             specifies final enum name, by default 'pfix'_enum if omitted.\n"
        "   --Enum enumname         longer version of -e.\n"
        "   --prefixEnum            I forgot.\n\n"
        "for bug reporting, contact me on my email or report an issue on the github repo."
    );
}

bool vfasttables_check_arg(char* arg){
    if(strcmp(arg, "--help") == 0){
        vfasttables_print_help();
        return true;
    }

    if(strcmp(arg, "--version") == 0){
        vfasttables_print_version();
        return true;
    }

    return false;
}
