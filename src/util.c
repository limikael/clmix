#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

/**
 * Exit with an error.
 */
void fail(char *format, ...) {
	va_list args;

	va_start(args,format);
	vprintf (format,args);
	va_end(args);	

	printf("\n");
	exit(1);
}

/**
 * Allocate memory, exit on fail.
 */
void *nicemalloc(size_t size) {
	void *p=malloc(size);

	if (!p)
		fail("Out of memory");

	return p;
}

/**
 * Reallocate memory, exit on fail.
 */
void *nicerealloc(void *org, size_t size) {
	void *p;

	if (org)
		p=realloc(org,size);

	else
		p=malloc(size);

	if (!p)
		fail("Out of memory");

	return p;
}

/**
 * Do nothing.
 */
void noop(char *s, ...) {
}

/**
 * Sleep for a number of milliseconds.
 */
void millisleep(int milli) {
	if (milli<0)
		return;

	usleep(milli*1000);
}

/**
 * Get time in millisecs.
 */
unsigned long millitime() {
	struct timeval timeval;
	struct timezone timezone;
	unsigned long res=0;

	gettimeofday(&timeval,&timezone);
	res+=timeval.tv_sec*1000;
	res+=timeval.tv_usec/1000;

	return res;
}
