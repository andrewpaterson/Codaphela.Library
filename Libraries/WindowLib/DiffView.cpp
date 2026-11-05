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
#include <math.h>
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "DiffView.h"


//////////////////////////////////////////////////////////////////////////
//
// The titles are in pTitleFont and the texts in pFamily.  psColours NULL
// is IntelliJ IDEA's dark theme's (SDiffViewColours::Init).
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pFamily, SDiffViewColours* psColours)
{
	PreInit();

	CGridContainer::Init(pWindow);
	if (psColours)
	{
		memcpy(&msColours, psColours, sizeof(SDiffViewColours));
	}
	else
	{
		msColours.Init();
	}
	mcDiff.Init("");
	mcProducer.Init();
	mcMapper.Init(&mcDiff);
	memset(&msScrolls, 0, sizeof(SDiffViewScrolls));

	mpLeftTitle = OMalloc<CLabel>(pWindow, pTitleFont, "");
	mpLeftTitle->SetAlignment(CSH_Left, CSV_Center);
	mpRightTitle = OMalloc<CLabel>(pWindow, pTitleFont, "");
	mpRightTitle->SetAlignment(CSH_Left, CSV_Center);

	mpLeftEditor = OMalloc<CCodeEditor>(pWindow, pFamily, "");
	mpLeftEditor->SetAlignment(CSH_Left, CSV_Top);
	mpLeftEditor->GetTextArea()->SetEditable(false);
	mpRightEditor = OMalloc<CCodeEditor>(pWindow, pFamily, "");
	mpRightEditor->SetAlignment(CSH_Left, CSV_Top);
	mpRightEditor->GetTextArea()->SetEditable(false);

	mpLeftPane = OMalloc<CScrollPane>(pWindow, pTheme);
	mpLeftPane->SetContent(mpLeftEditor);
	mpLeftPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpLeftPane->GetScrollEvent()->Add(this, &CDiffView::Scrolled);
	mpRightPane = OMalloc<CScrollPane>(pWindow, pTheme);
	mpRightPane->SetContent(mpRightEditor);
	mpRightPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpRightPane->GetScrollEvent()->Add(this, &CDiffView::Scrolled);

	mpDividerDraw = OMalloc<CDiffDividerDraw>(msColours.sDivider);
	mpDivider = OMalloc<CCanvas>(pWindow, mpDividerDraw);
	mpDivider->SetDesiredSize(DIFF_VIEW_DIVIDER_WIDTH, 1);
	mpDivider->SetAlignment(CSH_Stretch, CSV_Stretch);

	AddComponent(mpLeftTitle, 0, 0, 1, 1, 1, 0);
	AddComponent(mpRightTitle, 2, 0, 1, 1, 1, 0);
	AddComponent(mpLeftPane, 0, 1, 1, 1, 1, 1);
	AddComponent(mpDivider, 1, 1, 1, 1, 0, 1);
	AddComponent(mpRightPane, 2, 1, 1, 1, 1, 1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpLeftTitle);
	M_Pointer(mpRightTitle);
	M_Pointer(mpLeftEditor);
	M_Pointer(mpRightEditor);
	M_Pointer(mpLeftPane);
	M_Pointer(mpRightPane);
	M_Pointer(mpDivider);
	M_Pointer(mpDividerDraw);
	U_Data(SDiffViewColours, msColours);
	U_Unknown(CSimpleTextDiff, mcDiff);
	U_Unknown(CTextDiffProducer, mcProducer);
	U_Unknown(CTextDiffLineMapper, mcMapper);
	U_Data(SDiffViewScrolls, msScrolls);
}


