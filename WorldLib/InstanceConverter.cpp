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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Numbers.h"
#include "SceneConverter.h"
#include "InstanceConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceConverter::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceConverter::Kill(void)
{
	mpcSceneConverter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::Convert(CInstance* pcInstance)
{
	CGraphicsInstance*	pcGraphicsInstance;
	CMeshInstance*		pcMeshInstance;
	CCameraInstance*	pcCameraInstance;
	CLightInstance*		pcLightInstance;


	if (pcInstance->meType == TT_Mesh)
	{
		if (!ConvertMeshAndLinks(&pcGraphicsInstance, &pcMeshInstance, pcInstance->miObjectIndex, &pcInstance->maiConnections))
		{
			return false;
		}
		mpcSceneConverter->GetMapper()->AddGraphicsInstance(pcInstance->GetOI(), pcGraphicsInstance, pcMeshInstance);
	}
	else if (pcInstance->meType == TT_Camera)
	{
		if (!ConvertCamera(&pcCameraInstance, pcInstance->miObjectIndex, &pcInstance->maiConnections))
		{
			return false;
		}
	}
	else if (pcInstance->meType == TT_Light)
	{
		if (!ConvertLight(&pcLightInstance, pcInstance->miObjectIndex, &pcInstance->maiConnections))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceConverter::ConvertConnectionsAndIndices(CArrayIntAndPointer* pcConnectionAndIndex, CArrayIntMinimal* pcConnectionIndices)
{
	int				i;
	int				iConnectionIndex;
	CConnection*	pcConnection;

	pcConnectionAndIndex->Init();

	//I think you're trying to do some sort of state based ordering here.
	for (i = 0; i < pcConnectionIndices->NumElements(); i++)
	{
		iConnectionIndex = *pcConnectionIndices->Get(i);
		pcConnection = (CConnection*)mpcSceneConverter->GetScene()->mcConnectionTracker.GetWithID(iConnectionIndex);
		pcConnectionAndIndex->Add(pcConnection, iConnectionIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::ConvertMeshAndLinks(CGraphicsInstance** ppcGraphicsInstance, CMeshInstance** ppcMeshInstance, int iMeshIndex, CArrayIntMinimal* pcConnectionIndices)
{
	CGraphicsObject*		pcGraphicsObject;
	CMeshObject*			pcMeshObject;
	CMesh*					pcMesh;
	CGraphicsInstance*		pcGraphicsInstance;
	CMeshInstance*			pcMeshInstance;
	CArrayIntAndPointer			cConnectionAndIndex;
	
	*ppcGraphicsInstance = NULL;
	*ppcMeshInstance = NULL;
	pcMesh = (CMesh*)mpcSceneConverter->GetScene()->mcMeshTracker.GetWithID(iMeshIndex);

	ConvertConnectionsAndIndices(&cConnectionAndIndex, pcConnectionIndices);

	if (!mpcSceneConverter->ConvertMeshToGraphicsObject(&pcGraphicsObject, &pcMeshObject, pcMesh, &cConnectionAndIndex))
	{
		cConnectionAndIndex.Kill();
		return false;
	}

	pcGraphicsInstance = mpcSceneConverter->GetWorld()->CreateGraphicsInstance();
	pcGraphicsInstance->Set(pcGraphicsObject);

	pcMeshInstance = mpcSceneConverter->GetWorld()->CreateMeshInstance();
	ConvertLinkInstance(pcMeshInstance, pcMeshObject, pcGraphicsInstance, &cConnectionAndIndex);

    *ppcGraphicsInstance = pcGraphicsInstance;
	*ppcMeshInstance = pcMeshInstance;

	cConnectionAndIndex.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::ConvertCamera(CCameraInstance** ppcCameraInstance, int iCameraIndex, CArrayIntMinimal* pcConnectionIndices)
{
	CCamera*			pcCamera;
	CCameraInstance*	pcCameraInstance;
	CArrayIntAndPointer		cConnectionAndIndex;
	CConnection*		pcConnection;
	int					iConnectionIndex;
	SFloat4x4*			psWorldMatrix;

	ConvertConnectionsAndIndices(&cConnectionAndIndex, pcConnectionIndices);

	cConnectionAndIndex.Get(0, (void**)&pcConnection, &iConnectionIndex);

	if (!pcConnection)
	{
		gcUserError.Set("Cannot convert CameraInstance.  Cannot find a connection");
		return true;
	}

	pcCamera = (CCamera*)mpcSceneConverter->GetScene()->mcCameraTracker.GetWithID(iCameraIndex);

	pcCameraInstance = mpcSceneConverter->GetWorld()->CreateCamera();

	psWorldMatrix = &pcConnection->msWorldMatrix;
	pcCameraInstance->SetWorldTransform(mpcSceneConverter->GetWorld()->CreateMatrix());
	pcCameraInstance->GetWorldTransform()->Set(psWorldMatrix);
	pcCameraInstance->CreateAdjustment();
	Float4x4RotationX(&pcCameraInstance->GetAdjustmentTransform()->sD3DMatrix, Deg2Rad(-90));

	//Convert any animation this node may have.
	ConvertSequence(iConnectionIndex, pcCameraInstance->GetWorldTransform());

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::ConvertLight(CLightInstance** ppcLightInstance, int iLightIndex, CArrayIntMinimal* pcConnectionIndices)
{
	CLight*				pcLight;
	CLightInstance*		pcLightInstance;
	CArrayIntAndPointer		cConnectionAndIndex;
	CConnection*		pcConnection;
	int					iConnectionIndex;
	SFloat4x4*			psWorldMatrix;

	ConvertConnectionsAndIndices(&cConnectionAndIndex, pcConnectionIndices);

	cConnectionAndIndex.Get(0, (void**)&pcConnection, &iConnectionIndex);

	if (!pcConnection)
	{
		gcUserError.Set("Cannot convert LightInstance.  Cannot find a connection");
		return true;
	}

	pcLight = (CLight*)mpcSceneConverter->GetScene()->mcLightTracker.GetWithID(iLightIndex);

	pcLightInstance = mpcSceneConverter->GetWorld()->CreateLight();

	psWorldMatrix = (SFloat4x4*)&pcConnection->msWorldMatrix;

	pcLightInstance->SetMatrix(mpcSceneConverter->GetWorld()->CreateMatrix(psWorldMatrix));

	//Convert any animation this node may have.
	ConvertSequence(iConnectionIndex, pcLightInstance->GetMatrix());

	pcLightInstance->SetColour(
		pcLight->msColour.r * pcLight->mfIntensity * LIGHT_PLEASANT, 
		pcLight->msColour.g * pcLight->mfIntensity * LIGHT_PLEASANT, 
		pcLight->msColour.b * pcLight->mfIntensity * LIGHT_PLEASANT);

	if (pcLight->meType == LT_Omni)
	{
		if (pcLight->meDecay == LDT_None)
		{
			pcLightInstance->SetPoint(1.0f, 0.0f, 0.0f, CONVERSION_LIGHT_RANGE);
		}
		else if (pcLight->meDecay == LDT_Linear)
		{
			pcLightInstance->SetPoint(0.0f, (1.0f/(CONVERSION_LIGHT_RANGE*0.04f)), 0.0f, CONVERSION_LIGHT_RANGE);
		}
		else if (pcLight->meDecay = LDT_Square)
		{
			pcLightInstance->SetPoint(0.0f, 0.0f, (1.0f/(CONVERSION_LIGHT_RANGE*0.001f)), sqrtf(CONVERSION_LIGHT_RANGE));
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::ConvertLinkInstance(CMeshInstance* pcMeshInstance, CMeshObject* pcMeshObject, CGraphicsInstance* pcGraphicsInstance, CArrayIntAndPointer* pcConnectionsAndIndices)
{
	int					i;
	CConnection*		pcConnection;
	SFloat4x4*			psWorldMatrix;
	SFloat4x4*			psMatrix;
	SFloat4x4			sWorldInverse;
	float				fDeterminant;
	CMeshInstanceNode*	psLinkInstanceNode;
	CMeshObjectNode*	psLinkObjectNode;
	int					iConnectionIndex;

	pcConnectionsAndIndices->Get(0, (void**)&pcConnection, &iConnectionIndex);

	if (!pcConnection)
	{
		gcUserError.Set("Cannot convert MeshInstance.  Cannot find a connection");
		return false;
	}
	
	pcGraphicsInstance->SetFlag(GRAPH_INST_FLAGS_REVERSE_CULL, pcConnection->mbParity);

	psWorldMatrix = &pcConnection->msWorldMatrix;
	pcMeshInstance->SetWorldSpaceTransform(mpcSceneConverter->GetWorld()->CreateMatrix(psWorldMatrix));

	Float4x4Inverse(&sWorldInverse, &fDeterminant, psWorldMatrix);
	pcMeshInstance->SetInverseWorldSpaceTransform(mpcSceneConverter->GetWorld()->CreateMatrix(&sWorldInverse));

	pcGraphicsInstance->AddMatrixPtr(pcMeshInstance->GetWorldSpaceTransform());

	//Convert any animation this node may have.
	ConvertSequence(iConnectionIndex, pcMeshInstance->GetWorldSpaceTransform());

	//Convert matricies for skinning.
	for (i = 1; i <	pcConnectionsAndIndices->NumElements(); i++)
	{
		if (i == 1)
		{
			psLinkObjectNode = pcMeshObject->GetNodes()->GetHead();
		}
		else
		{
			psLinkObjectNode = pcMeshObject->GetNodes()->GetNext(psLinkObjectNode);
		}
		psLinkInstanceNode = pcMeshInstance->AddNode(psLinkObjectNode);
		pcConnectionsAndIndices->Get(i, (void**)&pcConnection, &iConnectionIndex);

		psMatrix = (SFloat4x4*)&pcConnection->msWorldMatrix;

		//Set the instance node to point to the object node.
		psLinkInstanceNode->pcObjectNode = psLinkObjectNode;

		//We don't transform the object away from its refernce position.
		psLinkInstanceNode->psSubObjectTransform = mpcSceneConverter->GetWorld()->CreateMatrix();

		//Setup the to world matrix transform.
		psLinkInstanceNode->psWorldTransform = mpcSceneConverter->GetWorld()->CreateMatrix();
		psLinkInstanceNode->psWorldTransform->Set(psMatrix);

		//Setup the final destination matrix.
		psLinkInstanceNode->psFinalTransform = mpcSceneConverter->GetWorld()->CreateMatrix();
		psLinkInstanceNode->TransformFromWorld();

		pcGraphicsInstance->AddMatrixPtr(psLinkInstanceNode->psFinalTransform);
		
		//Convert any animation this node may have.
		ConvertSequence(iConnectionIndex, psLinkInstanceNode->psWorldTransform);
		psLinkInstanceNode->eTransform = LINT_World;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceConverter::ConvertSequence(int iConnectionIndex, SMatrix* psMatrix)
{
	CSequence*	pcSequence;
	CMovement*	pcMovement;

	pcSequence = (CSequence*)mpcSceneConverter->GetScene()->mcSequenceTracker.GetWithID(iConnectionIndex);
	if (pcSequence)
	{
		return mpcSceneConverter->ConvertSequenceToMovement(&pcMovement, pcSequence, psMatrix);
	}
	return true;
}

