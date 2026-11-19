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
#ifndef __SCENE_CANVAS_DRAW_H__
#define __SCENE_CANVAS_DRAW_H__
#include "BaseLib/Chars.h"
#include "RasterLib/Rasteriser.h"
#include "RasterLib/RasterTarget.h"
#include "RasterRenderLib/RasterFrame.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "SupportLib/AnimationPlayer.h"
#include "SupportLib/Camera.h"
#include "SupportLib/Image.h"
#include "SupportLib/Scene.h"
#include "VulkanLib/VulkanFrame.h"
#include "VulkanLib/VulkanRenderFactory.h"
#include "WindowLib/CanvasDraw.h"
#include "WindowLib/ImageCanvasDraw.h"
#include "WindowLib/Label.h"
#include "WindowLib/WindowTheme.h"
#include "Turntable.h"


class CWindow;


//Draws a scene read from a scene file with Vulkan, or with the raster renderer (SwitchRenderer), into a cel the size of the
//canvas, which an image draw puts on the canvas.  Vulkan's frame is read back into the cel, whose bytes are B, G, R, X as the
//window's canvas has them.  With no device that has Vulkan 1.3, or told not to (UseVulkan), it's drawn by the rasteriser,
//and can't be switched.
//
//Its animations loop: a skeleton's on each instance of the skeleton, and the scene's own on the scene.  While they play the
//canvas is drawn again every SCENE_FRAME_MILLISECONDS (the window's PostAfter).  Paused, or not drawn as its tab isn't
//showing, nothing is posted and the window waits.
//
//The view is the draw's own camera, not the scene's: a turntable (STurntable), turning about a point so up stays up.
//It starts where the scene's first camera is, with its field of view, or with none it's framed on the scene posed at the
//start.  StepView goes to the scene's next or last camera, and after the last to the framed view, and round.  ResetView
//puts the view back where its camera, or the framed view, started it.
//
//The status label, which the page holds, says what's drawn and how (not which keys do what: those are whoever holds
//it's to say); it's set outside the paint.
//
//SetScene draws a scene made rather than read; SceneChanged says it changed, and ScreenToPlane is where a point on the canvas is
//on the plane of the screen through what the view turns about.
class CSceneCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CSceneCanvasDraw);
DESTRUCTABLE(CSceneCanvasDraw);
protected:
	Ptr<CImageCanvasDraw>		mpCelDraw;
	Ptr<CImage>					mpImage;  //What it's drawn on.
	uint32						muiBackground;  //0xAARRGGBB.
	SInt32Vec2					msSize;  //The canvas size mpImage was made for, -1, -1 for none yet.

	Ptr<CScene>					mpScene;
	CChars						mszName;  //The file's name, for the status.
	CArray<CAnimationPlayer>	mapPlayers;
	Ptr<CCamera>				mpCamera;

	CVulkanRenderFactory		mcVulkanFactory;
	Ptr<CVulkanRenderer>		mpVulkanRenderer;
	Ptr<CVulkanFrame>			mpVulkanFrame;
	CRasterRenderFactory		mcRasterFactory;  //Made when it's first drawn by the rasteriser.
	bool						mbRasterFactory;
	CRasteriser					mcRasteriser;
	CRasterTarget				mcTarget;  //On mpImage, made when it's first drawn by the rasteriser.
	Ptr<CRasterFrame>			mpRasterFrame;  //Made when it's first drawn by the rasteriser.
	bool						mbVulkan;  //Drawn with Vulkan, not the rasteriser.
	bool						mbUseVulkan;  //A scene is drawn with Vulkan if there's a device for it (UseVulkan).

	SFloat32Vec3				msCentre;  //Of the scene as it was posed at the start,
	float32						mfRadius;  //and how far it goes from there.
	size						muiView;  //The scene camera the view started from, or the number of cameras for the framed view.
	STurntable					msStart;  //Where ResetView puts the view.
	STurntable					msView;  //Where it is.

	bool						mbPlaying;
	int64						miLastFrame;  //When it was last drawn, 0 if it hasn't been.
	float64						mfDrawMilliseconds;  //Smoothed, what drawing a frame takes.
	int64						miStatusTime;  //When the status was last set.
	CLabel*						mpcStatus;  //The page's: not held.
	CCanvas*					mpcCanvas;  //What it last drew, which holds it, and its window: not held.
	CWindow*					mpcWindow;
	bool						mbPosted;  //The next frame is posted to mpcWindow.

public:
	void		Init(Ptr<CWindowTheme> pTheme, CLabel* pcStatus);
	void		Class(void) override;
	void		Free(void) override;
	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	void		UseVulkan(bool bUse);
	bool		ReadScene(const char* szFilename);
	void		SetScene(Ptr<CScene> pScene, const char* szName);
	void		SceneChanged(const char* szName);
	void		SetCooked(Ptr<CMeshEditor> pEditor, Ptr<CCookedMesh> pCooked);
	bool		ScreenToPlane(int32 x, int32 y, SFloat32Vec3* psDest);
	Ptr<CImage>	GetImage(void) const;
	bool		Draw(Ptr<CCanvas> pCanvas) override;
	void		NextFrame(void);

	void		SwitchRenderer(void);
	void		SwitchPlaying(void);
	void		ResetView(void);
	void		StepView(bool bForward);
	void		Orbit(int32 iRight, int32 iDown);
	void		Pan(int32 iRight, int32 iDown);
	void		Zoom(float32 fFactor);

protected:
	void		MakePlayers(void);
	void		Advance(int64 iNow);
	void		FindBounds(void);
	void		StartView(void);
	bool		StartFromCamera(Ptr<CCamera> pCamera);
	void		StartFramed(void);
	void		PlaceCamera(void);
	bool		DrawRaster(void);
	void		SetStatus(void);
	void		Changed(void);
};


#endif // __SCENE_CANVAS_DRAW_H__

