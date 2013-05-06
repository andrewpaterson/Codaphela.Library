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
#ifndef __LIGHT_INSTANCE_H__
#define __LIGHT_INSTANCE_H__
#include "DirectXLib/Direct3DGlobal.h"
#include "StandardLib/StandardHeader.h"
#include "BaseLib/ArraySimple.h"


#define LIGHT_PLEASANT	(0.125f)


class CLightInstance : public CUnknown
{
BASE_FUNCTIONS(CLightInstance);
protected:
	D3DLIGHT9	msLight;
	SMatrix*	mpsMatrix;
	int			miSlot;
	BOOL		mbEnabled;

public:
	void 		Init(void);
	void 		Kill(void);
	BOOL 		Use(int iSlot);
	void 		Unuse(void);
	void 		SetColour(float r, float g, float b);
	void 		SetGrey(float g);
	SMatrix*	GetMatrix(void);
	void 		SetMatrix(SMatrix* psMatrix);
	void 		SetPoint(float fConstAtten, float fLinearAtten, float fQuadraticAtten, float fRange);
	void 		SetDirectional(float fConstAtten, float fRange);
	BOOL		IsEnabled(void);
	void		SetEnabled(BOOL bEnabled);
};


typedef CArraySimple<CLightInstance*> CArrayPtrLight;


#endif //__LIGHT_INSTANCE_H__

