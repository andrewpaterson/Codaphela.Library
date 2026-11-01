#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshDiscEditor.h"
#include "SupportLib/MeshRingEditor.h"
#include "SupportLib/MeshConeEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


void AssertFace(CMeshFace* pcMeshFace, int iCorner1, int iCorner2, int iCorner3);
void AssertCornerCount(CMeshCorner* pcMeshCorner, int iEdgeCount, int iFaceCount);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlane(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sXDirection;
	SFloat32Vec3				sYDirection;
	SFloat32Vec3				sStart;
	CMeshPlaneEditor*	pcPlaneEditor;
	CMeshEditor			cMeshEditor;
	int					iNumCorners;
	int					iNumFaces;
	int					iNumPositions;
	CMeshPolygon*		pcPolygon;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sStart.Init(0, 0, 0);
	
	pcPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>();
	pcPlaneEditor->Init(&sXDirection, &sYDirection, &sStart, 2, 2);

	cMeshEditor.Touch();

	iNumCorners = cMesh.NumCorners();;
	iNumFaces = cMesh.NumFaces();
	iNumPositions = cMesh.TestNumPositions();;

	AssertInt(9, iNumCorners);
	AssertInt(8, iNumFaces);
	AssertInt(9, iNumPositions);
	AssertInt(4, cMeshEditor.NumPolygons());
	AssertInt(16, cMeshEditor.NumEdges());
	AssertInt(12, cMeshEditor.NumVisibleEdges(1));

	AssertFloat3(SFloat32Vec3(0.0f, 0, 0), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(0.5f, 0, 0), cMesh.GetPosition(1), 3);
	AssertFloat3(SFloat32Vec3(1.0f, 0, 0), cMesh.GetPosition(2), 3);

	AssertFloat3(SFloat32Vec3(0.0f, 0.5f, 0), cMesh.GetPosition(3), 3);
	AssertFloat3(SFloat32Vec3(0.5f, 0.5f, 0), cMesh.GetPosition(4), 3);
	AssertFloat3(SFloat32Vec3(1.0f, 0.5f, 0), cMesh.GetPosition(5), 3);

	AssertFloat3(SFloat32Vec3(0.0f, 1, 0), cMesh.GetPosition(6), 3);
	AssertFloat3(SFloat32Vec3(0.5f, 1, 0), cMesh.GetPosition(7), 3);
	AssertFloat3(SFloat32Vec3(1.0f, 1, 0), cMesh.GetPosition(8), 3);

	AssertInt(9, cMesh.NumCorners());
	AssertInt(8, cMesh.NumFaces());
	AssertInt(16, cMesh.NumEdges());

	AssertCornerCount(cMesh.GetCorner(0), 2, 1);
	AssertCornerCount(cMesh.GetCorner(1), 4, 3);
	AssertCornerCount(cMesh.GetCorner(2), 3, 2);
	AssertCornerCount(cMesh.GetCorner(3), 4, 3);
	AssertCornerCount(cMesh.GetCorner(4), 6, 6);
	AssertCornerCount(cMesh.GetCorner(5), 4, 3);
	AssertCornerCount(cMesh.GetCorner(6), 3, 2);
	AssertCornerCount(cMesh.GetCorner(7), 4, 3);
	AssertCornerCount(cMesh.GetCorner(8), 2, 1);

	AssertFace(cMesh.GetFace(0), 0, 1, 3);
	AssertFace(cMesh.GetFace(1), 1, 4, 3);
	AssertFace(cMesh.GetFace(2), 1, 2, 4);
	AssertFace(cMesh.GetFace(3), 2, 5, 4);
	AssertFace(cMesh.GetFace(4), 3, 4, 6);
	AssertFace(cMesh.GetFace(5), 4, 7, 6);
	AssertFace(cMesh.GetFace(6), 4, 5, 7);
	AssertFace(cMesh.GetFace(7), 5, 8, 7);

	pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(0);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(0, pcPolygon->maiFaces.GetValue(0));
	AssertInt(1, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(1);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(2, pcPolygon->maiFaces.GetValue(0));
	AssertInt(3, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(2);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(4, pcPolygon->maiFaces.GetValue(0));
	AssertInt(5, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mcPolygons.mcPolygons.Get(3);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(6, pcPolygon->maiFaces.GetValue(0));
	AssertInt(7, pcPolygon->maiFaces.GetValue(1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBox(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sXDirection;
	SFloat32Vec3				sYDirection;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshBoxEditor*		pcBoxEditor;
	CMeshEditor			cMeshEditor;
	int					iNumCorners;
	int					iNumFaces;
	int					iNumPositions;
	CFileUtil			cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 4, 5, 6);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box4x5x6.OBJ");

	//I don't know why I'm testing this.
	AssertInt(32, cMesh.CornerChunkSize());
	AssertInt(72, cMesh.FaceChunkSize());
	AssertInt(32, cMesh.TestPositionChunkSize());

	iNumCorners = cMesh.NumCorners();;
	iNumFaces = cMesh.NumFaces();
	iNumPositions = cMesh.TestNumPositions();;

	AssertInt(150, iNumCorners);
	AssertInt(296, iNumFaces);
	AssertInt(150, iNumPositions);
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box4x5x6.OBJ", "Output/Shape/Box4x5x6.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 2, 2);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box1x2x2.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x2x2.OBJ", "Output/Shape/Box1x2x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 2);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box2x2x2.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x2x2.OBJ", "Output/Shape/Box2x2x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 1, 2);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box2x1x2.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x1x2.OBJ", "Output/Shape/Box2x1x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 1);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box2x2x1.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x2x1.OBJ", "Output/Shape/Box2x2x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 1, 2);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box1x1x2.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x1x2.OBJ", "Output/Shape/Box1x1x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 2, 1);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box1x2x1.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x2x1.OBJ", "Output/Shape/Box1x2x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 1, 1);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Box2x1x1.OBJ");
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x1x1.OBJ", "Output/Shape/Box2x1x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 1, 1);

	cMeshEditor.Touch();

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(8, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	iNumCorners = cMesh.NumCorners();;
	iNumFaces = cMesh.NumFaces();
	iNumPositions = cMesh.TestNumPositions();;

	AssertInt(8, iNumCorners);
	AssertInt(12, iNumFaces);
	AssertInt(8, iNumPositions);
	AssertInt(6, cMeshEditor.mcPolygons.NumUniqueNames());

	WriteMesh(&cMesh, "Output/Shape/Box1x1x1.OBJ");
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x1x1.OBJ", "Output/Shape/Box1x1x1.OBJ");

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestDisc(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshDiscEditor*	pcDiscEditor;
	CMeshEditor			cMeshEditor;

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcDiscEditor = cMeshEditor.AddModifier<CMeshDiscEditor>();
	pcDiscEditor->Init(&sZDirection, &sStart, 0.0f, 1.0f, 3, 1);

	cMeshEditor.Touch();

	//I don't know why I'm testing this.
	AssertInt(2, cMesh.CornerChunkSize());
	AssertInt(2, cMesh.FaceChunkSize());
	AssertInt(2, cMesh.TestPositionChunkSize());

	AssertInt(4, cMesh.NumCorners());
	AssertInt(3, cMesh.NumFaces());
	AssertInt(4, cMesh.TestNumPositions());
	AssertInt(3, cMeshEditor.NumPolygons());
	AssertInt(6, cMeshEditor.NumVisibleEdges(1));

	cMesh.Kill();
	cMeshEditor.Kill();

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcDiscEditor = cMeshEditor.AddModifier<CMeshDiscEditor>();
	pcDiscEditor->Init(&sZDirection, &sStart, 0.0f, 1.0f, 3, 2);

	cMeshEditor.Touch();

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(8, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	AssertInt(7, cMesh.NumCorners());
	AssertInt(9, cMesh.NumFaces());
	AssertInt(7, cMesh.TestNumPositions());
	AssertInt(6, cMeshEditor.NumPolygons());
	AssertInt(12, cMeshEditor.NumVisibleEdges(1));

	cMesh.Kill();
	cMeshEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRing(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshRingEditor*	pcRingEditor;
	CMeshEditor			cMeshEditor;
	CFileUtil			cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcRingEditor = cMeshEditor.AddModifier<CMeshRingEditor>();
	pcRingEditor->Init(&sZDirection, &sStart, 0.5f, 1.0f, 1.0f, 3, 2);

	cMeshEditor.Touch();

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(4, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	AssertInt(6, cMesh.NumCorners());
	AssertInt(6, cMesh.NumFaces());
	AssertInt(6, cMesh.TestNumPositions());
	AssertInt(3, cMeshEditor.NumPolygons());
	AssertInt(9, cMeshEditor.NumVisibleEdges(1));

	cMesh.Kill();
	cMeshEditor.Kill();

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcRingEditor = cMeshEditor.AddModifier<CMeshRingEditor>();
	pcRingEditor->Init(&sZDirection, &sStart, 0.5f, 1.0f, 3.0f, 12, 8);

	cMeshEditor.Touch();

	//I don't know why I'm testing this.
	AssertInt(32, cMesh.CornerChunkSize());
	AssertInt(32, cMesh.FaceChunkSize());
	AssertInt(32, cMesh.TestPositionChunkSize());

	AssertInt(96, cMesh.NumCorners());
	AssertInt(168, cMesh.NumFaces());
	AssertInt(96, cMesh.TestNumPositions());
	AssertInt(84, cMeshEditor.NumPolygons());
	AssertInt(180, cMeshEditor.NumVisibleEdges(1));

	cMesh.Kill();
	cMeshEditor.Kill();

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCone(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshConeEditor*	pcConeEditor;
	CMeshEditor			cMeshEditor;
	CFileUtil			cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	pcConeEditor = cMeshEditor.AddModifier<CMeshConeEditor>();
	pcConeEditor->Init(&sZDirection, &sStart, 1.0f, 3.0f, 6.0f, 20, 3, 4, 11);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Cone20x3x4x11.OBJ");

	//I don't know why I'm testing this.
	AssertInt(72, cMesh.CornerChunkSize());
	AssertInt(108, cMesh.FaceChunkSize());
	AssertInt(72, cMesh.TestPositionChunkSize());

	AssertInt(322, cMesh.NumCorners());
	AssertInt(640, cMesh.NumFaces());
	AssertInt(322, cMesh.TestNumPositions());
	AssertInt(340, cMeshEditor.NumPolygons());
	AssertInt(3, cMeshEditor.mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Cone20x3x4x11.OBJ", "Output/Shape/Cone20x3x4x11.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pcConeEditor = cMeshEditor.AddModifier<CMeshConeEditor>();
	pcConeEditor->Init(&sZDirection, &sStart, 1.0f, 1.0f, 2.0f, 3, 1, 1, 2);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Cone3x1x1x2.OBJ");

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(8, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	AssertInt(8, cMesh.NumCorners());
	AssertInt(12, cMesh.NumFaces());
	AssertInt(8, cMesh.TestNumPositions());
	AssertInt(9, cMeshEditor.NumPolygons());
	AssertInt(3, cMeshEditor.mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Cone3x1x1x2.OBJ", "Output/Shape/Cone3x1x1x2.OBJ");

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCylinder(void)
{
	CMesh					cMesh;
	SFloat32Vec3					sZDirection;
	SFloat32Vec3					sStart;
	CMeshCylinderEditor*	pcCylinderEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	pcCylinderEditor = cMeshEditor.AddModifier<CMeshCylinderEditor>();
	pcCylinderEditor->Init(&sZDirection, &sStart, 1.0f, 2.0f, 8, 3, 4);

	cMeshEditor.Touch();
	WriteMesh(&cMesh, "Output/Shape/Cylinder8x3x4x11.OBJ");

	cMeshEditor.RemoveModifiers();
	cMesh.Kill();
	AssertFile("Input/Shape/Cylinder8x3x4x11.OBJ", "Output/Shape/Cylinder8x3x4x11.OBJ");
	cMeshEditor.Kill();

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshShapes(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestPlane();
	TestBox();
	TestDisc();
	TestRing();
	TestCone();
	TestCylinder();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFace(CMeshFace* pcMeshFace, int iCorner1, int iCorner2, int iCorner3)
{
	AssertInt(iCorner1, pcMeshFace->asCorner[0]);
	AssertInt(iCorner2, pcMeshFace->asCorner[1]);
	AssertInt(iCorner3, pcMeshFace->asCorner[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerCount(CMeshCorner* pcMeshCorner, int iEdgeCount, int iFaceCount)
{
	AssertSize(iEdgeCount, pcMeshCorner->aiEdge.NumElements());
	AssertSize(iFaceCount, pcMeshCorner->aiFace.NumElements());
}

