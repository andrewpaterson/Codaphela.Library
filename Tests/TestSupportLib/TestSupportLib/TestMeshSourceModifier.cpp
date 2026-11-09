#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshConeEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshDiscEditor.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/MeshRingEditor.h"
#include "SupportLib/MeshSmoothFaces.h"
#include "TestLib/Assert.h"


size CountMeshMemoryObjects(const char* szClassName);


enum ETestMeshShape
{
	TMS_Plane,
	TMS_Box,
	TMS_Disc,
	TMS_Ring,
	TMS_Cone,
	TMS_Cylinder,
	TMS_NumShapes
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshSourceAddShape(CMeshModifierStack* pcStack, ETestMeshShape eShape)
{
	SFloat32Vec3	sX;
	SFloat32Vec3	sY;
	SFloat32Vec3	sZ;
	SFloat32Vec3	sStart;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	switch (eShape)
	{
	case TMS_Plane:
		pcStack->AddModifier<CMeshPlaneEditor>()->Init(&sX, &sY, &sStart, 2, 2);
		break;
	case TMS_Box:
		pcStack->AddModifier<CMeshBoxEditor>()->Init(&sX, &sY, &sZ, &sStart, 2, 2, 2);
		break;
	case TMS_Disc:
		pcStack->AddModifier<CMeshDiscEditor>()->Init(&sZ, &sStart, 0.0f, 1.0f, 3, 1);
		break;
	case TMS_Ring:
		pcStack->AddModifier<CMeshRingEditor>()->Init(&sZ, &sStart, 1.0f, 1.0f, 1.0f, 4, 3);
		break;
	case TMS_Cone:
		pcStack->AddModifier<CMeshConeEditor>()->Init(&sZ, &sStart, 0.5f, 1.0f, 1.0f, 4, 1, 1, 3);
		break;
	case TMS_Cylinder:
		pcStack->AddModifier<CMeshCylinderEditor>()->Init(&sZ, &sStart, 1.0f, 1.0f, 4, 1, 3);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh of as many of a shape as are asked for, from an editor that applied
// its modifiers twice.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshSourceEditorMesh(ETestMeshShape eShape, int iCopies, int* piNumPolygons)
{
	Ptr<CMesh>	pMesh;
	CMeshEditor	cEditor;
	int			i;

	pMesh = OMalloc<CMesh>();
	cEditor.Init(pMesh);
	for (i = 0; i < iCopies; i++)
	{
		TestMeshSourceAddShape(&cEditor.mcModifiers, eShape);
	}
	cEditor.mcModifiers.ApplyAll(&cEditor);
	cEditor.mcModifiers.ApplyAll(&cEditor);
	*piNumPolygons = cEditor.NumPolygons();
	cEditor.Kill();
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// Two of a shape are exactly twice one, neither touching the other, so each
// shape numbers its corners from where the mesh has got to.  Applying them
// again gave the same mesh.
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshSourceEditorAddsShape(ETestMeshShape eShape)
{
	Ptr<CMesh>	pOne;
	Ptr<CMesh>	pTwo;
	int			iOnePolygons;
	int			iTwoPolygons;
	int			iNumCorners;
	int			iNumFaces;
	int			i;
	int			k;

	pOne = TestMeshSourceEditorMesh(eShape, 1, &iOnePolygons);
	pTwo = TestMeshSourceEditorMesh(eShape, 2, &iTwoPolygons);

	iNumCorners = pOne->NumCorners();
	iNumFaces = pOne->NumFaces();
	AssertTrue(iNumFaces > 0);
	AssertInt(iNumCorners * 2, pTwo->NumCorners());
	AssertInt(iNumFaces * 2, pTwo->NumFaces());
	AssertInt(pOne->NumEdges() * 2, pTwo->NumEdges());
	AssertInt(iOnePolygons * 2, iTwoPolygons);

	//The second's faces are the first's, moved along.
	for (i = 0; i < iNumFaces; i++)
	{
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			AssertInt(pOne->GetFaceCorners(i)[k] + iNumCorners, pTwo->GetFaceCorners(iNumFaces + i)[k]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceEditorAddsShapes(void)
{
	int	iShape;

	for (iShape = 0; iShape < TMS_NumShapes; iShape++)
	{
		TestMeshSourceEditorAddsShape((ETestMeshShape)iShape);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A shape is added to what a mesh already has, however many times it is
// applied, and clearing the editor means starting again from nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceEditorAddsToMesh(void)
{
	CMesh		cMesh;
	CMeshEditor	cEditor;
	int			i;

	cMesh.Init();
	cMesh.AddCorner(0, 0, 0);
	cEditor.Init(&cMesh);

	cEditor.mcModifiers.ApplyAll(&cEditor);
	AssertInt(1, cMesh.NumCorners());

	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Disc);
	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Plane);
	for (i = 0; i < 2; i++)
	{
		cEditor.mcModifiers.ApplyAll(&cEditor);
		AssertInt(1 + 4 + 9, cMesh.NumCorners());
		AssertInt(3 + 8, cMesh.NumFaces());
		AssertInt(1, cMesh.GetFaceCorners(0)[0]);
	}

	cEditor.ClearConnectivity();
	AssertInt(0, cMesh.NumCorners());
	cEditor.mcModifiers.ApplyAll(&cEditor);
	AssertInt(4 + 9, cMesh.NumCorners());
	cEditor.mcModifiers.ApplyAll(&cEditor);
	AssertInt(4 + 9, cMesh.NumCorners());
	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A shape added to a collapsed mesh leaves its polygons, names and visible
// edges as they were, and collapsing again makes that the next base.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceEditorAddsToCollapsed(void)
{
	CMesh				cMesh;
	CMeshEditor			cEditor;
	CMeshPlaneEditor*	pcPlane;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sStart;
	int					iVisibleEdges;
	int					i;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cEditor.Init(&cMesh);
	pcPlane = cEditor.AddModifier<CMeshPlaneEditor>();
	pcPlane->Init(&sX, &sY, &sStart, 2, 2);
	pcPlane->SetFaceName(3);
	cEditor.Collapse();

	AssertInt(4, cEditor.NumPolygons());
	iVisibleEdges = cEditor.NumVisibleEdges(1);
	AssertInt(12, iVisibleEdges);

	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Disc);
	for (i = 0; i < 2; i++)
	{
		cEditor.mcModifiers.ApplyAll(&cEditor);
		AssertInt(9 + 4, cMesh.NumCorners());
		AssertInt(8 + 3, cMesh.NumFaces());
		AssertInt(4 + 3, cEditor.NumPolygons());
		AssertInt(iVisibleEdges + 6, cEditor.NumVisibleEdges(1));
		AssertInt(3, cMesh.GetFaceName(7));
		AssertInt(0, cMesh.GetFaceName(8));
		AssertInt(9, cMesh.GetFaceCorners(8)[0]);
	}
	cEditor.Collapse();

	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Disc);
	cEditor.mcModifiers.ApplyAll(&cEditor);
	cEditor.mcModifiers.ApplyAll(&cEditor);
	AssertInt(9 + 4 + 4, cMesh.NumCorners());
	AssertInt(4 + 3 + 3, cEditor.NumPolygons());
	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A smoothed mesh keeps its corner normals under a new shape, and one with
// materials keeps what it had too.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceEditorKeepsNormals(void)
{
	CMesh				cMesh;
	CMeshEditor			cEditor;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sStart;
	SMeshNormalFace*	psNormalFace;
	int					i;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshPlaneEditor>()->Init(&sX, &sY, &sStart, 2, 2);
	cEditor.AddModifier<CMeshSmoothFaces>()->Init();
	cEditor.Collapse();
	AssertSize(8 + 9, cMesh.mcNormals.mcNormals.NumElements());

	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Disc);
	for (i = 0; i < 2; i++)
	{
		cEditor.mcModifiers.ApplyAll(&cEditor);
		AssertSize(8 + 3 + 9, cMesh.mcNormals.mcNormals.NumElements());
		psNormalFace = cMesh.mcNormals.mcFaces.Get(0);
		AssertTrue(psNormalFace->aiCornerNormals[0] >= 8 + 3);
		AssertFloat(1.0f, cMesh.mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[0])->z, 3);
		AssertInt(-1, cMesh.mcNormals.mcFaces.Get(8)->aiCornerNormals[0]);
	}
	cEditor.Kill();

	cMesh.ClearConnectivity();
	cMesh.AddCorner(0, 0, 0);
	cMesh.mcMaterials.mbInUse = true;
	cEditor.Init(&cMesh);
	TestMeshSourceAddShape(&cEditor.mcModifiers, TMS_Plane);
	cEditor.mcModifiers.ApplyAll(&cEditor);
	AssertInt(1 + 9, cMesh.NumCorners());
	AssertTrue(cMesh.mcMaterials.InUse());
	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A stack with no source starts from an empty mesh and adds each shape to a
// new one, leaving each mesh it was given as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceStackAddsShapes(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pOutput;
	Ptr<CMesh>			pFlattened;

	cStack.Init();
	TestMeshSourceAddShape(&cStack, TMS_Plane);
	TestMeshSourceAddShape(&cStack, TMS_Disc);

	pOutput = cStack.GetOutput();
	AssertInt(9 + 4, pOutput->NumCorners());
	AssertInt(8 + 3, pOutput->NumFaces());
	AssertInt(0, cStack.GetSource()->NumCorners());
	AssertInt(9, pOutput->GetFaceCorners(8)[0]);
	AssertSize(2, CountMeshMemoryObjects("CMesh"));  //The empty source and the output: what was between has gone.

	cStack.Touch();
	pOutput = cStack.GetOutput();
	AssertInt(9 + 4, pOutput->NumCorners());
	AssertInt(8 + 3, pOutput->NumFaces());

	cStack.Flatten();
	pFlattened = cStack.GetSource();
	TestMeshSourceAddShape(&cStack, TMS_Disc);
	pOutput = cStack.GetOutput();
	AssertInt(9 + 4 + 4, pOutput->NumCorners());
	AssertInt(9 + 4, pFlattened->NumCorners());

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a mesh has stays on its own faces when a shape is added after them.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceStackKeepsDetails(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;
	int					iNormal;
	SLogConfig			sLogConfig;

	pSource = OMalloc<CMesh>();
	pSource->mcNormals.mbInUse = true;
	pSource->mcColours.mbInUse = true;
	pSource->mcColours.mcColours.Add(0xFF112233);
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(1, 0, 0);
	pSource->AddCorner(0, 1, 0);
	pSource->AddFace(0, 1, 2);
	pSource->SetFaceName(0, 7);
	pSource->AddNormal(0, 0.6f, 0.8f);
	psNormalFace = pSource->mcNormals.mcFaces.Get(0);
	psNormalFace->aiCornerNormals[0] = 1;
	psNormalFace->aiCornerNormals[1] = 1;
	pSource->mcColours.mcFaces.Get(0)->aiCornerColours[2] = 0;

	cStack.Init(pSource);
	TestMeshSourceAddShape(&cStack, TMS_Disc);
	pOutput = cStack.GetOutput();

	AssertInt(3 + 4, pOutput->NumCorners());
	AssertInt(1 + 3, pOutput->NumFaces());
	AssertInt(3, pOutput->GetFaceCorners(1)[0]);
	AssertInt(7, pOutput->GetFaceName(0));

	psNormalFace = pOutput->mcNormals.mcFaces.Get(0);
	iNormal = psNormalFace->aiCornerNormals[0];
	AssertInt(4, iNormal);
	AssertInt(4, psNormalFace->aiCornerNormals[1]);
	AssertInt(-1, psNormalFace->aiCornerNormals[2]);
	AssertFloat(0.8f, pOutput->mcNormals.mcNormals.Get(iNormal)->z, 3);
	AssertInt(-1, pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);

	AssertInt(0, pOutput->mcColours.mcFaces.Get(0)->aiCornerColours[2]);
	AssertInt(-1, pOutput->mcColours.mcFaces.Get(1)->aiCornerColours[0]);
	AssertInt((int)0xFF112233, (int)pOutput->mcColours.mcColours.GetValue(0));
	cStack.Kill();

	//What a new mesh wouldn't keep leaves the mesh as it is.
	pSource = OMalloc<CMesh>();
	pSource->mcMaterials.mbInUse = true;
	cStack.Init(pSource);
	TestMeshSourceAddShape(&cStack, TMS_Disc);
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(cStack.GetOutput() == pSource);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSourceModifier(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshSourceEditorAddsShapes();
	TestMeshSourceEditorAddsToMesh();
	TestMeshSourceEditorAddsToCollapsed();
	TestMeshSourceEditorKeepsNormals();
	TestMeshSourceStackAddsShapes();
	TestMeshSourceStackKeepsDetails();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
