/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/SystemAllocator.h"
#include "MeshInstance.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstanceNode::Init(void)
{
	memset(this, 0, sizeof(CMeshInstanceNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstanceNode::Kill(void)
{
	//Does nothing.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstanceNode::TransformFromSubObject(void)
{
	SFloat4x4	sOut;

	Float4x4Multiply(&sOut, &pcMeshInstance->GetWorldSpaceTransform()->sD3DMatrix, &pcObjectNode->psToSubObjectSpaceFromZeroSpace->sD3DMatrix);
	Float4x4Multiply(&sOut, &sOut, &psSubObjectTransform->sD3DMatrix);
	Float4x4Multiply(&psFinalTransform->sD3DMatrix, &sOut, &pcObjectNode->psToZeroSpaceFromSubObjectSpace->sD3DMatrix);

	if (psWorldTransform)
	{
		//Do this...
	}
	psFinalTransform->Fix();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstanceNode::TransformFromWorld(void)
{
	psFinalTransform->sD3DMatrix = psWorldTransform->sD3DMatrix;
	if (psSubObjectTransform)
	{
		//Do this...
	}
	psFinalTransform->Fix();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstanceNode::Transform(void)
{
	if (eTransform == LINT_SubObject)
	{
		TransformFromSubObject();
	}
	else if (eTransform == LINT_World)
	{
		TransformFromWorld();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshInstanceNode::Update(void)
{
	return psFinalTransform->Update();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::Init(void)
{
	mpcMeshObject = NULL;
	mpcScratchPad = NULL;
	mpsWorldSpaceTransform = NULL;	
	mpsInverseWorldSpaceTransform = NULL;
	mcNodes.Init();
	macVertexPtrCache.Init();
	macNormalPtrCache.Init();
	meVertexCaching = LCF_NoCache;
	meNormalCaching = LCF_NoCache;
	masVertexCache.Init();
	masNormalCache.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::Init(CMeshObject* pcMeshObject, bool bCacheVertices, bool bCacheNormals, CScratchPad* pcScratchPad)
{
	Init();
	mpcScratchPad = pcScratchPad;
	Cache(pcMeshObject, bCacheVertices, bCacheNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::Kill(void)
{
	mcNodes.Kill();  //No need to call kill on each node.
	macVertexPtrCache.Kill();
	macNormalPtrCache.Kill();
	masVertexCache.Kill();
	masNormalCache.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::Cache(CMeshObject* pcMeshObject, bool bCacheVertices, bool bCacheNormals)
{
	mpcMeshObject = pcMeshObject;

	if (bCacheVertices)
	{
		SetSkinnedVertexCachePointers();
	}
	if (bCacheNormals)
	{
		SetSkinnedNormalCachePointers();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshInstanceNode* CMeshInstance::AddNode(CMeshObjectNode* pcMeshObjectNode)
{
	CMeshInstanceNode*	psLinkInstanceNode;
	
	if (mcNodes.GetHead() == NULL)
	{
		psLinkInstanceNode = mcNodes.InsertBeforeHead();
	}
	else
	{
		psLinkInstanceNode = mcNodes.InsertAfterNode(mcNodes.GetTail());
	}
	
	psLinkInstanceNode->pcObjectNode = pcMeshObjectNode;
	psLinkInstanceNode->eTransform = LINT_None;
	psLinkInstanceNode->pcMeshInstance = this;
	psLinkInstanceNode->psSubObjectTransform = NULL;
	psLinkInstanceNode->psWorldTransform = NULL;
	psLinkInstanceNode->psFinalTransform = NULL;
	return psLinkInstanceNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::Update(void)
{
	CalculateTransforms();
	UpdateMatrixState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::CalculateTransforms(void)
{
	CMeshInstanceNode*	psNode;

	psNode = mcNodes.GetHead();
	while (psNode)
	{
		psNode->Transform();
		psNode = mcNodes.GetNext(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::UpdateMatrixState(void)
{
	CMeshInstanceNode*	psNode;
	bool				bAnyMatriciesChanged;

	if (!((meVertexCaching == LCF_NoCache) && (meNormalCaching == LCF_NoCache)))
	{
		bAnyMatriciesChanged = mpsInverseWorldSpaceTransform->Update();
		if (!bAnyMatriciesChanged)
		{
			psNode = mcNodes.GetHead();
			while (psNode)
			{
				bAnyMatriciesChanged = psNode->Update();
				if (bAnyMatriciesChanged)
				{
					break;
				}

				psNode = mcNodes.GetNext(psNode);
			}
		}

		if (bAnyMatriciesChanged)
		{
			UpdateCachedVectors();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::UpdateCachedVectors(void)
{
	if (!mpcMeshObject->IsSkinned())
	{
		UpdateUnskinnedCaches();
	}
	else
	{
		UpdateSkinnedVertexCache();
		UpdateSkinnedNormalCache();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::UpdateUnskinnedCaches(void)
{
	if (meVertexCaching == LCF_UseCache)
	{
		if (masVertexCache.NumElements() == 0)
		{
			masVertexCache.Kill();
			masVertexCache.Init();
			masVertexCache.Resize(mpcMeshObject->GetVerticies()->NumElements());
		}
		Float3TransformCoords(masVertexCache.GetData(), sizeof(SFloat3), mpcMeshObject->GetVerticies()->GetData(), sizeof(SFloat3), &mpsWorldSpaceTransform->sD3DMatrix, mpcMeshObject->GetVerticies()->NumElements());
	}
	if (meNormalCaching == LCF_UseCache)
	{
		if (masNormalCache.NumElements() == 0)
		{
			masNormalCache.Kill();
			masNormalCache.Init();
			masNormalCache.Resize(mpcMeshObject->GetNormals()->NumElements());
		}
		Float3TransformNormals(masNormalCache.GetData(), sizeof(SFloat3), mpcMeshObject->GetNormals()->GetData(), sizeof(SFloat3), &mpsWorldSpaceTransform->sD3DMatrix, mpcMeshObject->GetNormals()->NumElements());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::UpdateSkinnedVertexCache(void)
{
	size				i;
	CMeshInstanceNode*	psNode;
	SFloat3*			pvScratchVectors;
	SSkinnedVector*		psSkinnedVector;

	if (meVertexCaching == LCF_UseCache)
	{
		mpcScratchPad->Reset();
		pvScratchVectors = (SFloat3*)mpcScratchPad->Add(mpcMeshObject->GetVerticies()->NumElements() * sizeof(SFloat3));
		Float3TransformCoords(pvScratchVectors, sizeof(SFloat3), mpcMeshObject->GetVerticies()->GetData(), sizeof(SFloat3), &mpsWorldSpaceTransform->sD3DMatrix, mpcMeshObject->GetVerticies()->NumElements());

		psNode = mcNodes.GetHead();
		while (psNode)
		{
			pvScratchVectors = (SFloat3*)mpcScratchPad->Add(psNode->pcObjectNode->GetVerticies()->NumElements() * sizeof(SFloat3));
			Float3TransformCoords(pvScratchVectors, sizeof(SFloat3), psNode->pcObjectNode->GetVerticies()->GetData(), sizeof(SFloat3), &psNode->psFinalTransform->sD3DMatrix, psNode->pcObjectNode->GetVerticies()->NumElements());

			psNode = mcNodes.GetNext(psNode);
		}

		psSkinnedVector = macVertexPtrCache.Get(0);
		for (i = 0; i < macVertexPtrCache.NumElements(); i++)
		{
			psSkinnedVector[i].Cache();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::UpdateSkinnedNormalCache(void)
{
	size				i;
	CMeshInstanceNode*	psNode;
	SFloat3*			pvScratchVectors;
	SSkinnedVector*		psSkinnedVector;

	if (meNormalCaching == LCF_UseCache)
	{
		mpcScratchPad->Reset();
		pvScratchVectors = (SFloat3*)mpcScratchPad->Add(mpcMeshObject->GetNormals()->NumElements() * sizeof(SFloat3));
		Float3TransformNormals(pvScratchVectors, sizeof(SFloat3), mpcMeshObject->GetNormals()->GetData(), sizeof(SFloat3), &mpsWorldSpaceTransform->sD3DMatrix, mpcMeshObject->GetNormals()->NumElements());

		psNode = mcNodes.GetHead();
		while (psNode)
		{
			pvScratchVectors = (SFloat3*)mpcScratchPad->Add(psNode->pcObjectNode->GetNormals()->NumElements() * sizeof(SFloat3));
			Float3TransformNormals(pvScratchVectors, sizeof(SFloat3), psNode->pcObjectNode->GetNormals()->GetData(), sizeof(SFloat3), &psNode->psFinalTransform->sD3DMatrix, psNode->pcObjectNode->GetNormals()->NumElements());

			psNode = mcNodes.GetNext(psNode);
		}

		psSkinnedVector = macNormalPtrCache.Get(0);
		for (i = 0; i < macNormalPtrCache.NumElements(); i++)
		{
			psSkinnedVector[i].Cache();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::SetSkinnedVertexCachePointers(void)
{
	int							i;
	SSkinnedLinkObjectNode*		psSkinnedLinkObjectNode;
	int							iNumVerts;
	SSkinnedVector*				psSkinnedVector;
	CMapIntInt					cMapNodeToScratchPad;

	if (mpcMeshObject->IsSkinned())
	{
		cMapNodeToScratchPad.Init(&gcSystemAllocator, false);
		GenerateVertexScratchPadMap(&cMapNodeToScratchPad);

		iNumVerts = mpcMeshObject->GetSkinnedVertexPtrs()->NumElements();

		masVertexCache.Init();
		masVertexCache.Resize(iNumVerts);
		masVertexCache.Zero();

		macVertexPtrCache.Init();
		macVertexPtrCache.Resize(iNumVerts);
		for (i = 0; i < iNumVerts; i++)
		{
			psSkinnedLinkObjectNode = mpcMeshObject->GetSkinnedVertexPtrs()->Get(i);
			psSkinnedVector = macVertexPtrCache.Get(i);

			AssignCachedVertex(psSkinnedVector, psSkinnedLinkObjectNode);
		}
		cMapNodeToScratchPad.Kill();
	}
	meVertexCaching = LCF_UseCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::SetSkinnedNormalCachePointers(void)
{
	int							i;
	SSkinnedLinkObjectNode*		psSkinnedLinkObjectNode;
	int							iNumNormals;
	SSkinnedVector*				psSkinnedVector;
	CMapIntInt					cMapNodeToScratchPad;

	if (mpcMeshObject->IsSkinned())
	{
		cMapNodeToScratchPad.Init(&gcSystemAllocator, false);
		GenerateNormalScratchPadMap(&cMapNodeToScratchPad);

		iNumNormals = mpcMeshObject->GetSkinnedNormalPtrs()->NumElements();

		masNormalCache.Init();
		masNormalCache.Resize(iNumNormals);
		masNormalCache.Zero();

		macNormalPtrCache.Init();
		macNormalPtrCache.Resize(iNumNormals);
		for (i = 0; i < iNumNormals; i++)
		{
			psSkinnedLinkObjectNode = mpcMeshObject->GetSkinnedNormalPtrs()->Get(i);
			psSkinnedVector = macNormalPtrCache.Get(i);

			AssignCachedNormal(psSkinnedVector, psSkinnedLinkObjectNode);
		}
		cMapNodeToScratchPad.Kill();
	}
	meNormalCaching = LCF_UseCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::GenerateVertexScratchPadMap(CMapIntInt* pcMap)
{
	SFloat3*				pvScratchVectors;
	CMeshInstanceNode*			psNode;

	mpcScratchPad->Reset();
	pvScratchVectors = (SFloat3*)mpcScratchPad->Add(mpcMeshObject->GetVerticies()->NumElements() * sizeof(SFloat3));

	pcMap->Put((int)((size_t)mpcMeshObject), (int)((size_t)pvScratchVectors));

	psNode = mcNodes.GetHead();
	while (psNode)
	{
		pvScratchVectors = (SFloat3*)mpcScratchPad->Add(psNode->pcObjectNode->GetVerticies()->NumElements() * sizeof(SFloat3));
		pcMap->Put((int)((size_t)psNode), (int)((size_t)pvScratchVectors));

		psNode = mcNodes.GetNext(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::AssignCachedVertex(SSkinnedVector* psSkinnedVector, SSkinnedLinkObjectNode* psSkinnedLinkObjectNode)
{
	int					iWeightNum;
	CMeshObjectNode*	pcMeshObjectNode;

	for (iWeightNum = 0; iWeightNum < 4; iWeightNum++)
	{
		psSkinnedVector->afWeights[iWeightNum] = psSkinnedLinkObjectNode->afWeights[iWeightNum];

		if ((void*)psSkinnedLinkObjectNode->apcNodes[iWeightNum] == (void*)mpcMeshObject)
		{
			psSkinnedVector->apsSource[iWeightNum] = mpcMeshObject->GetVerticies()->Get(psSkinnedLinkObjectNode->iVertIndex[iWeightNum]);
		}
		else
		{
			pcMeshObjectNode = psSkinnedLinkObjectNode->apcNodes[iWeightNum];
			if (pcMeshObjectNode == NULL)
			{
				psSkinnedVector->apsSource[iWeightNum] = NULL;
			}
			else
			{
				psSkinnedVector->apsSource[iWeightNum] = pcMeshObjectNode->GetVerticies()->Get(psSkinnedLinkObjectNode->iVertIndex[iWeightNum]);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::GenerateNormalScratchPadMap(CMapIntInt* pcMap)
{
	SFloat3*				pvScratchVectors;
	CMeshInstanceNode*			psNode;

	mpcScratchPad->Reset();
	pvScratchVectors = (SFloat3*)mpcScratchPad->Add(mpcMeshObject->GetNormals()->NumElements() * sizeof(SFloat3));

	pcMap->Put((int)((size_t)mpcMeshObject), (int)((size_t)pvScratchVectors));

	psNode = mcNodes.GetHead();
	while (psNode)
	{
		pvScratchVectors = (SFloat3*)mpcScratchPad->Add(psNode->pcObjectNode->GetNormals()->NumElements() * sizeof(SFloat3));
		pcMap->Put((int)((size_t)psNode), (int)((size_t)pvScratchVectors));

		psNode = mcNodes.GetNext(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::AssignCachedNormal(SSkinnedVector* psSkinnedVector, SSkinnedLinkObjectNode* psSkinnedLinkObjectNode)
{
	int					iWeightNum;
	CMeshObjectNode*	pcMeshObjectNode;

	for (iWeightNum = 0; iWeightNum < 4; iWeightNum++)
	{
		psSkinnedVector->afWeights[iWeightNum] = psSkinnedLinkObjectNode->afWeights[iWeightNum];

		if ((void*)psSkinnedLinkObjectNode->apcNodes[iWeightNum] == (void*)mpcMeshObject)
		{
			psSkinnedVector->apsSource[iWeightNum] = mpcMeshObject->GetNormals()->Get(psSkinnedLinkObjectNode->iVertIndex[iWeightNum]);
		}
		else
		{
			pcMeshObjectNode = psSkinnedLinkObjectNode->apcNodes[iWeightNum];
			if (pcMeshObjectNode == NULL)
			{
				psSkinnedVector->apsSource[iWeightNum] = NULL;
			}
			else
			{
				psSkinnedVector->apsSource[iWeightNum] = pcMeshObjectNode->GetNormals()->Get(psSkinnedLinkObjectNode->iVertIndex[iWeightNum]);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CMeshInstance::GetWorldSpaceTransform(void)
{
	return mpsWorldSpaceTransform;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLinkListLinkInstanceNode* CMeshInstance::GetNodes(void)
{
	return &mcNodes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::SetWorldSpaceTransform(SMatrix* psMatrix)
{
	mpsWorldSpaceTransform = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshInstance::SetInverseWorldSpaceTransform(SMatrix* psMatrix)
{
	mpsInverseWorldSpaceTransform = psMatrix;
}

