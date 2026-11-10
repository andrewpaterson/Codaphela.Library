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
#ifndef __TEXT_AREA_SHAPES_H__
#define __TEXT_AREA_SHAPES_H__
#include "BaseLib/TextDocument.h"
#include "SupportLib/GeometryMap.h"
#include "TextBackgrounds.h"


class CTextAreaLayout;


//Draws a text area's backgrounds and selection as rectangles into geometry maps, where its layout puts its rows and
//lines.  The layout and the document are the text area's: the shapes only use them.
class CTextAreaShapes
{
protected:
	CTextAreaLayout*	mpcLayout;
	CTextDocument*		mpcDocument;

public:
	void	Init(CTextAreaLayout* pcLayout, CTextDocument* pcDocument);
	void	Kill(void);

	void	AddBackgrounds(Ptr<CGeometryMap> pMap, CTextBackgrounds* pcBackgrounds, int32 iWidth);
	void	AddRange(Ptr<CGeometryMap> pMap, size uiStart, size uiEnd, ARGB32 sColour);
	void	AddLine(Ptr<CGeometryMap> pMap, size uiLine, ARGB32 sColour, int32 iWidth);

protected:
	void	AddLines(Ptr<CGeometryMap> pMap, STextAreaBackground* psBackground, int32 iWidth);
	void	AddBoundary(Ptr<CGeometryMap> pMap, STextAreaBackground* psBackground, int32 iWidth);
};


#endif // __TEXT_AREA_SHAPES_H__

