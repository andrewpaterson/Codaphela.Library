//Memory allocation and general functions for the 65C64, in C++ for the 65C64 generator.
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>


int		errno = 0;


//The heap is 0008:0000 to 000F:FFFF, or from 0020:0000 to the top of memory when the memory size register
//(00FF:0300) says there is more than 2MB, or from 0100:0000 (past the devices' page) when there is more than 16MB.  A block has its size (with this header) and, while free, the next free
//block.  malloc takes the first free block that is big enough, splitting off the rest, or takes from the end of
//the heap.  The free list is in address order: free joins a block with the free blocks either side of it, and a
//free block at the end of the heap is given back to it.
#define HEAP_START			0x80000
#define HEAP_END			0x100000
#define HIGH_HEAP_START		0x200000
#define DEVICES_START		0xFF0000		//The devices' page, 00FF:0000 to 00FF:FFFF, is never memory.
#define DEVICES_END			0x1000000
#define MEMORY_SIZE			(*(volatile unsigned int*)0x00FF0300)


struct SHeapBlock
{
	size_t			uiSize;
	SHeapBlock*		psNext;
};


SHeapBlock*		gpsFreeBlocks = NULL;
char*			gpcHeapTop = NULL;
char*			gpcHeapEnd = NULL;


static void InitHeap(void)
{
	unsigned int	uiMemorySize;

	uiMemorySize = MEMORY_SIZE;
	if (uiMemorySize > DEVICES_END)
	{
		gpcHeapTop = (char*)DEVICES_END;
		gpcHeapEnd = (char*)uiMemorySize;
	}
	else if (uiMemorySize > HIGH_HEAP_START)
	{
		gpcHeapTop = (char*)HIGH_HEAP_START;
		gpcHeapEnd = (char*)((uiMemorySize < DEVICES_START) ? uiMemorySize : DEVICES_START);
	}
	else
	{
		gpcHeapTop = (char*)HEAP_START;
		gpcHeapEnd = (char*)HEAP_END;
	}
}


void* malloc(size_t uiBytes)
{
	SHeapBlock*		psBlock;
	SHeapBlock*		psPrevious;
	SHeapBlock*		psRest;
	size_t			uiSize;

	if (!gpcHeapEnd)
	{
		InitHeap();
	}

	//Checked before rounding up so that a size near SIZE_MAX can't wrap to a small block.
	if (uiBytes > 0x7FFFFFF0)
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

	if ((size_t)(gpcHeapEnd - gpcHeapTop) < uiSize)
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
//Parses an integer of at most ullMaximum (its sign is in *pbNegative).  *pbOverflow is set if it is bigger.
static bool ParseInteger(const char* sz, char** pszEnd, int iBase, unsigned long long ullMaximum, unsigned long long* pullValue, bool* pbNegative, bool* pbOverflow)
{
	const char*			szStart;
	unsigned long long	ullValue;
	unsigned long long	ullLimit;
	int					iLimitDigit;
	int					iDigit;
	bool				bDigits;

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

	ullLimit = ullMaximum / (unsigned)iBase;
	iLimitDigit = (int)(ullMaximum % (unsigned)iBase);
	ullValue = 0;
	bDigits = false;
	*pbOverflow = false;
	for (;;)
	{
		iDigit = DigitValue(*sz);
		if (iDigit >= iBase)
		{
			break;
		}
		if ((ullValue > ullLimit) || ((ullValue == ullLimit) && (iDigit > iLimitDigit)))
		{
			*pbOverflow = true;
		}
		else
		{
			ullValue = ullValue * (unsigned)iBase + (unsigned)iDigit;
		}
		bDigits = true;
		sz++;
	}

	if (pszEnd)
	{
		*pszEnd = (char*)(bDigits ? sz : szStart);
	}
	*pullValue = ullValue;
	return bDigits;
}


//Out of range values are LONG_MIN or LONG_MAX, and errno is ERANGE.
long strtol(const char* sz, char** pszEnd, int iBase)
{
	unsigned long long	ullValue;
	bool				bNegative;
	bool				bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, ULONG_MAX, &ullValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bNegative)
	{
		if (bOverflow || (ullValue > (unsigned long)LONG_MAX + 1))
		{
			errno = ERANGE;
			return LONG_MIN;
		}
		return (long)((unsigned long)0 - (unsigned long)ullValue);
	}
	if (bOverflow || (ullValue > (unsigned long)LONG_MAX))
	{
		errno = ERANGE;
		return LONG_MAX;
	}
	return (long)ullValue;
}


