#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshTorusEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/Torus.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"


//////////////////////////////////////////////////////////////////////////
//
// A sphere editor's mesh is closed, a ball, faces out of the sphere of
// the same values and is on it.  Flipped, every face faces in, and a
// sphere one bigger is one from every corner.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMeasuresSphere(void)
{
	Ptr<CMeshEditor>		pEditor;
	Ptr<CMeshSphereEditor>	pSphereEditor;
	Ptr<CSphere>			pSphere;
	Ptr<CMesh>				pMesh;
	SFloat32Vec3			sAxis;
	SFloat32Vec3			sCentre;

	sAxis.Init(0, 0, 1);
	sCentre.Init(1, -2, 3);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pSphereEditor = pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 2.0f, 16, 8);

	pMesh = pEditor->GetOutput();
	AssertTrue(pMesh->NumFaces() > 0);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pSphere));
	AssertInt(0, CountMeshFacesWithNoArea(&pMesh));
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pSphere), 4);

	pSphereEditor->SetFlipFaces(true);
	pEditor->mcModifiers.Touch();
	pMesh = pEditor->GetOutput();
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(pMesh->NumFaces(), CountMeshFacesFacingIn(&pMesh, &pSphere));

	pSphere = OMalloc<CSphere>(&sCentre, 3.0f);
	AssertFloat(1.0f, FurthestMeshCornerFromSurface(&pMesh, &pSphere), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A box editor's mesh and the box of the same values: its edges and
// corners are where three planes meet, as an isosurface's are to be.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMeasuresBox(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CBox>			pBox;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sXDirection;
	SFloat32Vec3		sYDirection;
	SFloat32Vec3		sZDirection;
	SFloat32Vec3		sStart;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Box			sBox;

	sXDirection.Init(2, 0, 0);
	sYDirection.Init(0, 4, 0);
	sZDirection.Init(0, 0, 6);
	sStart.Init(-1, -2, -3);
	sCentre.Init(0, 0, 0);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.0f, &asAxes[1], 2.0f, &asAxes[2], 3.0f);
	pBox = OMalloc<CBox>(&sBox);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 2);
	pMesh = pEditor->GetOutput();
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pBox));
	AssertInt(0, CountMeshFacesWithNoArea(&pMesh));
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pBox), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A cylinder's and a torus's editors' meshes and the shapes of the same
// values.  A torus is a ring, so its corners less edges plus faces is 0.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMeasuresRevolved(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CCylinder>		pCylinder;
	Ptr<CTorus>			pTorus;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sStart;

	sAxis.Init(0, 0, 1);
	sStart.Init(0, 0, -1);
	pCylinder = OMalloc<CCylinder>(&sAxis, &sStart, 2.0f, 5.0f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshCylinderEditor>(&sAxis, &sStart, 2.0f, 5.0f, 24, 2, 3);
	pMesh = pEditor->GetOutput();
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pCylinder));
	AssertInt(0, CountMeshFacesWithNoArea(&pMesh));
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pCylinder), 4);

	pTorus = OMalloc<CTorus>(&sAxis, &sStart, 3.0f, 1.0f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshTorusEditor>(&sAxis, &sStart, 3.0f, 1.0f, 24, 12);
	pMesh = pEditor->GetOutput();
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(0, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pTorus));
	AssertInt(0, CountMeshFacesWithNoArea(&pMesh));
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pTorus), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A plane is open all round its edge: 3 by 2 cells have 10 edges on it.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMeasuresOpen(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sXDirection;
	SFloat32Vec3		sYDirection;
	SFloat32Vec3		sStart;

	sXDirection.Init(3, 0, 0);
	sYDirection.Init(0, 2, 0);
	sStart.Init(0, 0, 0);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshPlaneEditor>(&sXDirection, &sYDirection, &sStart, 3, 2);
	pMesh = pEditor->GetOutput();
	AssertInt(10, CountMeshOpenEdges(&pMesh));
	AssertInt(1, MeshEulerCharacteristic(&pMesh));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMeasures(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshMeasuresSphere();
	TestMeshMeasuresBox();
	TestMeshMeasuresRevolved();
	TestMeshMeasuresOpen();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

