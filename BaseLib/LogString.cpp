#include <stdio.h>
#include "Chars.h"
#include "FileUtil.h"
#include "StringHelper.h"
#include "FloatPrinter.h"
#include "EscapeCodes.h"
#include "LogString.h"


char gaszLogToStringScratchPad[LOG_TO_STRING_SCRATCH_PAD_SIZE][LOG_TO_STRING_MAX_LENGTH];
size  giLogToStringCount = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size IncrementLogToStringCount(void)
{
	size	uiCount;
	
	uiCount = giLogToStringCount;
	giLogToStringCount++;
	if (giLogToStringCount >= LOG_TO_STRING_SCRATCH_PAD_SIZE)
	{
		giLogToStringCount = 0;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(uint8 c)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char c)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CharToString(char16 c)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return CharToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* BoolToString(bool b)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return BoolToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, b);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ByteToString(int8 i, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	ByteToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, i, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ByteToString(uint8 ui, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	ByteToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, ui, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ShortToString(int16 i, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	ShortToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, i, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ShortToString(uint16 ui, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	ShortToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, ui, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToString(int32 i, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	IntToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, i, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToString(uint32 ui, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	IntToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, ui, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LongToString(int64 lli, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	LongToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, lli, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LongToString(uint64 ulli, uint16 iBase)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	LongToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, ulli, iBase);
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FloatToString(float f, uint16 iDecimals)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return FloatToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, f, iDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* DoubleToString(double d, uint16 iDecimals)
{
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	return DoubleToString(gaszLogToStringScratchPad[uiCount], LOG_TO_STRING_MAX_LENGTH, d, iDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToFlags(uint32 iInt)
{
	char*	sz;
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	return FlagsToString(sz, LOG_TO_STRING_MAX_LENGTH, iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ShortToFlags(uint16 iShort)
{
	char* sz;
	size	uiCount;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	return FlagsToString(sz, LOG_TO_STRING_MAX_LENGTH, iShort);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SizeToString(size i, uint16 iBase)
{
	if (sizeof(size) == sizeof(uint16))
	{
		return ShortToString((uint16)i, iBase);
	}
	else if (sizeof(size) == sizeof(uint32))
	{
		return IntToString((uint32)i, iBase);
	}
	else if (sizeof(size) == sizeof(uint64))
	{
		return LongToString((uint64)i, iBase);
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
	size	uiCount;
	CChars	sz;

	uiCount = IncrementLogToStringCount();
	sz.Init();
	sz.AppendHexHiLo(&pv, sizeof(size));
	StrCpySafe(gaszLogToStringScratchPad[uiCount], sz.Text(), LOG_TO_STRING_MAX_LENGTH);
	sz.Kill();
	return gaszLogToStringScratchPad[uiCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* MethodToString(const char* szFile, size iLine, const char* szFunction)
{
	size		uiCount;
	char*		sz;
	CChars		szOutput;
	CFileUtil	cFileUtil;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];

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
	size	uiCount;
	char*	sz;

	if (szStart != NULL)
	{
		if (szLastCharInclusive == NULL)
		{
			return szStart;
		}
		else
		{
			uiCount = IncrementLogToStringCount();
			sz = gaszLogToStringScratchPad[uiCount];
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

