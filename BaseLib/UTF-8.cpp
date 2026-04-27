#include "UTF-8.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Init(CChars* sz)
{
	CUnicodeReader::Init();
	mszText = (uint8*)sz->Text();
	muiTextLength = sz->Length();
	muiPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Init(char* sz)
{
	CUnicodeReader::Init();
	mszText = (uint8*)sz;
	muiTextLength = StrLen(sz);
	muiPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Kill(void)
{
	mszText = NULL;
	muiTextLength = 0;
	muiPos = 0;
	CUnicodeReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUTF8::GetByteOrderMark(void)
{
	uint32  uiBOM;

	uiBOM = *(uint32*)(&mszText[muiPos]);
	uiBOM &= 0xFFFFFF;
	if (uiBOM == UTF8_BOM)
	{
		muiPos += 3;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUTF8::GetCodePointUint16(void)
{
	uint8	c;
	uint8	c2;
	uint16	c16;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	c = mszText[muiPos];
	if (c < 0x7F)
	{
		muiPos += 1;
		return c;
	}
		
	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= muiTextLength)
		{
			return (uint16)muiError;
		}

		c2 = mszText[muiPos + 1];
		c16 = ((c & 0x1F) << 6) | (c2 & 0x3F);
		muiPos += 2;
		return c16;
	}

	if ((c & 0xF0) == 0xE0)
	{
		return muiTooSmall;
	}

	if ((c & 0xF8) == 0xF0)
	{
		return muiTooSmall;
	}

	return muiError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUTF8::GetCodePointUint32(void)
{
	uint8	c;
	uint8	c2;
	uint8	c3;
	uint8	c4;
	uint32	c32;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	c = mszText[muiPos];
	if (c < 0x7F)
	{
		muiPos += 1;
		return c;
	}

	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= muiTextLength)
		{
			return muiError;
		}

		c2 = mszText[muiPos + 1];
		c32 = ((c & 0x1F) << 6) | (c2 & 0x3F);
		muiPos += 2;
		return c32;
	}

	if ((c & 0xF0) == 0xE0) 
	{
		if (muiPos + 2 >= muiTextLength)
		{
			return muiError;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
		{
			return muiError;
		}

		c32 = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
		if (c32 < 0x0800 || (c32 >= 0xD800 && c32 <= 0xDFFF))
		{
			return muiError;
		}
		muiPos += 3;
		return c32;
	}

	if ((c & 0xF8) == 0xF0) 
	{
		if (muiPos + 3 >= muiTextLength)
		{
			return muiError;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];
		c4 = mszText[muiPos + 3];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80) || ((c4 & 0xC0) != 0x80))
		{
			return muiError;
		}

		c32 = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
		if (c32 < 0x10000 || c32 > 0x10FFFF)
		{
			return muiError;
		}
		muiPos += 4;
		return c32;
	}

	return muiTooSmall;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetCodePointMulti(uint8* puiBuffer, size uiBufferLength)
{
	size	uiLength;
	bool	bLastZWJ;
	size	uiStartPos;
	size	uiTotalElementLength;
	size	uiUsedBuffer;
	size	uiBufferPos;
	size	uiCodePointLength;
	uint32	c32;
	uint32	c16;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	bLastZWJ = true;
	uiStartPos = muiPos;
	uiTotalElementLength = 0;
	uiUsedBuffer = 0;
	uiBufferPos = 0;
	for (;;)
	{
		uiLength = GetUTF8ElementLength();
		if (uiLength == muiError)
		{
			muiPos = uiStartPos;
			return muiError;
		}

		if (uiLength == UNICODE_ZWJ)
		{
			if (bLastZWJ)
			{
				muiPos = uiStartPos;
				return muiError;
			}

			muiPos += 3;
			uiTotalElementLength += 3;
			bLastZWJ = true;
			uiBufferPos = AppendCodePoint((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);
			uiUsedBuffer += 1;
		}
		else
		{
			if (bLastZWJ)
			{
				if (uiLength <= 2)
				{
					c16 = GetCodePointUint16();
					uiCodePointLength = GetUnicodeCodePointLength(c16);
					uiBufferPos = AppendCodePoint(c16, uiCodePointLength, puiBuffer, uiBufferPos, uiBufferLength);
					uiUsedBuffer += uiCodePointLength;
				}
				else
				{
					c32 = GetCodePointUint32();
					uiCodePointLength = GetUnicodeCodePointLength(c32);
					uiBufferPos = AppendCodePoint(c32, uiCodePointLength, puiBuffer, uiBufferPos, uiBufferLength);
					uiUsedBuffer += uiCodePointLength;
				}
				
				uiTotalElementLength += uiLength;
			}
			else
			{
				if (uiTotalElementLength > 0)
				{
					if (uiBufferLength < uiTotalElementLength)
					{
						muiPos = uiStartPos;
						return muiError;
					}
				}
				AppendCodePoint((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);  //Append a final trailing 0 if possible but don't include it in the buffer size.
				return uiUsedBuffer;
			}
			bLastZWJ = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::PeekUTFBytes(void)
{
	size	uiLength;
	bool	bLastZWJ;
	size	uiStartPos;
	size	uiTotalLength;

	bLastZWJ = true;
	uiStartPos = muiPos;
	uiTotalLength = 0;
	for (;;)
	{
		uiLength = GetUTF8ElementLength();
		if (uiLength == UNICODE_ZWJ)
		{
			if (bLastZWJ)
			{
				muiPos = uiStartPos;
				return muiError;
			}
			muiPos += 3;
			uiTotalLength += 3;
			bLastZWJ = true;
		}
		else
		{
			if (bLastZWJ)
			{
				uiTotalLength += uiLength;
				muiPos += uiLength;
			}
			else
			{
				muiPos = uiStartPos;
				return uiTotalLength;
			}
			bLastZWJ = false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetUTF8ElementLength(void)
{
	uint8	c;
	uint8	c2;
	uint8	c3;
	uint8	c4;
	uint32	c32;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	c = mszText[muiPos];
	if (c < 0x7F)
	{
		return 1;
	}
		
	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= muiTextLength)
		{
			return muiError;
		}
		return 2;
	}

	if ((c & 0xF0) == 0xE0) 
	{
		if (muiPos + 2 >= muiTextLength)
		{
			return muiError;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
		{
			return muiError;
		}

		c32 = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
		if (c32 < 0x0800 || (c32 >= 0xD800 && c32 <= 0xDFFF))
		{
			return muiError;
		}
		if (c32 == UNICODE_ZWJ)
		{
			return c32;
		}

		return 3;
	}

	if ((c & 0xF8) == 0xF0) 
	{
		if (muiPos + 3 >= muiTextLength)
		{
			return muiError;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];
		c4 = mszText[muiPos + 3];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80) || ((c4 & 0xC0) != 0x80))
		{
			return muiError;
		}

		c32 = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
		if (c32 < 0x10000 || c32 > 0x10FFFF)
		{
			return muiError;
		}
		return 4;
	}

	return muiError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetPosition(void) { return muiPos; }

