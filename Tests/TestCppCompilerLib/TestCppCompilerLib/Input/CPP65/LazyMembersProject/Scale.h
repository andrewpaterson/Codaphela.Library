#ifndef __SCALE_H__
#define __SCALE_H__


#define SCALE_NAME		Double
#define SCALE_FACTOR	2
#include "Scale.inl"
#undef SCALE_NAME
#undef SCALE_FACTOR

#define SCALE_NAME		Triple
#define SCALE_FACTOR	3
#include "Scale.inl"
#undef SCALE_NAME
#undef SCALE_FACTOR


//A string made by stringifying: the class starts in the header but has a token no file spelled.
#define SCALE_NAMED(c)	const char* Name(void) { return #c; }
class CFour
{
public:
	SCALE_NAMED(CFour)
	int		Value(void) { return 4; }
};


#endif // __SCALE_H__
