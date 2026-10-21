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
	msSize.Init(0, 0);
	miWordWidth = 0;

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
	U_Int16(miFontLineHeight);
	U_2Int32(msPosition);
	U_2Int32(msSize);
	U_Data(CUTF16, mcUTF16);
	U_Pointer(mpcDraw);
	U_Data(CArrayTextLayoutGlyph, masWord);
	U_Int32(miWordWidth);

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
	miFontLineHeight = 0;
	msPosition.Init(0, 0);
	msSize.Init(0, 0);
	mpcDraw = &pDraw;
	mcUTF16.Init(NULL, 0);
	masWord.Init();
	miWordWidth = 0;
	pcFont = NULL;
	uiNumRuns = pText->NumRuns();
	for (uiRunIndex = 0; uiRunIndex < uiNumRuns; uiRunIndex++)
	{
		pcRun = pText->GetRun(uiRunIndex);
		pcFont = pcRun->GetFont();
		miFontLineHeight = pcFont->GetLineHeight();
		if (miFontLineHeight > miRowHeight)
		{
			miRowHeight = miFontLineHeight;
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
				EndWord();
				NewLine();
			}
		}
	}
	EndWord();
	msSize.y = msPosition.y + miRowHeight;
	masWord.Kill();
	mcUTF16.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the widest line and the height of all the lines in the
// text last laid out.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextLayout::GetSize(void)
{
	return msSize;
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
				Glyph(pGlyph, uiCodePoint16 == UNICODE_SPACE);
			}
			else
			{
				uiCodePoint32 = mcUTF16.GetCodePointUint32();
				if (!mcUTF16.IsError(uiCodePoint32))
				{
					uiCodePointLength = mcUTF16.GetUnicodeCodePointLength(uiCodePoint32);
					pGlyph = pcFont->GetGlyph(uiCodePoint32, uiCodePointLength);
					Glyph(pGlyph, uiCodePoint32 == UNICODE_SPACE);
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
			Glyph(pGlyph, uiCodePoint32 == UNICODE_SPACE);
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
		Glyph(pGlyph, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Without a width a glyph is placed straight away.  With a width the
// glyphs of a word are held until the word ends so that the whole word
// can go on the next line if it doesn't fit on this one.  Spaces end words
// and are never moved to the next line.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::Glyph(Ptr<CGlyph> pGlyph, bool bSpace)
{
	STextLayoutGlyph*	psWordGlyph;

	if (pGlyph.IsNotNull())
	{
		if (miWidth < 0)
		{
			PlaceGlyph(pGlyph, miFontLineHeight, true);
		}
		else if (bSpace)
		{
			EndWord();

			//Spaces at the end of a line don't make the text any wider.
			PlaceGlyph(pGlyph, miFontLineHeight, false);
		}
		else
		{
			psWordGlyph = masWord.Add();
			psWordGlyph->pcGlyph = &pGlyph;
			psWordGlyph->iLineHeight = miFontLineHeight;
			miWordWidth += pGlyph->GetFullWidth();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Places the word being held.  A word that doesn't fit after what is
// already on the line starts the next line.  A word too wide for a line by
// itself is broken between glyphs.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::EndWord(void)
{
	size				uiNumGlyphs;
	size				ui;
	STextLayoutGlyph*	psWordGlyph;
	Ptr<CGlyph>			pGlyph;

	uiNumGlyphs = masWord.NumElements();
	if (uiNumGlyphs == 0)
	{
		return;
	}

	if ((msPosition.x > 0) && (msPosition.x + miWordWidth > miWidth))
	{
		NewLine();
	}

	for (ui = 0; ui < uiNumGlyphs; ui++)
	{
		psWordGlyph = masWord.Get(ui);
		pGlyph = psWordGlyph->pcGlyph;
		if ((msPosition.x > 0) && (msPosition.x + pGlyph->GetFullWidth() > miWidth))
		{
			NewLine();
		}
		PlaceGlyph(pGlyph, psWordGlyph->iLineHeight, true);
	}

	masWord.ReInit();
	miWordWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::PlaceGlyph(Ptr<CGlyph> pGlyph, int16 iLineHeight, bool bSize)
{
	mpcDraw->Place(pGlyph, msPosition.x, msPosition.y);
	msPosition.x += pGlyph->GetFullWidth();
	if (bSize && (msPosition.x > msSize.x))
	{
		msSize.x = msPosition.x;
	}
	if (iLineHeight > miRowHeight)
	{
		miRowHeight = iLineHeight;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An empty line is as high as the font of the run it is in.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::NewLine(void)
{
	msPosition.y += miRowHeight;
	msPosition.x = 0;
	miRowHeight = miFontLineHeight;
}


//////////////////////////////////////////////////////////////////////////
//
// The width lines are wrapped to, -1 for lines only to end at line feeds.
// Text must be laid out again for a new width to change it.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::SetWidth(int32 iWidth)
{
	miWidth = iWidth;
}


int32 CTextLayout::GetWidth(void) { return miWidth; }

