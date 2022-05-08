#include <stdio.h>
#include "Chars.h"
#include "FileUtil.h"
#include "StringHelper.h"
#include "FloatPrinter.h"
#include "EscapeCodes.h"
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
	return CharToString((unsigned char)c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(unsigned char c)
{
	char	sz[10];
	CChars	szDest;

	szDest.Init();
	szDest.Append('\'');
	szDest.Append(GetEscapeString(c, sz));
	szDest.Append('\'');

	int iCount = IncrementLogToStringCount();
	szDest.CopyIntoBuffer(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH);
	szDest.Kill();
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char16 c)
{
	char	sz[10];
	CChars	szDest;

	szDest.Init();
	szDest.Append('\'');
	szDest.Append(GetEscapeString(c, sz));
	szDest.Append('\'');

	int iCount = IncrementLogToStringCount();
	szDest.CopyIntoBuffer(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH);
	szDest.Kill();
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* BoolToString(BOOL b)
{
	int iCount = IncrementLogToStringCount();

	if (b)
	{
		StrCpySafe(gaszLogToStringScratchPad[iCount], "true", LOG_TO_STRING_MAX_LENGTH);
	}
	else
	{
		StrCpySafe(gaszLogToStringScratchPad[iCount], "false", LOG_TO_STRING_MAX_LENGTH);
	}

	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToString(int i, int iBase)
{
	int iCount = IncrementLogToStringCount();

	IntToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, i, iBase);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FloatToString(float f, int iDecimals)
{
	int iCount = IncrementLogToStringCount();

	return FloatToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, f, iDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* DoubleToString(double d, int iDecimals)
{
	int iCount = IncrementLogToStringCount();

	return DoubleToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, d, iDecimals);
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
	char* sz;

	sz = gaszLogToStringScratchPad[iCount];
	iIndex = 0;
	for (i = 31; i >= 0; i--)
	{
		iBit = (1 << i) & iInt;
		if (iBit)
		{
			sz[iIndex] = '1';
			iIndex++;
		}
		else
		{
			sz[iIndex] = '0';
			iIndex++;
		}

		if ((i % 8 == 0) && (i != 0))
		{
			sz[iIndex] = ' ';
			iIndex++;
		}
	}
	
	sz[iIndex] = '\0';
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

	IntToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, lli, iBase);
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
			//Should be StrCpySafe but a Safe version hasn't been written yet.
			StrCpy(sz, szStart, szLastCharInclusive);
		}
		return sz;
	}
	else
	{
		return gszEmptyString;
	}
}

