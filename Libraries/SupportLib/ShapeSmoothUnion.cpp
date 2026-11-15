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
#include "ShapeSmoothUnion.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSmoothUnion::Init(Ptr<CShape> pShape1, Ptr<CShape> pShape2, float32 fRadius)
{
	PreInit();
	CShape::Init();
	mpShape1 = pShape1;
	mpShape2 = pShape2;
	mfRadius = fRadius;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSmoothUnion::Class(void)
{
	CShape::Class();
	M_Pointer(mpShape1);
	M_Pointer(mpShape2);
	U_Float32(mfRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSmoothUnion::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// How much of the first shape is in the blend at a point: 1 where it's
// nearer by mfRadius or more, 0 where the second is, and a half where
// they're as near.
//
//////////////////////////////////////////////////////////////////////////
float32 CShapeSmoothUnion::Blend(float32 fDistance1, float32 fDistance2)
{
	if (mfRadius <= 0.0f)
	{
		return (fDistance1 <= fDistance2) ? 1.0f : 0.0f;
	}
	return minf(maxf(0.5f + 0.5f * (fDistance2 - fDistance1) / mfRadius, 0.0f), 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CShapeSmoothUnion::Distance(SFloat32Vec3* psPosition)
{
	float32		fDistance1;
	float32		fDistance2;
	float32		fBlend;

	fDistance1 = mpShape1->Distance(psPosition);
	fDistance2 = mpShape2->Distance(psPosition);
	fBlend = Blend(fDistance1, fDistance2);
	return fDistance2 + (fDistance1 - fDistance2) * fBlend - mfRadius * fBlend * (1.0f - fBlend);
}


//////////////////////////////////////////////////////////////////////////
//
// The blend of the two gradients: the terms the blend's own change brings
// cancel where it's changing, so this is exact.
//
//////////////////////////////////////////////////////////////////////////
void CShapeSmoothUnion::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sGradient1;
	SFloat32Vec3	sGradient2;
	float32			fBlend;

	fBlend = Blend(mpShape1->Distance(psPosition), mpShape2->Distance(psPosition));
	mpShape1->Gradient(&sGradient1, psPosition);
	mpShape2->Gradient(&sGradient2, psPosition);
	*psDest = sGradient1 * fBlend + sGradient2 * (1.0f - fBlend);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSmoothUnion::GetBounds(SFloat32Bounds* psDest)
{
	SFloat32Bounds	sBounds;
	float32			fGrow;

	mpShape1->GetBounds(psDest);
	mpShape2->GetBounds(&sBounds);
	psDest->Add(&sBounds);
	fGrow = mfRadius / 4.0f;
	psDest->sMinimum.Init(psDest->sMinimum.x - fGrow, psDest->sMinimum.y - fGrow, psDest->sMinimum.z - fGrow);
	psDest->sMaximum.Init(psDest->sMaximum.x + fGrow, psDest->sMaximum.y + fGrow, psDest->sMaximum.z + fGrow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeSmoothUnion::IsExact(void)
{
	return false;
}
