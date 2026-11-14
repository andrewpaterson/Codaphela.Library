/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_FRAME_H__
#define __RASTER_FRAME_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/IndexedGeneral.h"
#include "StandardLib/Array.h"
#include "SupportLib/Camera.h"
#include "RasterLib/Rasteriser.h"
#include "RenderLib/RenderScene.h"


class CInstance;
class CInstanceMesh;


//A skinned instance mesh's own vertices, kept from one frame to the next.  It's found by the instance mesh's object index,
//so the frame doesn't keep the instance mesh, and one that's gone can't be taken for another.  It goes in a frame that
//doesn't draw it.  bMirrorReported is whether a bone mirroring has been logged for it.
struct SRasterFramePosed
{
	OIndex			oiInstanceMesh;
	CRasterPosed*	pcPosed;
	bool			bMirrorReported;
	bool			bUsed;
};


typedef CArrayTemplate<SRasterInstance>		CArrayRasterInstance;
typedef CArrayTemplate<SRasterFramePosed>	CArrayRasterFramePosed;
typedef CArrayTemplate<SRasterBone>			CArrayRasterBone;


//A frame of a scene as the raster renderer draws it, made by Extract from a CRenderScene whose renderer is a CRasterRenderer,
//after Extract updates the render scene.
//
//Each rigid mesh of each instance is an SRasterInstance placed by its connection's world matrix, at the renderer's scale.  Its
//axes are the matrix's, scale, mirror and all, and its light axes their rotation alone.  The frame holds the raster meshes its
//instances point at, so that they stay while they're drawn.
//
//A skinned mesh is posed by the frame, each of its matrices being its inverse matrix times the world matrix of the instance
//mesh's connection for it, placed as a rigid mesh would be.  Its instance has the posed vertices, and no axes of its own.  It
//isn't drawn if a connection is missing, which binding it logged (CInstanceMesh::BindBones), or if a matrix mirrors, which is
//an error logged the first frame it happens, and not again while the instance mesh is drawn or tried.  A skin its raster mesh
//couldn't take isn't drawn.
//
//The camera is the scene camera, if there is one.  It's set from it when the scene camera moves, with the eye where the scene
//camera is and the point it turns around in front of it: as far as the middle of what's drawn the first time, and as far as
//it was after.  The viewer's controls then move it (GetCamera), and the next Extract writes where it is back into the scene
//camera's connection.  The scene's field of view is across, and makes the focal length for the target's width.
//
//The rasteriser's lights are the scene's: ambient lights add up to its ambient, directional lights are far away, and omni and
//spot lights are point lights, a spot's cone not being used yet.  A light's amount is its intensity times its colour's
//brightness, of the most a shade can be.  A scene with no lights keeps the light the rasteriser has always had.
class CRasterFrame : public CObject
{
CONSTRUCTABLE(CRasterFrame);
DESTRUCTABLE(CRasterFrame);
protected:
	Ptr<CRenderScene>		mpRenderScene;
	Ptr<CCamera>			mpCamera;
	CArrayRasterInstance	masInstances;
	CArray<CRasterMesh>		mapMeshes;
	CArrayRasterFramePosed	masPosed;
	CArrayRasterBone		masBones;  //The matrices of the skinned mesh being posed.
	CRasterCamera			mcCamera;
	SFloat4x4				msCameraWorld;  //The scene camera's world matrix when the camera last matched it.
	size					muiCameraChanges;  //The camera's changes when it last matched the scene camera.
	bool					mbCameraSet;

public:
	void				Init(Ptr<CRenderScene> pRenderScene, Ptr<CCamera> pCamera);
	void				Class(void) override;
	void				Free(void) override;

	void				Extract(CRasteriser* pcRasteriser, int32 iWidth);
	void				Draw(CRasteriser* pcRasteriser, CRasterTarget* pcTarget);

	CRasterCamera*		GetCamera(void);
	SRasterInstance*	GetInstances(void);
	size				NumInstances(void);

protected:
	void				ExtractInstances(int32 iScale);
	bool				ExtractPosed(CInstance* pcInstance, CInstanceMesh* pcInstanceMesh, CRasterMesh* pcRasterMesh, int32 iScale);
	SRasterFramePosed*	GetPosed(OIndex oiInstanceMesh);
	void				ReleaseUnusedPosed(void);
	void				ReleasePosed(SRasterFramePosed* psPosed);
	void				ExtractCamera(int32 iWidth, int32 iScale);
	void				SetCameraFromScene(SFloat4x4* psWorld, int32 iScale);
	void				WriteCamera(int32 iScale);
	void				ExtractLights(CRasterLights* pcLights, int32 iScale);
	int32				FindPivotDistance(SFloat32Vec3* psEye);
};


#endif // __RASTER_FRAME_H__