//Out of range values are ULONG_MAX, and errno is ERANGE.  A negative number is negated as an unsigned long.
unsigned long strtoul(const char* sz, char** pszEnd, int iBase)
{
	unsigned long long	ullValue;
	bool				bNegative;
	bool				bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, ULONG_MAX, &ullValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bOverflow)
	{
		errno = ERANGE;
		return ULONG_MAX;
	}
	return bNegative ? (unsigned long)0 - (unsigned long)ullValue : (unsigned long)ullValue;
}


//Out of range values are LLONG_MIN or LLONG_MAX, and errno is ERANGE.
long long strtoll(const char* sz, char** pszEnd, int iBase)
{
	unsigned long long	ullValue;
	bool				bNegative;
	bool				bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, ULLONG_MAX, &ullValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bNegative)
	{
		if (bOverflow || (ullValue > (unsigned long long)LLONG_MAX + 1))
		{
			errno = ERANGE;
			return LLONG_MIN;
		}
		return (long long)((unsigned long long)0 - ullValue);
	}
	if (bOverflow || (ullValue > (unsigned long long)LLONG_MAX))
	{
		errno = ERANGE;
		return LLONG_MAX;
	}
	return (long long)ullValue;
}


//Out of range values are ULLONG_MAX, and errno is ERANGE.  A negative number is negated as an unsigned long long.
unsigned long long strtoull(const char* sz, char** pszEnd, int iBase)
{
	unsigned long long	ullValue;
	bool				bNegative;
	bool				bOverflow;

	if (!ParseInteger(sz, pszEnd, iBase, ULLONG_MAX, &ullValue, &bNegative, &bOverflow))
	{
		return 0;
	}
	if (bOverflow)
	{
		errno = ERANGE;
		return ULLONG_MAX;
	}
	return bNegative ? (unsigned long long)0 - ullValue : ullValue;
}


static bool MatchIgnoreCase(const char* sz, const char* szWord)
{
	while (*szWord)
	{
		if (tolower((unsigned char)*sz) != *szWord)
		{
			return false;
		}
		sz++;
		szWord++;
	}
	return true;
}


