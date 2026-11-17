/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneViewLib

Codaphela SceneViewLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneViewLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneViewLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/Timer.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "RenderLib/RenderScene.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/Window.h"
#include "SceneCanvasDraw.h"


#define SCENE_FRAME_MILLISECONDS	16		//Between frames while it plays, about 60 a second.
#define SCENE_STATUS_MILLISECONDS	500		//Between changes to the status's frame time while it plays.
#define SCENE_LONGEST_STEP			0.1f	//Seconds the animations move on by at most in a frame, after a pause say.
#define SCENE_RASTER_UNITS			(1 << 20)	//The raster renderer's units the scene's radius is.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Init(Ptr<CWindowTheme> pTheme, CLabel* pcStatus)
{
	ARGB32	sColour;

	PreInit();

	CCanvasDraw::Init();
	mpCelDraw = OMalloc<CImageCanvasDraw>(Ptr<CImageCel>(), false);
	sColour = pTheme->GetColour(TC_Editor);
	muiBackground = 0xFF000000 | ((uint32)Get8BitRedColour(sColour) << 16) | ((uint32)Get8BitGreenColour(sColour) << 8) | (uint32)Get8BitBlueColour(sColour);
	msSize.Init(-1, -1);

	mszName.Init();
	mapPlayers.Init();
	mpCamera = OMalloc<CCamera>();
	mpCamera->mpConnection = OMalloc<CConnection>();

	mcVulkanFactory.Init();
	mcRasteriser.Init();
	mcRasteriser.SetFilter(RF_Bilinear);
	memset(&mcTarget, 0, sizeof(mcTarget));
	mbRasterFactory = false;
	mbVulkan = true;
	mbUseVulkan = true;

	msCentre.Init(0, 0, 0);
	mfRadius = 1.0f;
	muiView = 0;
	msStart.Init();
	msView.Init();
	PlaceCamera();

	mbPlaying = true;
	miLastFrame = 0;
	mfDrawMilliseconds = 0;
	miStatusTime = 0;
	mpcStatus = pcStatus;
	mpcCanvas = NULL;
	mpcWindow = NULL;
	mbPosted = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpCelDraw);
	M_Pointer(mpImage);
	U_UInt32(muiBackground);
	U_2Int32(msSize);
	M_Pointer(mpScene);
	U_String(mszName);
	M_Embedded(mapPlayers);
	M_Pointer(mpCamera);
	U_Unknown(CVulkanRenderFactory, mcVulkanFactory);
	M_Pointer(mpVulkanRenderer);
	M_Pointer(mpVulkanFrame);
	U_Unknown(CRasterRenderFactory, mcRasterFactory);
	U_Unknown(CRasteriser, mcRasteriser);
	U_Unknown(CRasterTarget, mcTarget);
	U_Bool(mbRasterFactory);
	M_Pointer(mpRasterFrame);
	U_Bool(mbVulkan);
	U_Bool(mbUseVulkan);
	U_3Float32(msCentre);
	U_Float32(mfRadius);
	U_Size(muiView);
	U_Unknown(STurntable, msStart);
	U_Unknown(STurntable, msView);
	U_Bool(mbPlaying);
	U_Int64(miLastFrame);
	U_Float64(mfDrawMilliseconds);
	U_Int64(miStatusTime);
	U_Pointer(mpcStatus);
	U_Pointer(mpcCanvas);
	U_Pointer(mpcWindow);
	U_Bool(mbPosted);
}


