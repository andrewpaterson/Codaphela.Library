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
//#include <mmsystem.h>
#include "BaseLib/Logger.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "CoreLib/Operators.h"
#include "StandardLib/Unknowns.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/Scene.h"
#include "MeshConverter.h"
#include "MaterialConverter.h"
#include "TextureConverter.h"
#include "AnimationConverter.h"
#include "InstanceConverter.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::Init(void)
{
	mpcCurrentState = NULL;
	mpcCurrentMaterial= NULL;
	mpsCurrentCamera = NULL;
	mpcLastMaterial = NULL;

	mcGraphicsMaterialList.Init();
	mcGraphicsMaterialList.KillElements(FALSE);
	mcGraphicsTextureList.Init();
	mcGraphicsObjectList.Init();
	mcGraphicsStateList.Init();
	mcGraphicsInstanceList.Init();
	mcMatrixList.Init(512);
	mcAnimationManager.Init();
	mcPositionList.Init();
	mcRotationList.Init();
	mcAnimMatrixList.Init();

	mcMeshInstanceList.Init();
	mcMeshObjectList.Init();

	mcWorldCameraList.Init();
	mcWorldLightList.Init();
	mcViewportList.Init();
	mcParticleSystemList.Init();
	mcMovementList.Init();

	gcD3D.CreateMatrix(&msIdentity);
	SetupGreyMaterial();
	SetupWhiteMaterial();
	SetupViewport();
	SetRenderStates();

	mbClearBuffer = TRUE;
	miClearColour = 0xccddee;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::Kill(void)
{
	mcMovementList.Kill();
	mcParticleSystemList.Kill();
	mcViewportList.Kill();
	mcWorldLightList.Kill();
	mcWorldCameraList.Kill();

	mcPositionList.Kill();
	mcRotationList.Kill();
	mcAnimMatrixList.Kill();
	mcGraphicsInstanceList.Kill();
	mcGraphicsStateList.Kill();
	mcGraphicsObjectList.Kill();
//	mcGraphicsTextureList.Kill();
	mcGraphicsMaterialList.Kill();
	mcMatrixList.Kill();

	mcMeshInstanceList.Kill();
	mcMeshObjectList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::BeginFrame(void)
{
	gcD3D.BeginScene();

	mpcFullViewport->Draw();
	if (mbClearBuffer)
	{
		gcD3D.ClearBuffer(miClearColour);
	}

	mpsCurrentCamera->Use();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::EndFrame(HWND hWnd)
{
	gcD3D.EndScene();
	gcD3D.Present(hWnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::Draw(void)
{
	CGraphicsInstance*	pcGraphicsInstance;
	CArrayBlock			cArrayTranslucent;
	int					i;

	cArrayTranslucent.Init(sizeof(CGraphicsInstance*), 128);
	for (i = 0; i < mcGraphicsInstanceList.NumElements(); i++)
	{
		pcGraphicsInstance = mcGraphicsInstanceList.Get(i);
		if (!(pcGraphicsInstance->GetGraphicsObject()->HasFlag(GRAPH_OBJ_FLAGS_HAS_TRANSLUCENY)))
		{
			pcGraphicsInstance->Draw();
		}
		else
		{
			cArrayTranslucent.Add(&pcGraphicsInstance);
		}
	}

	for (i = 0; i < cArrayTranslucent.NumElements(); i++)
	{
		pcGraphicsInstance = *(CGraphicsInstance**)cArrayTranslucent.Get(i);
		pcGraphicsInstance->Draw();
	}
	cArrayTranslucent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::Update(void)
{
	CMovement*			pcMovement;
	CMeshInstance*		pcMeshInstance;
	CGraphicsInstance*	pcGraphicsInstance;
	int					i;

	gcTimer.Update();

	for (i = 0; i < mcMovementList.NumElements(); i++)
	{
		pcMovement = mcMovementList.Get(i);
		pcMovement->Update();
	}

	for (i = 0; i < mcMeshInstanceList.NumElements(); i++)
	{
		pcMeshInstance = mcMeshInstanceList.Get(i);
		pcMeshInstance->Update();
	}

	for (i = 0; i < mcGraphicsInstanceList.NumElements(); i++)
	{
		pcGraphicsInstance = mcGraphicsInstanceList.Get(i);
		pcGraphicsInstance->Update();
	}
}


//////////////////////////////////////////////////////////////////////////
//  Graphics States
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CWorld::CreateGraphicsState(void)
{
	CGraphicsState*		pcGraphicsState;

	pcGraphicsState = mcGraphicsStateList.Add();
	if (pcGraphicsState)
	{
		pcGraphicsState->Init();
	}
	return pcGraphicsState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetGraphicsState(CGraphicsState* pcGraphicsState)
{
	if (mpcCurrentState != pcGraphicsState)
	{
		pcGraphicsState->Draw();
		mpcCurrentState = pcGraphicsState;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseGraphicsState(CGraphicsState* pcGraphicsState)
{
	if (mpcCurrentState == pcGraphicsState)
	{
		mpcCurrentState = NULL;
	}
	mcGraphicsStateList.Remove(pcGraphicsState);
}


//////////////////////////////////////////////////////////////////////////
//  Graphics Materials
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CWorld::CreateGraphicsMaterial(void)
{
	CGraphicsMaterial*		pcGraphicsMaterial;

	pcGraphicsMaterial = mcGraphicsMaterialList.Add();
	if (pcGraphicsMaterial)
	{
		pcGraphicsMaterial->Init();
	}
	return pcGraphicsMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial)
{
	if (mpcCurrentMaterial != pcGraphicsMaterial)
	{
		UnsetLastGraphicsMaterial(pcGraphicsMaterial);
		pcGraphicsMaterial->Draw();
		mpcCurrentMaterial= pcGraphicsMaterial;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::InvalidateGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial)
{
	if (mpcCurrentMaterial == pcGraphicsMaterial)
	{
		UnsetLastGraphicsMaterial(NULL);
		mpcCurrentMaterial = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::UnsetLastGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial)
{
	if (mpcLastMaterial != NULL)
	{
		mpcLastMaterial->UnsetRenderStates();
	}
	mpcLastMaterial = pcGraphicsMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial)
{
	InvalidateGraphicsMaterial(pcGraphicsMaterial);
	mcGraphicsMaterialList.Remove(pcGraphicsMaterial);
}


//////////////////////////////////////////////////////////////////////////
//  Graphics Textures
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CWorld::CreateGraphicsTexture(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, BOOL bDynamic)
{
	CGraphicsTexture*	pcGraphicsTexture;

	pcGraphicsTexture = mcGraphicsTextureList.Add();
	pcGraphicsTexture->Init(iWidth, iHeight, D3DFormat, eUsage, bDynamic);
	return pcGraphicsTexture;
}


//////////////////////////////////////////////////////////////////////////
//  Graphics Objects
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsObject* CWorld::CreateGraphicsObject(BOOL bDynamicBuffers)
{
	CGraphicsObject*		pcGraphicsObject;

	pcGraphicsObject = mcGraphicsObjectList.Add();
	if (pcGraphicsObject)
	{
		pcGraphicsObject->Init(bDynamicBuffers, this);
		pcGraphicsObject->AddViewport(mpcFullViewport);
	}
	return pcGraphicsObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::DrawGraphicsObject(CGraphicsObject* pcGraphicsObject)
{
	pcGraphicsObject->Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseGraphicsObject(CGraphicsObject* pcGraphicsObject)
{
	if (pcGraphicsObject)
	{
		//This calls kill on the graphics object.
		mcGraphicsObjectList.Remove(pcGraphicsObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//  Graphics Instances
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsInstance* CWorld::CreateGraphicsInstance(void)
{
	CGraphicsInstance*		pcGraphicsInstance;

	pcGraphicsInstance = mcGraphicsInstanceList.Add();
	if (pcGraphicsInstance)
	{
		pcGraphicsInstance->Init(this);
	}
	return pcGraphicsInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseGraphicsInstance(CGraphicsInstance* pcGraphicsInstance)
{
	mcGraphicsInstanceList.Remove(pcGraphicsInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::DrawGraphicsInstance(CGraphicsInstance* pcGraphicsInstance)
{
	//It's retarded to ever call draw twice on the same instance in the
	//same frame... so lets assume we're not and just draw it regardless.
	pcGraphicsInstance->Draw();
}


//////////////////////////////////////////////////////////////////////////
//	Quick Values
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CWorld::GetIdentityMatrix(void)
{
	return &msIdentity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CWorld::GetXUnitVector(void)
{
	return msIdentity.Across();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CWorld::GetYUnitVector(void)
{
	return msIdentity.Up();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CWorld::GetZUnitVector(void)
{
	return msIdentity.At();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetupGreyMaterial(void)
{
	mpcGreyMaterial = CreateGraphicsMaterial();
	mpcGreyMaterial->SetGreyDefaults(0);
	mpcGreyState = CreateGraphicsState();
	mpcGreyState->SetDefaults(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetupWhiteMaterial(void)
{
	mpcWhiteMaterial = CreateGraphicsMaterial();
	mpcWhiteMaterial->SetWhiteDefaults(0);
	mpcWhiteState = CreateGraphicsState();
	mpcWhiteState->SetDefaults(0);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetupViewport(void)
{
	mpcFullViewport = CreateViewport();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CWorld::GetGreyGraphicsMaterial(void)
{
	return mpcGreyMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CWorld::GetGreyGraphicsState(void)
{
	return mpcGreyState;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CWorld::GetWhiteGraphicsMaterial(void)
{
	return mpcWhiteMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CWorld::GetWhiteGraphicsState(void)
{
	return mpcWhiteState;
}


//////////////////////////////////////////////////////////////////////////
//	Cameras
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCameraInstance* CWorld::CreateCamera(void)
{
	CCameraInstance*		pcCameraInstance;

	pcCameraInstance = mcWorldCameraList.Add();
	if (pcCameraInstance)
	{
		pcCameraInstance->Init(this);
	}
	return pcCameraInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCameraInstance* CWorld::CreateCamera(float width, float height)
{
	SMatrix*			psMatrix;
	CCameraInstance*	pcCameraInstance;
	float				fAspect;
	SFloat3				sFrom(0, 1500, 0);
	SFloat3				sTo(0, 0, 0);
	SFloat3				sUp(0, 0, -1);

	psMatrix = CreateMatrix();
	pcCameraInstance = CreateCamera();
	SetCurrentCamera(pcCameraInstance);
	pcCameraInstance->SetWorldMatrix(psMatrix);
	pcCameraInstance->LookAt(&sFrom, &sTo, &sUp);
	fAspect = width/height;
	pcCameraInstance->Set(45.0f, fAspect, 1, 5000);
	return pcCameraInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseCamera(CCameraInstance* pcCameraInstance)
{
	mcWorldCameraList.Remove(pcCameraInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCameraInstance* CWorld::GetCurrentCamera(void)
{
	return mpsCurrentCamera;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetCurrentCamera(CCameraInstance* psCamera)
{
	mpsCurrentCamera = psCamera;
}



//////////////////////////////////////////////////////////////////////////
//	Lights
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLightInstance* CWorld::CreateLight(void)
{
	CLightInstance*		pcLightInstance;

	pcLightInstance = mcWorldLightList.Add();
	if (pcLightInstance)
	{
		pcLightInstance->Init();
	}
	return pcLightInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseLight(CLightInstance* pcLightInstance)
{
	mcWorldLightList.Remove(pcLightInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::UseLights(CArrayPtrLight* papLights)
{
	int					i;
	CLightInstance*		pcLightInstance;

	for (i = 0; i < papLights->NumElements(); i++)
	{
		pcLightInstance = *papLights->Get(i);
		pcLightInstance->Use(i);
	}

	for (; i < 8; i++)
	{
		gcD3D.LightDisable(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//	Viewports
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewportInstance* CWorld::CreateViewport(void)
{
	CViewportInstance*		pcViewport;

	pcViewport = mcViewportList.Add();
	if (pcViewport)
	{
		pcViewport->Init(this);
	}
	return pcViewport;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseViewport(CViewportInstance* pcViewport)
{
	mcViewportList.Remove(pcViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CParticleSystem* CWorld::CreateParticleSystem(void)
{
	CParticleSystem*	pcParticleSystem;

	pcParticleSystem = mcParticleSystemList.Add();
	if (pcParticleSystem)
	{
		pcParticleSystem->Init();
	}
	return pcParticleSystem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseParticleSystem(CParticleSystem* pcParticleSystem)
{
	mcParticleSystemList.Remove(pcParticleSystem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMovement* CWorld::CreateMovement(void)
{
	CMovement*	pcMovement;

	pcMovement = mcMovementList.Add();
	if (pcMovement)
	{
		pcMovement->Init();
	}
	return pcMovement;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseMovement(CMovement* pcMovement)
{
	mcMovementList.Remove(pcMovement);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnimation* CWorld::CreateAnimation(int iCelChunkSize, char* szName)
{
	CAnimation*		pcAnimation;

	pcAnimation = mcAnimationManager.Create(iCelChunkSize, szName);
	return pcAnimation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseAnimation(CAnimation* pcAnimation)
{
	mcAnimationManager.Release(pcAnimation);  //Animation.Kill is called by the list in the manager.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetAnimationCelHappenOnceFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, CelFunc Func, float fFractionalTimeToOccur)
{
	mcAnimationManager.SetCelHappenOnceFunction(pcAnimationInvoking, iInvokingCel, Func, fFractionalTimeToOccur);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetAnimationCelInvokeAnimationFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, int iFlags, CAnimation* pcAnimationToSet, CAnimation** ppcAnimtionPtrToChange, CelFunc Func, float fFractionalTimeToOccur, float fSpeedMultiplier)
{
	mcAnimationManager.SetCelInvokeAnimationFunction(pcAnimationInvoking, iInvokingCel, iFlags, pcAnimationToSet, ppcAnimtionPtrToChange, Func, fFractionalTimeToOccur, fSpeedMultiplier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPosition* CWorld::CreatePosition(char* szName)
{
	CPosition*		pcItem;

	pcItem = mcPositionList.Add();
	if (pcItem)
	{
		pcItem->Init();
	}
	return pcItem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleasePosition(CPosition* pcItem)
{
	mcPositionList.Remove(pcItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRotation* CWorld::CreateRotation(char* szName)
{
	CRotation*		pcItem;

	pcItem = mcRotationList.Add();
	if (pcItem)
	{
		pcItem->Init();
	}
	return pcItem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseRotation(CRotation* pcItem)
{
	mcRotationList.Remove(pcItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransformNode* CWorld::CreateAnimMatrix(char* szName)
{
	CTransformNode*		pcItem;

	pcItem = mcAnimMatrixList.Add();
	if (pcItem)
	{
		pcItem->Init();
	}
	return pcItem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseAnimMatrix(CTransformNode* pcItem)
{
	mcAnimMatrixList.Remove(pcItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CWorld::CreateMatrix(void)
{
	SMatrix*	psMatrix;

	psMatrix = mcMatrixList.InsertAfterTail();
	gcD3D.CreateMatrix(psMatrix);
	return psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CWorld::CreateMatrix(SFloat4x4* psValue)
{
	SMatrix*	psMatrix;

	psMatrix = CreateMatrix();
	psMatrix->Set(psValue);
	return psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseMatrix(SMatrix* psMatrix)
{
	mcMatrixList.Remove(psMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshObject* CWorld::CreateMeshObject(void)
{
	CMeshObject*		pcMeshObject;

	pcMeshObject = mcMeshObjectList.Add();
	if (pcMeshObject)
	{
		pcMeshObject->Init();
	}
	return pcMeshObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseMeshObject(CMeshObject* pcMeshObject)
{
	if (pcMeshObject)
	{
		//This calls kill on the Link object.
		mcMeshObjectList.Remove(pcMeshObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshInstance* CWorld::CreateMeshInstance(void)
{
	CMeshInstance*		pcMeshInstance;

	pcMeshInstance = mcMeshInstanceList.Add();
	if (pcMeshInstance)
	{
		pcMeshInstance->Init();
	}
	return pcMeshInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::ReleaseMeshInstance(CMeshInstance* pcMeshInstance)
{
	if (pcMeshInstance)
	{
		//This calls kill on the link instance.
		mcMeshInstanceList.Remove(pcMeshInstance);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorld::SetRenderStates(void)
{
	gcD3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	gcD3D.SetRenderState(D3DRS_LIGHTING, TRUE);
	gcD3D.SetRenderState(D3DRS_AMBIENT, 0x00000000);
	gcD3D.SetRenderState(D3DRS_FOGENABLE, FALSE);
	gcD3D.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	gcD3D.SetRenderState(D3DRS_ZENABLE, TRUE);
	gcD3D.SetRenderState(D3DRS_SPECULARENABLE, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL InitMain(int iAd, D3DDEVTYPE devType, HWND hWndScreen, int dx, int dy, D3DFORMAT fmt, BOOL bStencil, BOOL bExclusive, HWND hWndInput)
{
	gcLogger.Init();
	gcLogger.Add("Main world initialisation...\n");
	TypeConverterInit();
	ReturnOnFalse(InitDirectX(iAd, devType, hWndScreen, dx, dy, fmt, bStencil, bExclusive, hWndInput));
	TypesInit();
	OperatorsInit();
	UnknownsInit();
	gcLogger.Add("Main world initialisation.  Done.\n");
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void KillMain(void)
{
	gcLogger.Add("Main world destruction...\n");
	UnknownsKill();
	OperatorsKill();
	TypesKill();
	KillDirectX();
	TypeConverterKill();
	gcLogger.Add("Main world destruction.  Done.\n");
}

