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
#ifndef __LINE_ZONES_H__
#define __LINE_ZONES_H__
#include "StandardLib/Object.h"
#include "CodeEditor.h"
#include "LineZone.h"
#include "LineZoneSource.h"


#define LINE_ZONES_MARK			'+'		//In the gutter: the line has pages.
#define LINE_ZONES_OPEN_MARK	'-'		//And they are shown under it.


//A code editor's lines' pages from a source (SetSource), such as a build's listings: each line with pages is marked
//in the gutter, and clicking a mark shows the line's pages under it (a CLineZone), clicking again hides them.  Editing
//the code puts the source out of date, as its lines may not be the code's now, until a source is set again: the marks
//go and the zones are greyed.  Setting the source again brings each zone up to date with its line's pages, or closes
//it if the line has none now; a zone whose line is deleted goes with it.  The zones are the code editor's line
//components that are CLineZones: it keeps them, and drops them with their lines.
class CLineZones : public CObject
{
CONSTRUCTABLE(CLineZones);
DESTRUCTABLE(CLineZones);
protected:
	Ptr<CCodeEditor>	mpCodeEditor;
	Ptr<CWindowTheme>	mpTheme;
	Ptr<CFont>			mpTitleFont;
	Ptr<CFontFamily>	mpTextFamily;
	ARGB32				msBackground;
	ARGB32				msText;
	CLineZoneSource*	mpcSource;		//NULL for none; whatever set it keeps it.
	bool				mbOutOfDate;	//The code was edited since the source was set.

public:
	void				Init(Ptr<CCodeEditor> pCodeEditor, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pTextFamily, ARGB32 sBackground, ARGB32 sText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetSource(CLineZoneSource* pcSource);
	bool				Toggle(size uiLine);
	void				GutterClicked(CCodeEditor* pcEditor, size uiLine);
	void				CodeChanged(void);

	Ptr<CLineZone>		FindZone(size uiLine);
	size				NumZones(void);
	Ptr<CLineZone>		GetZone(size uiIndex);
	bool				IsOutOfDate(void);

protected:
	Ptr<CLineZone>		GetZoneComponent(size uiComponent);
	void				RemoveZone(Ptr<CLineZone> pZone);
	void				MarkLine(size uiLine);
	bool				HasPages(size uiLine);
};


#endif // __LINE_ZONES_H__