//Decimal (with an exponent), inf, infinity and nan.  Up to 19 significant digits are kept and scaled by powers of
//ten, so the result is within an ulp or so but not always correctly rounded.  Out of range values are HUGE_VAL (or
//0) with errno ERANGE.
double strtod(const char* sz, char** pszEnd)
{
	const char*			szStart;
	unsigned long long	ullDigits;
	int					iDigits;
	int					iExponent;
	int					iExponentValue;
	bool				bNegative;
	bool				bNegativeExponent;
	bool				bAny;
	double				dValue;
	double				dScale;
	double				dPower;
	int					iPower;

	szStart = sz;
	while (isspace((unsigned char)*sz))
	{
		sz++;
	}
	bNegative = false;
	if ((*sz == '-') || (*sz == '+'))
	{
		bNegative = *sz == '-';
		sz++;
	}

	if (MatchIgnoreCase(sz, "inf"))
	{
		sz += MatchIgnoreCase(sz, "infinity") ? 8 : 3;
		if (pszEnd)
		{
			*pszEnd = (char*)sz;
		}
		return bNegative ? -__builtin_huge_val() : __builtin_huge_val();
	}
	if (MatchIgnoreCase(sz, "nan"))
	{
		if (pszEnd)
		{
			*pszEnd = (char*)(sz + 3);
		}
		ullDigits = 0x7FF8000000000000ULL;
		memcpy(&dValue, &ullDigits, sizeof(double));
		return dValue;
	}

	ullDigits = 0;
	iDigits = 0;
	iExponent = 0;
	bAny = false;
	while ((*sz >= '0') && (*sz <= '9'))
	{
		if (iDigits < 19)
		{
			ullDigits = ullDigits * 10 + (unsigned)(*sz - '0');
			if (ullDigits != 0)
			{
				iDigits++;
			}
		}
		else
		{
			iExponent++;
		}
		bAny = true;
		sz++;
	}
	if (*sz == '.')
	{
		sz++;
		while ((*sz >= '0') && (*sz <= '9'))
		{
			if (iDigits < 19)
			{
				ullDigits = ullDigits * 10 + (unsigned)(*sz - '0');
				if (ullDigits != 0)
				{
					iDigits++;
				}
				iExponent--;
			}
			bAny = true;
			sz++;
		}
	}
	if (!bAny)
	{
		if (pszEnd)
		{
			*pszEnd = (char*)szStart;
		}
		return 0.0;
	}

	if ((*sz == 'e') || (*sz == 'E'))
	{
		const char*	szExponent;

		szExponent = sz;
		sz++;
		bNegativeExponent = false;
		if ((*sz == '-') || (*sz == '+'))
		{
			bNegativeExponent = *sz == '-';
			sz++;
		}
		if ((*sz >= '0') && (*sz <= '9'))
		{
			iExponentValue = 0;
			while ((*sz >= '0') && (*sz <= '9'))
			{
				if (iExponentValue < 10000)
				{
					iExponentValue = iExponentValue * 10 + (*sz - '0');
				}
				sz++;
			}
			iExponent += bNegativeExponent ? -iExponentValue : iExponentValue;
		}
		else
		{
			sz = szExponent;
		}
	}
	if (pszEnd)
	{
		*pszEnd = (char*)sz;
	}

	dValue = (double)ullDigits;
	if (dValue != 0.0)
	{
		//10^|exponent| by squaring, then one multiply or divide.
		iPower = (iExponent < 0) ? -iExponent : iExponent;
		dScale = 1.0;
		dPower = 10.0;
		while (iPower > 0)
		{
			if (iPower & 1)
			{
				dScale *= dPower;
			}
			dPower *= dPower;
			iPower >>= 1;
		}
		if (iExponent < 0)
		{
			//Very small values are divided in two steps so the scale doesn't overflow first.
			if (iExponent < -300)
			{
				dValue /= 1e300;
				dScale /= 1e300;
			}
			dValue /= dScale;
		}
		else
		{
			dValue *= dScale;
		}
		if ((dValue == 0.0) || (dValue > 1.7976931348623157e308))
		{
			errno = ERANGE;
		}
	}
	return bNegative ? -dValue : dValue;
}


float strtof(const char* sz, char** pszEnd)
{
	return (float)strtod(sz, pszEnd);
}


double atof(const char* sz)
{
	return strtod(sz, NULL);
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


//Stops at once, unsuccessfully: neither the atexit functions nor the destructors of globals run.
void abort(void)
{
	_Exit(EXIT_FAILURE);
}


//The functions atexit registered, called last first by exit and when main returns.
#define ATEXIT_MAX		32
typedef void (*FAtExit)(void);
static FAtExit	gafAtExit[ATEXIT_MAX];
static int		giAtExits;


//Registers fFunction to be called by exit.  Returns -1 when ATEXIT_MAX are registered already.
int atexit(void (*fFunction)(void))
{
	if (giAtExits == ATEXIT_MAX)
	{
		return -1;
	}
	gafAtExit[giAtExits] = fFunction;
	giAtExits++;
	return 0;
}


//Calls the atexit functions, last first.  The start up calls it (when stdlib.cpp is linked) after main returns,
//before the destructors of globals.
extern "C" void __RunAtExit(void);
void __RunAtExit(void)
{
	while (giAtExits > 0)
	{
		giAtExits--;
		gafAtExit[giAtExits]();
	}
}


//The linker's fini table: each global's R_Finalise, in link order.  _Exit is in the start up.
typedef void (*FFinalise)(void);
extern "C" char	_fini_start;
extern "C" char	_fini_end;


//Runs the atexit functions and the finalisers (the destructors of globals) backwards, as returning from main
//does, then stops.
void exit(int iStatus)
{
	FFinalise*	pfFinalise;

	__RunAtExit();
	pfFinalise = (FFinalise*)&_fini_end;
	while (pfFinalise != (FFinalise*)&_fini_start)
	{
		pfFinalise--;
		(*pfFinalise)();
	}
	_Exit(iStatus);
}
