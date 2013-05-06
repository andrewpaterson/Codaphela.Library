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
#ifndef __WORLD_H__
#define __WORLD_H__
#include "DirectXLib/DirectXGlobal.h"
#include "AnimationManager.h"
#include "Quad2D.h"
#include "WorldLists.h"
#include "BaseLib/MemoryStack.h"
#include "MeshInstance.h"
#include "GraphicsInstance.h"
#include "GraphicsTexture.h"


class CMesh;
class CMaterial;
class CMaterialConverter;
class CTextureConverter;
class CMeshConverter;
class CAnimationConverter;
class CInstanceConverter;
class CImage;
class CScene;
class CHierachy;
class CConnection;
class CSequence;
class CInstance;


typedef CArrayType<CMeshObject>				CMeshObjectList;
typedef CArrayType<CMeshInstance>			CMeshInstanceList;
typedef CArrayType<CGraphicsMaterial>		CGraphicsMaterialList;
typedef CArrayType<CGraphicsTexture>		CGraphicsTextureList;
typedef CArrayType<CGraphicsState>			CGraphicsStateList;
typedef CArrayType<CGraphicsObject>			CGraphicsObjectList;
typedef CArrayType<CGraphicsInstance>		CGraphicsInstanceList;
typedef CArrayType<CPosition>				CPositionList;
typedef CArrayType<CRotation>				CRotationList;
typedef CArrayType<CTransformNode>			CAnimMatrixList;
typedef CLinkListTemplateFreeList<SMatrix>	CTransformNodeList;


BOOL InitMain(int iAd, D3DDEVTYPE devType, HWND hWndScreen, int dx, int dy, D3DFORMAT fmt, BOOL bStencil, BOOL bExcluseive, HWND hWndInput);
void KillMain(void);


class CWorld
{
public:
	CGraphicsMaterialList	mcGraphicsMaterialList;
	CGraphicsObjectList		mcGraphicsObjectList;
	CGraphicsStateList		mcGraphicsStateList;
	CGraphicsInstanceList	mcGraphicsInstanceList;
	CWorldCameraList		mcWorldCameraList;
	CWorldLightList			mcWorldLightList;
	CWorldViewportList		mcViewportList;
	CParticleSystemList		mcParticleSystemList;
	CTransformNodeList		mcMatrixList;
	CMeshInstanceList		mcMeshInstanceList;
	CMeshObjectList			mcMeshObjectList;
	CMovementList			mcMovementList;
	CGraphicsTextureList	mcGraphicsTextureList;
	CAnimationManager		mcAnimationManager;
	CPositionList			mcPositionList;
	CRotationList			mcRotationList;
	CAnimMatrixList			mcAnimMatrixList;

	CGraphicsState*			mpcCurrentState;
	CGraphicsMaterial*		mpcCurrentMaterial;

	CCameraInstance*		mpsCurrentCamera;
	CGraphicsMaterial*		mpcLastMaterial;

	BOOL					mbClearBuffer;
	int						miClearColour;

	SMatrix					msIdentity;
	CGraphicsMaterial*		mpcGreyMaterial;  //Lit material
	CGraphicsState*			mpcGreyState;
	CGraphicsMaterial*		mpcWhiteMaterial;  //Unlit material
	CGraphicsState*			mpcWhiteState;
	CViewportInstance*		mpcFullViewport;

	void 				Init(void);
	void 				Kill(void);
	void				Draw(void);
	void				Update(void);
	void 				BeginFrame(void);
	void 				EndFrame(HWND hWnd);

	CGraphicsState* 	CreateGraphicsState(void);
	void				SetGraphicsState(CGraphicsState* pcGraphicsState);
	void				ReleaseGraphicsState(CGraphicsState* pcGraphicsState);

	CGraphicsMaterial*	CreateGraphicsMaterial(void);
	void				SetGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial);
	void				UnsetLastGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial);
	void				InvalidateGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial);
	void				ReleaseGraphicsMaterial(CGraphicsMaterial* pcGraphicsMaterial);

	CGraphicsTexture*	CreateGraphicsTexture(int iWidth, int iHeight, D3DFORMAT D3DFormat, ED3DTextureUsage eUsage, BOOL bDynamic);

	CGraphicsObject*	CreateGraphicsObject(BOOL bDynamicBuffers);
	void				DrawGraphicsObject(CGraphicsObject* pcGraphicsObject);
	void				ReleaseGraphicsObject(CGraphicsObject* pcGraphicsObject);

	CGraphicsInstance*	CreateGraphicsInstance(void);
	void				DrawGraphicsInstance(CGraphicsInstance* pcGraphicsInstance);
	void				ReleaseGraphicsInstance(CGraphicsInstance* pcGraphicsInstance);

	CCameraInstance*	CreateCamera(void);
	CCameraInstance*	CreateCamera(float width, float height);
	void				ReleaseCamera(CCameraInstance* pcCameraInstance);
	CCameraInstance*	GetCurrentCamera(void);
	void				SetCurrentCamera(CCameraInstance* pcCameraInstance);

	CLightInstance*		CreateLight(void);
	void				ReleaseLight(CLightInstance* pcLightInstance);
	void				UseLights(CArrayPtrLight* papLights);

	CViewportInstance*	CreateViewport(void);
	void				ReleaseViewport(CViewportInstance* pcViewport);

	CParticleSystem*	CreateParticleSystem(void);
	void				ReleaseParticleSystem(CParticleSystem* pcParticleSystem);

	CMovement*			CreateMovement(void);
	void				ReleaseMovement(CMovement* pcMovement);

	CAnimation*			CreateAnimation(int iCelChunkSize, char* szName = NULL);
	void				ReleaseAnimation(CAnimation* pcAnimation);
	void				SetAnimationCelHappenOnceFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, CelFunc Func, float fFractionalTimeToOccur);
	void				SetAnimationCelInvokeAnimationFunction(CAnimation* pcAnimationInvoking, int iInvokingCel, int iFlags, CAnimation* pcAnimationToSet, CAnimation** ppcAnimtionPtrToChange, CelFunc Func, float fFractionalTimeToOccur, float fSpeedMultiplier = 1.0f);

	CPosition*			CreatePosition(char* szName = NULL);
	void				ReleasePosition(CPosition* pcItem);

	CRotation*			CreateRotation(char* szName = NULL);
	void				ReleaseRotation(CRotation* pcItem);

	CTransformNode*			CreateAnimMatrix(char* szName = NULL);
	void				ReleaseAnimMatrix(CTransformNode* pcItem);

	SMatrix*			CreateMatrix(void);
	SMatrix*			CreateMatrix(SFloat4x4* psValue);
	void				ReleaseMatrix(SMatrix* psMatrix);

	SMatrix*			GetIdentityMatrix(void);
	SFloat3*			GetXUnitVector(void);
	SFloat3*			GetYUnitVector(void);
	SFloat3*			GetZUnitVector(void);
	void				SetupGreyMaterial(void);
	void				SetupWhiteMaterial(void);
	void				SetupViewport(void);
	CGraphicsMaterial*	GetGreyGraphicsMaterial(void);
	CGraphicsState*		GetGreyGraphicsState(void);
	CGraphicsMaterial*	GetWhiteGraphicsMaterial(void);
	CGraphicsState*		GetWhiteGraphicsState(void);
	void				SetRenderStates(void);

	CMeshObject*		CreateMeshObject(void);
	void				ReleaseMeshObject(CMeshObject* pcMeshObject);

	CMeshInstance*		CreateMeshInstance(void);
	void				ReleaseMeshInstance(CMeshInstance* pcMeshInstance);
};


#endif //__WORLD_H__

