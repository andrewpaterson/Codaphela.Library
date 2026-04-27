#include "UTF-16.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF16::Init(uint16* sz, size length)
{
    CUnicodeReader::Init();
    mszText = sz;
    muiTextLength = length;
    muiPos = 0;
    mbLittleEndian = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF16::Kill(void)
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
void CUTF16::SetBigEndian(void)
{
    mbLittleEndian = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CUTF16::GetByteOrderMark(void)
{
    uint16  uiBOM;

    uiBOM = mszText[muiPos];
    if (uiBOM == UTF16_BIG_ENDIAN_BOM)
    {
        SetBigEndian();
        muiPos += 1;
        return true;
    }
    else if (uiBOM == UTF16_LITTLE_ENDIAN_BOM)
    {
        muiPos += 1;
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
uint16 CUTF16::GetCodePointUint16(void)
{
    uint16  c;

    if (muiPos >= muiTextLength)
    {
        return 0;
    }

    c = mszText[muiPos];
    if (!(c >= 0xD800 && c <= 0xDBFF))
    {
        muiPos += 1;
        return c;
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
uint32 CUTF16::GetCodePointUint32(void)
{
    uint16  c;
    uint16  c2;
    uint32  c32;

    if (muiPos >= muiTextLength)
    {
        return 0;
    }

    c = mszText[muiPos];
    if (!(c >= 0xD800 && c <= 0xDBFF))
    {
        muiPos += 1;
        return c;
    }
    else
    {
        if (muiPos + 1 >= muiTextLength)
        {
            return muiError;
        }

        c2 = mszText[muiPos + 1];
        if (c2 >= 0xDC00 && c2 <= 0xDFFF)
        {
            c32 = 0x10000 + (((c & 0x03FF) << 10) | (c2 & 0x03FF));
            muiPos += 2;
            return c32;
        }
        else
        {
            return muiError;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF16::GetCodePointMulti(uint8* puiBuffer, size uiBufferLength)
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
        uiLength = GetUTF16ElementLength();
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

            muiPos += 1;
            uiTotalElementLength += 1;
            bLastZWJ = true;
            uiBufferPos = AppendCodePoint((uint16)0, 1, puiBuffer, uiBufferPos, uiBufferLength);
            uiUsedBuffer += 1;
        }
        else
        {
            if (bLastZWJ)
            {
                if (uiLength <= 1)
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
size CUTF16::PeekUTFBytes(void)
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
        uiLength = GetUTF16ElementLength();
        if (uiLength == UNICODE_ZWJ)
        {
            if (bLastZWJ)
            {
                muiPos = uiStartPos;
                return muiError;
            }
            muiPos += 1;
            uiTotalLength += 1;
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
                return uiTotalLength * sizeof(uint16);
            }
            bLastZWJ = false;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF16::GetUTF16ElementLength(void)
{
    uint16  c;
    uint16  c2;
    uint32  c32;

    if (muiPos >= muiTextLength)
    {
        return 0;
    }

    c = mszText[muiPos];
    if (c == UNICODE_ZWJ)
    {
        return UNICODE_ZWJ;
    }

    if (!(c >= 0xD800 && c <= 0xDBFF))
    {
        return 1;
    }
    else
    {
        if (muiPos + 1 >= muiTextLength)
        {
            return muiError;
        }

        c2 = mszText[muiPos + 1];
        if (c2 >= 0xDC00 && c2 <= 0xDFFF)
        {
            c32 = 0x10000 + (((c & 0x03FF) << 10) | (c2 & 0x03FF));
            return 2;
        }
        else
        {
            return muiError;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF16::GetPosition(void) { return muiPos; }

