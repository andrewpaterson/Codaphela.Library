/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#include "BaseLib/FloatHelper.h"
#include "ShapeTransform.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeTransform::Init(Ptr<CShape> pShape, SFloat4x4* psMatrix)
{
	float32		fDeterminant;
	float32		afScales[3];
	bool		bResult;

	PreInit();
	CShape::Init();
	mpShape = pShape;
	msMatrix = *psMatrix;
	bResult = Float4x4Inverse(&msInverse, &fDeterminant, &msMatrix);

	afScales[0] = SquareRoot(msMatrix.x.x * msMatrix.x.x + msMatrix.x.y * msMatrix.x.y + msMatrix.x.z * msMatrix.x.z);
	afScales[1] = SquareRoot(msMatrix.y.x * msMatrix.y.x + msMatrix.y.y * msMatrix.y.y + msMatrix.y.z * msMatrix.y.z);
	afScales[2] = SquareRoot(msMatrix.z.x * msMatrix.z.x + msMatrix.z.y * msMatrix.z.y + msMatrix.z.z * msMatrix.z.z);
	mfScale = minf(afScales[0], minf(afScales[1], afScales[2]));
	mbEvenScale = (maxf(afScales[0], maxf(afScales[1], afScales[2])) - mfScale) <= mfScale * 1e-5f;
	PostInit();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeTransform::Class(void)
{
	CShape::Class();
	M_Pointer(mpShape);
	U_4x4Float32(msMatrix);
	U_4x4Float32(msInverse);
	U_Float32(mfScale);
	U_Bool(mbEvenScale);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeTransform::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CShapeTransform::Distance(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sLocal;

	Float32Vec3TransformCoord(&sLocal, psPosition, &msInverse);
	return mpShape->Distance(&sLocal) * mfScale;
}


//////////////////////////////////////////////////////////////////////////
//
// The shape's gradient in its frame, back through msInverse the other way
// (its transpose), as a change of the point here changes the point there
// by msInverse.
//
//////////////////////////////////////////////////////////////////////////
void CShapeTransform::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sLocal;
	SFloat32Vec3	sGradient;

	Float32Vec3TransformCoord(&sLocal, psPosition, &msInverse);
	mpShape->Gradient(&sGradient, &sLocal);
	psDest->Init((msInverse.x.x * sGradient.x + msInverse.x.y * sGradient.y + msInverse.x.z * sGradient.z) * mfScale,
				 (msInverse.y.x * sGradient.x + msInverse.y.y * sGradient.y + msInverse.y.z * sGradient.z) * mfScale,
				 (msInverse.z.x * sGradient.x + msInverse.z.y * sGradient.y + msInverse.z.z * sGradient.z) * mfScale);
}


//////////////////////////////////////////////////////////////////////////
//
// The shape's bounds' corners, moved.  Bounds as large as a float32 goes
// stay that large.
//
//////////////////////////////////////////////////////////////////////////
void CShapeTransform::GetBounds(SFloat32Bounds* psDest)
{
	SFloat32Bounds	sBounds;
	SFloat32Vec3	sCorner;
	SFloat32Vec3	sMoved;
	int				i;

	mpShape->GetBounds(&sBounds);
	if ((sBounds.sMinimum.x <= -FLOAT32_BOUNDS_LARGEST) || (sBounds.sMaximum.x >= FLOAT32_BOUNDS_LARGEST))
	{
		*psDest = sBounds;
		return;
	}

	psDest->Init();
	for (i = 0; i < 8; i++)
	{
		sCorner.Init((i & 1) ? sBounds.sMaximum.x : sBounds.sMinimum.x, (i & 2) ? sBounds.sMaximum.y : sBounds.sMinimum.y, (i & 4) ? sBounds.sMaximum.z : sBounds.sMinimum.z);
		Float32Vec3TransformCoord(&sMoved, &sCorner, &msMatrix);
		psDest->Add(&sMoved);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeTransform::IsExact(void)
{
	return mbEvenScale && mpShape->IsExact();
}
