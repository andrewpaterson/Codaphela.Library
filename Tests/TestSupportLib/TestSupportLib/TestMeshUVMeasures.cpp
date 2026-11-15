#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshUVMeasures.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A unit square at height z, anticlockwise seen from +z, its corners' UVs
// (u0, v0), (u0 + fWidth, v0) and so on: its own UVs, so faces of other
// squares can share its UVs' values without sharing its UVs.
//
//////////////////////////////////////////////////////////////////////////
static void TestUVAddSquare(CMesh* pcMesh, float32 z, float32 u0, float32 v0, float32 fWidth)
{
	CMeshUVLayer*	pcLayer;
	SMeshUVFace*	psUVFace;
	int				iCorner;
	int				iUV;
	int				iFace;

	iCorner = pcMesh->NumCorners();
	pcLayer = pcMesh->mcUVs.mcLayers.Get(0);
	iUV = (int)pcLayer->mcUVs.NumElements();
	pcMesh->AddCorner(0, 0, z);
	pcMesh->AddCorner(1, 0, z);
	pcMesh->AddCorner(1, 1, z);
	pcMesh->AddCorner(0, 1, z);
	pcMesh->AddUV(u0, v0);
	pcMesh->AddUV(u0 + fWidth, v0);
	pcMesh->AddUV(u0 + fWidth, v0 + 1.0f);
	pcMesh->AddUV(u0, v0 + 1.0f);

	iFace = pcMesh->NumFaces();
	pcMesh->AddFace(iCorner, iCorner + 1, iCorner + 2);
	pcMesh->AddFace(iCorner, iCorner + 2, iCorner + 3);
	psUVFace = pcLayer->mcFaces.Get(iFace);
	psUVFace->aiUVs[0] = iUV;
	psUVFace->aiUVs[1] = iUV + 1;
	psUVFace->aiUVs[2] = iUV + 2;
	psUVFace = pcLayer->mcFaces.Get(iFace + 1);
	psUVFace->aiUVs[0] = iUV;
	psUVFace->aiUVs[1] = iUV + 2;
	psUVFace->aiUVs[2] = iUV + 3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestUVMeshInit(CMesh* pcMesh)
{
	pcMesh->Init();
	pcMesh->mcUVs.mbInUse = true;
	pcMesh->mcUVs.AddLayer(pcMesh->GetConnectivity());
}


//////////////////////////////////////////////////////////////////////////
//
// A square mapped to itself, 64 texels across at 64 texels to the unit,
// isn't stretched at all; squeezed to half its width in u it's twice as
// stretched one way as the other and has half the texels; mirrored, its
// faces are turned over.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVMeasuresStretch(void)
{
	CMesh			cMesh;
	SMeshUVStretch	sStretch;

	TestUVMeshInit(&cMesh);
	TestUVAddSquare(&cMesh, 0, 0, 0, 1);
	AssertInt(0, CountMeshUVFacesMissing(&cMesh, 0));
	AssertInt(0, CountMeshUVFacesTurned(&cMesh, 0));
	AssertInt(0, CountMeshUVsOutside(&cMesh, 0));
	AssertInt(0, CountMeshUVOverlaps(&cMesh, 0));
	MeasureMeshUVStretch(&sStretch, &cMesh, 0, 64, 64, 64);
	AssertInt(2, sStretch.iNumFaces);
	AssertFloat(1.0f, sStretch.fWorstAngle, 5);
	AssertFloat(1.0f, sStretch.fMeanAngle, 5);
	AssertFloat(1.0f, sStretch.fLeastArea, 5);
	AssertFloat(1.0f, sStretch.fMostArea, 5);
	cMesh.Kill();

	TestUVMeshInit(&cMesh);
	TestUVAddSquare(&cMesh, 0, 0, 0, 0.5f);
	MeasureMeshUVStretch(&sStretch, &cMesh, 0, 64, 64, 64);
	AssertFloat(2.0f, sStretch.fWorstAngle, 5);
	AssertFloat(2.0f, sStretch.fMeanAngle, 5);
	AssertFloat(0.5f, sStretch.fLeastArea, 5);
	AssertFloat(0.5f, sStretch.fMostArea, 5);
	AssertInt(0, CountMeshUVFacesTurned(&cMesh, 0));
	cMesh.Kill();

	TestUVMeshInit(&cMesh);
	TestUVAddSquare(&cMesh, 0, 1, 0, -1);
	AssertInt(2, CountMeshUVFacesTurned(&cMesh, 0));
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A second square on the same UVs overlaps the first face for face (the
// faces of one square only touch, along their diagonal); moved half across
// it overlaps three pairs and two of its UVs are outside.  A face with no
// UVs is missing them.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVMeasuresOverlaps(void)
{
	CMesh	cMesh;

	TestUVMeshInit(&cMesh);
	TestUVAddSquare(&cMesh, 0, 0, 0, 1);
	TestUVAddSquare(&cMesh, 1, 0, 0, 1);
	AssertInt(2, CountMeshUVOverlaps(&cMesh, 0));
	AssertInt(0, CountMeshUVsOutside(&cMesh, 0));
	cMesh.Kill();

	TestUVMeshInit(&cMesh);
	TestUVAddSquare(&cMesh, 0, 0, 0, 1);
	TestUVAddSquare(&cMesh, 1, 0.5f, 0, 1);
	AssertInt(3, CountMeshUVOverlaps(&cMesh, 0));
	AssertInt(2, CountMeshUVsOutside(&cMesh, 0));

	cMesh.AddCorner(0, 0, 2);
	cMesh.AddCorner(1, 0, 2);
	cMesh.AddCorner(0, 1, 2);
	cMesh.AddFace(8, 9, 10);
	AssertInt(1, CountMeshUVFacesMissing(&cMesh, 0));
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A 30 by 30 grid of squares laid flat over the UVs: no overlaps, though
// every face touches its neighbours; one face more on the same UVs as one
// of them is one overlap.  (The grid of cells the faces are sorted into
// is then 42 across.)
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVMeasuresGrid(void)
{
	CMesh			cMesh;
	CMeshUVLayer*	pcLayer;
	SMeshUVFace*	psUVFace;
	int				x;
	int				y;
	int				iFace;

	TestUVMeshInit(&cMesh);
	pcLayer = cMesh.mcUVs.mcLayers.Get(0);
	for (y = 0; y <= 30; y++)
	{
		for (x = 0; x <= 30; x++)
		{
			cMesh.AddCorner((float32)x, (float32)y, 0);
			cMesh.AddUV((float32)x / 30.0f, (float32)y / 30.0f);
		}
	}
	for (y = 0; y < 30; y++)
	{
		for (x = 0; x < 30; x++)
		{
			iFace = cMesh.NumFaces();
			cMesh.AddFace((y * 31) + x, (y * 31) + x + 1, ((y + 1) * 31) + x + 1);
			cMesh.AddFace((y * 31) + x, ((y + 1) * 31) + x + 1, ((y + 1) * 31) + x);
			psUVFace = pcLayer->mcFaces.Get(iFace);
			psUVFace->aiUVs[0] = (y * 31) + x;
			psUVFace->aiUVs[1] = (y * 31) + x + 1;
			psUVFace->aiUVs[2] = ((y + 1) * 31) + x + 1;
			psUVFace = pcLayer->mcFaces.Get(iFace + 1);
			psUVFace->aiUVs[0] = (y * 31) + x;
			psUVFace->aiUVs[1] = ((y + 1) * 31) + x + 1;
			psUVFace->aiUVs[2] = ((y + 1) * 31) + x;
		}
	}
	AssertInt(1800, cMesh.NumFaces());
	AssertInt(0, CountMeshUVOverlaps(&cMesh, 0));
	AssertInt(0, CountMeshUVFacesTurned(&cMesh, 0));

	cMesh.AddCorner(0, 0, 1);
	cMesh.AddCorner(1, 0, 1);
	cMesh.AddCorner(1, 1, 1);
	iFace = cMesh.NumFaces();
	cMesh.AddFace(961, 962, 963);
	psUVFace = pcLayer->mcFaces.Get(iFace);
	psUVFace->aiUVs[0] = (15 * 31) + 15;
	psUVFace->aiUVs[1] = (15 * 31) + 16;
	psUVFace->aiUVs[2] = (16 * 31) + 16;
	AssertInt(1, CountMeshUVOverlaps(&cMesh, 0));
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVMeasures(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshUVMeasuresStretch();
	TestMeshUVMeasuresOverlaps();
	TestMeshUVMeasuresGrid();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

