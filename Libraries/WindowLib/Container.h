/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __CONTAINER_H__
#define __CONTAINER_H__
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
	bool		mbSizedByContent;  //The desired size is the required size on an axis without one set, see SetSizedByContent.

public:
			void		Init(Ptr<CWindow> pWindow);
			void		Class(void) override;

			bool		GetContainerBounds(CInt32Rect* pcDest);
			void		SetContainerBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			bool		ClipsChildren(void) override;
			void		SetClipsChildren(bool bClipsChildren);
			void		SetSizedByContent(bool bSizedByContent);
			SInt32Vec2	GetDesiredSize(void) override;

			void		Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override =0;  //A container lays its components out itself, it doesn't pass its area on as a component does.

	virtual void		SetRequiredSize(void) =0;  //Works out msRequiredSize from the container's components.
			SInt32Vec2	GetRequiredSize(void);

protected:
	void		LayoutComponentInArea(Ptr<CComponent> pComponent, SInt32Vec2 sAreaPosition, SInt32Vec2 sAreaSize);
	void		SetActualBounds(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
	SInt32Vec2	GetDesiredOrRequiredSize(void);
};


#endif // __CONTAINER_H__

