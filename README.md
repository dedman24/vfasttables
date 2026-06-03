# vfasttables
'vfasttables' is a gperf-like command-line utility for when one's trying to implement a programming language tokeniser that allows one to generate very fast hashtables in a reasonable amount of time.\
it is based on the same core algorithm as gperf, but it is smaller, written in C99 and with a more permissive licence.\
the source code itself is also much smaller.

## compiling
`gcc -o vfast -O3 vfasttables.c`

## usage
`./vfast src`\
type `./vfast --help` for more information.
