/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __HALF_SPACE_H__
#define __HALF_SPACE_H__
#include "BaseLib/GeometricTypes.h"


class CHalfSpace
{
public:
	SFloat3*	mpsNormal;		
	float		d;				//d of plane.

	void 	Init(SFloat3* mpsNormal);
	void 	Set(SFloat3* v1, SFloat3* v2, SFloat3* v3);
	void 	Set2(SFloat3* psPosition1, SFloat3* psVector1, SFloat3* psVector2);
	void	Set(SFloat3* psPosition1, SFloat3* psNormal);
	void 	SetZPlane(SFloat3* v1, SFloat3* v2);  //plane is perpendicular to z-axis.
	bool 	Contains(SFloat3* psPosition);
	bool	NotContains(SFloat3* psPosition);
	bool	On(SFloat3* psPosition);
	bool	Parallel(CHalfSpace* psOther);
	int		FindFurthestPoint(SFloat3* psPoints, int iStride, int iNumPoints);
	int		FindUnsignedFurthestPoint(SFloat3* psPoints, int iStride, int iNumPoints);
	int		FindFurthestPoint(SFloat3* psPoints, int iStride, int* aiIndices, int iNumIndices);
	void	D(SFloat3* psPosition1, SFloat3* psNormal);
};


#endif // __HALF_SPACE_H__

