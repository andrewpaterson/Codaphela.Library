/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_SUBNET_GRAPH_H__
#define __KICAD_SCHEM_SUBNET_GRAPH_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "KicadSchemSubnet.h"


typedef CArrayTemplate<SKicadSchemPoint>	CArrayKicadSchemPoint;


//The way a subnet's wires are drawn, as points joined along its wires: the ends of its wires, its junctions and any
//points added (the pins and labels that are connected).  A wire is split at every point on it.  FindPath goes along
//the wires from a point to the nearest of a set of points, so the route KiCad drew between two of a subnet's pins
//can be followed.
class CKicadSchemSubnetGraph
{
protected:
	CKicadSchemSubnet*		mpcSubnet;
	CArrayKicadSchemPoint	masPoints;
	CArrayInt				maiEdgeStarts;
	CArrayInt				maiEdgeEnds;

public:
	void				Init(CKicadSchemSubnet* pcSubnet);
	void				Kill(void);

	size				AddPoint(SKicadSchemPoint* psPoint);
	void				Build(void);

	size				NumPoints(void);
	SKicadSchemPoint*	GetPoint(size uiIndex);
	size				NumEdges(void);
	bool				FindPath(size uiFrom, CArrayInt* paiTargets, CArrayInt* paiPath);

	static float64		GetBendFraction(CArrayKicadSchemPoint* pasPath, bool* pbHorizontal);

protected:
	size				FindPoint(SKicadSchemPoint* psPoint);
	void				AddWireEdges(CKicadSchemWire* pcWire);
};


#endif // __KICAD_SCHEM_SUBNET_GRAPH_H__

