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
#ifndef __CODE_EDITOR_H__
#define __CODE_EDITOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/FontFamily.h"
#include "Container.h"
#include "TextArea.h"


typedef CArrayTemplate<uint32>	CArrayCodeEditorGapIds;


//Source code to edit: a gutter of line numbers on the left of a text area (GetTextArea) of the code, and components
//shown under lines of the code (AddLineComponent), each in a gap the text area leaves below its line, as wide as the
//code and as high as the component wants.  A line component stays with its line as the code is edited and goes when
//its line is deleted.  The gutter is a text area too, in the code's font with the same gaps, so its numbers are on
//the code's rows; the code isn't word wrapped and its spans shouldn't change the font's size or the rows wouldn't
//match.  The gutter's first column is left for marks next to the numbers.  The code editor is meant to be the content
//of a CScrollPane: it is as big as the gutter and code need, and the text area keeps its caret in view.
class CCodeEditor : public CContainer
{
CONSTRUCTABLE(CCodeEditor);
DESTRUCTABLE(CCodeEditor);
protected:
	Ptr<CTextArea>			mpGutter;
	Ptr<CTextArea>			mpTextArea;
	CArray<CComponent>		maLineComponents;
	CArrayCodeEditorGapIds	mauiGapIds;  //Of each line component's gap in the text area.
	size					muiGutterLines;  //The lines the gutter was last numbered for.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CTextArea>		GetTextArea(void);
	Ptr<CTextArea>		GetGutter(void);
	void				SetGutterColour(ARGB32 sColour);
	int32				GetGutterWidth(void);

	bool				AddLineComponent(size uiLine, Ptr<CComponent> pComponent);
	bool				RemoveLineComponent(Ptr<CComponent> pComponent);
	void				RemoveAllLineComponents(void);
	size				NumLineComponents(void);
	Ptr<CComponent>		GetLineComponent(size uiIndex);
	size				GetLineOf(Ptr<CComponent> pComponent);
	Ptr<CComponent>		GetComponentOn(size uiLine);

	void				SetRequiredSize(void) override;
	SInt32Vec2			GetDesiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

protected:
	size				FindLineComponent(Ptr<CComponent> pComponent);
	int32				GetGapHeight(Ptr<CComponent> pComponent);
	void				Update(void);
	void				UpdateLineComponents(void);
	void				UpdateGutter(void);
};


#endif // __CODE_EDITOR_H__

