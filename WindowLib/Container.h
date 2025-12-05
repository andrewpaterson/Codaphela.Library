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
#include "BasicComponent.h"
#include "ContainerBounds.h"


class CContainer : public CBasicComponent
{
CONSTRUCTABLE(CContainer);
DESTRUCTABLE(CContainer);
protected:
	SContainerBounds		msBounds;

public:
			void	Init(Ptr<CWindow> pWindow);
			void	Class(void);
			void 	Free(void);

			bool	GetContainerBounds(SContainerBounds* psDest);
			void	SetBounds(SInt2 sPosition, SInt2 sAreaSize);

	virtual void	SetRequiredSize(void) =0;
			void	Layout(SInt2 sPosition, SInt2 sAreaSize) override;
};


#endif // __CONTAINER_H__

