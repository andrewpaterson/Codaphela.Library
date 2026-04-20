#include "FastMemcpy.h"
#include "Unicode.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size GetUnicodeCodePointLength(uint16 uiChar)
{
	if (uiChar < 0xff)
	{
		return 1;
	}
	return 2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size GetUnicodeCodePointLength(uint32 uiChar)
{
	if (uiChar < 0xff)
	{
		return 1;
	}
	if (uiChar < 0xffff)
	{
		return 2;
	}
	if (uiChar < 0xffffff)
	{
		return 3;
	}
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUnicode::Init(void)
{
	muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::Append(uint32 uiCodePoint, size uiLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
{
	if (uiLength + uiBufferPos > uiBufferLength)
	{
		return muiError;
	}

	if (uiLength == 1)
	{
		memcpy_fast_1byte(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else if (uiLength == 2)
	{
		memcpy_fast_2bytes(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else if (uiLength == 3)
	{
		memcpy_fast_3bytes(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else if (uiLength == 4)
	{
		memcpy_fast_4bytes(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else
	{
		return muiError;
	}

	return uiLength + uiBufferPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::Append(uint16 uiCodePoint, size uiLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
{
	if (uiLength + uiBufferPos > uiBufferLength)
	{
		return muiError;
	}

	if (uiLength == 1)
	{
		memcpy_fast_1byte(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else if (uiLength == 2)
	{
		memcpy_fast_2bytes(&puiBuffer[uiBufferPos], &uiCodePoint);
	}
	else
	{
		return muiError;
	}

	return uiLength + uiBufferPos;
}