//////////////////////////////////////////////////////////////////////////
//
// The panes' scroll events don't hold the view, so it takes its function
// off them.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Free(void)
{
	if (mpLeftPane.IsNotNull())
	{
		mpLeftPane->GetScrollEvent()->Remove(this);
	}
	if (mpRightPane.IsNotNull())
	{
		mpRightPane->GetScrollEvent()->Remove(this);
	}
	mcMapper.Kill();
	mcProducer.Kill();
	mcDiff.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiffView::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiffView::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The old text (szText1) on the left and the new on the right.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ShowTexts(const char* szTitle1, const char* szText1, const char* szTitle2, const char* szText2)
{
	mcProducer.ProduceFromText(&mcDiff, szTitle1, szText1, szTitle2, szText2);
	Update();
	ScrollToBlock(0);
}


//////////////////////////////////////////////////////////////////////////
//
// The file names are the titles.  False if either couldn't be read (it is
// shown empty).
//
//////////////////////////////////////////////////////////////////////////
bool CDiffView::ShowFiles(const char* szFilename1, const char* szFilename2)
{
	bool	bResult;

	bResult = mcProducer.ProduceFromFiles(&mcDiff, szFilename1, szFilename2);
	Update();
	ScrollToBlock(0);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::SetWhitespace(ETextDiffWhitespace eWhitespace)
{
	mcProducer.SetWhitespace(eWhitespace);
	mcProducer.Produce(&mcDiff);
	Update();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::SetHighlight(ETextDiffHighlight eHighlight)
{
	mcProducer.SetHighlight(eHighlight);
	mcProducer.Produce(&mcDiff);
	Update();
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the diff as it is now: each side's text and title, then each
// block's colours.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Update(void)
{
	lsize	uiBlock;

	ShowSide(TDS_Left);
	ShowSide(TDS_Right);
	for (uiBlock = 0; uiBlock < mcDiff.NumBlocks(); uiBlock++)
	{
		ShowBlock(mcDiff.GetBlock(uiBlock));
	}
	memset(&msScrolls, 0, sizeof(SDiffViewScrolls));
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The side's lines joined with line feeds, whatever ended them, so the
// text area's line n is the content's line n.  Text that hasn't changed
// is kept, with its spans (syntax highlighting), and only its colours for
// the diff are cleared.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ShowSide(ETextDiffSide eSide)
{
	CTextContent*	pcContent;
	CChars			szText;
	lsize			uiLine;
	Ptr<CTextArea>	pTextArea;

	pcContent = GetContent(eSide);
	szText.Init();
	for (uiLine = 0; uiLine < pcContent->NumLines(); uiLine++)
	{
		if (uiLine > 0)
		{
			szText.Append('\n');
		}
		szText.Append(pcContent->GetLineText(uiLine), (size)pcContent->GetLineLength(uiLine));
	}
	pTextArea = GetEditor(eSide)->GetTextArea();
	if (szText.Equals(pTextArea->GetText()))
	{
		pTextArea->ClearBackgrounds();
	}
	else
	{
		pTextArea->SetText(szText.Text());
	}
	GetEditor(eSide)->ClearChangeBars();
	GetTitle(eSide)->SetText(pcContent->GetTitle());
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The block's lines on each side, and the characters that changed in it.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ShowBlock(STextDiffBlock* psBlock)
{
	ETextDiffBlockType	eType;
	lsize				uiFragment;
	STextDiffFragment*	psFragment;
	ARGB32				sChanges;

	eType = psBlock->GetType();
	ShowBlockSide(TDS_Left, psBlock->uiStartLine1, psBlock->uiEndLine1, eType);
	ShowBlockSide(TDS_Right, psBlock->uiStartLine2, psBlock->uiEndLine2, eType);

	sChanges = msColours.GetChanges(eType);
	for (uiFragment = 0; uiFragment < psBlock->uiNumFragments; uiFragment++)
	{
		psFragment = mcDiff.GetFragment(psBlock, uiFragment);
		if (psFragment->uiEnd1 > psFragment->uiStart1)
		{
			mpLeftEditor->GetTextArea()->AddBackground(MapOffset(TDS_Left, psFragment->uiStart1), MapOffset(TDS_Left, psFragment->uiEnd1), sChanges);
		}
		if (psFragment->uiEnd2 > psFragment->uiStart2)
		{
			mpRightEditor->GetTextArea()->AddBackground(MapOffset(TDS_Right, psFragment->uiStart2), MapOffset(TDS_Right, psFragment->uiEnd2), sChanges);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A band behind the block's lines on one side and their numbers, or a
// thin line where they would be if the side has none.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ShowBlockSide(ETextDiffSide eSide, lsize uiStartLine, lsize uiEndLine, ETextDiffBlockType eType)
{
	Ptr<CCodeEditor>	pEditor;

	pEditor = GetEditor(eSide);
	if (uiEndLine > uiStartLine)
	{
		pEditor->GetTextArea()->AddLineBackground((size)uiStartLine, (size)uiEndLine, msColours.GetLines(eType));
		pEditor->AddChangeBand((size)uiStartLine, (size)uiEndLine, msColours.GetLines(eType));
	}
	else
	{
		pEditor->GetTextArea()->AddLineBoundary((size)uiStartLine, DIFF_VIEW_BOUNDARY, msColours.GetChanges(eType));
		pEditor->AddChangeBand((size)uiStartLine, (size)uiStartLine, msColours.GetChanges(eType));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text area's position of an offset in the side's content.  An
// offset in a line's separator is at its line feed in the text area
// (which the last line hasn't).
//
//////////////////////////////////////////////////////////////////////////
size CDiffView::MapOffset(ETextDiffSide eSide, lsize uiOffset)
{
	CTextContent*	pcContent;
	lsize			uiLine;
	lsize			uiColumn;
	lsize			uiMaximum;

	pcContent = GetContent(eSide);
	if (pcContent->NumLines() == 0)
	{
		return 0;
	}
	uiLine = pcContent->FindLine(uiOffset);
	uiColumn = uiOffset - pcContent->GetLineStart(uiLine);
	uiMaximum = pcContent->GetLineLength(uiLine);
	if ((uiLine + 1 < pcContent->NumLines()) && (pcContent->GetLineSeparator(uiLine) != TLS_None))
	{
		uiMaximum++;
	}
	if (uiColumn > uiMaximum)
	{
		uiColumn = uiMaximum;
	}
	return GetEditor(eSide)->GetTextArea()->GetLineStart((size)uiLine) + (size)uiColumn;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines of the side scrolled are mapped to the other side, then the
// shapes on the divider follow the sides.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CGridContainer::Layout(sPosition, sAreaSize);
	if (msScrolls.bToBlock && (mpLeftPane->GetActualSize().y > 0))
	{
		ScrollToBlockNow();
	}
	UpdateDivider();
}


//////////////////////////////////////////////////////////////////////////
//
// Scrolls both sides to the block uiBlock when the view is next laid out
// (when the sides' heights are known): its first lines a quarter of the
// way down each side.  False if there is no such block.
//
//////////////////////////////////////////////////////////////////////////
bool CDiffView::ScrollToBlock(lsize uiBlock)
{
	if (uiBlock >= mcDiff.NumBlocks())
	{
		return false;
	}
	msScrolls.bToBlock = true;
	msScrolls.uiBlock = uiBlock;
	InvalidateLayout();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Each side is scrolled itself, rather than one following the other: a
// block with no lines on one side would put that side after the block's
// lines on the other.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ScrollToBlockNow(void)
{
	STextDiffBlock*	psBlock;
	int32			x;

	//The diff may have been made again since it was asked for.
	msScrolls.bToBlock = false;
	if (msScrolls.uiBlock >= mcDiff.NumBlocks())
	{
		return;
	}
	psBlock = mcDiff.GetBlock(msScrolls.uiBlock);
	x = mpLeftPane->GetScrollPosition().x;
	ScrollSide(TDS_Left, x, (int32)psBlock->uiStartLine1 * GetRowHeight(TDS_Left) - mpLeftPane->GetActualSize().y / 4);
	ScrollSide(TDS_Right, x, (int32)psBlock->uiStartLine2 * GetRowHeight(TDS_Right) - mpRightPane->GetActualSize().y / 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The side is scrolled (as far as it can be), and its scroll is known to
// be the view's.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ScrollSide(ETextDiffSide eSide, int32 x, int32 y)
{
	Ptr<CScrollPane>	pPane;
	SInt32Vec2			sBefore;
	SInt32Vec2			sAfter;

	pPane = GetPane(eSide);
	sBefore = pPane->GetScrollPosition();
	pPane->SetScrollPosition(x, y);
	sAfter = pPane->GetScrollPosition();
	if ((sAfter.x != sBefore.x) || (sAfter.y != sBefore.y))
	{
		msScrolls.abExpecting[eSide] = true;
		msScrolls.asExpected[eSide] = sAfter;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A side's scroll event.  The other side is scrolled to match unless this
// is the scroll the view gave it.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::Scrolled(CScrollPane* pcPane)
{
	ETextDiffSide	eSide;
	SInt32Vec2		sPosition;

	eSide = (pcPane == &mpLeftPane) ? TDS_Left : TDS_Right;
	if (msScrolls.abExpecting[eSide])
	{
		msScrolls.abExpecting[eSide] = false;
		sPosition = pcPane->GetScrollPosition();
		if ((sPosition.x == msScrolls.asExpected[eSide].x) && (sPosition.y == msScrolls.asExpected[eSide].y))
		{
			return;
		}
	}
	ScrollToMatch(eSide);
}


//////////////////////////////////////////////////////////////////////////
//
// The other side is scrolled to put the line mapped from the middle of
// eFrom at its middle, and sideways as far as eFrom is.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::ScrollToMatch(ETextDiffSide eFrom)
{
	ETextDiffSide		eTo;
	Ptr<CScrollPane>	pTo;
	float64				fLine;
	int32				x;
	int32				y;

	eTo = (eFrom == TDS_Left) ? TDS_Right : TDS_Left;
	pTo = GetPane(eTo);
	fLine = mcMapper.TransferLine(eFrom, GetMiddleLine(eFrom));
	x = GetPane(eFrom)->GetScrollPosition().x;
	y = (int32)floor(fLine * GetRowHeight(eTo) - pTo->GetActualSize().y / 2.0 + 0.5);
	ScrollSide(eTo, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The fractional line at the vertical middle of the side's view.  Rows
// are all the font's height: the code isn't wrapped.
//
//////////////////////////////////////////////////////////////////////////
float64 CDiffView::GetMiddleLine(ETextDiffSide eSide)
{
	Ptr<CScrollPane>	pPane;

	pPane = GetPane(eSide);
	return (pPane->GetScrollPosition().y + pPane->GetActualSize().y / 2.0) / GetRowHeight(eSide);
}


//////////////////////////////////////////////////////////////////////////
//
// Each block whose lines can be seen on either side is a shape joining
// its rows on the left to its rows on the right, as the sides are
// scrolled now.
//
//////////////////////////////////////////////////////////////////////////
void CDiffView::UpdateDivider(void)
{
	CArrayDiffDividerShape	asShapes;
	SDiffDividerShape*		psShape;
	STextDiffBlock*			psBlock;
	ETextDiffBlockType		eType;
	lsize					uiBlock;
	float64					fTop;
	float64					fHeight;
	float64					fLeftTop;
	float64					fLeftBottom;
	float64					fRightTop;
	float64					fRightBottom;

	fTop = mpDivider->GetPosition().y;
	fHeight = mpDivider->GetActualSize().y;
	asShapes.Init();
	for (uiBlock = 0; uiBlock < mcDiff.NumBlocks(); uiBlock++)
	{
		psBlock = mcDiff.GetBlock(uiBlock);
		fLeftTop = GetLineTop(TDS_Left, psBlock->uiStartLine1) - fTop;
		fLeftBottom = GetLineTop(TDS_Left, psBlock->uiEndLine1) - fTop;
		fRightTop = GetLineTop(TDS_Right, psBlock->uiStartLine2) - fTop;
		fRightBottom = GetLineTop(TDS_Right, psBlock->uiEndLine2) - fTop;
		if (((fLeftBottom < -DIFF_DIVIDER_OUTLINE) && (fRightBottom < -DIFF_DIVIDER_OUTLINE)) || ((fLeftTop > fHeight + DIFF_DIVIDER_OUTLINE) && (fRightTop > fHeight + DIFF_DIVIDER_OUTLINE)))
		{
			continue;
		}

		eType = psBlock->GetType();
		psShape = asShapes.Add();
		psShape->fLeftTop = fLeftTop;
		psShape->fLeftBottom = fLeftBottom;
		psShape->fRightTop = fRightTop;
		psShape->fRightBottom = fRightBottom;
		psShape->sFill = msColours.GetShape(eType);
		psShape->sOutline = msColours.GetOutline(eType);
	}
	if (mpDividerDraw->SetShapes(&asShapes))
	{
		mpDivider->Invalidate();
	}
	asShapes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The window's y of the top of the side's line uiLine (or the bottom of
// the last line for the number of lines), as it is scrolled now.
//
//////////////////////////////////////////////////////////////////////////
float64 CDiffView::GetLineTop(ETextDiffSide eSide, lsize uiLine)
{
	return GetEditor(eSide)->GetTextArea()->GetPosition().y + (float64)uiLine * GetRowHeight(eSide);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CDiffView::GetRowHeight(ETextDiffSide eSide)
{
	return GetEditor(eSide)->GetTextArea()->GetFont()->GetLineHeight();
}


CSimpleTextDiff* CDiffView::GetDiff(void) { return &mcDiff; }
CTextDiffProducer* CDiffView::GetProducer(void) { return &mcProducer; }
CTextContent* CDiffView::GetContent(ETextDiffSide eSide) { return (eSide == TDS_Left) ? mcDiff.GetContent1() : mcDiff.GetContent2(); }
Ptr<CCodeEditor> CDiffView::GetEditor(ETextDiffSide eSide) { return (eSide == TDS_Left) ? mpLeftEditor : mpRightEditor; }
Ptr<CScrollPane> CDiffView::GetPane(ETextDiffSide eSide) { return (eSide == TDS_Left) ? mpLeftPane : mpRightPane; }
Ptr<CLabel> CDiffView::GetTitle(ETextDiffSide eSide) { return (eSide == TDS_Left) ? mpLeftTitle : mpRightTitle; }
Ptr<CDiffDividerDraw> CDiffView::GetDividerDraw(void) { return mpDividerDraw; }

