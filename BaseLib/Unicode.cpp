#include "Logger.h"
#include <string.h>
#include "Unicode.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUnicode::Init(void)
{
	muiError = UNICODE_NON_CHARACTER;
	muiTooSmall = UNICODE_REPLACEMENT_CHARACTER;
	muiZWJ = UNICODE_ZWJ;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUnicode::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::GetUnicodeCodePointLength(uint16 uiChar)
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
size CUnicode::GetUnicodeCodePointLength(uint32 uiChar)
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
size CUnicode::AppendCodePoint(uint32 uiCodePoint, size uiCodePointLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
{
	if (uiCodePointLength + uiBufferPos > uiBufferLength)
	{
		return muiError;
	}

	if (uiCodePointLength == 1)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 1);
	}
	else if (uiCodePointLength == 2)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 2);
	}
	else if (uiCodePointLength == 3)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 3);
	}
	else if (uiCodePointLength == 4)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 4);
	}
	else
	{
		return muiError;
	}

	return uiCodePointLength + uiBufferPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::AppendCodePoint(uint16 uiCodePoint, size uiCodePointLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
{
	if (uiCodePointLength + uiBufferPos > uiBufferLength)
	{
		return muiError;
	}

	if (uiCodePointLength == 1)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 1);
	}
	else if (uiCodePointLength == 2)
	{
		memcpy(&puiBuffer[uiBufferPos], &uiCodePoint, 2);
	}
	else
	{
		return muiError;
	}

	return uiCodePointLength + uiBufferPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::GetUTF8Length(uint32 uiCodePoint)
{
	if ((uiCodePoint > 0x10FFFF) || (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF))
	{
		return muiError;
	}

	if (uiCodePoint <= 0x7F)
	{
		return 1;
	}
	else if (uiCodePoint <= 0x7FF)
	{
		return 2;
	}
	else if (uiCodePoint <= 0xFFFF)
	{
		return  3;
	}
	else
	{
		return 4;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUnicode::GetUTF8ElementUint16(uint32 uiCodePoint)
{
	uint16	uiHigh;
	uint16	uiLow;

	if ((uiCodePoint > 0x10FFFF) || (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF))
	{
		return muiError;
	}

	if (uiCodePoint <= 0x7F)
	{
		return uiCodePoint;
	}
	else if (uiCodePoint <= 0x7FF)
	{
		uiLow = 0xC0 | (uiCodePoint >> 6);
		uiHigh = 0x80 | (uiCodePoint & 0x3F);
		return  (uint32)uiHigh << 8 | uiLow;
	}
	else
	{
		return muiTooSmall;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUnicode::GetUTF8ElementUint32(uint32 uiCodePoint)
{
	uint16	uiHigh;
	uint16	ui3;
	uint16	ui2;
	uint16	uiLow;

	if ((uiCodePoint > 0x10FFFF) || (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF))
	{
		return muiError;
	}

	if (uiCodePoint <= 0x7F) 
	{
		return uiCodePoint;
	}
	else if (uiCodePoint <= 0x7FF) 
	{
		uiLow = 0xC0 | (uiCodePoint >> 6);
		ui2 = 0x80 | (uiCodePoint & 0x3F);
		return  (uint32)ui2 << 8 | uiLow;
	}
	else if (uiCodePoint <= 0xFFFF) 
	{
		uiLow = 0xE0 | (uiCodePoint >> 12);
		ui2 = 0x80 | ((uiCodePoint >> 6) & 0x3F);
		ui3 = 0x80 | (uiCodePoint & 0x3F);
		return  (uint32)ui3 << 16 | (uint32)ui2 << 8 | uiLow;
	}
	else 
	{
		uiLow = 0xF0 | (uiCodePoint >> 18);
		ui2 = 0x80 | ((uiCodePoint >> 12) & 0x3F);
		ui3 = 0x80 | ((uiCodePoint >> 6) & 0x3F);
		uiHigh = 0x80 | (uiCodePoint & 0x3F);
		return (uint32)uiHigh << 24 | (uint32)ui3 << 16 | (uint32)ui2 << 8 | uiLow;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::GetUTF16Length(uint32 uiCodePoint)
{
	if (uiCodePoint > 0x10FFFF)
	{
		return muiError;
	}

	if (uiCodePoint <= 0xFFFF)
	{
		if (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF)
		{
			return muiError;
		}
		return 1;  //one uint16
	}
	else
	{
		return 2;  //two uint16s
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUnicode::GetUTF16ElementUint16(uint32 uiCodePoint)
{
	if (uiCodePoint > 0x10FFFF)
	{
		return muiError;
	}

	if (uiCodePoint <= 0xFFFF)
	{
		if (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF)
		{
			return muiError;
		}
		return uiCodePoint;
	}
	else
	{
		return muiTooSmall;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUnicode::GetUTF16ElementUint32(uint32 uiCodePoint)
{
	uint16	uiHighSurrogate;
	uint16	uiLowSurrogate;

	if (uiCodePoint > 0x10FFFF) 
	{
		return muiError;
	}

	if (uiCodePoint <= 0xFFFF) 
	{
		if (uiCodePoint >= 0xD800 && uiCodePoint <= 0xDFFF) 
		{
			return muiError;
		}
		return uiCodePoint;
	}
	else 
	{
		uiCodePoint &= 0xFFFF;
		uiHighSurrogate = 0xD800 + (uiCodePoint >> 10);
		uiLowSurrogate = 0xDC00 + (uiCodePoint & 0x3FF);

		return ((uint32)uiLowSurrogate << 16) + uiHighSurrogate;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUnicode::GetError(void) { return muiError; }
uint16 CUnicode::GetZWJCodePoint(void) { return muiZWJ; }


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EUnicodeEncoding GetUnicodeEncoding(void* puiData, size uiDataLength)
{
	uint32	uiData32;
	uint16	uiData16;

	if (uiDataLength <= 1)
	{
		return UE_UTF8;
	}
	else if (uiDataLength == 2)
	{
		uiData16 = *((uint16*)puiData);

		if ((uiData16 & UTF16_BOM_MASK) == UTF16_LITTLE_ENDIAN_BOM)
		{
			return UE_UTF16LE;
		}
		else if ((uiData16 & UTF16_BOM_MASK) == UTF16_BIG_ENDIAN_BOM)
		{
			return UE_UTF16BE;
		}
		else
		{
			return UE_UTF8;
		}
	}
	else
	{
		uiData32 = *((uint32*)puiData);

		if ((uiData32 & UTF8_BOM_MASK) == UTF8_BOM)
		{
			return UE_UTF8BOM;
		}
		else if ((uiData32 & UTF16_BOM_MASK) == UTF16_LITTLE_ENDIAN_BOM)
		{
			return UE_UTF16LE;
		}
		else if ((uiData32 & UTF16_BOM_MASK) == UTF16_BIG_ENDIAN_BOM)
		{
			return UE_UTF16BE;
		}
		else
		{
			return UE_UTF8;
		}
	}
}

