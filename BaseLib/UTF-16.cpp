#include "UTF-16.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF16::Init(uint16* sz)
{
    mszText = sz;
    muiTextLength = 0;
    if (sz)
    {
        while (sz[muiTextLength] != 0) ++muiTextLength;
    }
    muiPos = 0;
    muiCodeLength = 0;
    muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CUTF16::Init(uint16* sz, size length)
{
    mszText = sz;
    muiTextLength = length;
    muiPos = 0;
    muiCodeLength = 0;
    muiError = UNICODE_ERROR;
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
    muiCodeLength = 0;
    muiError = UNICODE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 CUTF16::GetUint16(void)
{
    uint32 cp = GetUint32();
    return (cp <= 0xFFFF) ? (uint16)cp : 0xFFFD;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 CUTF16::GetUint32(void)
{
    uint16  c;
    uint16  c2;
    uint32  cp;

    if (muiPos >= muiTextLength)
    {
        muiCodeLength = 0;
        return 0;
    }

    c = mszText[muiPos];
    if (c >= 0xD800 && c <= 0xDBFF)
    {
        if (muiPos + 1 >= muiTextLength)
        {
            muiCodeLength = 0;
            return 0xFFFD;
        }

        c2 = mszText[muiPos + 1];
        if (c2 >= 0xDC00 && c2 <= 0xDFFF)
        {
            cp = 0x10000 + (((c & 0x03FF) << 10) | (c2 & 0x03FF));
            muiPos += 2;
            muiCodeLength = 2;
            return cp;
        }
        
        muiPos += 1;
        muiCodeLength = 1;
        return 0xFFFD;
    }

    if (c >= 0xDC00 && c <= 0xDFFF)
    {
        muiPos += 1;
        muiCodeLength = 1;
        return 0xFFFD;
    }

    muiPos += 1;
    muiCodeLength = 1;
    return c;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF16::GetMulti(uint16* puiBuffer, size uiBufferLength)
{
    size uiStartPos = muiPos;
    size uiTotalLength = 0;
    bool bLastZWJ = true;

    for (;;)
    {
        size uiLength = GetElementLength();
        if (uiLength == muiError)
        {
            muiPos = uiStartPos;
            muiCodeLength = 0;
            return muiError;
        }

        if (uiLength == UNICODE_ZWJ)
        {
            if (bLastZWJ)
            {
                muiPos = uiStartPos;
                muiCodeLength = 0;
                return muiError;
            }
            // ZWJ is always 1 uint16 in UTF-16
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
                // End of grapheme cluster
                if (uiTotalLength > 0)
                {
                    if (uiBufferLength < uiTotalLength)
                    {
                        muiPos = uiStartPos;
                        muiCodeLength = 0;
                        return muiError;
                    }
                    memcpy(puiBuffer, &mszText[uiStartPos], uiTotalLength * sizeof(uint16));
                }
                muiCodeLength = uiTotalLength;
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
size CUTF16::Step(void)
{
    size uiStartPos = muiPos;
    size uiTotalLength = 0;
    bool bLastZWJ = true;

    for (;;)
    {
        size uiLength = GetElementLength();
        if (uiLength == UNICODE_ZWJ)
        {
            if (bLastZWJ)
            {
                muiPos = uiStartPos;
                muiCodeLength = 0;
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
                muiCodeLength = uiTotalLength;
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
size CUTF16::GetElementLength(void)
{
    uint16  c;
    uint16  c2;
    uint32  cp;

    if (muiPos >= muiTextLength)
    {
        return 0;
    }

    c = mszText[muiPos];
    if (c >= 0xD800 && c <= 0xDBFF)
    {
        if (muiPos + 1 >= muiTextLength)
            return muiError;

        c2 = mszText[muiPos + 1];
        if (c2 >= 0xDC00 && c2 <= 0xDFFF)
        {
            cp = 0x10000 + (((c & 0x03FF) << 10) | (c2 & 0x03FF));
            if (cp == UNICODE_ZWJ)
            {
                return UNICODE_ZWJ;
            }
            return 2;
        }
        return muiError;
    }

    if (c == UNICODE_ZWJ)
    {
        return UNICODE_ZWJ;
    }

    return 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CUTF16::GetPosition(void) { return muiPos; }
size CUTF16::GetError(void) { return muiError; }
size CUTF16::GetCodeLength(void) { return muiCodeLength; }

