#include "TextUTF16Short.h"
#include "TextUTF16Long.h"
#include "TextUTF16Multi.h"
#include "TextLayout.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Init(void)
{
	Init(-1);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Init(int32 iWidth)
{
	PreInit();

	miWidth = iWidth;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Class(void)
{
	U_Int32(miWidth);
	U_Int16(miRowHeight);
	U_2Int32(msPosition);
	U_Data(CUTF16, mcUTF16);
	U_Pointer(mpcDraw);

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextLayout::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextLayout::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Layout(Ptr<CBaseFontDraw> pDraw, Ptr<CText> pText)
{
	size				uiNumRuns;
	size				uiRunIndex;
	CTextRun*			pcRun;
	size				uiNumElements;
	size				uiElementIndex;
	CTextElement*		pcTextElement;
	CTextUTF16Multi*	pcUTFMulti;
	CFont*				pcFont;
	Ptr<CGlyph>			pGlyph;

	miRowHeight = 0;
	msPosition.Init(0, 0);
	mpcDraw = &pDraw;
	mcUTF16.Init(NULL, 0);
	pcFont = NULL;
	uiNumRuns = pText->NumRuns();
	for (uiRunIndex = 0; uiRunIndex < uiNumRuns; uiRunIndex++)
	{
		pcRun = pText->GetRun(uiRunIndex);
		pcFont = pcRun->GetFont();
		if (pcFont->GetLineHeight() > miRowHeight)
		{
			miRowHeight = pcFont->GetLineHeight();
		}
		uiNumElements = pcRun->NumElements();
		for (uiElementIndex = 0; uiElementIndex < uiNumElements; uiElementIndex++)
		{
			pcTextElement = pcRun->GetElement(uiElementIndex);
			if (pcTextElement->IsUTF16Short())
			{
				Layout(pcFont, (CTextUTF16Short*)pcTextElement);
			}
			else if (pcTextElement->IsUTF16Long())
			{
				Layout(pcFont, (CTextUTF16Long*)pcTextElement);
			}
			else if (pcTextElement->IsUTF16Multi())
			{
				pcUTFMulti = (CTextUTF16Multi*)pcTextElement;
				Layout(pcFont, pcUTFMulti);
			}
			else if (pcTextElement->IsNewLine())
			{
				NewLine();
			}
		}
	}
	mcUTF16.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Layout(CFont* pcFont, CTextUTF16Short* pcUTFShort)
{
	size				uiCharIndex;
	uint16*				puiInt16;
	size				uiNumChars;
	uint16				uiCodePoint16;
	uint32				uiCodePoint32;
	size				uiCodePointLength;
	Ptr<CGlyph>			pGlyph;

	puiInt16 = pcUTFShort->GetChars();
	uiNumChars = pcUTFShort->NumChars();
	mcUTF16.Reset(puiInt16, uiNumChars);
	for (uiCharIndex = 0; uiCharIndex < uiNumChars; uiCharIndex++)
	{
		uiCodePoint16 = mcUTF16.GetCodePointUint16();
		if (uiCodePoint16 != 0)
		{
			if (!mcUTF16.IsTooSmall(uiCodePoint16))
			{
				uiCodePointLength = mcUTF16.GetUnicodeCodePointLength(uiCodePoint16);
				pGlyph = pcFont->GetGlyph(uiCodePoint16, uiCodePointLength);
				Glyph(pGlyph);
			}
			else
			{
				uiCodePoint32 = mcUTF16.GetCodePointUint32();
				if (!mcUTF16.IsError(uiCodePoint32))
				{
					uiCodePointLength = mcUTF16.GetUnicodeCodePointLength(uiCodePoint32);
					pGlyph = pcFont->GetGlyph(uiCodePoint32, uiCodePointLength);
					Glyph(pGlyph);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Layout(CFont* pcFont, CTextUTF16Long* pcUTFLong)
{
	size				uiCharIndex;
	uint32*				puiInt32;
	size				uiNumChars;
	uint32				uiCodePoint32;
	size				uiCodePointLength;
	Ptr<CGlyph>			pGlyph;

	puiInt32 = pcUTFLong->GetChars();
	uiNumChars = pcUTFLong->NumChars();
	mcUTF16.Reset(puiInt32, uiNumChars);
	for (uiCharIndex = 0; uiCharIndex < uiNumChars; uiCharIndex++)
	{
		uiCodePoint32 = mcUTF16.GetCodePointUint32();
		if (!mcUTF16.IsError(uiCodePoint32))
		{
			uiCodePointLength = mcUTF16.GetUnicodeCodePointLength(uiCodePoint32);
			pGlyph = pcFont->GetGlyph(uiCodePoint32, uiCodePointLength);
			Glyph(pGlyph);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Layout(CFont* pcFont, CTextUTF16Multi* pcUTFMulti)
{
	uint8*				puiInt8;
	size				uiByteLength;
	size				uiCodePointLength;
	Ptr<CGlyph>			pGlyph;
	uint8				auiBuffer[20];

	puiInt8 = pcUTFMulti->GetChar();
	uiByteLength = pcUTFMulti->CharByteLength();
	mcUTF16.Reset(puiInt8, uiByteLength);
	uiCodePointLength = mcUTF16.GetCodePointMulti(auiBuffer, 20);
	if (!mcUTF16.IsError(uiCodePointLength))
	{
		pGlyph = pcFont->GetGlyph(auiBuffer, uiCodePointLength);
		Glyph(pGlyph);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Glyph(Ptr<CGlyph> pGlyph)
{
	if (pGlyph.IsNotNull())
	{
		mpcDraw->Place(pGlyph, msPosition.x, msPosition.y);
		msPosition.x += pGlyph->GetFullWidth();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::NewLine(void)
{
	msPosition.y += miRowHeight;
	msPosition.x = 0;
	miRowHeight = 0;
}

