#include "UTF-8.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Init(CChars* sz)
{
	mpsz = sz;
	muiPos = 0;
	muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF8::Kill(void)
{
	mpsz = NULL;
	muiPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUTF8::GetUint16(void)
{
	uint8	c;
	uint8	c2;
	uint8*	pc;
	uint16	c16;

	if (muiPos >= mpsz->Length())
	{
		return 0;
	}

	pc = (uint8*)mpsz->Text();
	c = pc[muiPos];
	if (c < 0x7F)
	{
		muiPos += 1;
		return c;
	}
		
	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= mpsz->Length())
		{
			return 0xFFFF;
		}

		c2 = pc[muiPos + 1];
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
	uint8*	pc;
	uint32	c32;

	if (muiPos >= mpsz->Length())
	{
		return 0;
	}

	pc = (uint8*)mpsz->Text();
	c = pc[muiPos];
	if (c < 0x7F)
	{
		muiPos += 1;
		return c;
	}
		
	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= mpsz->Length())
		{
			return 0xFFFF;
		}

		c2 = pc[muiPos + 1];
		c32 = ((c & 0x1F) << 6) | (c2 & 0x3F);
		muiPos += 2;
		return c32;
	}

	if ((c & 0xF0) == 0xE0) 
	{
		if (muiPos + 2 >= mpsz->Length())
		{
			return 0xFFFF;
		}

		c2 = pc[muiPos + 1];
		c3 = pc[muiPos + 2];

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
		if (muiPos + 3 >= mpsz->Length())
		{
			return 0xFFFF;
		}

		c2 = pc[muiPos + 1];
		c3 = pc[muiPos + 2];
		c4 = pc[muiPos + 3];

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
	size	uiTotalLength;

	if (muiPos >= mpsz->Length())
	{
		return 0;
	}

	bLastZWJ = true;
	uiStartPos = muiPos;
	uiTotalLength = 0;
	for (;;)
	{
		uiLength = GetElementLength();
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
				if (uiTotalLength > 0)
				{
					if (uiBufferLength < uiTotalLength)
					{
						muiPos = uiStartPos;
						return muiError;
					}
					memcpy(puiBuffer, mpsz->Text(uiStartPos), uiTotalLength);
				}
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
size CUTF8::GetLength(void)
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
		uiLength = GetElementLength();
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
size CUTF8::GetElementLength(void)
{
	uint8	c;
	uint8	c2;
	uint8	c3;
	uint8	c4;
	uint8*	pc;
	uint32	c32;

	if (muiPos >= mpsz->Length())
	{
		return 0;
	}

	pc = (uint8*)mpsz->Text();
	c = pc[muiPos];
	if (c < 0x7F)
	{
		return 1;
	}
		
	if ((c & 0xE0) == 0xC0)
	{
		if (muiPos + 1 >= mpsz->Length())
		{
			return muiError;
		}
		return 2;
	}

	if ((c & 0xF0) == 0xE0) 
	{
		if (muiPos + 2 >= mpsz->Length())
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
		if (muiPos + 3 >= mpsz->Length())
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
size CUTF8::GetPosition(void)
{
	return muiPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF8::GetError(void)
{
	return muiError;
}

