//What it makes depends on WIDE (defined by Wide.cpp before it includes it), on INNER_ADD and on whether Inner.h was
//included before it.
#ifndef CACHE_H
#define CACHE_H
#include "Inner.h"

#ifdef WIDE
#define CACHE_SCALE	3
#else
#define CACHE_SCALE	2
#endif


int Scaled(int x);


inline int CacheScale(void)
{
	return CACHE_SCALE * INNER_BASE;
}


#endif // CACHE_H
