/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_RENDER_FACTORY_H__
#define __RASTER_RENDER_FACTORY_H__
#include "RenderLib/RenderFactory.h"


//Makes raster renderers, each with the scale it's given, and the meshes they draw.
class CRasterRenderFactory : public CRenderFactory
{
CONSTRUCTABLE(CRasterRenderFactory);
protected:
	int32	miScale;

public:
	void				Init(int32 iScale);
	void				Kill(void);

	Ptr<CRenderer>		CreateRenderer(void) override;
	Ptr<CRenderMesh>	CreateMesh(Ptr<CRenderer> pRenderer, CMesh* pcMesh) override;
};


#endif // __RASTER_RENDER_FACTORY_H__

