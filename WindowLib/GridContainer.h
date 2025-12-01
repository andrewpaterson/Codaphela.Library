#ifndef __GRID_CONTAINER_H__
#define __GRID_CONTAINER_H__
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
#include "BaseLib/ArrayInt2D.h"
#include "GridCell.h"
#include "Container.h"


class CGridContainer : public CContainer
{
protected:
	CArrayGridCells		macCells;
	CArrayInt2D			maiCellIndices;	

public:
	void Init(Ptr<CWindow> pWindow);
	void Free(void);
};


#endif // __GRID_CONTAINER_H__

