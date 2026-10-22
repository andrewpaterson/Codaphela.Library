#ifndef __CONTAINER_H__
#define __CONTAINER_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "Component.h"
#include "SupportLib/Int32Rect.h"


class CContainer : public CComponent
{
CONSTRUCTABLE(CContainer);
DESTRUCTABLE(CContainer);
protected:
	CInt32Rect	mcContainerBounds;
	SInt32Vec2	msRequiredSize;  //What the container's components need, see SetRequiredSize.
	bool		mbClipsChildren;  //Children are only drawn inside the container bounds.  True unless turned off, for an overlay say.

public:
			void		Init(Ptr<CWindow> pWindow);
			void		Class(void) override;
			void 		Free(void) override;

			bool		GetContainerBounds(CInt32Rect* pcDest);
			void		SetContainerBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			bool		ClipsChildren(void) override;
			void		SetClipsChildren(bool bClipsChildren);

			void		Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

	virtual void		SetRequiredSize(void) =0;  //Works out msRequiredSize from the container's components.
			SInt32Vec2	GetRequiredSize(void);

protected:
	void		LayoutComponentInArea(Ptr<CComponent> pComponent, SInt32Vec2 sAreaPosition, SInt32Vec2 sAreaSize);
	void		SetActualBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
	SInt32Vec2	GetDesiredOrRequiredSize(void);
};


#endif // __CONTAINER_H__

