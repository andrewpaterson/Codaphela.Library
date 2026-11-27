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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MESH_CHARTS_H__
#define __MESH_CHARTS_H__
#include "BaseLib/ArrayInt.h"
#include "Mesh.h"


//The angle a grown chart's faces may be from its average normal, in radians (60 degrees).
#define MESH_CHARTS_ANGLE		(1.0471976f)


//A mesh's faces cut into charts, pieces each laid flat across its own normal (UnwrapPlan.txt, Charts): which chart each face is
//in, each chart's faces, and each chart's normal, a unit.
//
//MakeBox: each face goes to whichever of the six axis directions its normal is nearest, and faces of one direction joined by
//an edge are a chart, its normal that axis.
//
//Grow: from each face not yet in a chart (in order), a chart grows across edges, taking next whichever face beside it is
//nearest its average normal (weighted by area) while that's within fMaxAngle.  A chart laid flat across its normal can't fold
//where no face is a right angle or more from it; the average moves as a chart grows, so a face that ends up further than that
//(more than 85 degrees) leaves it and starts or joins another.
//
//Regrow: the faces of some charts grown again (at a smaller angle, by a chart that overlaps itself laid flat), the others kept.
//Design page: Docs/libraries/supportlib/unwrapping.md
class CMeshCharts
{
public:
	CArrayLargeInt			maiFaceCharts;
	CArrayLargeInt			maiChartStarts;  //Each chart's first in maiChartFaces, and one more after the last.
	CArrayLargeInt			maiChartFaces;
	CArrayLargeFloat32Vec3	masNormals;

	void	Init(void);
	void	Kill(void);

	void	MakeBox(CMesh* pcMesh);
	void	Grow(CMesh* pcMesh, float32 fMaxAngle = MESH_CHARTS_ANGLE);
	void	Regrow(CMesh* pcMesh, CArrayInt* paiCharts, float32 fMaxAngle);

	int		NumCharts(void) const;
	int		NumFaces(int iChart);
	int		GetFace(int iChart, int i);

protected:
	void	GrowFree(CMesh* pcMesh, float32 fMaxAngle);
	void	ListFaces(int iNumFaces, int iNumCharts);
};


//A face's normal, its length twice the face's area.
void	MeshFaceAreaNormal(SFloat32Vec3* psDest, CMesh* pcMesh, int iFace);


#endif // __MESH_CHARTS_H__

