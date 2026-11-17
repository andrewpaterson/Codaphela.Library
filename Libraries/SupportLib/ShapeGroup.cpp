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
#include "ShapeGroup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeGroup::Init(void)
{
	PreInit();
	CShape::Init();
	mapShapes.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeGroup::Class(void)
{
	CShape::Class();
	M_Embedded(mapShapes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeGroup::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeGroup::Add(Ptr<CShape> pShape)
{
	return mapShapes.Add(pShape);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CShapeGroup::NumShapes(void)
{
	return mapShapes.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CShape> CShapeGroup::GetShape(size uiShape)
{
	return mapShapes.Get(uiShape);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CShapeGroup::Distance(SFloat32Vec3* psPosition)
{
	float32		fDistance;

	Choose(psPosition, &fDistance);
	return fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
// The chosen shape's.  With no shapes, nothing.
//
//////////////////////////////////////////////////////////////////////////
void CShapeGroup::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	CShape*		pcShape;
	float32		fDistance;

	pcShape = Choose(psPosition, &fDistance);
	if (pcShape != NULL)
	{
		pcShape->Gradient(psDest, psPosition);
	}
	else
	{
		psDest->Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeGroup::IsExact(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The shape whose distance Prefers over every other's, the first of those
// that are as good, and its distance.  NULL, as far as can be, with none.
//
//////////////////////////////////////////////////////////////////////////
CShape* CShapeGroup::Choose(SFloat32Vec3* psPosition, float32* pfDistance)
{
	CShape*		pcShape;
	CShape*		pcChosen;
	size		uiNumShapes;
	size		i;
	float32		fDistance;

	pcChosen = NULL;
	*pfDistance = FLOAT32_BOUNDS_LARGEST;
	uiNumShapes = mapShapes.NumElements();
	for (i = 0; i < uiNumShapes; i++)
	{
		pcShape = mapShapes.GetPlain(i);
		fDistance = pcShape->Distance(psPosition);
		if ((pcChosen == NULL) || Prefers(fDistance, *pfDistance))
		{
			pcChosen = pcShape;
			*pfDistance = fDistance;
		}
	}
	return pcChosen;
}
