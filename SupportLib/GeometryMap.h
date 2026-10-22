/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __GEOMETRY_MAP_H__
#define __GEOMETRY_MAP_H__
#include "BaseLib/ArrayTemplate.h"
#include "ColourARGB32.h"
#include "ImageAccessor.h"
#include "BaseMap.h"


enum EGeometryCommand
{
	GC_Rectangle,
};


//A shape to draw.  The bounds are in map pixels, right and bottom exclusive.
struct SGeometryCommand
{
	EGeometryCommand	eType;
	CInt32Rect			cBounds;
	ARGB32				sColour;
};


typedef CArrayTemplate<SGeometryCommand>	CArrayGeometryCommand;


//A map of shapes drawn in colours, such as the lines under and through text.  The commands are plain data (not
//objects) so a map can hold many.  A shape is blended onto the viewport the way a translucent cel is: its colour is
//blended with what is there by its alpha, and where the viewport has opacity the shape is added to it.  On a
//transparent canvas, cleared to nothing, that leaves premultiplied pixels like a cel's.
class CGeometryMap : public CBaseMap
{
CONSTRUCTABLE(CGeometryMap);
DESTRUCTABLE(CGeometryMap);
protected:
	CArrayGeometryCommand	masCommands;
	Ptr<CImage>				mpViewport;

public:
	void				Init(void);
	void 				Free(void) override;
	void				Class(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				Clear(void);
	void				AddRectangle(int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, ARGB32 sColour);
	size				NumCommands(void);
	SGeometryCommand*	GetCommand(size uiIndex);

	bool				Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters) override;
	void				SetViewport(Ptr<CImage> pViewport) override;
	Ptr<CImage>			GetViewport(void);

protected:
	void				FillRectangle(CImageAccessor* pcAccessor, bool bOpacity, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, ARGB32 sColour);
};


#endif // __GEOMETRY_MAP_H__

