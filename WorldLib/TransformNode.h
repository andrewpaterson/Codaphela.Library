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
#ifndef __TRANSFORM_NODE_H_
#define __TRANSFORM_NODE_H_
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayPointer.h"
#include "CoreLib/ArrayVariable.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/DirectXGlobal.h"
#include "Interpolator.h"


enum EMatrixType
{
	MT_SphericalLinearMatrix,
};


typedef CInterpolator<SFloat4x4> __CTransformNodeInterpolator;
class CTransformNodeInterpolator : public __CTransformNodeInterpolator
{
public:
	void	Init(void);
	void	Kill(void);
	void	Interpolate(float fPos, SFloat4x4* psMatrix1, SFloat4x4* psMatrix2);
	void	Update(float fPos);
};


class CTransformNode : public CUnknown
{
BASE_FUNCTIONS(CTransformNode);
protected:
	STypedPointer	msType;

public:
	void 			Init(void);
	void			Kill(void);
	void*			Set(EMatrixType eType);
	void			Update(float fFractionalTime);
	SFloat4x4*		GetMatrix(void);
};


void MatrixFunction(void* psCel, float fFractionalTime, void* pvData);


#endif // __TRANSFORM_NODE_H_

