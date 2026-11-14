/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COOKED_MESH_H__
#define __COOKED_MESH_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Float32Vec2.h"
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Object.h"
#include "SupportLib/Material.h"


class CMesh;


//The triangles of a mesh drawn with one material, transparent or not: uiNumIndices of the cooked mesh's indices from
//uiFirstIndex, three to a triangle.  iSlot is the mesh's material slot, SCENE_FILE_NONE's -1 for none, and uiColour the colour
//a triangle is with no vertex colours, 0xAARRGGBB: the material's diffuse colour, or a grey with none.
struct SCookedSubmesh
{
	int32	iSlot;
	uint32	uiColour;
	uint32	uiFirstIndex;
	uint32	uiNumIndices;
	bool	bTransparent;
};


//A skinned vertex's weights: uiCount of them from uiFirst in the cooked mesh's weights.
struct SCookedSkinVertex
{
	uint32	uiFirst;
	uint32	uiCount;
};


//How much of a vertex a matrix moves.  A vertex's add up to one, most first.
struct SCookedSkinWeight
{
	uint32	uiMatrix;
	float32	fWeight;
};


typedef CArrayTemplate<SFloat32Vec3>			CArrayCookedVec3;
typedef CArrayTemplate<SFloat32Vec2>			CArrayCookedVec2;
typedef CArrayTemplatePrimitive<uint32>			CArrayCookedUint;
typedef CArrayTemplatePrimitive<int32>			CArrayCookedInt;
typedef CArrayTemplate<SCookedSubmesh>			CArrayCookedSubmesh;
typedef CArrayTemplate<SCookedSkinVertex>		CArrayCookedSkinVertex;
typedef CArrayTemplate<SCookedSkinWeight>		CArrayCookedSkinWeight;


//A mesh as any renderer wants it, made once by Cook from a CMesh, in floats: its vertices unique, each a corner of the mesh with
//one normal, one UV (of the first layer) and one colour, and its triangles in submeshes by material and by whether they're
//transparent.  A renderer builds what it draws from it, and it's let go.
//
//Positions and normals are streams of their own, so that a change of shape or pose need only touch them.  maiCorners is the
//mesh's corner each vertex is, for updating it in place.  A stream the mesh has nothing for is empty: no UVs, no colours, no skin.
//A vertex with no normal has a zero one.
//
//A triangle is transparent if its first corner's colour is, or its material's diffuse colour is.  A skinned mesh's vertices are
//where it was bound, each with its position's weights, made to add up to one, most first, and none of nothing.
//
//It holds its materials, in submesh order, so it's an object.
class CCookedMesh : public CObject
{
CONSTRUCTABLE(CCookedMesh);
DESTRUCTABLE(CCookedMesh);
public:
	CArrayCookedVec3		masPositions;
	CArrayCookedVec3		masNormals;
	CArrayCookedVec2		masUVs;
	CArrayCookedUint		mauiColours;
	CArrayCookedInt			maiCorners;
	CArrayCookedSkinVertex	masSkinVertices;
	CArrayCookedSkinWeight	masSkinWeights;
	uint32					muiNumMatrices;
	CArrayCookedUint		mauiIndices;
	CArrayCookedSubmesh		masSubmeshes;
	CArrayMaterial			mapMaterials;  //One for each submesh, NULL for none.
	SFloat32Vec3			msMin;
	SFloat32Vec3			msMax;

	void	Init(void);
	void	Free(void) override;
	void	Class(void) override;

	void	Cook(CMesh* pcMesh);

	size	NumVertices(void);
	size	NumTriangles(void);
	bool	IsSkinned(void);
	bool	HasColours(void);
	bool	HasUVs(void);

protected:
	void	CookVertices(CMesh* pcMesh, CArrayCookedInt* paiFaceVertices, CArrayCookedInt* paiFaceSubmeshes);
	void	CookIndices(CArrayCookedInt* paiFaceVertices, CArrayCookedInt* paiFaceSubmeshes);
	void	CookBounds(void);
};


#endif // __COOKED_MESH_H__

