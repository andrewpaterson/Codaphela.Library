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
#ifndef __MESH_UNWRAP_H__
#define __MESH_UNWRAP_H__
#include "MeshModifier.h"
#include "MeshCharts.h"


#define MESH_UNWRAP_PAGE		128		//The virtual texture's page, in texels: a layout's sides are whole pages.
#define MESH_UNWRAP_GUTTER		4		//Texels between a chart and its neighbours, at the finest level.


//How a mesh is cut into charts (CMeshCharts).
enum EMeshUnwrapCharts
{
	MUC_Box,	//By the axis direction each face is nearest.
	MUC_Grown,	//Grown across edges while faces stay within mfChartAngle of a chart's average normal.
};


//Gives a mesh UV layer 0 (UnwrapPlan.txt): its faces cut into charts (meCharts), each laid flat across its normal, turned to
//its smallest bounding rectangle, and the charts packed into a layout of texels at mfTexelsPerUnit texels to a unit of the
//mesh, its sides whole pages.  The UVs are each corner's place in the layout, 0 to 1 across it; a face anticlockwise seen from
//outside is anticlockwise in UV, u to the right and v up.  Corners of one chart share their UVs, and a corner on a seam has one
//for each chart it's in.
//
//Laid flat across its normal, a chart is stretched where its faces slope from it (by 1 / cos of the angle along the slope), and
//can't fold, as no face is a right angle from it.  It can overlap itself (round a torus's inside): its faces are grown again
//at half the angle, until no chart does.  The rectangles, a gutter round each, are packed tallest first along a
//skyline, not yet onto pages.  A layout's size and how many charts it has are kept (miLayoutWidth and so on) once it has been
//made, for a bake.  Everything else the mesh has is kept, and its other UV layers.
class CMeshUnwrap : public CMeshModifier
{
CONSTRUCTABLE(CMeshUnwrap);
DESTRUCTABLE(CMeshUnwrap);
public:
	float32				mfTexelsPerUnit;
	EMeshUnwrapCharts	meCharts;
	float32				mfChartAngle;  //Radians, for MUC_Grown.
	int					miPageSize;
	int					miGutter;

	int					miLayoutWidth;  //In texels, as last made.
	int					miLayoutHeight;
	int					miNumCharts;

	void		Init(float32 fTexelsPerUnit, EMeshUnwrapCharts eCharts = MUC_Box);
	void		Class(void) override;

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_UNWRAP_H__

