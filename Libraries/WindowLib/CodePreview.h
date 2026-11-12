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
#ifndef __CODE_PREVIEW_H__
#define __CODE_PREVIEW_H__
#include "GridContainer.h"
#include "ScrollPane.h"
#include "Label.h"
#include "CodeEditor.h"
#include "WindowTheme.h"


//A place in a file's code shown to be looked at, as IntelliJ IDEA's previews of what Find in Files and Find Usages
//found: the file's name (its title) in a line above, and under it its code, with line numbers, that can't be edited,
//scrolled so the place is in the middle of the view with its text coloured (TC_SearchMatch) and its line as the
//caret's.  The code is given once for a title (SetCode) and each place in it after (ShowPlace), so going from one
//result to the next in a file doesn't read the file again.  Whoever gives the code can colour it (GetTextArea).
//
//A window not shown yet has no size: the place is scrolled to at a later tick, once it has.
class CCodePreview : public CGridContainer
{
CONSTRUCTABLE(CCodePreview);
DESTRUCTABLE(CCodePreview);
protected:
	Ptr<CWindowTheme>	mpTheme;
	Ptr<CLabel>			mpTitle;
	Ptr<CCodeEditor>	mpEditor;
	Ptr<CScrollPane>	mpScrollPane;
	CChars				mszTitle;  //The file shown, empty if none.
	bool				mbCentred;  //The place has been scrolled to since it was last shown.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pCodeFamily);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			IsShowing(const char* szTitle);
	void			SetCode(const char* szTitle, const char* szCode);
	void			ShowPlace(size uiStart, size uiLength);
	void			Clear(void);

	char*				GetTitle(void);
	Ptr<CCodeEditor>	GetEditor(void);
	Ptr<CTextArea>		GetTextArea(void);
	Ptr<CScrollPane>	GetScrollPane(void);

protected:
	void			Centre(void);
};


#endif // __CODE_PREVIEW_H__

