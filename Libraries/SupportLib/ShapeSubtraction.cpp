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
#include "ShapeSubtraction.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSubtraction::Init(Ptr<CShape> pShape, Ptr<CShape> pCut)
{
	PreInit();
	CShape::Init();
	mpShape = pShape;
	mpCut = pCut;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSubtraction::Class(void)
{
	CShape::Class();
	M_Pointer(mpShape);
	M_Pointer(mpCut);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSubtraction::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CShapeSubtraction::Distance(SFloat32Vec3* psPosition)
{
	return maxf(mpShape->Distance(psPosition), -mpCut->Distance(psPosition));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSubtraction::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	if (mpShape->Distance(psPosition) >= -mpCut->Distance(psPosition))
	{
		mpShape->Gradient(psDest, psPosition);
	}
	else
	{
		mpCut->Gradient(psDest, psPosition);
		*psDest = -*psDest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeSubtraction::GetBounds(SFloat32Bounds* psDest)
{
	mpShape->GetBounds(psDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeSubtraction::IsExact(void)
{
	return false;
}