//////////////////////////////////////////////////////////////////////////
//
// The frames go before their renderers, and the meshes, which the render
// scenes hold, before their device.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Free(void)
{
	if (mbPosted)
	{
		mpcWindow->CancelPosted(this);
	}
	mpRasterFrame = NULL;
	mpVulkanFrame = NULL;
	mpVulkanRenderer = NULL;
	if (mcTarget.mpiDepths != NULL)
	{
		mcTarget.Kill();
	}
	mcRasteriser.Kill();
	if (mbRasterFactory)
	{
		mcRasterFactory.Kill();
	}
	mcVulkanFactory.Kill();
	mszName.Kill();
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas draw isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Before a scene is set or read: false has it drawn by the rasteriser
// whether or not there's a device for Vulkan, which is then never looked
// for.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::UseVulkan(bool bUse)
{
	mbUseVulkan = bUse;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the scene, starts its animations and sets the view.  False if it
// can't be read, which leaves nothing drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::ReadScene(const char* szFilename)
{
	CFileUtil	cFileUtil;
	Ptr<CScene>	pScene;
	CChars		szName;

	pScene = OMalloc<CScene>();
	if (!pScene->ReadFile(szFilename))
	{
		return false;
	}
	szName.Init();
	cFileUtil.GetFileName(&szName, szFilename);
	SetScene(pScene, szName.Text());
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A scene made rather than read: its view starts as a read one's does,
// from its first camera or framed on it.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::SetScene(Ptr<CScene> pScene, const char* szName)
{
	mpScene = pScene;
	mszName.Set(szName);

	MakePlayers();
	Advance(0);
	FindBounds();
	StartView();

	mbVulkan = false;
	if (mbUseVulkan)
	{
		mpVulkanRenderer = mcVulkanFactory.CreateRenderer();
		mbVulkan = mpVulkanRenderer->IsReady();
	}
	if (mbVulkan)
	{
		mpVulkanFrame = OMalloc<CVulkanFrame>(OMalloc<CRenderScene>(mpScene, &mcVulkanFactory, mpVulkanRenderer), mpCamera);
	}
	SetStatus();
}


//////////////////////////////////////////////////////////////////////////
//
// What the status calls the scene, and that it changed: its meshes'
// editors' counters say what to make again, and the canvas is drawn again.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::SceneChanged(const char* szName)
{
	mszName.Set(szName);
	SetStatus();
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the ray through the middle of the canvas's pixel x, y meets the
// plane of the screen through what the view turns about.  False before
// the canvas has been drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::ScreenToPlane(int32 x, int32 y, SFloat32Vec3* psDest)
{
	return msView.ScreenToPlane(x, y, msSize.x, msSize.y, psDest);
}


//////////////////////////////////////////////////////////////////////////
//
// What was last drawn, its bytes B, G, R, X: NULL before the first draw.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CSceneCanvasDraw::GetImage(void) const
{
	return mpImage;
}


//////////////////////////////////////////////////////////////////////////
//
// A skeleton's animation is played on each instance of the skeleton, and
// the scene's own on the scene.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::MakePlayers(void)
{
	Ptr<CSceneAnimation>	pAnimation;
	Ptr<CInstance>			pInstance;
	size					i;
	size					j;

	mapPlayers.RemoveAll();
	for (i = 0; i < mpScene->mapAnimations.NumElements(); i++)
	{
		pAnimation = mpScene->mapAnimations.Get(i);
		if (pAnimation->mpSkeleton.IsNull())
		{
			mapPlayers.Add(OMalloc<CAnimationPlayer>(pAnimation, true));
		}
		else
		{
			for (j = 0; j < mpScene->mapInstances.NumElements(); j++)
			{
				pInstance = mpScene->mapInstances.Get(j);
				if (pInstance->mpSkeleton == pAnimation->mpSkeleton)
				{
					mapPlayers.Add(OMalloc<CAnimationPlayer>(pAnimation, pInstance, true));
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the animations on to iNow, by the time since the last frame, or
// none for the first, and places the scene's connections.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Advance(int64 iNow)
{
	Ptr<CAnimationPlayer>	pPlayer;
	float32					fSeconds;
	size					i;

	fSeconds = 0;
	if ((miLastFrame != 0) && mbPlaying)
	{
		fSeconds = (float32)(iNow - miLastFrame) / 1000.0f;
		if (fSeconds > SCENE_LONGEST_STEP)
		{
			fSeconds = SCENE_LONGEST_STEP;
		}
	}
	miLastFrame = iNow;

	for (i = 0; i < mapPlayers.NumElements(); i++)
	{
		pPlayer = mapPlayers.Get(i);
		pPlayer->Advance(fSeconds);
		pPlayer->Apply();
	}
	mpScene->UpdateConnections();
}


//////////////////////////////////////////////////////////////////////////
//
// The middle of where the scene's meshes are posed, and how far they go
// from it.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::FindBounds(void)
{
	CArrayFloat32Vec3	asPositions;
	Ptr<CInstance>		pInstance;
	SFloat32Vec3		sMin;
	SFloat32Vec3		sMax;
	SFloat32Vec3*		psPosition;
	SFloat32Vec3		sOut;
	float32				fRadius;
	bool				bAny;
	size				i;
	size				j;
	size				k;

	bAny = false;
	sMin.Init(0, 0, 0);
	sMax.Init(0, 0, 0);
	asPositions.Init();
	for (i = 0; i < mpScene->mapInstances.NumElements(); i++)
	{
		pInstance = mpScene->mapInstances.Get(i);
		for (j = 0; j < pInstance->mapMeshes.NumElements(); j++)
		{
			if (!pInstance->mapMeshes.Get(j)->Pose(&asPositions))
			{
				continue;
			}
			for (k = 0; k < asPositions.NumElements(); k++)
			{
				psPosition = asPositions.Get(k);
				if (!bAny)
				{
					sMin = *psPosition;
					sMax = *psPosition;
					bAny = true;
				}
				sMin.Init(fminf(sMin.x, psPosition->x), fminf(sMin.y, psPosition->y), fminf(sMin.z, psPosition->z));
				sMax.Init(fmaxf(sMax.x, psPosition->x), fmaxf(sMax.y, psPosition->y), fmaxf(sMax.z, psPosition->z));
			}
		}
	}
	asPositions.Kill();

	msCentre.Init((sMin.x + sMax.x) * 0.5f, (sMin.y + sMax.y) * 0.5f, (sMin.z + sMax.z) * 0.5f);
	sOut.Init(sMax.x - msCentre.x, sMax.y - msCentre.y, sMax.z - msCentre.z);
	fRadius = sqrtf(sOut.x * sOut.x + sOut.y * sOut.y + sOut.z * sOut.z);
	mfRadius = (fRadius > 0) ? fRadius : 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// The scene's first camera, or the framed view if it has none.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::StartView(void)
{
	muiView = 0;
	if ((mpScene->mapCameras.NumElements() == 0) || !StartFromCamera(mpScene->mapCameras.Get(0)))
	{
		muiView = mpScene->mapCameras.NumElements();
		StartFramed();
	}
	ResetView();
}


//////////////////////////////////////////////////////////////////////////
//
// The next scene camera, or the last, going round through the framed view
// after the last camera.  A camera that isn't placed is passed over.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::StepView(bool bForward)
{
	size	uiNumViews;
	size	i;

	uiNumViews = mpScene->mapCameras.NumElements() + 1;
	for (i = 0; i < uiNumViews; i++)
	{
		muiView = bForward ? ((muiView + 1) % uiNumViews) : ((muiView + uiNumViews - 1) % uiNumViews);
		if (muiView == uiNumViews - 1)
		{
			StartFramed();
			break;
		}
		if (StartFromCamera(mpScene->mapCameras.Get(muiView)))
		{
			break;
		}
	}
	ResetView();
	SetStatus();
}


//////////////////////////////////////////////////////////////////////////
//
// Where pCamera is now (STurntable::From).  False if it isn't placed.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::StartFromCamera(Ptr<CCamera> pCamera)
{
	if (pCamera.IsNull() || pCamera->mpConnection.IsNull())
	{
		return false;
	}
	return msStart.From(&pCamera->mpConnection->msWorldMatrix, pCamera->mfFOV, &msCentre, mfRadius);
}


//////////////////////////////////////////////////////////////////////////
//
// Looking at all of the scene (STurntable::Frame).
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::StartFramed(void)
{
	msStart.Frame(&msCentre, mfRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::ResetView(void)
{
	msView = msStart;
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The camera is where the view is.  Near and far keep the scene in front
// of it.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::PlaceCamera(void)
{
	msView.World(&mpCamera->mpConnection->msWorldMatrix);
	mpCamera->mpConnection->msLocalMatrix = mpCamera->mpConnection->msWorldMatrix;
	mpCamera->mfFOV = msView.fFOV;
	mpCamera->mfNear = mfRadius * 0.001f;
	mpCamera->mfFar = (msView.fDistance + mfRadius) * 4.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Orbit(int32 iRight, int32 iDown)
{
	msView.Orbit(iRight, iDown);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Pan(int32 iRight, int32 iDown)
{
	msView.Pan(iRight, iDown, msSize.x);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// More than 1 is closer.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Zoom(float32 fFactor)
{
	msView.Zoom(fFactor);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::SwitchPlaying(void)
{
	mbPlaying = !mbPlaying;
	SetStatus();
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The raster renderer's scale makes the scene's radius SCENE_RASTER_UNITS
// of its units.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::SwitchRenderer(void)
{
	if (mpVulkanRenderer.IsNull() || !mpVulkanRenderer->IsReady())
	{
		return;
	}
	mbVulkan = !mbVulkan;
	mfDrawMilliseconds = 0;
	SetStatus();
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// Something drawn has changed: the canvas is drawn again at the next paint.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::Changed(void)
{
	if (mpcCanvas)
	{
		mpcCanvas->Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Posted by Draw while it plays.
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::NextFrame(void)
{
	mbPosted = false;
	if (GetDebugTickCount() - miStatusTime >= SCENE_STATUS_MILLISECONDS)
	{
		SetStatus();
	}
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneCanvasDraw::SetStatus(void)
{
	CChars	szStatus;

	miStatusTime = GetDebugTickCount();
	if (mpcStatus == NULL)
	{
		return;
	}
	szStatus.Init(mszName.Text());
	szStatus.Append(": ");
	if (mbVulkan)
	{
		szStatus.Append("Vulkan on ");
		szStatus.Append(mpVulkanRenderer->GetDeviceName());
	}
	else if (mpVulkanRenderer.IsNotNull() && mpVulkanRenderer->IsReady())
	{
		szStatus.Append("the raster renderer");
	}
	else
	{
		szStatus.Append("the raster renderer, as there's no device with Vulkan 1.3");
	}
	if (mfDrawMilliseconds > 0)
	{
		szStatus.Append(", ");
		szStatus.Append((int32)(mfDrawMilliseconds + 0.5));
		szStatus.Append(" ms a frame");
	}
	if (mapPlayers.NumElements() > 0)
	{
		szStatus.Append(mbPlaying ? ", playing" : ", paused");
	}
	if (mpScene->mapCameras.NumElements() > 0)
	{
		if (muiView < mpScene->mapCameras.NumElements())
		{
			szStatus.Append(", camera ");
			szStatus.Append((int32)(muiView + 1));
			szStatus.Append(" of ");
			szStatus.Append((int32)mpScene->mapCameras.NumElements());
		}
		else
		{
			szStatus.Append(", the whole scene");
		}
	}
	szStatus.Append(".");
	mpcStatus->SetText(szStatus.Text());
	szStatus.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The rasteriser's frame and target are made the first time.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::DrawRaster(void)
{
	int32	iScale;

	if (mpRasterFrame.IsNull())
	{
		iScale = (int32)(SCENE_RASTER_UNITS / mfRadius);
		mcRasterFactory.Init((iScale > 0) ? iScale : 1);
		mbRasterFactory = true;
		mpRasterFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(mpScene, &mcRasterFactory, mcRasterFactory.CreateRenderer()), mpCamera);
	}
	if (mcTarget.mpiDepths == NULL)
	{
		if (!mcTarget.Init((uint32*)mpImage->GetData(), msSize.x, msSize.y, msSize.x))
		{
			return false;
		}
	}
	mcTarget.Clear(muiBackground);
	mpRasterFrame->Draw(&mcRasteriser, &mcTarget);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// B, G, R, X is what both renderers write and what the window's canvas
// holds.
//
//////////////////////////////////////////////////////////////////////////
bool CSceneCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2	sSize;
	int64		iNow;
	int64		iStart;
	float64		fMilliseconds;
	bool		bDrawn;

	sSize = pCanvas->GetActualSize();
	if ((sSize.x <= 0) || (sSize.y <= 0) || mpScene.IsNull())
	{
		return true;
	}
	mpcCanvas = &pCanvas;
	mpcWindow = &pCanvas->GetWindow();

	if ((sSize.x != msSize.x) || (sSize.y != msSize.y))
	{
		if (mcTarget.mpiDepths != NULL)
		{
			mcTarget.Kill();
			memset(&mcTarget, 0, sizeof(mcTarget));
		}
		mpImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_IGNORED, CHANNEL_STOP);
		mpCelDraw->SetCel(OMalloc<CImageCel>(mpImage));
		msSize = sSize;
	}

	iNow = GetDebugTickCount();
	Advance(iNow);
	PlaceCamera();

	iStart = GetDebugTickCount();
	if (mbVulkan)
	{
		bDrawn = mpVulkanFrame->Draw((uint32*)mpImage->GetData(), msSize.x, msSize.y, muiBackground);
	}
	else
	{
		bDrawn = DrawRaster();
	}
	fMilliseconds = (float64)(GetDebugTickCount() - iStart);
	mfDrawMilliseconds = (mfDrawMilliseconds > 0) ? (mfDrawMilliseconds * 0.9 + fMilliseconds * 0.1) : fMilliseconds;

	if (mbPlaying && !mbPosted && (mapPlayers.NumElements() > 0))
	{
		mpcWindow->PostAfter(SCENE_FRAME_MILLISECONDS, this, &CSceneCanvasDraw::NextFrame);
		mbPosted = true;
	}
	if (!bDrawn)
	{
		return false;
	}
	return mpCelDraw->Draw(pCanvas);
}

