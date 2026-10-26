#ifndef __STD_RANDOM_H__
#define __STD_RANDOM_H__
#include "PrimitiveTypes.h"
#include "MersenneTwister.h"


class CRandom
{
private:
	CMersenneTwister	mcRNG;

public:
	void	Init(void);
	void	Init(uint32 iSeed);
	void	Kill(void);
		
	int32	Next(int32 iMin, int32 iMaxInclusive);
};


#endif // __STD_RANDOM_H__

