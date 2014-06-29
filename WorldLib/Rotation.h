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
#ifndef __ROTATION_H__
#define __ROTATION_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/ArrayIntAndPointer.h"
#include "CoreLib/ArrayVariable.h"
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/DirectXGlobal.h"
#include "Interpolator.h"


enum ERotationType
{
	RT_SphericalLinearRotation,
};


typedef CInterpolator<SQuaternion> __CRotationInterpolator;
class CRotationInterpolator : public __CRotationInterpolator
{
public:
	void	Init(void);
	void	Kill(void);
	void	Interpolate(float fPos, SQuaternion* psRotation1, SQuaternion* psRotation2);
	void	Update(float fPos);
};


class CRotation : public CUnknown
{
BASE_FUNCTIONS(CRotation);
protected:
	SIntAndPointer	msType;

public:
	void 			Init(void);
	void			Kill(void);
	void*			Set(ERotationType eType);
	void			Update(float fFractionalTime);
	SQuaternion*	GetRotation(void);
};


void RotationFunction(void* psCel, float fFractionalTime, void* pvData);


#endif // __ROTATION_H__

