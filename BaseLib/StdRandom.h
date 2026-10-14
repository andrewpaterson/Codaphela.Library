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
	void	Init(uint iSeed);
	void	Kill(void);
		
	int		Next(int iMin, int iMaxInclusive);
};


#endif // __STD_RANDOM_H__

