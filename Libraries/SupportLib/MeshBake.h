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
#ifndef __MESH_BAKE_H__
#define __MESH_BAKE_H__
#include "Image.h"
#include "MeshModifier.h"
#include "MeshUnwrap.h"
#include "SolidTexture.h"


//A texture iWidth by iHeight texels of pcTexture on pcMesh's faces, through UV layer iLayer (UnwrapPlan.txt, Bake).  Each
//texel whose centre is inside a face's UV triangle is the texture's colour at the point and normal its barycentric position
//gives (the corners' normals if the mesh has them, the face's if not), averaged over the face's texel width; then every texel
//no face covers takes the colour of the nearest one that is, so filtering at a seam or a gutter finds the chart's colour, not
//the background's.  Rows are from the top, a texel's row v times iHeight, as the renderers sample.  An 8 bit red, green, blue
//image; NULL if the mesh hasn't the layer.  A mip level is the same at half the size, as finer levels are never shrunk.
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight);


//Bakes mpTexture onto the mesh at the size of mpUnwrap's layout (UV layer 0, made by mpUnwrap before it on the stack), and gives
//every face one material whose diffuse image it is, its colour white so the image is the colour.  The mesh's own materials are
//replaced: the texture is all its colour.  mpImage is the last image baked.
class CMeshBake : public CMeshModifier
{
CONSTRUCTABLE(CMeshBake);
DESTRUCTABLE(CMeshBake);
public:
	Ptr<CSolidTexture>	mpTexture;
	Ptr<CMeshUnwrap>	mpUnwrap;
	Ptr<CImage>			mpImage;

	void		Init(Ptr<CSolidTexture> pTexture, Ptr<CMeshUnwrap> pUnwrap);
	void		Class(void) override;

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_BAKE_H__

