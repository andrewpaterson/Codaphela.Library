#ifndef __MERSENNE_TWISTER_H__
#define __MERSENNE_TWISTER_H__
#include "PrimitiveTypes.h"


#define MERSENNE_TWISTER_STATE_SIZE		624
#define MERSENNE_TWISTER_DEFAULT_SEED	5489


//MT19937 32 bit Mersenne Twister.  Unlike CRandom (std::mt19937 with std::uniform_int_distribution)
//the mapping onto a range is not implementation defined so a seed gives the same sequence on all platforms.
class CMersenneTwister
{
private:
	uint32	mauiState[MERSENNE_TWISTER_STATE_SIZE];
	uint32	muiIndex;

	void	Twist(void);

public:
	void	Init(void);
	void	Init(uint iSeed);
	void	Kill(void);

	uint32	Next(void);
	int		Next(int iMin, int iMaxInclusive);
};


#endif // __MERSENNE_TWISTER_H__

