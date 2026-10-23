//Memory allocation and general functions for the 65C64, in C++ for the 65C64 generator.
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>


//The heap is 0008:0000 to 000F:FFFF.  A block has its size (with this header) and, while free, the next free
//block.  malloc takes the first free block that is big enough, splitting off the rest, or takes from the end of
//the heap.  The free list is in address order: free joins a block with the free blocks either side of it, and a
//free block at the end of the heap is given back to it.
#define HEAP_START		0x80000
#define HEAP_END		0x100000


struct SHeapBlock
{
	size_t			uiSize;
	SHeapBlock*		psNext;
};


SHeapBlock*		gpsFreeBlocks = NULL;
char*			gpcHeapTop = (char*)HEAP_START;


void* malloc(size_t uiBytes)
{
	SHeapBlock*		psBlock;
	SHeapBlock*		psPrevious;
	SHeapBlock*		psRest;
	size_t			uiSize;

	//Checked before rounding up so that a size near SIZE_MAX can't wrap to a small block.
	if (uiBytes > HEAP_END - HEAP_START)
	{
		return NULL;
	}
	uiSize = ((uiBytes + 3) & ~3) + sizeof(SHeapBlock);
	psPrevious = NULL;
	for (psBlock = gpsFreeBlocks; psBlock; psBlock = psBlock->psNext)
	{
		if (psBlock->uiSize >= uiSize)
		{
			if (psBlock->uiSize >= uiSize + sizeof(SHeapBlock) + 4)
			{
				psRest = (SHeapBlock*)((char*)psBlock + uiSize);
				psRest->uiSize = psBlock->uiSize - uiSize;
				psRest->psNext = psBlock->psNext;
				psBlock->uiSize = uiSize;
				psBlock->psNext = psRest;
			}
			if (psPrevious)
			{
				psPrevious->psNext = psBlock->psNext;
			}
			else
			{
				gpsFreeBlocks = psBlock->psNext;
			}
			return (char*)psBlock + sizeof(SHeapBlock);
		}
		psPrevious = psBlock;
	}

	if (gpcHeapTop + uiSize > (char*)HEAP_END)
	{
		return NULL;
	}
	psBlock = (SHeapBlock*)gpcHeapTop;
	psBlock->uiSize = uiSize;
	gpcHeapTop += uiSize;
	return (char*)psBlock + sizeof(SHeapBlock);
}


void free(void* pv)
{
	SHeapBlock*		psBlock;
	SHeapBlock*		psPrevious;
	SHeapBlock*		psNext;
	SHeapBlock**	ppsLink;
	SHeapBlock**	ppsPreviousLink;

	if (!pv)
	{
		return;
	}
	psBlock = (SHeapBlock*)((char*)pv - sizeof(SHeapBlock));

	//Find where the block goes in the free list.  *ppsLink is the link to the free block after it and
	//*ppsPreviousLink the link to the one before it.
	ppsPreviousLink = NULL;
	psPrevious = NULL;
	ppsLink = &gpsFreeBlocks;
	while (*ppsLink && (*ppsLink < psBlock))
	{
		ppsPreviousLink = ppsLink;
		psPrevious = *ppsLink;
		ppsLink = &psPrevious->psNext;
	}
	psNext = *ppsLink;

	if (psNext && ((char*)psBlock + psBlock->uiSize == (char*)psNext))
	{
		psBlock->uiSize += psNext->uiSize;
		psNext = psNext->psNext;
	}
	if (psPrevious && ((char*)psPrevious + psPrevious->uiSize == (char*)psBlock))
	{
		psPrevious->uiSize += psBlock->uiSize;
		psBlock = psPrevious;
		ppsLink = ppsPreviousLink;
	}

	if ((char*)psBlock + psBlock->uiSize == gpcHeapTop)
	{
		//The last block in the heap is given back, so it is the last in the free list too.
		gpcHeapTop = (char*)psBlock;
		*ppsLink = psNext;
	}
	else
	{
		psBlock->psNext = psNext;
		*ppsLink = psBlock;
	}
}


void* calloc(size_t uiCount, size_t uiBytes)
{
	void*	pv;

	//uiCount * uiBytes must not wrap.
	if ((uiBytes != 0) && (uiCount > (size_t)-1 / uiBytes))
	{
		return NULL;
	}
	pv = malloc(uiCount * uiBytes);
	if (pv)
	{
		memset(pv, 0, uiCount * uiBytes);
	}
	return pv;
}


void* realloc(void* pv, size_t uiBytes)
{
	SHeapBlock*	psBlock;
	void*		pvNew;
	size_t		uiOld;

	if (!pv)
	{
		return malloc(uiBytes);
	}
	psBlock = (SHeapBlock*)((char*)pv - sizeof(SHeapBlock));
	uiOld = psBlock->uiSize - sizeof(SHeapBlock);
	if (uiOld >= uiBytes)
	{
		return pv;
	}
	pvNew = malloc(uiBytes);
	if (pvNew)
	{
		memcpy(pvNew, pv, uiOld);
		free(pv);
	}
	return pvNew;
}


