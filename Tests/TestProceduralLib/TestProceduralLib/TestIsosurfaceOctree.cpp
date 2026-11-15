#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/ShapeSubtraction.h"
#include "SupportLib/ShapeIntersection.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"
#include "IsosurfaceMeshes.h"
#include "TestGyroid.h"


//////////////////////////////////////////////////////////////////////////
//
// A sphere from cells of 1 down to 0.0625: closed, facing out, its
// corners within 0.005 of it and its volume within 0.2%, with a quarter
// of the corners a uniform grid of the smallest cell makes.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceOctreeSphere(void)
{
	Ptr<CSphere>	pSphere;
	Ptr<CMesh>		pMesh;
	Ptr<CMesh>		pUniform;
	SFloat32Vec3	sCentre;
	int				iLeaves;
	float32			fVolume;

	sCentre.Init(0.1f, 0.2f, 0.3f);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pMesh = OctreeMesh(&pSphere, 0.0625f, 1.0f, 0.005f, &iLeaves);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pSphere));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pSphere) < 0.005f);
	fVolume = 4.18879f * 8.0f;
	AssertTrue(fabsf(MeshVolume(&pMesh) - fVolume) < (fVolume * 0.002f));

	pUniform = DualContouringMesh(&pSphere, 0.0625f);
	AssertTrue((pMesh->NumCorners() * 3) < pUniform->NumCorners());
}


//////////////////////////////////////////////////////////////////////////
//
// A turned box: closed, its flat faces left coarse (half the corners of a
// uniform grid of the smallest cell), and its volume within 0.1%.  Its
// corners are within a quarter of the smallest cell, as the cell holding
// one may not cross all three of its faces (Isosurface.txt's Known
// limits).  Faces face out but one, by a corner clamped off a crease (as
// in TestDualContouringBox).
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceOctreeBox(void)
{
	Ptr<CBox>		pBox;
	Ptr<CMesh>		pMesh;
	Ptr<CMesh>		pUniform;
	SFloat32Box		sBox;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sCorner;
	float32			fFurthest;
	int				iLeaves;
	int				i;

	sCentre.Init(0.05f, 0.03f, 0.02f);
	asAxes[0].Init(2, 1, 2);
	asAxes[0] /= 3.0f;
	asAxes[1].Init(-2, 2, 1);
	asAxes[1] /= 3.0f;
	Float32Vec3Cross(&asAxes[2], &asAxes[0], &asAxes[1]);
	sBox.Init(&sCentre, &asAxes[0], 1.1f, &asAxes[1], 0.9f, &asAxes[2], 0.7f);
	pBox = OMalloc<CBox>(&sBox);
	pMesh = OctreeMesh(&pBox, 0.0625f, 1.0f, 0.005f, &iLeaves);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertTrue(CountMeshFacesFacingIn(&pMesh, &pBox) <= 1);
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pBox) < 0.019f);
	fFurthest = 0.0f;
	for (i = 0; i < 8; i++)
	{
		sBox.GetCorner(&sCorner, i);
		fFurthest = fmaxf(fFurthest, NearestMeshCorner(&pMesh, &sCorner));
	}
	AssertTrue(fFurthest < 0.016f);
	AssertTrue(fabsf(MeshVolume(&pMesh) - 5.544f) < 0.0055f);

	pUniform = DualContouringMesh(&pBox, 0.0625f);
	AssertTrue((pMesh->NumCorners() * 2) < pUniform->NumCorners());
}


//////////////////////////////////////////////////////////////////////////
//
// A box with a V notch along its top, 0.34 wide and 0.17 deep, under a
// third of the largest cell: the octree splits where its centres differ
// from their corners and keeps it, closed and facing out, with corners at
// its bottom and the volume of the box less the notch (32 - 0.115).  A
// uniform grid of the largest cell loses it.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceOctreeNotch(void)
{
	Ptr<CShapeSubtraction>	pNotched;
	Ptr<CMesh>				pMesh;
	SFloat32Box				sBox;
	SFloat32Box				sCut;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			asAxes[3];
	SFloat32Vec3			sPoint;
	int						iLeaves;
	int						i;

	sCentre.Init(0.02f, 0.01f, 0.03f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 2.0f, &asAxes[1], 2.0f, &asAxes[2], 1.0f);
	asAxes[1].Init(0, 0.70710678f, 0.70710678f);
	asAxes[2].Init(0, -0.70710678f, 0.70710678f);
	sCentre.Init(0, 0.05f, 1.03f);
	sCut.Init(&sCentre, &asAxes[0], 2.5f, &asAxes[1], 0.12f, &asAxes[2], 0.12f);
	pNotched = OMalloc<CShapeSubtraction>(OMalloc<CBox>(&sBox), OMalloc<CBox>(&sCut));

	pMesh = OctreeMesh(&pNotched, 0.0625f, 1.0f, 0.005f, &iLeaves);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pNotched));
	AssertTrue(fabsf(MeshVolume(&pMesh) - 31.8848f) < 0.01f);
	for (i = 0; i < 3; i++)
	{
		sPoint.Init(-1.0f + (float32)i, 0.05f, 1.03f - 0.1697f);
		AssertTrue(NearestMeshCorner(&pMesh, &sPoint) < 0.02f);
	}

	pMesh = DualContouringMesh(&pNotched, 1.0f);
	AssertTrue(MeshVolume(&pMesh) > 31.99f);
}


//////////////////////////////////////////////////////////////////////////
//
// A gyroid cut by a sphere winds through every size of cell: closed, so
// nothing cracks where sizes meet, and its own faces face out (its
// curves split cells small enough that none holds two pieces of it).
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceOctreeGyroid(void)
{
	Ptr<CShapeIntersection>	pShape;
	Ptr<CSphere>			pSphere;
	Ptr<CTestGyroid>		pGyroid;
	Ptr<CMesh>				pMesh;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			sNormal;
	SFloat32Vec3			sMiddle;
	SFloat32Vec3			sGradient;
	int						i;
	int						iLeaves;
	int						iGyroidFaces;
	int						iFacingIn;

	sCentre.Init(0.05f, -0.1f, 0.15f);
	pSphere = OMalloc<CSphere>(&sCentre, 3.0f);
	pGyroid = OMalloc<CTestGyroid>(2.0f);
	pShape = OMalloc<CShapeIntersection>();
	pShape->Add(pSphere);
	pShape->Add(pGyroid);
	pMesh = OctreeMesh(&pShape, 0.05f, 0.8f, 0.01f, &iLeaves);
	AssertTrue(pMesh->NumFaces() > 10000);
	AssertInt(0, CountMeshOpenEdges(&pMesh));

	iGyroidFaces = 0;
	iFacingIn = 0;
	for (i = 0; i < pMesh->NumFaces(); i++)
	{
		MeshFaceNormal(&pMesh, i, &sNormal, &sMiddle);
		if (pSphere->Distance(&sMiddle) < -0.2f)
		{
			iGyroidFaces++;
			pGyroid->Gradient(&sGradient, &sMiddle);
			if (Float32Vec3Dot(&sNormal, &sGradient) <= 0.0f)
			{
				iFacingIn++;
			}
		}
	}
	AssertTrue(iGyroidFaces > 10000);
	AssertInt(0, iFacingIn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceOctree(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestIsosurfaceOctreeSphere();
	TestIsosurfaceOctreeBox();
	TestIsosurfaceOctreeNotch();
	TestIsosurfaceOctreeGyroid();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

