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
#ifndef __MESH_OBJECT_H__
#define __MESH_OBJECT_H__
#include "VectorArray.h"
#include "BaseLib/LinkedListTemplate.h"
#include "StandardLib/StandardHeader.h"
#include "DirectXLib/Direct3DGlobal.h"


//A better name for this might be mesh object.  Only link instances of a link object are drawable.


//This is ONE matrix in a skinned transform hierachy.
class CMeshObjectNode
{
public:
	SMatrix*		psToZeroSpaceFromSubObjectSpace;
	SMatrix*		psToSubObjectSpaceFromZeroSpace;
	CArrayVector	mcVerticies;
	CArrayVector	mcNormals;

	void 			Init(void);
	void 			Init(int iNumVerticies, int iNumNormals);
	void 			Kill(void);
	void 			SetVertex(int iVertex, SFloat3* psVec);
	void 			SetNormal(int iNormal, SFloat3* psVec);
	CArrayVector*	GetVerticies(void);
	CArrayVector*	GetNormals(void);
};
typedef CLinkedListTemplate<CMeshObjectNode>	CLinkListLinkObjectNode;


struct SSkinnedLinkObjectNode
{
	CMeshObjectNode*	apcNodes[4];  //This can contain the CMeshObject, so cast as necessary
	int					iVertIndex[4];  //index into the CArrayVector within the node.
	float				afWeights[4];
};
typedef CArrayTemplate<SSkinnedLinkObjectNode> CArraySkinnedLinkObjectNodes;


//The CMeshObject is itself a CMeshObjectNode however the to and from matricies are both the identity matrix.
//Thus the CMeshObject is the zero'th world matrix and the first link node is the one'th world matrix etc...
//There is a maximum of 255 LinkObjectNodes (which is a directX limitation).
class CMeshObject : public CUnknown
{
BASE_FUNCTIONS(CMeshObject);
protected:
	CLinkListLinkObjectNode			mcNodes;
	CArrayVector					mcVerticies;  //Each vertex (and normal) may be duplicated up to 4 times depending on the number of
	CArrayVector					mcNormals;	  //CMeshObjectNodes affecting it.  See above comment.  This is too optimize transforms
												  //for D3DXVec3TransformCoord and D3DXVec3TransformNormal.

	CArraySkinnedLinkObjectNodes	mcSkinnedVertexPtrs;  //Only valid if one or more nodes.  This size is unrelated to mcVerticies size.
	CArraySkinnedLinkObjectNodes	mcSkinnedNormalPtrs;  //Only valid if one or more nodes.  This size is unrelated to mcNormals size.
	
public:
	void 							Init(void);
	void 							Init(int iNumVerticies, int iNumNormals);
	void 							Kill(void);
	void 							SetVertex(int iVertex, SFloat3* psVec);
	void 							SetNormal(int iNormal, SFloat3* psVec);
	BOOL 							IsSkinned(void);

	CArrayVector*					GetVerticies(void);
	CArrayVector*					GetNormals(void);
	CLinkListLinkObjectNode*		GetNodes(void);
	CArraySkinnedLinkObjectNodes*	GetSkinnedVertexPtrs(void);
	CArraySkinnedLinkObjectNodes*	GetSkinnedNormalPtrs(void);
};


#endif // __MESH_OBJECT_H__

