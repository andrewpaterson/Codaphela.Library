#include "UTF-8.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Init(CChars* sz)
{
	mszText = (uint8*)sz->Text();
	muiTextLength = sz->Length();
	muiPos = 0;
	muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Init(char* sz)
{
	mszText = (uint8*)sz;
	muiTextLength = StrLen(sz);
	muiPos = 0;
	muiError = UNICODE_ERROR;
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
	muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUTF8::GetUint16(void)
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
			return 0xFFFF;
		}

		c2 = mszText[muiPos + 1];
		c16 = ((c & 0x1F) << 6) | (c2 & 0x3F);
		muiPos += 2;
		return c16;
	}

	if ((c & 0xF0) == 0xE0)
	{
		return 0xFFFD;
	}

	if ((c & 0xF8) == 0xF0)
	{
		return 0xFFFD;
	}

	return 0xFFFF;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUTF8::GetUint32(void)
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
			return 0xFFFF;
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
			return 0xFFFF;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
		{
			return 0xFFFF;
		}

		c32 = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
		if (c32 < 0x0800 || (c32 >= 0xD800 && c32 <= 0xDFFF))
		{
			return 0xFFFF;
		}
		muiPos += 3;
		return c32;
	}

	if ((c & 0xF8) == 0xF0) 
	{
		if (muiPos + 3 >= muiTextLength)
		{
			return 0xFFFF;
		}

		c2 = mszText[muiPos + 1];
		c3 = mszText[muiPos + 2];
		c4 = mszText[muiPos + 3];

		if (((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80) || ((c4 & 0xC0) != 0x80))
		{
			return 0xFFFF;
		}

		c32 = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
		if (c32 < 0x10000 || c32 > 0x10FFFF)
		{
			return 0xFFFF;
		}
		muiPos += 4;
		return c32;
	}

	return 0xFFFD;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetMulti(uint8* puiBuffer, size uiBufferLength)
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
			uiBufferPos = Append((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);
			uiUsedBuffer += 1;
		}
		else
		{
			if (bLastZWJ)
			{
				if (uiLength <= 2)
				{
					c16 = GetUint16();
					uiCodePointLength = GetUnicodeCodePointLength(c16);
					uiBufferPos = Append(c16, uiCodePointLength, puiBuffer, uiBufferPos, uiBufferLength);
					uiUsedBuffer += uiCodePointLength;
				}
				else
				{
					c32 = GetUint32();
					uiCodePointLength = GetUnicodeCodePointLength(c32);
					uiBufferPos = Append(c32, uiCodePointLength, puiBuffer, uiBufferPos, uiBufferLength);
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
				Append((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);  //Append a final trailing 0 if possible but don't include it in the buffer size.
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
size CUTF8::Step(void)
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
size CUTF8::Peek(void)
{
	size	uiPos;
	size	uiElementLength;

	uiPos = muiPos;
	uiElementLength = Step();
	muiPos = uiPos;

	return uiElementLength;
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
	uint8*	pc;
	uint32	c32;

	if (muiPos >= muiTextLength)
	{
		return 0;
	}

	pc = (uint8*)mszText;
	c = pc[muiPos];
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

		c2 = pc[muiPos + 1];
		c3 = pc[muiPos + 2];

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

		c2 = pc[muiPos + 1];
		c3 = pc[muiPos + 2];
		c4 = pc[muiPos + 3];

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
size CUTF8::Append(uint32 uiCodePoint, size uiLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
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
size CUTF8::Append(uint16 uiCodePoint, size uiLength, uint8* puiBuffer, size uiBufferPos, size uiBufferLength)
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetPosition(void) { return muiPos; }
size CUTF8::GetError(void) { return muiError; }


