#ifndef __STD_RANDOM_H__
#define __STD_RANDOM_H__
#include <random>
#include "PrimitiveTypes.h"


class CRandom
{
private:
	std::mt19937	mcRNG;

public:
	void	Init(void);
	void	Init(uint iSeed);
	void	Kill(void);
		
	int		Next(int iMin, int iMaxInclusive);
};


#endif // __STD_RANDOM_H__

