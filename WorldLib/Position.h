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
#ifndef __POSITION_H__
#define __POSITION_H__
#include "BaseLib/ArrayTypedPointer.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/GeometricTypes.h"
#include "CoreLib/ArrayVariable.h"
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/DirectXGlobal.h"
#include "Interpolator.h"


enum EPositionType
{
	PT_LinearPosition,
	PT_DirectedPosition,
};


typedef CInterpolator<SFloat3> __CPositionInterpolator;
class CPositionInterpolator  : public __CPositionInterpolator
{
public:
	void	Interpolate(float fPos, SFloat3* psPosition1, SFloat3* psPosition2);
};


//Linear Position implies Position in linear space, not linear time also.
class CLinearPositionInterpolator : public CPositionInterpolator
{
public:
	void Init(void);
	void Update(float fPos);
};


//This is actually a bezier curve.
class CDirectedInterpolator : public CPositionInterpolator
{
public:
	SFloat3		msDir1;
	SFloat3		msDir2;
	float		mfDir1Length;  //Negative to reverse direction...
	float		mfDir2Length;

	void Init(void);
	void Update(float fPos);
	void SetDirections(SFloat3* psStart, SFloat3* psEnd);
};


class CPosition : public CUnknown
{
BASE_FUNCTIONS(CPosition);
protected:
	SIntAndPointer	msType;

public:
	void 		Init(void);
	void		Kill(void);
	void*		Set(EPositionType eType);
	void		Update(float fFractionalTime);
	SFloat3*	GetPosition(void);
};


void PositionFunction(void* pvCel, float fFractionalTime, void* pvData);


#endif // __POSITION_H__

