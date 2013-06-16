#include "StringHelper.h"


#define LOG_TO_STRING_MAX	128


char gaszLogToStringScratchPad[LOG_TO_STRING_MAX][64];
int  giLogToStringCount = 0;


int IncrementLogToStringCount(void)
{
	int	iCount = giLogToStringCount;

	giLogToStringCount++;
	if (giLogToStringCount >= LOG_TO_STRING_MAX)
	{
		giLogToStringCount = 0;
	}
	return iCount;
}


char* IntToString(int i)
{
	int iCount = IncrementLogToStringCount();

	IToA(i, gaszLogToStringScratchPad[iCount], 10);
	return gaszLogToStringScratchPad[iCount];
}


char* LongLongToString(long long int lli)
{
	int iCount = IncrementLogToStringCount();

	IToA(lli, gaszLogToStringScratchPad[iCount], 10);
	return gaszLogToStringScratchPad[iCount];
}

