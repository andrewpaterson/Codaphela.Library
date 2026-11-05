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
#include "BaseLib/Chars.h"
#include "StandardLib/ClassDefines.h"
#include "JewelColours.h"
#include "Window.h"
#include "CodeEditor.h"


//////////////////////////////////////////////////////////////////////////
//
// szText (UTF8) is the code, in pFamily's default style.  The gutter's
// numbers are in the same font.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText)
{
	PreInit();

	maLineComponents.Init();
	mauiGapIds.Init();
	mauiLines.Init();
	muiGutterLines = 0;
	macMarks.Init();
	mbMarksChanged = false;
	masChangeBars.Init();
	mbChangeBarsChanged = false;
	msBandNumberColour = JEWEL_LINE_NUMBER_ACTIVE;
	mcGutterClicked.Init();
	mcChangeBarClicked.Init();
	mcChanged.Init();
	CContainer::Init(pWindow);
	SetSizedByContent(true);

	mpTextArea = OMalloc<CTextArea>(pWindow, pFamily, szText);
	mpTextArea->SetAlignment(CSH_Left, CSV_Top);
	muiSeenChanges = mpTextArea->GetChangeCount();

	//The gutter is only to read: a press in it passes to the code editor.
	mpGutter = OMalloc<CTextArea>(pWindow, pFamily, "");
	mpGutter->SetEditable(false);
	mpGutter->SetSelectable(false);
	mpGutter->SetCanGetFocus(false);
	mpGutter->SetColour(JEWEL_LINE_NUMBER);
	mpGutter->SetAlignment(CSH_Left, CSV_Top);
	mpGutterDraw = OMalloc<CClearCanvasDraw>(JEWEL_PANEL);
	mpGutterBackground = OMalloc<CCanvas>(pWindow, mpGutterDraw);

	//Line components are drawn over the code, and the gutter, on its background, over both: later children are
	//drawn over earlier ones and found under the pointer first.
	AddComponent(mpTextArea);
	AddComponent(mpGutterBackground);
	AddComponent(mpGutter);
	UpdateGutter();

	//A press on the gutter (or what isn't the code's) focuses the code editor, which gives the focus to its code.
	SetCanGetFocus(true);
	AddInputAction(CIC_ActivateComponent, this, &CCodeEditor::ClickAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::Free(void)
{
	mcChanged.Kill();
	mcGutterClicked.Kill();
	mcChangeBarClicked.Kill();
	masChangeBars.Kill();
	macMarks.Kill();
	mauiLines.Kill();
	mauiGapIds.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::Class(void)
{
	CContainer::Class();
	M_Pointer(mpGutter);
	M_Pointer(mpGutterBackground);
	M_Pointer(mpGutterDraw);
	M_Pointer(mpTextArea);
	M_Embedded(maLineComponents);
	U_Data(CArrayCodeEditorGapIds, mauiGapIds);
	U_Data(CArrayCodeEditorLines, mauiLines);
	U_Size(muiGutterLines);
	U_Data(CArrayCodeEditorMarks, macMarks);
	U_Bool(mbMarksChanged);
	U_Data(CArrayCodeEditorChangeBars, masChangeBars);
	U_Bool(mbChangeBarsChanged);
	U_UInt32(msBandNumberColour);
	U_Size(muiSeenChanges);
	U_Data(CGutterClickEvent, mcGutterClicked);
	U_Data(CGutterClickEvent, mcChangeBarClicked);
	U_Data(CComponentEvent<CCodeEditor>, mcChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodeEditor::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodeEditor::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of the line numbers.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::SetGutterColour(ARGB32 sColour)
{
	mpGutter->SetColour(sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the gutter as it would be laid out now.
//
//////////////////////////////////////////////////////////////////////////
int32 CCodeEditor::GetGutterWidth(void)
{
	UpdateGutter();
	return mpGutter->GetDesiredSize().x;
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent is shown under line uiLine of the code, in a gap as high as
// it wants.  False if there is no such line, the line has a component
// already or pComponent is already under a line.
//
//////////////////////////////////////////////////////////////////////////
bool CCodeEditor::AddLineComponent(size uiLine, Ptr<CComponent> pComponent)
{
	if ((uiLine >= mpTextArea->NumLines()) || (mpTextArea->GetLineGap(uiLine) > 0) || (FindLineComponent(pComponent) != MAX_SIZE))
	{
		return false;
	}

	mpTextArea->SetLineGap(uiLine, GetGapHeight(pComponent));
	maLineComponents.Add(pComponent);
	*mauiGapIds.Add() = mpTextArea->GetLineGapId(uiLine);
	*mauiLines.Add() = uiLine;
	AddComponent(pComponent);
	BringGutterToFront();

	//What it is laid out from has changed.
	InvalidateLayout();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent is no longer shown, and its line's gap closes.  False if it
// wasn't under a line.
//
//////////////////////////////////////////////////////////////////////////
bool CCodeEditor::RemoveLineComponent(Ptr<CComponent> pComponent)
{
	size	uiIndex;
	size	uiLine;

	uiIndex = FindLineComponent(pComponent);
	if (uiIndex == MAX_SIZE)
	{
		return false;
	}

	uiLine = mpTextArea->FindLineGap(*mauiGapIds.Get(uiIndex));
	if (uiLine != MAX_SIZE)
	{
		mpTextArea->SetLineGap(uiLine, 0);
	}
	RemoveLineComponentAt(uiIndex);

	//What it is laid out from has changed.
	InvalidateLayout();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets line component uiIndex and takes it out of the code editor.
// Its gap is left as it is.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::RemoveLineComponentAt(size uiIndex)
{
	Ptr<CComponent>	pComponent;

	pComponent = maLineComponents.Get(uiIndex);
	maLineComponents.RemoveAt(uiIndex);
	mauiGapIds.RemoveAt(uiIndex, true);
	mauiLines.RemoveAt(uiIndex, true);
	RemoveComponent(pComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::RemoveAllLineComponents(void)
{
	while (maLineComponents.Size() > 0)
	{
		RemoveLineComponent(maLineComponents.Get(maLineComponents.Size() - 1));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The line pComponent is under, MAX_SIZE if it isn't under one.
//
//////////////////////////////////////////////////////////////////////////
size CCodeEditor::GetLineOf(Ptr<CComponent> pComponent)
{
	size	uiIndex;

	uiIndex = FindLineComponent(pComponent);
	if (uiIndex == MAX_SIZE)
	{
		return MAX_SIZE;
	}
	return mpTextArea->FindLineGap(*mauiGapIds.Get(uiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// The component under line uiLine, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CCodeEditor::GetComponentOn(size uiLine)
{
	size	i;

	for (i = 0; i < maLineComponents.Size(); i++)
	{
		if (mpTextArea->FindLineGap(*mauiGapIds.Get(i)) == uiLine)
		{
			return maLineComponents.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The gutter and the code side by side, as high as the code with its
// gaps.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::SetRequiredSize(void)
{
	SInt32Vec2	sGutter;
	SInt32Vec2	sText;

	Update();
	sGutter = mpGutter->GetDesiredSize();
	sText = mpTextArea->GetDesiredSize();
	msRequiredSize.x = sGutter.x + ((sText.x > 0) ? sText.x : 0);
	msRequiredSize.y = (sText.y > sGutter.y) ? sText.y : sGutter.y;
}


//////////////////////////////////////////////////////////////////////////
//
// The gutter on the left, the code filling the rest of the area (or more
// if it needs it), and each line component in its line's gap.  Scrolled
// sideways in a scroll pane, the gutter is at the view's left.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32				iGutter;
	SInt32Vec2			sText;
	SInt32Vec2			sTextPosition;
	SInt32Vec2			sGutterPosition;
	SInt32Vec2			sSize;
	CInt32Rect			cView;
	size				i;
	size				uiLine;
	CInt32Rect			cGap;
	Ptr<CComponent>		pComponent;

	Update();
	SetContainerBounds(sPosition, sAreaSize);

	iGutter = mpGutter->GetDesiredSize().x;
	sText = mpTextArea->GetDesiredSize();
	sSize.x = (sText.x > sAreaSize.x - iGutter) ? sText.x : sAreaSize.x - iGutter;
	sSize.y = (sText.y > sAreaSize.y) ? sText.y : sAreaSize.y;
	sTextPosition.Init(sPosition.x + iGutter, sPosition.y);
	sGutterPosition = sPosition;
	if (GetScrollView(&cView) && (cView.GetLeft() > sGutterPosition.x))
	{
		sGutterPosition.x = cView.GetLeft();
	}
	mpGutterBackground->Layout(sGutterPosition, SInt32Vec2(iGutter, sSize.y));
	mpGutter->Layout(sGutterPosition, SInt32Vec2(iGutter, sSize.y));
	mpTextArea->Layout(sTextPosition, sSize);

	for (i = 0; i < maLineComponents.Size(); i++)
	{
		pComponent = maLineComponents.Get(i);
		uiLine = *mauiLines.Get(i);
		if (mpTextArea->GetLineGapBounds(uiLine, &cGap))
		{
			pComponent->Layout(SInt32Vec2(sTextPosition.x + cGap.GetLeft(), sTextPosition.y + cGap.GetTop()), SInt32Vec2(cGap.GetWidth(), cGap.GetHeight()));
		}
	}

	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCodeEditor::FindLineComponent(Ptr<CComponent> pComponent)
{
	size	i;

	for (i = 0; i < maLineComponents.Size(); i++)
	{
		if (&maLineComponents.Get(i) == &pComponent)
		{
			return i;
		}
	}
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
// The height of pComponent's gap: the height it wants, at least a pixel
// (a gap of nothing would be removed).
//
//////////////////////////////////////////////////////////////////////////
int32 CCodeEditor::GetGapHeight(Ptr<CComponent> pComponent)
{
	int32	iHeight;

	iHeight = pComponent->GetDesiredSize().y;
	return (iHeight > 0) ? iHeight : 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Catches up with edits to the code: the change event is called,
// line components whose lines went are removed, the others' gaps are as
// high as they want, and the gutter is numbered and gapped like the code.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::Update(void)
{
	size	uiChanges;

	uiChanges = mpTextArea->GetChangeCount();
	if (uiChanges != muiSeenChanges)
	{
		muiSeenChanges = uiChanges;
		mcChanged.Call(this);
	}
	UpdateLineComponents();
	UpdateGutter();
}


//////////////////////////////////////////////////////////////////////////
//
// Each line component's line is found from its gap, in one pass through
// the text area's gaps.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::UpdateLineComponents(void)
{
	size					i;
	size					uiIndex;
	size					uiLine;
	CArrayTextLayoutGap*	pasGaps;
	STextLayoutGap*			psGap;

	for (i = 0; i < mauiLines.NumElements(); i++)
	{
		*mauiLines.Get(i) = MAX_SIZE;
	}
	pasGaps = mpTextArea->GetLineGaps();
	for (i = 0; i < pasGaps->NumElements(); i++)
	{
		psGap = pasGaps->Get(i);
		uiIndex = FindGapId(psGap->uiId);
		if (uiIndex != MAX_SIZE)
		{
			*mauiLines.Get(uiIndex) = psGap->uiLine;
		}
	}

	//Changing a gap's height doesn't move the gaps; removing a component only changes the arrays after it.
	i = maLineComponents.Size();
	while (i > 0)
	{
		i--;
		uiLine = *mauiLines.Get(i);
		if (uiLine == MAX_SIZE)
		{
			RemoveLineComponentAt(i);
		}
		else
		{
			mpTextArea->SetLineGap(uiLine, GetGapHeight(maLineComponents.Get(i)));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the line component whose gap has id uiId, MAX_SIZE if
// none has.  The ids are ascending.
//
//////////////////////////////////////////////////////////////////////////
size CCodeEditor::FindGapId(uint32 uiId)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;
	uint32	uiMiddleId;

	uiLow = 0;
	uiHigh = mauiGapIds.NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		uiMiddleId = *mauiGapIds.Get(uiMiddle);
		if (uiMiddleId == uiId)
		{
			return uiMiddle;
		}
		if (uiMiddleId < uiId)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
// A row for each line of the code: its mark (or a space), its number
// (from 1) right aligned, and a space for the change bars.  Drawn as
// tiles if the code is.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::UpdateGutter(void)
{
	size	uiLines;
	size	uiDigits;
	size	uiLine;
	char	cMark;
	CChars	szRows;

	if (mpGutter->IsTiledText() != mpTextArea->IsTiledText())
	{
		mpGutter->SetTiledText(mpTextArea->IsTiledText());
	}

	uiLines = mpTextArea->NumLines();
	uiDigits = CountDigits(uiLines);
	if ((uiLines != muiGutterLines) || mbMarksChanged)
	{
		szRows.Init();
		for (uiLine = 0; uiLine < uiLines; uiLine++)
		{
			cMark = GetLineMark(uiLine);
			szRows.Append((cMark != '\0') ? cMark : ' ');
			szRows.Append(' ', (int)(uiDigits - CountDigits(uiLine + 1)));
			szRows.Append((int)(uiLine + 1));
			szRows.Append(' ');
			if (uiLine + 1 < uiLines)
			{
				szRows.Append('\n');
			}
		}
		mpGutter->SetText(szRows.Text());
		szRows.Kill();
		muiGutterLines = uiLines;
		mbMarksChanged = false;

		//Setting the text cleared them.
		mbChangeBarsChanged = true;
	}
	if (mbChangeBarsChanged)
	{
		DrawChangeBars(uiDigits);
		mbChangeBarsChanged = false;
	}
	mpGutter->SetLineGaps(mpTextArea->GetLineGaps());
}


//////////////////////////////////////////////////////////////////////////
//
// Each changed line's row has its last column, after its number, in its
// bar's colour, or the whole row for a band with its number in the band
// number colour.  Removed lines are a line at the top of the row below
// them.  Bars past the last line are cut off.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::DrawChangeBars(size uiDigits)
{
	size					i;
	size					uiLine;
	size					uiEnd;
	size					uiColumn;
	size					uiLines;
	SCodeEditorChangeBar*	psBar;

	mpGutter->ClearBackgrounds();
	mpGutter->ClearSpans();
	uiLines = mpGutter->NumLines();
	uiColumn = 1 + uiDigits;
	for (i = 0; i < masChangeBars.NumElements(); i++)
	{
		psBar = masChangeBars.Get(i);
		if (psBar->uiStartLine == psBar->uiEndLine)
		{
			mpGutter->AddLineBoundary((psBar->uiStartLine < uiLines) ? psBar->uiStartLine : uiLines, CODE_EDITOR_REMOVED_THICKNESS, psBar->sColour);
		}
		else if (psBar->bBand)
		{
			uiEnd = (psBar->uiEndLine < uiLines) ? psBar->uiEndLine : uiLines;
			if (psBar->uiStartLine < uiEnd)
			{
				mpGutter->AddLineBackground(psBar->uiStartLine, uiEnd, psBar->sColour);
			}
			for (uiLine = psBar->uiStartLine; uiLine < uiEnd; uiLine++)
			{
				mpGutter->AddSpan(mpGutter->GetLineStart(uiLine), uiColumn, msBandNumberColour);
			}
		}
		else
		{
			uiEnd = (psBar->uiEndLine < uiLines) ? psBar->uiEndLine : uiLines;
			for (uiLine = psBar->uiStartLine; uiLine < uiEnd; uiLine++)
			{
				mpGutter->AddBackground(mpGutter->GetLineStart(uiLine) + uiColumn, mpGutter->GetLineStart(uiLine) + uiColumn + 1, psBar->sColour);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The digits uiNumber is written in.
//
//////////////////////////////////////////////////////////////////////////
size CCodeEditor::CountDigits(size uiNumber)
{
	size	uiDigits;

	for (uiDigits = 1; uiNumber >= 10; uiNumber /= 10)
	{
		uiDigits++;
	}
	return uiDigits;
}


//////////////////////////////////////////////////////////////////////////
//
// cMark is shown next to line uiLine's number ('\0' for none).
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::SetLineMark(size uiLine, char cMark)
{
	if (GetLineMark(uiLine) == cMark)
	{
		return;
	}
	while (macMarks.NumElements() <= uiLine)
	{
		*macMarks.Add() = '\0';
	}
	*macMarks.Get(uiLine) = cMark;
	mbMarksChanged = true;

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The mark next to line uiLine, '\0' if it has none.
//
//////////////////////////////////////////////////////////////////////////
char CCodeEditor::GetLineMark(size uiLine)
{
	return (uiLine < macMarks.NumElements()) ? *macMarks.Get(uiLine) : '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::ClearLineMarks(void)
{
	if (macMarks.NumElements() > 0)
	{
		macMarks.ReInit();
		mbMarksChanged = true;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Lines uiStartLine up to uiEndLine changed, or with uiStartLine equal to
// uiEndLine, lines were removed above line uiStartLine (the number of
// lines for after the last).
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::AddChangeBar(size uiStartLine, size uiEndLine, ARGB32 sColour)
{
	AddChangeBar(uiStartLine, uiEndLine, sColour, false);
}


//////////////////////////////////////////////////////////////////////////
//
// As AddChangeBar, but the lines' whole rows of the gutter are coloured.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::AddChangeBand(size uiStartLine, size uiEndLine, ARGB32 sColour)
{
	AddChangeBar(uiStartLine, uiEndLine, sColour, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::AddChangeBar(size uiStartLine, size uiEndLine, ARGB32 sColour, bool bBand)
{
	SCodeEditorChangeBar*	psBar;

	if (uiEndLine < uiStartLine)
	{
		return;
	}
	psBar = masChangeBars.Add();
	psBar->uiStartLine = uiStartLine;
	psBar->uiEndLine = uiEndLine;
	psBar->sColour = sColour;
	psBar->bBand = bBand;
	mbChangeBarsChanged = true;
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::ClearChangeBars(void)
{
	if (masChangeBars.NumElements() > 0)
	{
		masChangeBars.ReInit();
		mbChangeBarsChanged = true;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of the line numbers on change bands; the editor's caret line
// number colour unless set.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::SetBandNumberColour(ARGB32 sColour)
{
	if (msBandNumberColour != sColour)
	{
		msBandNumberColour = sColour;
		mbChangeBarsChanged = true;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a change bar (not a band) is beside line uiLine: it changed, or
// lines were removed just above or below it.
//
//////////////////////////////////////////////////////////////////////////
bool CCodeEditor::IsChangeBarAt(size uiLine)
{
	size					i;
	SCodeEditorChangeBar*	psBar;

	for (i = 0; i < masChangeBars.NumElements(); i++)
	{
		psBar = masChangeBars.Get(i);
		if (psBar->bBand)
		{
			continue;
		}
		if (psBar->uiStartLine == psBar->uiEndLine)
		{
			if ((uiLine == psBar->uiStartLine) || (uiLine + 1 == psBar->uiStartLine))
			{
				return true;
			}
		}
		else if ((uiLine >= psBar->uiStartLine) && (uiLine < psBar->uiEndLine))
		{
			return true;
		}
	}
	return false;
}


CGutterClickEvent* CCodeEditor::GetChangeBarClickEvent(void) { return &mcChangeBarClicked; }
size CCodeEditor::NumChangeBars(void) { return masChangeBars.NumElements(); }
SCodeEditorChangeBar* CCodeEditor::GetChangeBar(size uiIndex) { return masChangeBars.SafeGet(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// A press that came to the code editor (it focussed it) gives the focus to
// the code, so what is typed goes there.  A press in the gutter calls the
// gutter click event with the line it was next to.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::ClickAction(SComponentInputEvent* psEvent)
{
	size	uiLine;
	size	uiPosition;
	int32	x;
	int32	y;

	mpWindow->GetFocus()->SetFocus(mpTextArea);

	//The press is relative to the code editor; the gutter can be scrolled along it.
	x = GetPosition().x + psEvent->x;
	y = GetPosition().y + psEvent->y;
	if (!mpGutter->IsPointIn(x, y))
	{
		return;
	}

	//The gutter's rows are the code's, and so are its lines.  The last column (after the number) is the change bars'.
	uiPosition = mpGutter->FindPositionAt(x - mpGutter->GetPosition().x, y - mpGutter->GetPosition().y);
	uiLine = mpGutter->FindLine(uiPosition);
	if (((uiPosition - mpGutter->GetLineStart(uiLine)) >= 1 + CountDigits(mpTextArea->NumLines())) && IsChangeBarAt(uiLine))
	{
		mcChangeBarClicked.Call(this, uiLine);
		return;
	}
	mcGutterClicked.Call(this, uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// pcRect (in the window) is shown right of the gutter, which is over the
// code's left when it is scrolled sideways.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::ScrollToShow(CInt32Rect* pcRect)
{
	CInt32Rect	cRect;

	cRect.Init(pcRect);
	cRect.msTopLeft.x -= mpGutter->GetActualSize().x;
	CContainer::ScrollToShow(&cRect);
}


//////////////////////////////////////////////////////////////////////////
//
// The colour the gutter is drawn on, over the code scrolled under it.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::SetGutterBackground(ARGB32 sColour)
{
	mpGutterDraw->SetColour(sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// The gutter's background and the gutter are the last children, so they
// are drawn over line components added since.
//
//////////////////////////////////////////////////////////////////////////
void CCodeEditor::BringGutterToFront(void)
{
	RemoveComponent(mpGutter);
	RemoveComponent(mpGutterBackground);
	AddComponent(mpGutterBackground);
	AddComponent(mpGutter);
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the gutter is pressed, with the code editor and the line
// the press was next to:
// pEditor->GetGutterClickEvent()->Add(this, &CMyContent::LineChosen).
//
//////////////////////////////////////////////////////////////////////////
CGutterClickEvent* CCodeEditor::GetGutterClickEvent(void)
{
	return &mcGutterClicked;
}


//////////////////////////////////////////////////////////////////////////
//
// Called with the code editor when it catches up with an edit to the
// code, before its line components and gutter are updated.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CCodeEditor>* CCodeEditor::GetChangeEvent(void)
{
	return &mcChanged;
}


Ptr<CTextArea> CCodeEditor::GetTextArea(void) { return mpTextArea; }
Ptr<CTextArea> CCodeEditor::GetGutter(void) { return mpGutter; }
size CCodeEditor::NumLineComponents(void) { return maLineComponents.Size(); }
Ptr<CComponent> CCodeEditor::GetLineComponent(size uiIndex) { return maLineComponents.Get(uiIndex); }

