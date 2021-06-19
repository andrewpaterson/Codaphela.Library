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
#ifndef __SHAPE_HELPER_H__
#define __SHAPE_HELPER_H__
#include "Sphere.h"
#include "Triangle.h"
#include "Plane.h"


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class CSphereHelper : public CSphere
{
public:
	SFloat3		msPosition;

	void Init(void);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class CTriangleHelper : public CTriangle
{
public:
	SFloat3		masPosition[3];
	SFloat3		msNormal;

	void Init(void);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class CPlaneHelper : public CPlane
{
public:
	SFloat3		msPosition;
	SFloat3		msNormal;

	void Init(void);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class CHalfSpaceHelper : public CHalfSpace
{
public:
	SFloat3		msNormal;

	void Init(void);
	void Kill(void);
};


#endif // !__SHAPE_HELPER_H__

