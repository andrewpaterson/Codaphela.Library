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
#ifndef __DIFF_VIEW_H__
#define __DIFF_VIEW_H__
#include "StandardLib/SimpleTextDiff.h"
#include "StandardLib/TextDiffProducer.h"
#include "StandardLib/TextDiffLineMapper.h"
#include "SupportLib/FontFamily.h"
#include "GridContainer.h"
#include "Label.h"
#include "Canvas.h"
#include "ScrollPane.h"
#include "CodeEditor.h"
#include "WindowTheme.h"
#include "DiffDividerDraw.h"
#include "DiffViewColours.h"


#define DIFF_VIEW_DIVIDER_WIDTH		40	//Pixels between the two sides.
#define DIFF_VIEW_BOUNDARY			2	//Pixels of the line where an empty side's lines would be.


//Where the view scrolled each side to (by ETextDiffSide), so that side's scroll there isn't taken for the user's.
struct SDiffViewScrolls
{
	SInt32Vec2	asExpected[2];
	bool		abExpecting[2];
};


//Two texts side by side with their differences shown, as IntelliJ IDEA's diff viewer shows them (its default,
//unaligned): the old text on the left and the new on the right, each in a code editor (line numbers in its gutter) in
//its own scroll pane under its title, with a divider between them.  It is read only; the texts can be selected and
//copied.
//
//The view owns its diff (GetDiff) and the producer that makes it (GetProducer).  ShowTexts and ShowFiles make the diff
//and show it; Update shows the diff again after whatever made it changed it.  Changing the whitespace or highlight
//policy makes the diff again from the texts it has.
//
//Each changed block's lines have a band behind them, the line numbers too, in its kind's colour (inserted, deleted or
//modified, from SDiffViewColours); the characters that changed in a modified block are a stronger shade.  A side with
//no lines in a block has a thin line where they would be.  On the divider each block is a shape joining its lines on
//the left to its lines on the right.
//
//Scrolling either side scrolls the other: the line at the middle of the scrolled side is mapped across the blocks
//(CTextDiffLineMapper) and the other side is scrolled to put that line at its middle; sideways the other side is
//scrolled the same.  A side scrolled by the view doesn't scroll the first back.
class CDiffView : public CGridContainer
{
CONSTRUCTABLE(CDiffView);
DESTRUCTABLE(CDiffView);
protected:
	Ptr<CLabel>				mpLeftTitle;
	Ptr<CLabel>				mpRightTitle;
	Ptr<CCodeEditor>		mpLeftEditor;
	Ptr<CCodeEditor>		mpRightEditor;
	Ptr<CScrollPane>		mpLeftPane;
	Ptr<CScrollPane>		mpRightPane;
	Ptr<CCanvas>			mpDivider;
	Ptr<CDiffDividerDraw>	mpDividerDraw;
	SDiffViewColours		msColours;
	CSimpleTextDiff			mcDiff;
	CTextDiffProducer		mcProducer;
	CTextDiffLineMapper		mcMapper;
	SDiffViewScrolls		msScrolls;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pFamily, SDiffViewColours* psColours = NULL);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				ShowTexts(const char* szTitle1, const char* szText1, const char* szTitle2, const char* szText2);
	bool				ShowFiles(const char* szFilename1, const char* szFilename2);
	void				SetWhitespace(ETextDiffWhitespace eWhitespace);
	void				SetHighlight(ETextDiffHighlight eHighlight);
	void				Update(void);

	CSimpleTextDiff*	GetDiff(void);
	CTextDiffProducer*	GetProducer(void);
	Ptr<CCodeEditor>	GetEditor(ETextDiffSide eSide);
	Ptr<CScrollPane>	GetPane(ETextDiffSide eSide);
	Ptr<CLabel>			GetTitle(ETextDiffSide eSide);
	Ptr<CDiffDividerDraw>	GetDividerDraw(void);
	float64				GetMiddleLine(ETextDiffSide eSide);

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void				Scrolled(CScrollPane* pcPane);

protected:
	void				ShowSide(ETextDiffSide eSide);
	void				ShowBlock(STextDiffBlock* psBlock);
	void				ShowBlockSide(ETextDiffSide eSide, lsize uiStartLine, lsize uiEndLine, ETextDiffBlockType eType);
	CTextContent*		GetContent(ETextDiffSide eSide);
	size				MapOffset(ETextDiffSide eSide, lsize uiOffset);
	int32				GetRowHeight(ETextDiffSide eSide);
	float64				GetLineTop(ETextDiffSide eSide, lsize uiLine);
	void				ScrollToMatch(ETextDiffSide eFrom);
	void				UpdateDivider(void);
};


#endif // __DIFF_VIEW_H__

