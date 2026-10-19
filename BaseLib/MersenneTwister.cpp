#include <time.h>
#include "MersenneTwister.h"


#define MERSENNE_TWISTER_SHIFT_SIZE		397
#define MERSENNE_TWISTER_MATRIX_A		0x9908b0df
#define MERSENNE_TWISTER_UPPER_MASK		0x80000000
#define MERSENNE_TWISTER_LOWER_MASK		0x7fffffff
#define MERSENNE_TWISTER_INIT_MULTIPLIER	1812433253


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMersenneTwister::Init(void)
{
	uint32	uiSeed;

	//Not cryptographic.  Mixes the wall clock, processor time and the stack address.
	uiSeed = (uint32)time(NULL);
	uiSeed ^= (uint32)clock() * MERSENNE_TWISTER_INIT_MULTIPLIER;
	uiSeed ^= (uint32)(ptr)(&uiSeed);
	Init(uiSeed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMersenneTwister::Init(uint32 iSeed)
{
	uint32	i;
	uint32	uiPrevious;

	mauiState[0] = iSeed;
	for (i = 1; i < MERSENNE_TWISTER_STATE_SIZE; i++)
	{
		uiPrevious = mauiState[i - 1];
		mauiState[i] = MERSENNE_TWISTER_INIT_MULTIPLIER * (uiPrevious ^ (uiPrevious >> 30)) + i;
	}

	//Force a twist before the first number is returned.
	muiIndex = MERSENNE_TWISTER_STATE_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMersenneTwister::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMersenneTwister::Twist(void)
{
	uint32	i;
	uint32	uiCombined;
	uint32	uiTwisted;

	for (i = 0; i < MERSENNE_TWISTER_STATE_SIZE; i++)
	{
		uiCombined = (mauiState[i] & MERSENNE_TWISTER_UPPER_MASK) | (mauiState[(i + 1) % MERSENNE_TWISTER_STATE_SIZE] & MERSENNE_TWISTER_LOWER_MASK);
		uiTwisted = uiCombined >> 1;
		if (uiCombined & 1)
		{
			uiTwisted ^= MERSENNE_TWISTER_MATRIX_A;
		}
		mauiState[i] = mauiState[(i + MERSENNE_TWISTER_SHIFT_SIZE) % MERSENNE_TWISTER_STATE_SIZE] ^ uiTwisted;
	}
	muiIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMersenneTwister::Next(void)
{
	uint32	y;

	if (muiIndex >= MERSENNE_TWISTER_STATE_SIZE)
	{
		Twist();
	}

	y = mauiState[muiIndex];
	muiIndex++;

	//Tempering.
	y ^= y >> 11;
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= y >> 18;
	return y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMersenneTwister::Next(int iMin, int iMaxInclusive)
{
	uint32	uiRange;
	uint32	uiThreshold;
	uint32	uiValue;

	if (iMaxInclusive <= iMin)
	{
		return iMin;
	}

	uiRange = (uint32)iMaxInclusive - (uint32)iMin + 1;
	if (uiRange == 0)
	{
		//The whole 32 bit range.
		return (int)Next();
	}

	//Reject the lowest (2^32 % uiRange) values so that every value in the range is equally likely.
	uiThreshold = (0 - uiRange) % uiRange;
	do
	{
		uiValue = Next();
	}
	while (uiValue < uiThreshold);

	return (int)((uint32)iMin + (uiValue % uiRange));
}

