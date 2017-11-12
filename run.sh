#!/bin/bash

export LPM_OUTPUT_FILE=/tmp/output
gcc -c -Wall -Werror -fpic lpm.c 
gcc -shared -o liblpm.o lpm.o -ldl
LD_PRELOAD=./liblpm.o /usr/bin/touch file
LD_PRELOAD=./liblpm.o /bin/ln -s file file_ln
LD_PRELOAD=./liblpm.o /bin/mv file file.log
LD_PRELOAD=./liblpm.o /bin/rm file_ln
LD_PRELOAD=./liblpm.o /bin/rm file.log
LD_PRELOAD=./liblpm.o /bin/mkdir a 
LD_PRELOAD=./liblpm.o /bin/rmdir a 