int abs(int i)
{
	return (i < 0) ? -i : i;
}


long labs(long i)
{
	return (i < 0) ? -i : i;
}


//The value of c as a digit, or 36 if it isn't one in any base.
static int DigitValue(char c)
{
	if (isdigit(c))
	{
		return c - '0';
	}
	if (islower(c))
	{
		return c - 'a' + 10;
	}
	if (isupper(c))
	{
		return c - 'A' + 10;
	}
	return 36;
}


//Reads the sign and magnitude of a number for strtol and strtoul.  *pbOverflow is set if the magnitude doesn't
//fit in an unsigned long.  Without any digits *pszEnd is sz and false is returned.
static bool ParseInteger(const char* sz, char** pszEnd, int iBase, unsigned long* pulValue, bool* pbNegative, bool* pbOverflow)
{
	const char*		szStart;
	unsigned long	ulValue;
	unsigned long	ulLimit;
	int				iLimitDigit;
	int				iDigit;
	bool			bDigits;

	szStart = sz;
	while (isspace(*sz))
	{
		sz++;
	}
	*pbNegative = false;
	if ((*sz == '-') || (*sz == '+'))
	{
		*pbNegative = *sz == '-';
		sz++;
	}

	//0x is only a prefix if a hex digit follows it; otherwise the 0 is the number.
	if (((iBase == 0) || (iBase == 16)) && (sz[0] == '0') && ((sz[1] == 'x') || (sz[1] == 'X')) && (DigitValue(sz[2]) < 16))
	{
		iBase = 16;
		sz += 2;
	}
	else if (iBase == 0)
	{
		iBase = (sz[0] == '0') ? 8 : 10;
	}

	ulLimit = ULONG_MAX / iBase;
	iLimitDigit = (int)(ULONG_MAX % iBase);
	ulValue = 0;
	bDigits = false;
	*pbOverflow = false;
	for (;;)
	{
		iDigit = DigitValue(*sz);
		if (iDigit >= iBase)
		{
			break;
		}
		if ((ulValue > ulLimit) || ((ulValue == ulLimit) && (iDigit > iLimitDigit)))
		{
			*pbOverflow = true;
		}
		else
		{
			ulValue = ulValue * iBase + iDigit;
		}
		bDigits = true;
		sz++;
	}

	if (pszEnd)
	{
		*pszEnd = (char*)(bDigits ? sz : szStart);
	}
	*pulValue = ulValue;
	return bDigits;
}


//Out of range values are LONG_MIN or LONG_MAX.
long strtol(const char* sz, char** pszEnd, int iBase)
{
	unsigned long	ulValue;
	bool			bNegative;
	bool			bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, &ulValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bNegative)
	{
		if (bOverflow || (ulValue > (unsigned long)LONG_MAX + 1))
		{
			return LONG_MIN;
		}
		return (long)((unsigned long)0 - ulValue);
	}
	if (bOverflow || (ulValue > (unsigned long)LONG_MAX))
	{
		return LONG_MAX;
	}
	return (long)ulValue;
}


//Out of range values are ULONG_MAX.  A negative number is negated as an unsigned long.
unsigned long strtoul(const char* sz, char** pszEnd, int iBase)
{
	unsigned long	ulValue;
	bool			bNegative;
	bool			bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, &ulValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bOverflow)
	{
		return ULONG_MAX;
	}
	return bNegative ? (unsigned long)0 - ulValue : ulValue;
}


int atoi(const char* sz)
{
	return (int)strtol(sz, NULL, 10);
}


long atol(const char* sz)
{
	return strtol(sz, NULL, 10);
}


//A linear congruential generator, as the C standard's example.
unsigned int	guiRandom = 1;


int rand(void)
{
	guiRandom = guiRandom * 1103515245 + 12345;
	return (int)((guiRandom / 65536) % (RAND_MAX + 1));
}


void srand(unsigned int uiSeed)
{
	guiRandom = uiSeed;
}


//An insertion sort.
void qsort(void* pvBase, size_t uiCount, size_t uiBytes, int (*fCompare)(const void*, const void*))
{
	char*	pcBase;
	char*	pcKey;
	size_t	i;
	size_t	j;
	size_t	k;
	char	c;

	pcBase = (char*)pvBase;
	for (i = 1; i < uiCount; i++)
	{
		for (j = i; j > 0; j--)
		{
			pcKey = pcBase + j * uiBytes;
			if (fCompare(pcKey - uiBytes, pcKey) <= 0)
			{
				break;
			}
			for (k = 0; k < uiBytes; k++)
			{
				c = pcKey[k];
				pcKey[k] = pcKey[k - uiBytes];
				pcKey[k - uiBytes] = c;
			}
		}
	}
}


void abort(void)
{
	for (;;)
	{
	}
}


void exit(int iStatus)
{
	abort();
}
