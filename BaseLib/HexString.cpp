#include "LogString.h"
#include "StringHelper.h"
#include "HexString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void HexJustify(char* sz, size uiWantedLength, char cPad)
{
	size	uiLen;
	size	uiShift;
	char*	szDest;

	uiLen = strlen(sz);
	uiShift = uiWantedLength - uiLen;

	if (uiShift > 0)
	{
		szDest = sz + uiShift;
		memmove(szDest, sz, uiLen);
		memset(sz, cPad, uiShift);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* HexByteToString(int8 i)
{
	size	uiCount;
	char*	sz;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	ByteToString(sz, LOG_TO_STRING_MAX_LENGTH, i, 16);
	HexJustify(sz, 2, '0');
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* HexByteToString(uint8 ui)
{
	size	uiCount;
	char*	sz;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	ByteToString(sz, LOG_TO_STRING_MAX_LENGTH, ui, 16);
	HexJustify(sz, 2, '0');
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* HexShortToString(int16 i)
{
	size	uiCount;
	char*	sz;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	ShortToString(sz, LOG_TO_STRING_MAX_LENGTH, i, 16);
	HexJustify(sz, 4, '0');
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* HexShortToString(uint16 ui)
{
	size	uiCount;
	char*	sz;

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	ShortToString(sz, LOG_TO_STRING_MAX_LENGTH, ui, 16);
	HexJustify(sz, 4, '0');
	return sz;
}

