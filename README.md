# Find Memory Leaks

LD_PRELOAD hack to save malloc and free operations in an sqlite database.

Based on something I wrote a long time ago using __malloc_hook / __free_hook in glibc. As those are now marked as 'deprecated' I rewrote using some suggestions found in: 

https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc

Build with:
```
gcc -Wall -o fml.so fml.c -shared -fPIC -ldl -lsqlite3
```
Load as:
```
LD_PRELOAD=./fml.so ./test
```
The malloc() and free() operations are recorded in ./malloc.db.

This is a toy! Use valgrind or something for real use cases! Highly unlikely to work well with threads! :) 
