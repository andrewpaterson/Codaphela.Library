/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __VIEWPORT_INSTANCE_H__
#define __VIEWPORT_INSTANCE_H__
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/Direct3DGlobal.h"
#include "SupportLib/Rectangle.h"


class CWorld;
class CViewportInstance : public CUnknown
{
BASE_FUNCTIONS(CViewportInstance);
protected:
	SViewport	msViewport;
	CWorld*		mpcWorld;

public:
	void		Init(void);
	void		Init(CWorld* pcWorld);
	void 		Kill(void);
	void 		Draw(void);
	void		Invalidate(void);
	void		Set(int iX, int iY, int iWidth, int iHeight);
	BOOL		Contains(int iLeft, int iTop, int iRight, int iBottom);
	BOOL		Contains(CRectangle* pcRect);
};


#endif // __VIEWPORT_INSTANCE_H__

