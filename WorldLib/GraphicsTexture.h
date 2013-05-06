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
#ifndef __GRAPHICS_TEXTURE_H__
#define __GRAPHICS_TEXTURE_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "StandardLib/StandardHeader.h"


class CGraphicsTexture : public CUnknown
{
BASE_FUNCTIONS(CGraphicsTexture);
protected:
	STexture	msTexture;

public:
	void 		Init(void);
	void 		Init(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, BOOL bDynamic);
	void 		Kill(void);
	int			GetWidth(void);
	int			GetHeight(void);
	STexture*	GetTexture(void);
};


#endif //__GRAPHICS_TEXTURE_H__

