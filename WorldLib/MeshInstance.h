/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __MESH_INSTANCE_H__
#define __MESH_INSTANCE_H__
#include "BaseLib/MapIntInt.h"
#include "BaseLib/ScratchPad.h"
#include "MeshObject.h"


class CMeshInstance;


enum ELinkNodeTransform
{
	LINT_None,
	LINT_SubObject,
	LINT_World,
};


enum ELinkCacheFlag
{
	LCF_NoCache,
	LCF_UseCache,
};


//To get the complete world transform the matricies must be applied in the following order:
//  psToZeroSpaceFromSubObjectSpace -> psSubObjectTransform -> psToSubObjectSpaceFromZeroSpace -> mpsWorldSpaceTransform
//I seem to recal the actual multiplication order is reversed though.
class CMeshInstanceNode
{
public:
	CMeshObjectNode*	pcObjectNode;			//For the To local space and from local space transforms.
	CMeshInstance*		pcMeshInstance;			//For the mpsWorldSpaceTransform.
	SMatrix*			psSubObjectTransform;	//In sub object space.  (From)
	SMatrix*			psWorldTransform;		//In world space (not adjusted with sub object offset).  (From)
	SMatrix*			psFinalTransform;		//Complete transform into world space.  (To)
												//Never write to this directly, use the Transform() function.
	ELinkNodeTransform	eTransform;

	void	Init(void);
	void	Kill(void);

	void	TransformFromSubObject(void);  //Setup psWorldSpaceTransform given psSubObjectTransform.
	void	TransformFromWorld(void);  //Setup psSubObjectTransform given psWorldSpaceTransform.
	void	Transform(void);
	BOOL	Update(void);
};


typedef CLinkListTemplate<CMeshInstanceNode>	CLinkListLinkInstanceNode;


//Remember that a CMeshInstance instance applies only to ONE mesh.  Should you wish to have a hierachical structure (like a man holding a box
//not a body with legs) then you must use a different class (Not written at this time).
class CMeshInstance : public CUnknown
{
BASE_FUNCTIONS(CMeshInstance);
protected:
	CMeshObject*				mpcMeshObject;
	SMatrix*					mpsWorldSpaceTransform;	
	SMatrix*					mpsInverseWorldSpaceTransform;
	CLinkListLinkInstanceNode	mcNodes;

	//All instance caching below.  There is no caching within the CMeshInstanceNodes themselves.
	ELinkCacheFlag				meVertexCaching;
	ELinkCacheFlag				meNormalCaching;
	CArraySkinnedVector			macVertexPtrCache;  //Each SkinnedVector points too a D3DXVECTOR3 in the masVertexCache.
	CArraySkinnedVector			macNormalPtrCache;	//Each SkinnedVector points too a D3DXVECTOR3 in the masNormalCache.
	CArrayVector				masVertexCache;  //The size of the array and ordering of verticies is the same as macVertexPtrCache.
	CArrayVector				masNormalCache;  //The size of the array and ordering of normals is the same as masNormalCache.

	CScratchPad*				mpcScratchPad;

public:
	void 						Init(void);
	void 						Init(CMeshObject* pcMeshObject, BOOL bCacheVertices, BOOL bCacheNormals, CScratchPad* pcScratchPad);
	void 						Kill(void);
	void 						Cache(CMeshObject* pcMeshObject, BOOL bCacheVertices, BOOL bCacheNormals);
	CMeshInstanceNode*			AddNode(CMeshObjectNode* pcMeshObjectNode);
	void						Update(void);  //This update is called by the world update.
	void						CalculateTransforms(void);
	void						UpdateMatrixState(void);
	void						UpdateCachedVectors(void);

	void						SetSkinnedVertexCachePointers(void);
	void						SetSkinnedNormalCachePointers(void);
	void						UpdateSkinnedVertexCache(void);
	void						UpdateSkinnedNormalCache(void);
	void						UpdateUnskinnedCaches(void);
	void						AssignCachedVertex(SSkinnedVector* psSkinnedVector, SSkinnedLinkObjectNode* psSkinnedLinkObjectNode);
	void						GenerateVertexScratchPadMap(CMapIntInt* pcMap);
	void						AssignCachedNormal(SSkinnedVector* psSkinnedVector, SSkinnedLinkObjectNode* psSkinnedLinkObjectNode);
	void						GenerateNormalScratchPadMap(CMapIntInt* pcMap);
	SMatrix*					GetWorldSpaceTransform(void);
	CLinkListLinkInstanceNode*	GetNodes(void);

	void						SetWorldSpaceTransform(SMatrix* psMatrix);
	void						SetInverseWorldSpaceTransform(SMatrix* psMatrix);
};


typedef CArrayTemplate<CMeshInstance*> CMeshInstancePtrArray;


#endif // __MESH_INSTANCE_H__

