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
#include "BaseLib/Jobs.h"
#include "Image.h"
#include "MeshModifier.h"
#include "SolidTexture.h"


//The most samples each way in a texel a CMeshBake takes.
#define MESH_BAKE_SAMPLES	4

//Where a CMeshBake's relief is lit from, in the mesh's space: above, to the left and in front, z being up.
#define MESH_BAKE_LIGHT_X	(-0.5f)
#define MESH_BAKE_LIGHT_Y	(-0.6f)
#define MESH_BAKE_LIGHT_Z	(0.62f)


//How a bake makes a texel of a texture.
//
//Height (TexturePlan.txt, Height): where the texture's height rises across a texel the surface is tilted, as if it were
//raised that far (Blinn's bump mapping).  The slope is found from the texel's own samples (so there are at least two each
//way when it's wanted), never from its neighbours in the image, so it's right across seams.  fRelief shades the colour by
//it: by how much more or less of the light at sLight the tilted surface faces than the smooth one does, times fRelief,
//which is what the bumps would look like lit from there, left in the colour as a renderer lights the smooth surface.  A
//texel that glows is shaded that much less.
//
//Jobs: with pcJobs the bake's rows are shared out in bands, a job each, and the bake waits for them.  The image is the
//same texel for texel however many threads there are.  The jobs' threads read the mesh and the texture through plain
//pointers, which the caller holds for them: a texture's TextureSample (and a shape's Distance, which CSolidCavity asks)
//must not copy, assign or let go of a Ptr, as those threads aren't registered to (StandardLib's ObjectsLock.h; a Debug
//build says when one does).
struct SMeshBakeSettings
{
	int				iSamples;  //Each way across the texel, spread evenly: the texel is their mean.
	float32			fRelief;  //How strongly the texture's height shades its colour; 0 for not at all, 1 as the light would.
	SFloat32Vec3	sLight;  //A unit, towards the light the relief is lit by, in the mesh's space.
	CJobs*			pcJobs;  //What bakes the rows; NULL for the caller's thread.

	void	Init(void);  //One sample, at the texel's centre; no relief; the caller's thread.
};


//A texture iWidth by iHeight texels of pcTexture on pcMesh's faces, through UV layer iLayer (UnwrapPlan.txt, Bake).  Each
//texel whose centre is inside a face's UV triangle is the texture's colour at the point and normal its barycentric position
//gives (the corners' normals if the mesh has them, the face's if not), averaged over the face's texel width; then every texel
//no face covers takes the colour of the nearest one that is, so filtering at a seam or a gutter finds the chart's colour, not
//the background's.  Rows are from the top, a texel's row v times iHeight, as the renderers sample.  With pauiTints (a colour
//for each face) each face's texels are its tint times the texture.  An 8 bit red, green, blue image; NULL if the mesh hasn't
//the layer.  A mip level is the same at half the size, as finer levels are never shrunk.  Without psSettings a texel is one
//sample.
//
//With pNormals it also gives the surface's normal at each texel, tilted by the texture's height: an 8 bit image of x, y and
//z in the mesh's space, each from -1 at 0 to 1 at 255.  Every texel is its own, so the normals need no tangents and meet
//across seams.
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints = NULL, SMeshBakeSettings* psSettings = NULL);
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints, SMeshBakeSettings* psSettings, Ptr<CImage>& pNormals);


//Bakes mpTexture onto the mesh through UV layer 0 at the layer's layout size (as CMeshUnwrap makes it), every mip level, and
//gives every face one material whose diffuse image (and its levels) it is, its colour white so the image is the colour.  Each
//face's own material's diffuse colour tints its texels, so the texture is all its colour.  mpImage is the last level 0 baked.
//
//Level 0 is one sample a texel.  A texture's noise averages itself over a texel, but an edge a ramp or a mask makes of it
//doesn't, and coarse levels of such a texture are wrong (a marble's veins fade and the whole lightens, TexturePlan.txt,
//Filtering).  So each level after has twice the samples each way, up to miSamples: with 4, about two and a half times the
//work of one sample everywhere, for coarse levels 4 to 10 times nearer the truth.
//
//The texture's height shades its colour (mfRelief, lit from msLight; SMeshBakeSettings says how), which takes two samples
//each way at level 0 too; with no relief, or a texture with no height, the colour is the texture's own.  With mbNormals the
//tilted normals are baked as well, the material's bump image and its levels (mpNormals the last level 0): neither renderer
//reads them yet.
//
//SetJobs gives it threads to bake on (SMeshBakeSettings, Jobs).  They aren't saved with it.
class CMeshBake : public CMeshModifier
{
CONSTRUCTABLE(CMeshBake);
DESTRUCTABLE(CMeshBake);
public:
	Ptr<CSolidTexture>	mpTexture;
	Ptr<CImage>			mpImage;
	Ptr<CImage>			mpNormals;
	int					miSamples;  //The most each way in a texel.
	float32				mfRelief;
	SFloat32Vec3		msLight;  //A unit.
	bool				mbNormals;
	CJobs*				mpcJobs;  //NULL to bake on the caller's thread.

	void		Init(Ptr<CSolidTexture> pTexture);
	void		Class(void) override;

	void		SetJobs(CJobs* pcJobs);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_BAKE_H__

