#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/HalfSpace.h"
#include "SupportLib/ShapeUnion.h"
#include "ProceduralLib/MeshShapeEditor.h"
#include "TestLib/Assert.h"
#include "MeshMeasures.h"


//////////////////////////////////////////////////////////////////////////
//
// How many objects of a class are in memory.
//
//////////////////////////////////////////////////////////////////////////
static size TestMeshShapeEditorCount(const char* szClassName)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CPointer			p;
	size				uiCount;

	uiCount = 0;
	oi = gcObjects.StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		p = gcObjects.TestGetFromMemory(oi);
		if (StringCompare(p->ClassName(), szClassName) == 0)
		{
			uiCount++;
		}
		oi = gcObjects.IterateMemory(&sIter);
	}
	p = NULL;
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere is meshed by its own editor, exactly: the same corners and
// faces as a CMeshSphereEditor of its values.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeEditorPrimitive(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshEditor>	pDirect;
	Ptr<CMesh>			pMesh;
	Ptr<CMesh>			pExpected;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		sZDirection;
	bool				bSame;
	int					i;

	sCentre.Init(1, -2, 3);
	sZDirection.Init(0, 0, 1);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(OMalloc<CSphere>(&sCentre, 2.0f), 0.1f, 1.0f);
	pMesh = pEditor->GetOutput();

	pDirect = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pDirect->AddModifier<CMeshSphereEditor>(&sZDirection, &sCentre, 2.0f, MESH_SHAPE_SEGMENTS, MESH_SHAPE_SEGMENTS / 2);
	pExpected = pDirect->GetOutput();

	AssertInt(pExpected->NumCorners(), pMesh->NumCorners());
	AssertInt(pExpected->NumFaces(), pMesh->NumFaces());
	bSame = true;
	for (i = 0; i < pMesh->NumCorners(); i++)
	{
		bSame &= (*pMesh->GetPosition(i) == *pExpected->GetPosition(i));
	}
	AssertTrue(bSame);
}


//////////////////////////////////////////////////////////////////////////
//
// A cylinder pushed through a box is an isosurface: closed and facing
// out, but for slivers under a hundredth of a smallest cell's face (two
// at the cylinder's rim, where two leaves fit vertices on the crease a
// thousandth apart), and with flat normals merged after it, normals in
// use.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeEditorIsosurface(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CShapeUnion>	pUnion;
	Ptr<CMesh>			pMesh;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sStart;

	sCentre.Init(0.03f, 0.04f, -0.6f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.5f, &asAxes[1], 1.5f, &asAxes[2], 0.8f);
	sAxis.Init(0, 0, 1);
	sStart.Init(0.1f, 0.05f, -0.5f);
	pUnion = OMalloc<CShapeUnion>();
	pUnion->Add(OMalloc<CBox>(&sBox));
	pUnion->Add(OMalloc<CCylinder>(&sAxis, &sStart, 0.7f, 1.6f));

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pUnion, 0.0625f, 0.5f);
	pMesh = pEditor->GetOutput();
	AssertTrue(pMesh->NumFaces() > 1000);
	AssertInt(0, CountMeshOpenEdges(&pMesh));
	AssertInt(2, MeshEulerCharacteristic(&pMesh));
	AssertInt(0, CountMeshFacesFacingIn(&pMesh, &pUnion, 0.0625f * 0.0625f * 0.01f));
	AssertTrue(FurthestMeshCornerFromSurface(&pMesh, &pUnion) < 0.02f);

	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(MESH_SHAPE_SPLIT_ANGLE);
	pMesh = pEditor->GetOutput();
	AssertTrue(pMesh->mcNormals.InUse());
	AssertInt(0, CountMeshOpenEdges(&pMesh));
}


//////////////////////////////////////////////////////////////////////////
//
// The modifier holds its shape: a change to the shape and a Touch mesh it
// again, and the shape goes when the editor does.  A half space, with no
// editor and endless bounds, makes nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeEditorHoldsShape(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CSphere>		pSphere;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		sNormal;
	SFloat32Plane		sPlane;

	sCentre.Init(1, -2, 3);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pSphere, 0.1f, 1.0f);
	pMesh = pEditor->GetOutput();
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pSphere), 4);

	pSphere->msSphere.fRadius = 3.0f;
	pEditor->mcModifiers.Touch();
	pMesh = pEditor->GetOutput();
	AssertFloat(0.0f, FurthestMeshCornerFromSurface(&pMesh, &pSphere), 4);

	pMesh = NULL;
	pSphere = NULL;
	AssertSize(1, TestMeshShapeEditorCount("CSphere"));
	pEditor = NULL;
	AssertSize(0, TestMeshShapeEditorCount("CSphere"));
	AssertSize(0, TestMeshShapeEditorCount("CMeshShapeEditor"));

	sNormal.Init(0, 0, 1);
	sPlane.Init(&sNormal, &sCentre);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(OMalloc<CHalfSpace>(&sPlane), 0.1f, 1.0f);
	AssertInt(0, pEditor->GetOutput()->NumFaces());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapeEditor(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshShapeEditorPrimitive();
	ObjectsFlush();
	TestMeshShapeEditorIsosurface();
	ObjectsFlush();
	TestMeshShapeEditorHoldsShape();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

