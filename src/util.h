#ifndef __UTIL_H__
#define __UTIL_H__

#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define TRACE printf
#else
#define TRACE noop
#endif

#include <stdio.h>

void fail(char *message, ...);
void *nicemalloc(size_t size);
void *nicerealloc(void *org, size_t size);
void noop(char *s, ...);
void millisleep(int milli);
unsigned long millitime();

#endif
