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
#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "BaseLib/Float32Bounds.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


//The step of Gradient's central differences, as a fraction of the shape's bounds' longest side.
#define SHAPE_GRADIENT_STEP		(1e-4f)


//A solid that says how far a point is from its surface (ShapePlan.txt).  Distance is negative inside, positive outside, and
//exactly zero is outside: Contains is Distance < 0.  Distance may be a bound, never more than the true distance (IsExact says
//which).  Gradient points away from the surface and needn't be a unit; a shape that can give it exactly overrides it, as
//central differences across a crease average its faces.  Bounds may be as large as a float32 goes (a half space).  AddMesh adds
//the mesh its own mesh editor makes of it, with the same values (ShapePlan.txt, section 5), for a primitive that has one.
class CMeshEditor;
//Design page: Docs/libraries/supportlib/shapes.md
class CShape : public CObject
{
CONSTRUCTABLE(CShape);
DESTRUCTABLE(CShape);
public:
	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	virtual float32	Distance(SFloat32Vec3* psPosition) = 0;
	virtual void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition);
	virtual void	GetBounds(SFloat32Bounds* psDest) = 0;
	virtual bool	IsExact(void);
	bool			Contains(SFloat32Vec3* psPosition);

	virtual bool	AddMesh(CMeshEditor* pcMeshEditor, int iSegments);  //iSegments around a circle.  False if it has no mesh editor.
};


//The segments around a circle a primitive's mesh has: iSegments, but at least 6, so half of them (a sphere's rings, a torus's
//tube's) is 3.
int ShapeMeshSegments(int iSegments);


#endif // __SHAPE_H__

