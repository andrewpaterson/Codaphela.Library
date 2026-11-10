#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Instance.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "RenderLib/RenderScene.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "RasterRenderLib/RasterRenderMesh.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A triangle with corners at 0 and 1.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestRenderSceneEditor(void)
{
	Ptr<CMeshEditor>	pEditor;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddFace(0, 1, 2);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestRenderSceneAddInstance(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor)
{
	Ptr<CInstance>	pInstance;

	pInstance = OMalloc<CInstance>();
	pInstance->mapMeshes.Add(OMalloc<CInstanceMesh>(pEditor));
	pScene->mapInstances.Add(pInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh is made once for an editor however many instances share it, at
// the renderer's scale, and isn't made again while nothing has changed.
// Nothing is made before the first update.
//
//////////////////////////////////////////////////////////////////////////
void TestRenderSceneCreate(void)
{
	CRasterRenderFactory	cFactory;
	Ptr<CScene>				pScene;
	Ptr<CRenderScene>		pRenderScene;
	Ptr<CMeshEditor>		pEditor;
	Ptr<CRasterRenderMesh>	pMesh;

	cFactory.Init(100);
	pScene = OMalloc<CScene>();
	pEditor = TestRenderSceneEditor();
	TestRenderSceneAddInstance(pScene, pEditor);
	TestRenderSceneAddInstance(pScene, pEditor);
	pRenderScene = OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer());
	AssertTrue(pRenderScene->GetMesh(pEditor).IsNull());

	pRenderScene->Update();
	AssertSize(1, pRenderScene->NumMeshes());
	pMesh = pRenderScene->GetMesh(pEditor);
	AssertTrue(pMesh.IsNotNull());
	AssertSize(3, pMesh->GetRasterMesh()->masVertices.NumElements());
	AssertSize(1, pMesh->GetRasterMesh()->NumTriangles());
	AssertInt(100, pMesh->GetRasterMesh()->msMax.x);
	AssertInt(100, pMesh->GetRasterMesh()->msMax.y);

	pRenderScene->Update();
	AssertTrue(pRenderScene->GetMesh(pEditor) == pMesh);

	pMesh = NULL;
	pRenderScene = NULL;
	pScene = NULL;
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A change to the editor's structure, shape or modifiers makes a new mesh
// at the next update, and the old one is left as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestRenderSceneReplace(void)
{
	CRasterRenderFactory	cFactory;
	Ptr<CScene>				pScene;
	Ptr<CRenderScene>		pRenderScene;
	Ptr<CMeshEditor>		pEditor;
	Ptr<CRasterRenderMesh>	pOld;
	Ptr<CRasterRenderMesh>	pNew;
	SFloat32Vec3			sPosition;

	cFactory.Init(100);
	pScene = OMalloc<CScene>();
	pEditor = TestRenderSceneEditor();
	TestRenderSceneAddInstance(pScene, pEditor);
	pRenderScene = OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer());
	pRenderScene->Update();
	pOld = pRenderScene->GetMesh(pEditor);

	pEditor->AddCorner(0, 0, 1);
	pEditor->AddFace(0, 1, 3);
	AssertTrue(pRenderScene->GetMesh(pEditor) == pOld);
	pRenderScene->Update();
	pNew = pRenderScene->GetMesh(pEditor);
	AssertFalse(pNew == pOld);
	AssertSize(2, pNew->GetRasterMesh()->NumTriangles());
	AssertSize(1, pOld->GetRasterMesh()->NumTriangles());
	AssertTrue(pNew->GetRenderer() == pOld->GetRenderer());

	pOld = pNew;
	sPosition.Init(2, 0, 0);
	pEditor->MovePosition(1, &sPosition);
	pRenderScene->Update();
	pNew = pRenderScene->GetMesh(pEditor);
	AssertFalse(pNew == pOld);

	pOld = pNew;
	pEditor->mcModifiers.Touch();
	pRenderScene->Update();
	pNew = pRenderScene->GetMesh(pEditor);
	AssertFalse(pNew == pOld);
	AssertSize(1, pRenderScene->NumMeshes());

	pOld = NULL;
	pNew = NULL;
	pRenderScene = NULL;
	pScene = NULL;
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh the scene's instances no longer use is let go at the next update.
//
//////////////////////////////////////////////////////////////////////////
void TestRenderSceneUnused(void)
{
	CRasterRenderFactory	cFactory;
	Ptr<CScene>				pScene;
	Ptr<CRenderScene>		pRenderScene;
	Ptr<CMeshEditor>		pKept;
	Ptr<CMeshEditor>		pGone;

	cFactory.Init(100);
	pScene = OMalloc<CScene>();
	pKept = TestRenderSceneEditor();
	pGone = TestRenderSceneEditor();
	TestRenderSceneAddInstance(pScene, pKept);
	TestRenderSceneAddInstance(pScene, pGone);
	pRenderScene = OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer());
	pRenderScene->Update();
	AssertSize(2, pRenderScene->NumMeshes());

	pScene->mapInstances.RemoveAt(1);
	pRenderScene->Update();
	AssertSize(1, pRenderScene->NumMeshes());
	AssertTrue(pRenderScene->GetMesh(pKept).IsNotNull());
	AssertTrue(pRenderScene->GetMesh(pGone).IsNull());

	pRenderScene = NULL;
	pScene = NULL;
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRenderScene(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRenderSceneCreate();
	TestRenderSceneReplace();
	TestRenderSceneUnused();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

