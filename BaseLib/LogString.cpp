#include <stdio.h>
#include "Chars.h"
#include "FileUtil.h"
#include "StringHelper.h"
#include "LogString.h"


char gaszLogToStringScratchPad[LOG_TO_STRING_SCRATCH_PAD_SIZE][LOG_TO_STRING_MAX_LENGTH];
int  giLogToStringCount = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int IncrementLogToStringCount(void)
{
	int	iCount = giLogToStringCount;

	giLogToStringCount++;
	if (giLogToStringCount >= LOG_TO_STRING_SCRATCH_PAD_SIZE)
	{
		giLogToStringCount = 0;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char c)
{
	int iCount = IncrementLogToStringCount();
	gaszLogToStringScratchPad[iCount][0] = c;
	gaszLogToStringScratchPad[iCount][1] = '\0';
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToString(int i, int iBase)
{
	int iCount = IncrementLogToStringCount();

	IToA(i, gaszLogToStringScratchPad[iCount], iBase);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FloatToString(float f, int iDecimals)
{
	char	sz[] = "%.2f";
	int iCount = IncrementLogToStringCount();
	if (iDecimals > 9)
	{
		iDecimals = 9;
	}

	sz[2] = '0' + iDecimals;
	sprintf(gaszLogToStringScratchPad[iCount], sz, f);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToFlags(int iInt)
{
	int iIndex;
	int iCount = IncrementLogToStringCount();
	int iBit;
	int	i;

	iIndex = 0;
	for (i = 31; i >= 0; i--)
	{
		iBit = (1 << i) & iInt;
		if (iBit)
		{
			gaszLogToStringScratchPad[iCount][iIndex] = '1';
			iIndex++;
		}
		else
		{
			gaszLogToStringScratchPad[iCount][iIndex] = '0';
			iIndex++;
		}

		if ((i % 8 == 0) && (i != 0))
		{
			gaszLogToStringScratchPad[iCount][iIndex] = ' ';
			iIndex++;
		}
	}
	
	gaszLogToStringScratchPad[iCount][iIndex] = '\0';
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ShortToFlags(int siShort)
{
	int iIndex;
	int iCount = IncrementLogToStringCount();
	int iBit;
	int	i;

	iIndex = 0;
	for (i = 15; i >= 0; i--)
	{
		iBit = (1 << i) & siShort;
		if (iBit)
		{
			gaszLogToStringScratchPad[iCount][iIndex] = '1';
			iIndex++;
		}
		else
		{
			gaszLogToStringScratchPad[iCount][iIndex] = '0';
			iIndex++;
		}

		if ((i % 8 == 0) && (i != 0))
		{
			gaszLogToStringScratchPad[iCount][iIndex] = ' ';
			iIndex++;
		}
	}

	gaszLogToStringScratchPad[iCount][iIndex] = '\0';
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LongLongToString(long long int lli, int iBase)
{
	int iCount = IncrementLogToStringCount();

	IToA(lli, gaszLogToStringScratchPad[iCount], iBase);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SizeToString(size_t i, int iBase)
{
	if (sizeof(size_t) == sizeof(int))
	{
		return IntToString((int)i, iBase);
	}
	else if (sizeof(size_t) == sizeof(long long int))
	{
		return LongLongToString((long long int)i, iBase);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* PointerToString(void* pv)
{
	int iCount = IncrementLogToStringCount();
	
	CChars	sz;

	sz.Init();
	sz.AppendHexHiLo(&pv, sizeof(size_t));
	strcpy(gaszLogToStringScratchPad[iCount], sz.Text());
	sz.Kill();
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* MethodToString(const char* szFile, int iLine, const char* szFunction)
{
	int		iCount = IncrementLogToStringCount();
	char*	sz = gaszLogToStringScratchPad[iCount];

	CChars		szOutput;
	CFileUtil	cFileUtil;

	szOutput.Init();
	szOutput.Append(szFile);
	cFileUtil.RemovePath(&szOutput);
	szOutput.Insert(0, '(');
	szOutput.Append(':');
	szOutput.Append(iLine);
	szOutput.Append(") ");
	szOutput.Append(szFunction);

	StrCpySafe(sz, szOutput.Text(), LOG_TO_STRING_MAX_LENGTH);
	szOutput.Kill();

	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StringToString(char* sz)
{
	if (sz != NULL)
	{
		return sz;
	}
	else
	{
		return gszEmptyString;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* StringToString(char* szStart, char* szLastCharInclusive)
{
	int		iCount;
	char*	sz;

	if (szStart != NULL)
	{
		if (szLastCharInclusive == NULL)
		{
			return szStart;
		}
		else
		{
			iCount = IncrementLogToStringCount();
			sz = gaszLogToStringScratchPad[iCount];
			StrCpy(sz, szStart, szLastCharInclusive);
		}
		return sz;
	}
	else
	{
		return gszEmptyString;
	}
}

