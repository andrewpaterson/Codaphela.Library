#include "BaseLib/Float4x4.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Instance.h"
#include "SupportLib/MeshBake.h"
#include "SupportLib/MeshUnwrap.h"
#include "RenderLib/RenderScene.h"
#include "RasterRenderLib/RasterFrame.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "ProceduralLib/MeshShapeEditor.h"
#include "ProceduralLib/MeshShapeNormals.h"
#include "ShapeSceneHelper.h"


#define SHAPE_SCENE_SCALE	1000


//////////////////////////////////////////////////////////////////////////
//
// pEditor's mesh in an instance of its own, where its shape put it.
//
//////////////////////////////////////////////////////////////////////////
void ShapeSceneInstance(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor)
{
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CConnection>	pConnection;

	pConnection = OMalloc<CConnection>();
	pConnection->msWorldMatrix.Identity();
	pConnection->msLocalMatrix.Identity();
	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstanceMesh->mapConnections.Add(pConnection);
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pConnection);
	pInstance->mapMeshes.Add(pInstanceMesh);
	pScene->mapInstances.Add(pInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// pShape meshed, its normals from the shape, unwrapped at fTexelsPerUnit
// and pTexture baked onto it.
//
//////////////////////////////////////////////////////////////////////////
void ShapeSceneAddTextured(Ptr<CScene> pScene, Ptr<CShape> pShape, Ptr<CSolidTexture> pTexture, float32 fTexelsPerUnit)
{
	Ptr<CMeshEditor>	pEditor;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pShape, 0.05f, 0.4f);
	pEditor->AddModifier<CMeshShapeNormals>(pShape);
	pEditor->AddModifier<CMeshUnwrap>(fTexelsPerUnit);
	pEditor->AddModifier<CMeshBake>(pTexture);
	ShapeSceneInstance(pScene, pEditor);
}


//////////////////////////////////////////////////////////////////////////
//
// A camera at psEye looking at the origin, with z up in the scene.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCamera> ShapeSceneCamera(SFloat32Vec3* psEye)
{
	Ptr<CCamera>	pCamera;
	SFloat4x4		sWorld;
	SFloat32Vec3	sBack;
	SFloat32Vec3	sUp;
	SFloat32Vec3	sRight;
	SFloat32Vec3	sSceneUp;

	Float32Vec3Normalise(&sBack, psEye);
	sSceneUp.Init(0, 0, 1);
	Float32Vec3Cross(&sRight, &sSceneUp, &sBack);
	sRight.Normalize();
	Float32Vec3Cross(&sUp, &sBack, &sRight);

	sWorld.Identity();
	sWorld.x.Init(sRight.x, sRight.y, sRight.z, 0.0f);
	sWorld.y.Init(sUp.x, sUp.y, sUp.z, 0.0f);
	sWorld.z.Init(sBack.x, sBack.y, sBack.z, 0.0f);
	sWorld.pos.Init(psEye->x, psEye->y, psEye->z, 1.0f);

	pCamera = OMalloc<CCamera>();
	pCamera->mfFOV = 3.14159265f / 3.0f;
	pCamera->mpConnection = OMalloc<CConnection>();
	pCamera->mpConnection->msWorldMatrix = sWorld;
	pCamera->mpConnection->msLocalMatrix = sWorld;
	return pCamera;
}


//////////////////////////////////////////////////////////////////////////
//
// pScene drawn by the raster renderer from psEye into iWidth by iHeight
// pixels, ARGB32 rows from the top, black where nothing is.  How many
// instances were drawn.
//
//////////////////////////////////////////////////////////////////////////
size ShapeSceneDraw(Ptr<CScene> pScene, SFloat32Vec3* psEye, uint32* puiPixels, int iWidth, int iHeight)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	CRasterTarget			cTarget;
	Ptr<CCamera>			pCamera;
	Ptr<CRasterFrame>		pFrame;
	size					uiInstances;

	pCamera = ShapeSceneCamera(psEye);
	pScene->mapCameras.Add(pCamera);

	cFactory.Init(SHAPE_SCENE_SCALE);
	cRasteriser.Init();
	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), pCamera);

	cTarget.Init(puiPixels, iWidth, iHeight, iWidth);
	cTarget.Clear(0xFF000000);
	pFrame->Draw(&cRasteriser, &cTarget);
	uiInstances = pFrame->NumInstances();

	cTarget.Kill();
	pFrame = NULL;
	pCamera = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
	return uiInstances;
}

