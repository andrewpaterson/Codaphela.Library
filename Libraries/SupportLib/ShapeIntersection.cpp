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
#include "ShapeIntersection.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeIntersection::GetBounds(SFloat32Bounds* psDest)
{
	SFloat32Bounds	sShape;
	size			i;

	psDest->Init();
	for (i = 0; i < mapShapes.NumElements(); i++)
	{
		mapShapes.Get(i)->GetBounds(&sShape);
		if (i == 0)
		{
			*psDest = sShape;
		}
		else
		{
			Float32Vec3Maximize(&psDest->sMinimum, &psDest->sMinimum, &sShape.sMinimum);
			Float32Vec3Minimize(&psDest->sMaximum, &psDest->sMaximum, &sShape.sMaximum);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShapeIntersection::Prefers(float32 fDistance, float32 fChosen)
{
	return fDistance > fChosen;
}
