/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "BaseLib/UTF8.h"
#include "TextAreaLayout.h"


//////////////////////////////////////////////////////////////////////////
//
// pcDocument is the text area's: its spans and gaps follow its edits.
// Nothing is read or laid out until ReadText and LayoutText.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::Init(CTextDocument* pcDocument, Ptr<CFontFamily> pFamily)
{
	PreInit();

	mpcDocument = pcDocument;
	mpFamily = pFamily;
	msStyle.Init();
	mcSpans.Init();
	mcGaps.Init();
	pcDocument->AddListener(&mcSpans);
	pcDocument->AddListener(&mcGaps);
	mbTextDirty = false;
	mbSizeDirty = false;
	mbWordWrap = false;
	miWidth = 0;
	muiTabWidth = 4;
	msTextSize.Init(0, 0);
	mauiStopStarts.Init();

	mpText = NULL;
	mbTextStale = false;
	mpTextLayout = OMalloc<CTextLayout>();
	mpSpriteMap = OMalloc<CSpriteMap>();
	mpGeometryMap = OMalloc<CGeometryMap>();
	mpFontDraw = OMalloc<CMapFontDraw>(mpSpriteMap, mpGeometryMap);
	mbTiledText = false;
	mbTiled = false;
	mpTiles = OMalloc<CTextAreaTiles>(pcDocument, &mauiStopStarts, &mcGaps, &msStyle);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::Class(void)
{
	U_Pointer(mpcDocument);
	M_Pointer(mpFamily);
	U_Data(STextStyle, msStyle);
	U_Unknown(CTextSpans, mcSpans);
	U_Unknown(CTextLineGaps, mcGaps);
	U_Bool(mbTextDirty);
	U_Bool(mbSizeDirty);
	U_Bool(mbWordWrap);
	U_Int32(miWidth);
	U_Size(muiTabWidth);
	U_2Int32(msTextSize);
	U_Unknown(CArrayTextAreaStopStart, mauiStopStarts);
	M_Pointer(mpText);
	U_Bool(mbTextStale);
	M_Pointer(mpTextLayout);
	M_Pointer(mpAppendLayout);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpGeometryMap);
	M_Pointer(mpFontDraw);
	U_Bool(mbTiledText);
	U_Bool(mbTiled);
	M_Pointer(mpTiles);
}


