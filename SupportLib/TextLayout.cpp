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
	U_Int16(miRowAscent);
	U_Int16(miRowDescent);
	U_Int16(miFontAscent);
	U_Int16(miFontDescent);
	U_2Int32(msPosition);
	U_2Int32(msSize);
	U_Data(CUTF16, mcUTF16);
	U_Pointer(mpcDraw);
	U_Data(CArrayTextLayoutGlyph, masWord);
	U_Data(CArrayTextLayoutLineGlyph, masLine);
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
//
// Runs in different fonts share a baseline on each line.  Each line's
// glyphs are drawn when the line ends, once its ascent is known.
//
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

	miRowAscent = 0;
	miRowDescent = 0;
	miFontAscent = 0;
	miFontDescent = 0;
	msPosition.Init(0, 0);
	msSize.Init(0, 0);
	mpcDraw = &pDraw;
	mcUTF16.Init(NULL, 0);
	masWord.Init();
	masLine.Init();
	miWordWidth = 0;
	pcFont = NULL;
	uiNumRuns = pText->NumRuns();
	for (uiRunIndex = 0; uiRunIndex < uiNumRuns; uiRunIndex++)
	{
		pcRun = pText->GetRun(uiRunIndex);
		pcFont = pcRun->GetFont();
		//A run moved up reaches higher above the baseline and less far below it.
		miFontAscent = pcFont->GetAscent() + pcRun->GetBaselineShift();
		miFontDescent = pcFont->GetDescent() - pcRun->GetBaselineShift();
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
	EndLine();
	msSize.y = msPosition.y + miRowAscent + miRowDescent;
	masLine.Kill();
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
	if (!mcUTF16.IsError((uint32)uiCodePointLength))
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
			PlaceGlyph(pGlyph, miFontAscent, miFontDescent, true);
		}
		else if (bSpace)
		{
			EndWord();

			//Spaces at the end of a line don't make the text any wider.
			PlaceGlyph(pGlyph, miFontAscent, miFontDescent, false);
		}
		else
		{
			psWordGlyph = masWord.Add();
			psWordGlyph->pcGlyph = &pGlyph;
			psWordGlyph->iAscent = miFontAscent;
			psWordGlyph->iDescent = miFontDescent;
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
		PlaceGlyph(pGlyph, psWordGlyph->iAscent, psWordGlyph->iDescent, true);
	}

	masWord.ReInit();
	miWordWidth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::PlaceGlyph(Ptr<CGlyph> pGlyph, int16 iAscent, int16 iDescent, bool bSize)
{
	STextLayoutLineGlyph*	psLineGlyph;

	psLineGlyph = masLine.Add();
	psLineGlyph->pcGlyph = &pGlyph;
	psLineGlyph->x = msPosition.x;
	psLineGlyph->iAscent = iAscent;

	msPosition.x += pGlyph->GetFullWidth();
	if (bSize && (msPosition.x > msSize.x))
	{
		msSize.x = msPosition.x;
	}
	if (iAscent > miRowAscent)
	{
		miRowAscent = iAscent;
	}
	if (iDescent > miRowDescent)
	{
		miRowDescent = iDescent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the glyphs on the line with their baselines on the line's
// baseline: the line's ascent below its top.  A line with no glyphs is
// the height of the font it ended in.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::EndLine(void)
{
	size					ui;
	STextLayoutLineGlyph*	psLineGlyph;
	int32					iBaseline;
	Ptr<CGlyph>				pGlyph;

	if (masLine.NumElements() == 0)
	{
		miRowAscent = miFontAscent;
		miRowDescent = miFontDescent;
	}

	iBaseline = msPosition.y + miRowAscent;
	for (ui = 0; ui < masLine.NumElements(); ui++)
	{
		psLineGlyph = masLine.Get(ui);
		pGlyph = psLineGlyph->pcGlyph;
		mpcDraw->Place(pGlyph, psLineGlyph->x, iBaseline - psLineGlyph->iAscent);
	}
	masLine.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A line is as high as the largest ascent plus the largest descent of the
// glyphs on it.  An empty line is as high as the font of the run it is in.
//
//////////////////////////////////////////////////////////////////////////
void CTextLayout::NewLine(void)
{
	EndLine();
	msPosition.y += miRowAscent + miRowDescent;
	msPosition.x = 0;
	miRowAscent = 0;
	miRowDescent = 0;
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

