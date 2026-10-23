//Memory allocation and general functions for the 65C64, in C++ for the 65C64 generator.
#include <stdlib.h>
#include <string.h>


//The heap is 0008:0000 to 000F:FFFF.  A block has its size (with this header) and, while free, the next free
//block.  malloc takes the first free block that is big enough, splitting off the rest, or takes from the end of
//the heap.  free puts the block on the free list; neighbours are not joined yet.
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
	SHeapBlock*	psBlock;

	if (!pv)
	{
		return;
	}
	psBlock = (SHeapBlock*)((char*)pv - sizeof(SHeapBlock));
	psBlock->psNext = gpsFreeBlocks;
	gpsFreeBlocks = psBlock;
}


void* calloc(size_t uiCount, size_t uiBytes)
{
	void*	pv;

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


long strtol(const char* sz, char** pszEnd, int iBase)
{
	return (long)strtoul(sz, pszEnd, iBase);
}


unsigned long strtoul(const char* sz, char** pszEnd, int iBase)
{
	unsigned long	ulValue;
	int				iDigit;
	bool			bNegative;

	while ((*sz == ' ') || (*sz == '\t') || (*sz == '\n') || (*sz == '\r'))
	{
		sz++;
	}
	bNegative = false;
	if ((*sz == '-') || (*sz == '+'))
	{
		bNegative = *sz == '-';
		sz++;
	}
	if (((iBase == 0) || (iBase == 16)) && (sz[0] == '0') && ((sz[1] == 'x') || (sz[1] == 'X')))
	{
		iBase = 16;
		sz += 2;
	}
	else if ((iBase == 0) && (sz[0] == '0'))
	{
		iBase = 8;
	}
	else if (iBase == 0)
	{
		iBase = 10;
	}

	ulValue = 0;
	for (;;)
	{
		if ((*sz >= '0') && (*sz <= '9'))
		{
			iDigit = *sz - '0';
		}
		else if ((*sz >= 'a') && (*sz <= 'z'))
		{
			iDigit = *sz - 'a' + 10;
		}
		else if ((*sz >= 'A') && (*sz <= 'Z'))
		{
			iDigit = *sz - 'A' + 10;
		}
		else
		{
			break;
		}
		if (iDigit >= iBase)
		{
			break;
		}
		ulValue = ulValue * iBase + iDigit;
		sz++;
	}
	if (pszEnd)
	{
		*pszEnd = (char*)sz;
	}
	return bNegative ? (unsigned long)(-(long)ulValue) : ulValue;
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
	guiRandom = guiRandom * 1103525245 + 12345;
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
