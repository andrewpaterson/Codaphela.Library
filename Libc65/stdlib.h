//Memory allocation and general functions.
#ifndef __LIBC65_STDLIB_H__
#define __LIBC65_STDLIB_H__
#include <stddef.h>


#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#define RAND_MAX		0x7fff


extern "C"
{
void*	malloc(size_t uiBytes);
void*	calloc(size_t uiCount, size_t uiBytes);
void*	realloc(void* pv, size_t uiBytes);
void	free(void* pv);

int		abs(int i);
long	labs(long i);
int		atoi(const char* sz);
long	atol(const char* sz);
long	strtol(const char* sz, char** pszEnd, int iBase);
unsigned long	strtoul(const char* sz, char** pszEnd, int iBase);

int		rand(void);
void	srand(unsigned int uiSeed);
void	qsort(void* pvBase, size_t uiCount, size_t uiBytes, int (*fCompare)(const void*, const void*));

void	abort(void);
void	exit(int iStatus);
void	_Exit(int iStatus);
}


#endif // __LIBC65_STDLIB_H__
