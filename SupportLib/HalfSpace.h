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
	SFloat32Vec3*	mpsNormal;		
	float32		d;				//d of plane.

	void 	Init(SFloat32Vec3* mpsNormal);
	void 	Set(SFloat32Vec3* v1, SFloat32Vec3* v2, SFloat32Vec3* v3);
	void 	Set2(SFloat32Vec3* psPosition1, SFloat32Vec3* psVector1, SFloat32Vec3* psVector2);
	void	Set(SFloat32Vec3* psPosition1, SFloat32Vec3* psNormal);
	void 	SetZPlane(SFloat32Vec3* v1, SFloat32Vec3* v2);  //plane is perpendicular to z-axis.
	bool 	Contains(SFloat32Vec3* psPosition);
	bool	NotContains(SFloat32Vec3* psPosition);
	bool	On(SFloat32Vec3* psPosition);
	bool	Parallel(CHalfSpace* psOther);
	int		FindFurthestPoint(SFloat32Vec3* psPoints, int iStride, int iNumPoints);
	int		FindUnsignedFurthestPoint(SFloat32Vec3* psPoints, int iStride, int iNumPoints);
	int		FindFurthestPoint(SFloat32Vec3* psPoints, int iStride, int* aiIndices, int iNumIndices);
	void	D(SFloat32Vec3* psPosition1, SFloat32Vec3* psNormal);
	void	Copy(CHalfSpace* psSource);
};


#endif // __HALF_SPACE_H__

