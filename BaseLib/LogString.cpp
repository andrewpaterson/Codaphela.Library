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
char* CharToString(unsigned char c)
{
	int iCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char c)
{
	int iCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char16 c)
{
	int iCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* BoolToString(BOOL b)
{
	int iCount = IncrementLogToStringCount();
	return BoolToString(gaszLogToStringScratchPad[iCount], LOG_TO_STRING_MAX_LENGTH, b);
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
	char* sz;
	int iCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[iCount];

	return FlagsToString(sz, LOG_TO_STRING_MAX_LENGTH, iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ShortToFlags(int iShort)
{
	char* sz;
	int iCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[iCount];

	return FlagsToString(sz, LOG_TO_STRING_MAX_LENGTH, iShort);
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
	StrCpySafe(gaszLogToStringScratchPad[iCount], sz.Text(), LOG_TO_STRING_MAX_LENGTH);
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