//////////////////////////////////////////////////////////////////////////
//
// The document isn't touched: it may have gone already.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::Free(void)
{
	mauiStopStarts.Kill();
	mcGaps.Kill();
	mcSpans.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The text needs reading again in the new family (ReadText).
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::SetFontFamily(Ptr<CFontFamily> pFamily)
{
	mpFamily = pFamily;
	mbTextDirty = true;
	mbSizeDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The style of text that isn't in a span, or where spans don't set a
// property.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::SetStyle(STextStyle* psStyle)
{
	if (!msStyle.EqualsFont(psStyle))
	{
		mbSizeDirty = true;
	}
	msStyle = *psStyle;
	mbTextDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of text that isn't in a span that sets a colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::SetColour(ARGB32 sColour)
{
	if (msStyle.sColour != sColour)
	{
		msStyle.sColour = sColour;
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiStart and uiLength are in bytes of the UTF8 text.  A span that starts
// or ends inside a character starts or ends at the next character.
// Returns false if the span goes past the end of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride)
{
	size	uiTextLength;

	uiTextLength = mpcDocument->Length();
	if ((uiStart > uiTextLength) || (uiLength > uiTextLength - uiStart))
	{
		return false;
	}
	if ((uiLength == 0) || (psOverride->uiSet == 0))
	{
		return true;
	}

	mcSpans.Add(uiStart, uiLength, psOverride);
	mbTextDirty = true;
	if (psOverride->ChangesFont())
	{
		mbSizeDirty = true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::ClearSpans(void)
{
	if (mcSpans.ChangesFont())
	{
		mbSizeDirty = true;
	}
	if (mcSpans.NumSpans() > 0)
	{
		mcSpans.Clear();
		mbTextDirty = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like spans, the gaps are laid out when the text is next laid out, so
// changing many is laid out once.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::GapsChanged(void)
{
	mbTextDirty = true;
	mbSizeDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if it changed: the text needs reading again (ReadText).
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::SetWordWrap(bool bWordWrap)
{
	if (mbWordWrap == bWordWrap)
	{
		return false;
	}
	mbWordWrap = bWordWrap;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Draws a fixed width font as tiles, each character in a cell of a tile
// layer, instead of as sprites.  Only used without word wrap.  True if it
// changed: the text needs reading again (ReadText).
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::SetTiledText(bool bTiledText)
{
	if (mbTiledText == bTiledText)
	{
		return false;
	}
	mbTiledText = bTiledText;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How many spaces apart the tab stops are.  The text needs laying out
// again, and reading again as tiles.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::SetTabWidth(size uiTabWidth)
{
	muiTabWidth = uiTabWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// The width the text area is laid out to: word wrapped lines are wrapped
// to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::SetWidth(int32 iWidth)
{
	miWidth = iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// Sprites laid out to another width than the lines are wrapped to now
// need laying out again.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::IsWrapWidthChanged(void)
{
	return !mbTiled && (mpTextLayout->GetWidth() != GetWrapWidth());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontFamily> CTextAreaLayout::GetFontFamily(void) { return mpFamily; }
STextStyle* CTextAreaLayout::GetStyle(void) { return &msStyle; }
size CTextAreaLayout::NumSpans(void) { return mcSpans.NumSpans(); }
CTextLineGaps* CTextAreaLayout::GetGaps(void) { return &mcGaps; }  //Changing them, call GapsChanged.
bool CTextAreaLayout::IsWordWrap(void) { return mbWordWrap; }
bool CTextAreaLayout::IsTiledText(void) { return mbTiledText; }
bool CTextAreaLayout::IsTiled(void) { return mbTiled; }  //Tiles were asked for and the font and word wrap allow them.
bool CTextAreaLayout::IsTextDirty(void) { return mbTextDirty; }
SInt32Vec2 CTextAreaLayout::GetTextSize(void) { return msTextSize; }  //As it was last laid out; UpdateSize first.
Ptr<CSpriteMap> CTextAreaLayout::GetSpriteMap(void) { return mpSpriteMap; }
Ptr<CGeometryMap> CTextAreaLayout::GetGeometryMap(void) { return mpGeometryMap; }
Ptr<CTileMap> CTextAreaLayout::GetTileMap(void) { return mpTiles->GetMap(); }


//////////////////////////////////////////////////////////////////////////
//
// The document's text was set: the spans and gaps were of the old text.
// It is read and laid out.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::TextSet(void)
{
	mcSpans.Clear();
	mcGaps.Clear();
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds szText (UTF8) to the end of the document's text in sColour.  Only
// what was added (and the line it joins) is read and laid out, as tiles
// or as sprites, so appending to a long text, an output pane say, costs
// what is added, not the whole text.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::Append(const char* szText, ARGB32 sColour)
{
	size				uiStart;
	bool				bAppendTiles;
	bool				bAppendSprites;
	STextStyleOverride	sOverride;

	bAppendTiles = CanAppendTiles();
	bAppendSprites = !bAppendTiles && CanAppendSprites();
	uiStart = mpcDocument->Length();
	mpcDocument->Append(szText);
	sOverride.Init();
	sOverride.SetColour(sColour);
	AddSpan(uiStart, mpcDocument->Length() - uiStart, &sOverride);
	if (bAppendTiles)
	{
		AppendTiles(uiStart);
		mbTextDirty = false;
		LayoutTiles();
	}
	else if (bAppendSprites)
	{
		AppendSprites(uiStart);
		mbTextDirty = false;
	}
	else
	{
		ReadText();
		LayoutText();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the text from the UTF8 in the fonts of its styles.  Without a font
// for the text area's style there is no text (the family logs why).
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::ReadText(void)
{
	mbTextDirty = false;
	mbSizeDirty = false;
	mbTextStale = false;
	FindStopStarts(0);

	mbTiled = UseTiles();
	if (mbTiled)
	{
		ReadTiles();
		return;
	}
	mpTiles->ClearTiles();

	mpText = ReadTextFrom(0);
}


//////////////////////////////////////////////////////////////////////////
//
// The style of each byte of the text from uiFrom to uiTo (exclusive, the
// first is uiFrom's): the text area's style with the spans over it,
// applied in the order they were added.  NULL if there are no spans, when
// every byte is in the text area's style.  Free it.
//
//////////////////////////////////////////////////////////////////////////
STextStyle* CTextAreaLayout::MakeStyles(size uiFrom, size uiTo)
{
	size		uiPosition;
	STextStyle*	psStyles;

	if (uiTo > mpcDocument->Length())
	{
		uiTo = mpcDocument->Length();
	}
	if ((mcSpans.NumSpans() == 0) || (uiTo <= uiFrom))
	{
		return NULL;
	}

	psStyles = (STextStyle*)malloc((uiTo - uiFrom) * sizeof(STextStyle));
	for (uiPosition = uiFrom; uiPosition < uiTo; uiPosition++)
	{
		psStyles[uiPosition - uiFrom] = msStyle;
	}
	mcSpans.Apply(psStyles, uiFrom, uiTo);
	return psStyles;
}


//////////////////////////////////////////////////////////////////////////
//
// The styles of the text from uiFrom to its end (see MakeStyles(uiFrom,
// uiTo)).
//
//////////////////////////////////////////////////////////////////////////
STextStyle* CTextAreaLayout::MakeStyles(size uiFrom)
{
	return MakeStyles(uiFrom, mpcDocument->Length());
}


//////////////////////////////////////////////////////////////////////////
//
// The text area's style, or a span, underlines, overlines or strikes
// through some of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::HasDecorations(void)
{
	if (msStyle.GetDecorations() != 0)
	{
		return true;
	}
	return mcSpans.HasDecorations(&msStyle);
}


//////////////////////////////////////////////////////////////////////////
//
// A font change is made wherever the style of the text from uiFrom on
// (MakeStyles) changes, at its byte after uiFrom.  Text before the first
// change is in pDefaultFont.  A style the family has no font for is drawn
// in pDefaultFont (the family logs an error).
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift, size uiFrom)
{
	size				uiTextLength;
	size				uiPosition;
	STextStyle*			psStyles;
	STextStyle*			psCurrent;
	Ptr<CFont>			pFont;
	int16				iBaselineShift;

	psStyles = MakeStyles(uiFrom);
	if (psStyles == NULL)
	{
		return;
	}

	uiTextLength = mpcDocument->Length() - uiFrom;
	psCurrent = &msStyle;
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		if (!psStyles[uiPosition].Equals(psCurrent))
		{
			pFont = mpFamily->GetRunFont(&psStyles[uiPosition], &iBaselineShift);
			if (pFont.IsNull())
			{
				pFont = pDefaultFont;
				iBaselineShift = iDefaultBaselineShift;
			}
			pasChanges->Add()->Init(uiPosition, &pFont, iBaselineShift, psStyles[uiPosition].GetDecorations(), psStyles[uiPosition].sColour);
			psCurrent = &psStyles[uiPosition];
		}
	}
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph, wrapped to the actual width if word wrap
// is on, and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::LayoutText(void)
{
	Ptr<CFont>			pFont;
	STextLayoutLine*	psLine;

	if (mbTextStale)
	{
		//Text was appended since the whole was read: it is all laid out again, so it is all read again.
		ReadText();
	}
	if (mbTiled)
	{
		LayoutTiles();
		return;
	}

	mpTextLayout->SetWidth(GetWrapWidth());
	mpTextLayout->SetLineGaps(mcGaps.GetGaps());
	mpTextLayout->SetTabWidth(GetTabPixels());

	mpFontDraw->Begin();
	if (mpText.IsNotNull())
	{
		mpTextLayout->Layout(mpFontDraw, mpText);
		msTextSize = mpTextLayout->GetSize();
	}
	else
	{
		msTextSize.Init(0, 0);
	}
	mpFontDraw->End();

	//Even a text area with no text is a line high, above its gap.  With no characters the layout had no font to make
	//its line high.
	pFont = GetFont();
	if (pFont.IsNotNull() && (mpcDocument->Length() == 0) && mpText.IsNotNull() && (mpTextLayout->NumLines() == 1))
	{
		psLine = mpTextLayout->GetLine(0);
		psLine->iHeight = pFont->GetLineHeight();
		psLine->iBaseline = pFont->GetAscent();
		msTextSize.y = psLine->iHeight + mcGaps.GetHeight(0);
	}
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text is read and laid out again if its spans, style or gaps changed
// since it last was.  True if it was.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::UpdateText(void)
{
	if (!mbTextDirty)
	{
		return false;
	}
	ReadText();
	LayoutText();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Before the size is asked for, spans or a style that can change it are
// applied.  True if the text was laid out again.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::UpdateSize(void)
{
	return mbSizeDirty && UpdateText();
}


//////////////////////////////////////////////////////////////////////////
//
// The width lines are wrapped to, -1 if they aren't.  A text area that
// hasn't been laid out yet has no width (SetWidth) so its lines aren't
// wrapped.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextAreaLayout::GetWrapWidth(void)
{
	int32	iWidth;

	iWidth = miWidth;
	if (mbWordWrap && (iWidth > 0))
	{
		return iWidth;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels between tab stops when the text is laid out as sprites: the tab
// width's spaces of the font.  0 (tabs have no width) without a font or a
// space in it.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextAreaLayout::GetTabPixels(void)
{
	Ptr<CFont>		pFont;
	Ptr<CGlyph>		pSpace;

	pFont = GetFont();
	if (pFont.IsNull())
	{
		return 0;
	}
	pSpace = pFont->GetGlyph((uint16)' ', 1);
	if (pSpace.IsNull())
	{
		return 0;
	}
	return (int32)muiTabWidth * pSpace->GetFullWidth();
}


//////////////////////////////////////////////////////////////////////////
//
// The colours of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CTextAreaLayout::GetFontColours(void)
{
	return mpFamily->GetColours(msStyle.GetFontStyle(), msStyle.fPixelSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The white font of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTextAreaLayout::GetFont(void)
{
	Ptr<CFontColours>	pColours;

	pColours = GetFontColours();
	if (pColours.IsNull())
	{
		return NULL;
	}
	return pColours->GetWhite();
}


//////////////////////////////////////////////////////////////////////////
//
// The byte each of the layout's stops is before, found the way the text
// is read from uiFrom on (0 for all of it; else the old end, when text
// was appended): a stop for each character but carriage returns, then one
// at the end of the text.  Stops found before at or after uiFrom go.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::FindStopStarts(size uiFrom)
{
	size	uiPosition;
	size	uiLength;
	char*	szText;

	while ((mauiStopStarts.NumElements() > 0) && (*mauiStopStarts.Tail() >= uiFrom))
	{
		mauiStopStarts.RemoveTail();
	}
	szText = mpcDocument->Text();
	uiLength = mpcDocument->Length();
	uiPosition = uiFrom;
	while (uiPosition < uiLength)
	{
		if (szText[uiPosition] != '\r')
		{
			*mauiStopStarts.Add() = uiPosition;
		}
		uiPosition += mpcDocument->GetNavigation()->CharacterLength(uiPosition);
	}
	*mauiStopStarts.Add() = uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The row line uiLine ends on: the row of its line feed, or the last row.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::FindLastRow(size uiLine)
{
	if (uiLine + 1 < mpcDocument->NumLines())
	{
		return GetLayoutStop(FindStop(mpcDocument->GetLineStart(uiLine + 1) - 1))->uiLine;
	}
	return NumLayoutLines() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the layout is of the text as it is.  Without a font there is no
// layout, and the caret and selection aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::HasRows(void)
{
	return (mbTiled || mpText.IsNotNull()) && (NumLayoutLines() > 0) && (NumLayoutStops() == mauiStopStarts.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// The first stop at or after uiPosition.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::FindStop(size uiPosition)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = mauiStopStarts.NumElements() - 1;
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (*mauiStopStarts.Get(uiMiddle) < uiPosition)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// A row ends where a word didn't fit on it, not at a line feed or the end
// of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::IsRowWrapped(size uiRow)
{
	STextLayoutLine*	psLine;
	size				uiLast;

	if (uiRow + 1 >= NumLayoutLines())
	{
		return false;
	}
	psLine = GetLayoutLine(uiRow);
	uiLast = psLine->uiFirstStop + psLine->uiNumStops - 1;
	return mpcDocument->Text()[*mauiStopStarts.Get(uiLast)] != '\n';
}


//////////////////////////////////////////////////////////////////////////
//
// The row uiPosition is on.  At the start of a wrapped row and bRowEnd it
// is the end of the row above.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::FindRow(size uiPosition, bool bRowEnd)
{
	size	uiRow;

	uiRow = GetLayoutStop(FindStop(uiPosition))->uiLine;
	if (bRowEnd && (uiRow > 0) && (GetRowStart(uiRow) == uiPosition) && IsRowWrapped(uiRow - 1))
	{
		uiRow--;
	}
	return uiRow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::GetRowStart(size uiRow)
{
	return *mauiStopStarts.Get(GetLayoutLine(uiRow)->uiFirstStop);
}


//////////////////////////////////////////////////////////////////////////
//
// The last position on the row: before its line feed, at the end of the
// text, or for a wrapped row the start of the next row with pbRowEnd true.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::GetRowEnd(size uiRow, bool* pbRowEnd)
{
	STextLayoutLine*	psLine;

	*pbRowEnd = false;
	if (uiRow + 1 >= NumLayoutLines())
	{
		return mpcDocument->Length();
	}

	psLine = GetLayoutLine(uiRow);
	if (IsRowWrapped(uiRow))
	{
		*pbRowEnd = true;
		return *mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops);
	}
	return mpcDocument->GetNavigation()->FindLineEnd(*mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The x of the row's last position.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextAreaLayout::GetRowRight(size uiRow)
{
	STextLayoutLine*	psLine;
	STextLayoutStop*	psStop;

	psLine = GetLayoutLine(uiRow);
	psStop = GetLayoutStop(psLine->uiFirstStop + psLine->uiNumStops - 1);
	if (IsRowWrapped(uiRow))
	{
		return psStop->x + psStop->iWidth;
	}
	return psStop->x;
}


//////////////////////////////////////////////////////////////////////////
//
// The row y (relative to the text area's top) is in.  Above the text it
// is the first, below it the last.  In a line's gap it is the row above.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::FindRowAt(int32 y)
{
	size				uiRow;
	size				uiNumRows;

	uiNumRows = NumLayoutLines();
	for (uiRow = 0; uiRow < uiNumRows - 1; uiRow++)
	{
		if (y < GetLayoutLine(uiRow + 1)->iTop)
		{
			break;
		}
	}
	return uiRow;
}


//////////////////////////////////////////////////////////////////////////
//
// The position on the row nearest x.  The end of a wrapped row is the
// start of the next with pbRowEnd true.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd)
{
	STextLayoutLine*	psLine;
	size				uiStop;
	size				uiEnd;
	size				uiBest;
	int32				iBestDistance;
	int32				iDistance;
	bool				bRowEnd;

	psLine = GetLayoutLine(uiRow);
	uiEnd = psLine->uiFirstStop + psLine->uiNumStops;
	uiBest = psLine->uiFirstStop;
	iBestDistance = abs(GetLayoutStop(uiBest)->x - x);
	for (uiStop = uiBest + 1; uiStop < uiEnd; uiStop++)
	{
		iDistance = abs(GetLayoutStop(uiStop)->x - x);
		if (iDistance < iBestDistance)
		{
			uiBest = uiStop;
			iBestDistance = iDistance;
		}
	}

	*pbRowEnd = false;
	if (IsRowWrapped(uiRow) && (abs(GetRowRight(uiRow) - x) < iBestDistance))
	{
		return GetRowEnd(uiRow, pbRowEnd);
	}
	if (!IsRowWrapped(uiRow) && (uiBest == uiEnd - 1))
	{
		//The last stop of a row that isn't wrapped is its line feed or the end of the text.
		return GetRowEnd(uiRow, &bRowEnd);
	}
	return *mauiStopStarts.Get(uiBest);
}


//////////////////////////////////////////////////////////////////////////
//
// The x of the caret at uiPosition, relative to the text area's left.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextAreaLayout::GetPositionX(size uiPosition, bool bRowEnd)
{
	size	uiStop;
	size	uiRow;

	uiStop = FindStop(uiPosition);
	uiRow = FindRow(uiPosition, bRowEnd);
	if (uiRow != GetLayoutStop(uiStop)->uiLine)
	{
		return GetRowRight(uiRow);
	}
	return GetLayoutStop(uiStop)->x;
}


//////////////////////////////////////////////////////////////////////////
//
// The width of a space, to show a line feed is selected.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextAreaLayout::GetNewLineWidth(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	if (pFont.IsNull())
	{
		return 0;
	}
	return (int32)pFont->CalculateWidth((char*)" ");
}


//////////////////////////////////////////////////////////////////////////
//
// The top of line uiLine's first row and the bottom of its last, above
// its gap.  Text with no characters has its one line laid out without
// stops.  False if the line isn't laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::GetLineRows(size uiLine, int32* piTop, int32* piBottom)
{
	STextLayoutLine*	psFirst;
	STextLayoutLine*	psLast;

	if (uiLine >= mpcDocument->NumLines())
	{
		return false;
	}
	if (HasRows())
	{
		psFirst = GetLayoutLine(GetLayoutStop(FindStop(mpcDocument->GetLineStart(uiLine)))->uiLine);
		psLast = GetLayoutLine(FindLastRow(uiLine));
	}
	else if ((uiLine == 0) && (NumLayoutLines() > 0))
	{
		psFirst = GetLayoutLine(0);
		psLast = psFirst;
	}
	else
	{
		return false;
	}

	*piTop = psFirst->iTop;
	*piBottom = psLast->iTop + psLast->iHeight;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::UseTiles(void)
{
	Ptr<CFont>	pFont;

	if (!mbTiledText || mbWordWrap || mpFamily.IsNull())
	{
		return false;
	}
	pFont = GetFont();
	return pFont.IsNotNull() && pFont->IsFixedWidth();
}


//////////////////////////////////////////////////////////////////////////
//
// The text read as tiles (CTextAreaTiles) in the colours of the spans,
// and its underlines, overlines and strike throughs drawn over them.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::ReadTiles(void)
{
	STextStyle*	psStyles;

	//The sprites of the text read before are gone.
	mpText = NULL;

	psStyles = MakeStyles();
	mpTiles->Read(GetFontColours(), (int32)muiTabWidth, psStyles, &msTextSize);

	//The lines along the text are drawn over the tiles, so they can go past a cell into the ones next to it.
	mpFontDraw->Begin();
	mpTiles->Decorate(mpFontDraw, GetFont()->GetDescription(), psStyles);
	mpFontDraw->End();
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom on (0 for all of it) in the fonts of its styles,
// its font changes from uiFrom.  NULL without a font for the text area's
// style.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CText> CTextAreaLayout::ReadTextFrom(size uiFrom)
{
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	CArrayTextFontChange	asChanges;
	Ptr<CFont>				pFont;
	Ptr<CText>				pText;
	int16					iBaselineShift;

	pFont = mpFamily->GetRunFont(&msStyle, &iBaselineShift);
	if (pFont.IsNull())
	{
		return NULL;
	}

	//The reader's font has no baseline shift or decorations so a style with them needs a change at the start.
	asChanges.Init();
	if ((iBaselineShift != 0) || (msStyle.GetDecorations() != 0))
	{
		asChanges.Add()->Init(0, &pFont, iBaselineShift, msStyle.GetDecorations(), msStyle.sColour);
	}
	MakeFontChanges(&asChanges, pFont, iBaselineShift, uiFrom);

	cUTF8.Init(&mpcDocument->Text()[uiFrom]);
	cReader.Init(&pFont);
	pText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();
	return pText;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether text appended can be laid out from the start of the old last
// line on rather than the whole text laid out again: it is laid out as
// sprites, up to date, at the width it is laid out to now, and nothing is
// decorated (decorations are drawn over the whole text).
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::CanAppendSprites(void)
{
	if (mbTiled || mbTextDirty || mbSizeDirty || mpText.IsNull() || UseTiles())
	{
		return false;
	}
	if ((mpTextLayout->GetWidth() != GetWrapWidth()) || (mpTextLayout->NumLines() == 0) || (mpTextLayout->NumStops() != mauiStopStarts.NumElements()))
	{
		return false;
	}
	return !HasDecorations();
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom (the old end) on laid out after the lines before
// it: the old last line (from the line feed before it) and what was
// appended are read and laid out by themselves, from the top of that
// line, and replace its rows (CTextLayout::ReplaceLines) and their
// sprites.  CanAppendSprites says when.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::AppendSprites(size uiFrom)
{
	size				uiTextLine;
	size				uiTailStart;
	size				uiLayoutLine;
	size				uiSprite;
	int32				iTop;
	Ptr<CFont>			pFont;
	Ptr<CText>			pText;
	CArrayTextLayoutGap	asGaps;

	//The line the old end is on, and the row it starts.
	uiTextLine = mpcDocument->NumLines() - 1;
	while ((uiTextLine > 0) && (mpcDocument->GetLineStart(uiTextLine) > uiFrom))
	{
		uiTextLine--;
	}
	uiTailStart = mpcDocument->GetLineStart(uiTextLine);
	uiLayoutLine = mpTextLayout->GetStop(FindStop(uiTailStart))->uiLine;
	iTop = mpTextLayout->GetLine(uiLayoutLine)->iTop;
	FindStopStarts(uiFrom);

	//Its rows' sprites go: they were placed after those of the rows above, below their tops.
	uiSprite = mpSpriteMap->NumSprites();
	while ((uiSprite > 0) && (mpSpriteMap->GetSprite(uiSprite - 1)->sPosition.y >= iTop))
	{
		uiSprite--;
	}
	mpSpriteMap->RemoveSprites(uiSprite);

	pText = ReadTextFrom(uiTailStart);
	if (mpAppendLayout.IsNull())
	{
		mpAppendLayout = OMalloc<CTextLayout>();
	}
	asGaps.Init();
	mcGaps.GetGapsFrom(uiTextLine, &asGaps);
	mpAppendLayout->SetWidth(mpTextLayout->GetWidth());
	mpAppendLayout->SetLineGaps(&asGaps);
	mpAppendLayout->SetTabWidth(GetTabPixels());
	asGaps.Kill();

	mpFontDraw->SetOffset(0, iTop);
	mpFontDraw->BeginAppend();
	if (pText.IsNotNull())
	{
		mpAppendLayout->Layout(mpFontDraw, pText);
	}
	mpFontDraw->End();
	mpFontDraw->SetOffset(0, 0);
	mpTextLayout->ReplaceLines(uiLayoutLine, &mpAppendLayout);
	pText = NULL;
	mbTextStale = true;

	msTextSize = mpTextLayout->GetSize();
	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether text appended can be added to the tiles read before rather than
// the whole text read again: they are up to date, and nothing is decorated
// (decorations are drawn over the whole text).
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaLayout::CanAppendTiles(void)
{
	if (!mbTiled || mbTextDirty || mbSizeDirty || mpText.IsNotNull() || !UseTiles())
	{
		return false;
	}
	if ((mpTiles->NumStops() == 0) || (mpTiles->NumStops() != mauiStopStarts.NumElements()))
	{
		return false;
	}
	return !HasDecorations();
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom (the old end) on added to the tiles, as ReadTiles
// would read it: its stops and lines after those before, the layer made
// big enough, and only the rows of its lines written (the first of them
// the old last line, which it may go on).  CanAppendTiles says when.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::AppendTiles(size uiFrom)
{
	STextStyle*	psStyles;

	FindStopStarts(uiFrom);
	psStyles = MakeStyles(mpcDocument->GetLineStart(mpTiles->GetAppendLine()));
	mpTiles->Append(GetFontColours(), (int32)muiTabWidth, psStyles, &msTextSize);
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// The tiles were laid out when they were read.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::LayoutTiles(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lines and stops of the text as it was last laid out: by
// mpTextLayout, or from the columns when it is tiles.
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::NumLayoutLines(void)
{
	return mbTiled ? mpTiles->NumLines() : mpTextLayout->NumLines();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextAreaLayout::NumLayoutStops(void)
{
	return mbTiled ? mpTiles->NumStops() : mpTextLayout->NumStops();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextLayoutLine* CTextAreaLayout::GetLayoutLine(size uiLine)
{
	return mbTiled ? mpTiles->GetLine(uiLine) : mpTextLayout->GetLine(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextLayoutStop* CTextAreaLayout::GetLayoutStop(size uiStop)
{
	return mbTiled ? mpTiles->GetStop(uiStop) : mpTextLayout->GetStop(uiStop);
}


//////////////////////////////////////////////////////////////////////////
//
// The style of the byte at uiPosition of the UTF8 text: the text area's
// style with every span that covers it applied in the order they were
// added.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaLayout::GetStyleAt(size uiPosition, STextStyle* psStyle)
{
	*psStyle = msStyle;
	mcSpans.ApplyAt(uiPosition, psStyle);
}
