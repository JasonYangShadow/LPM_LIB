#!/bin/bash

gcc -c -Wall -Werror -fpic lpm.c 
gcc -shared -o liblpm.o lpm.o -ldl
gcc -shared -o liblpm.o lpm.o -ldl
LD_PRELOAD=./liblpm.o /usr/bin/touch file
LD_PRELOAD=./liblpm.o /bin/rm file
LD_PRELOAD=./liblpm.o /bin/mkdir a 
LD_PRELOAD=./liblpm.o /bin/rmdir a 

