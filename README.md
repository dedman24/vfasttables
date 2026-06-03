# vfasttables
'vfasttables' is a gperf-like command-line utility for when one's trying to implement a programming language tokeniser that allows one to generate very fast hashtables in a reasonable amount of time.\
it is based on the same core algorithm as gperf, but it is smaller, written in C99 and with a more permissive licence.\
the source code itself is also much smaller.

## compiling
`gcc -o vfast -O3 vfasttables.c`

## usage
`./vfast src`\
type `./vfast --help` for more information.
### formatting of src file
the file specified in src is formatted as:\
`string-len string tok-len tok `\
where `string-len` is the length of the string (number), `string` the string to be hashed, `tok-len` the length of the token, `tok` the name of the token within the final C code itself.\
\
for example, if we wanted to have the strings `hello`, `hi` & `what's up` hash into the tokens `GREET0`, `GREET1`, `GREET2`, one would write:\
`5 hello 6 GREET0`\
`2 hi 6 GREET1`\
`9 what's up 6 GREET2`

## inner workings
 - parse the input file.
 - find the lowest number of indices so that, for each string, the characters of said string at those indices uniquely identify each string.
 - find the _associated values_ so that the hash function computes a different output for each input.
 - write the result out.
