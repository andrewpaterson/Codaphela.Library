#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/FloatHelper.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshDiscEditor.h"
#include "SupportLib/MeshRingEditor.h"
#include "SupportLib/MeshConeEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshTubeEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshGeosphereEditor.h"
#include "SupportLib/MeshTorusEditor.h"
#include "SupportLib/MeshWriter.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TextParser.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


void AssertFace(CMeshFace* pcMeshFace, int iCorner1, int iCorner2, int iCorner3);
void AssertCornerCount(CMeshCorner* pcMeshCorner, int iEdgeCount, int iFaceCount);


//A written position is rounded to 1 / 65536 (BINARY_PRECISION).  One that came from a sine can land on either side of a
//rounding on different builds.
#define OBJ_SHAPE_POSITION_TOLERANCE	(2.0f / 65536.0f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertOBJPositionLine(CChars* pszExpected, CChars* pszActual)
{
	CTextParser	cExpected;
	CTextParser	cActual;
	float32		fExpected;
	float32		fActual;
	int			i;

	cExpected.Init(pszExpected->Text(), pszExpected->Length());
	cActual.Init(pszActual->Text(), pszActual->Length());
	AssertTritrue(cExpected.GetExactCharacter('v'));
	AssertTritrue(cActual.GetExactCharacter('v'));
	for (i = 0; i < 3; i++)
	{
		AssertTritrue(cExpected.GetFloat(&fExpected));
		AssertTritrue(cActual.GetFloat(&fActual));
		AssertTrue(fabsf(fExpected - fActual) <= OBJ_SHAPE_POSITION_TOLERANCE);
	}
	cActual.Kill();
	cExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The same as AssertFile for a shape made with sines and cosines, except
// that its positions can differ by what they are rounded to.
//
//////////////////////////////////////////////////////////////////////////
static void AssertOBJShape(const char* szExpected, const char* szActual)
{
	CTextFile	cExpected;
	CTextFile	cActual;
	CArrayChars	aszExpected;
	CArrayChars	aszActual;
	CChars*		pszExpected;
	CChars*		pszActual;
	size		i;
	size		uiNumLines;

	cExpected.Init();
	cActual.Init();
	AssertTrue(cExpected.Read(szExpected));
	AssertTrue(cActual.Read(szActual));
	cExpected.PassifyNewlines();
	cActual.PassifyNewlines();

	aszExpected.Fake();
	aszActual.Fake();
	cExpected.mcText.SplitLines(&aszExpected);
	cActual.mcText.SplitLines(&aszActual);

	uiNumLines = aszExpected.NumElements();
	AssertSize(uiNumLines, aszActual.NumElements());
	for (i = 0; i < uiNumLines; i++)
	{
		pszExpected = aszExpected.Get(i);
		pszActual = aszActual.Get(i);
		if (strncmp(pszExpected->Text(), "v ", 2) == 0)
		{
			AssertOBJPositionLine(pszExpected, pszActual);
		}
		else
		{
			//The lines are in the file's text, so they aren't terminated.
			AssertSize(pszExpected->Length(), pszActual->Length());
			AssertTrue(memcmp(pszExpected->Text(), pszActual->Text(), pszExpected->Length()) == 0);
		}
	}

	aszActual.Kill();
	aszExpected.Kill();
	cActual.Kill();
	cExpected.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A face's normal, as CMeshNormals makes it, and the middle of its corners.
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeFaceNormal(CMesh* pcMesh, int iFace, SFloat32Vec3* psNormal, SFloat32Vec3* psMiddle)
{
	int*			piCorners;
	SFloat32Vec3	sVec1;
	SFloat32Vec3	sVec2;

	piCorners = pcMesh->GetFaceCorners(iFace);
	sVec1 = *pcMesh->GetPosition(piCorners[0]) - *pcMesh->GetPosition(piCorners[1]);
	sVec2 = *pcMesh->GetPosition(piCorners[2]) - *pcMesh->GetPosition(piCorners[1]);
	Float32Vec3Cross(psNormal, &sVec2, &sVec1);

	*psMiddle = *pcMesh->GetPosition(piCorners[0]) + *pcMesh->GetPosition(piCorners[1]);
	psMiddle->Add(pcMesh->GetPosition(piCorners[2]));
	*psMiddle /= 3.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// Every edge is on two faces, so the shape has no holes, and corners less
// edges plus faces is iEulerCharacteristic: 2 for a ball, 0 for a ring.
//
//////////////////////////////////////////////////////////////////////////
static void AssertShapeClosed(CMesh* pcMesh, int iEulerCharacteristic)
{
	int			i;
	int			iNumEdges;
	CMeshEdge*	pcEdge;
	bool		bClosed;

	bClosed = true;
	iNumEdges = pcMesh->NumEdges();
	for (i = 0; i < iNumEdges; i++)
	{
		pcEdge = pcMesh->GetEdge(i);
		if ((pcEdge->maiFace[0] == -1) || (pcEdge->maiFace[1] == -1))
		{
			bClosed = false;
		}
	}
	AssertTrue(bClosed);
	AssertInt(iEulerCharacteristic, pcMesh->NumCorners() - iNumEdges + pcMesh->NumFaces());
}


//////////////////////////////////////////////////////////////////////////
//
// Every corner is fRadius from psCentre, and every face faces away from it
// (or towards it if bInward).
//
//////////////////////////////////////////////////////////////////////////
static void AssertBallShape(CMesh* pcMesh, SFloat32Vec3* psCentre, float32 fRadius, bool bInward)
{
	int				i;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;
	SFloat32Vec3	sOut;
	bool			bOnSurface;
	bool			bFacing;

	bOnSurface = true;
	for (i = 0; i < pcMesh->NumCorners(); i++)
	{
		sOut = *pcMesh->GetPosition(i) - *psCentre;
		if (fabsf(sOut.Magnitude() - fRadius) > 0.0001f)
		{
			bOnSurface = false;
		}
	}
	AssertTrue(bOnSurface);

	bFacing = true;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		TestShapeFaceNormal(pcMesh, i, &sNormal, &sMiddle);
		sOut = sMiddle - *psCentre;
		if ((Float32Vec3Dot(&sNormal, &sOut) > 0.0f) == bInward)
		{
			bFacing = false;
		}
	}
	AssertTrue(bFacing);
}


//////////////////////////////////////////////////////////////////////////
//
// How far psPosition is along psZDirection (which is normalised) from the
// origin, and what is left across it.
//
//////////////////////////////////////////////////////////////////////////
static void SplitAlongAxis(SFloat32Vec3* psPosition, SFloat32Vec3* psZDirection, float32* pfAlong, SFloat32Vec3* psAcross)
{
	SFloat32Vec3	sAlong;

	*pfAlong = Float32Vec3Dot(psPosition, psZDirection);
	sAlong.Init(psZDirection);
	sAlong *= *pfAlong;
	*psAcross = *psPosition - sAlong;
}


//////////////////////////////////////////////////////////////////////////
//
// Each face of a shape around psZDirection through the origin, from 0 to
// fLength, faces the way its name says: up (1), out (2), down (3) or in (4).
//
//////////////////////////////////////////////////////////////////////////
static void AssertCylindricalFacesOutward(CMesh* pcMesh, SFloat32Vec3* psZDirection, float32 fLength)
{
	int				i;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sMiddle;
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sOut;
	float32			fAlong;
	bool			bFacing;
	int				iName;

	bFacing = true;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		TestShapeFaceNormal(pcMesh, i, &sNormal, &sMiddle);
		SplitAlongAxis(&sMiddle, psZDirection, &fAlong, &sAcross);
		iName = pcMesh->GetFaceName(i);
		if (iName == 1)
		{
			sOut.Init(psZDirection);
			bFacing &= fabsf(fAlong - fLength) < 0.0001f;
		}
		else if (iName == 3)
		{
			sOut.Init(psZDirection);
			sOut *= -1.0f;
			bFacing &= fabsf(fAlong) < 0.0001f;
		}
		else if (iName == 2)
		{
			sOut = sAcross;
		}
		else
		{
			sOut = sAcross;
			sOut *= -1.0f;
		}
		bFacing &= Float32Vec3Dot(&sNormal, &sOut) > 0.0f;
	}
	AssertTrue(bFacing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CountFacesNamed(CMesh* pcMesh, int iName)
{
	int	i;
	int	iCount;

	iCount = 0;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		if (pcMesh->GetFaceName(i) == iName)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlane(void)
{
	CMesh					cMesh;
	SFloat32Vec3			sXDirection;
	SFloat32Vec3			sYDirection;
	SFloat32Vec3			sStart;
	Ptr<CMeshPlaneEditor>	pPlaneEditor;
	CMeshEditor				cMeshEditor;
	int						iNumCorners;
	int						iNumFaces;
	int						iNumPositions;
	CMeshPolygon*			pcPolygon;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sStart.Init(0, 0, 0);
	
	pPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>(&sXDirection, &sYDirection, &sStart, 2, 2);

	cMeshEditor.Collapse();

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

	pcPolygon = cMeshEditor.mpMesh->mcPolygons.mcPolygons.Get(0);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(0, pcPolygon->maiFaces.GetValue(0));
	AssertInt(1, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mpMesh->mcPolygons.mcPolygons.Get(1);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(2, pcPolygon->maiFaces.GetValue(0));
	AssertInt(3, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mpMesh->mcPolygons.mcPolygons.Get(2);
	AssertInt(0, pcPolygon->miName);
	AssertSize(2, pcPolygon->maiFaces.NumElements());
	AssertInt(4, pcPolygon->maiFaces.GetValue(0));
	AssertInt(5, pcPolygon->maiFaces.GetValue(1));

	pcPolygon = cMeshEditor.mpMesh->mcPolygons.mcPolygons.Get(3);
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
	SFloat32Vec3		sXDirection;
	SFloat32Vec3		sYDirection;
	SFloat32Vec3		sZDirection;
	SFloat32Vec3		sStart;
	Ptr<CMeshBoxEditor>	pBoxEditor;
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

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 4, 5, 6);

	cMeshEditor.Collapse();
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
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box4x5x6.OBJ", "Output/Shape/Box4x5x6.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 2, 2);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box1x2x2.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x2x2.OBJ", "Output/Shape/Box1x2x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 2);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box2x2x2.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x2x2.OBJ", "Output/Shape/Box2x2x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 1, 2);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box2x1x2.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x1x2.OBJ", "Output/Shape/Box2x1x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 1);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box2x2x1.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x2x1.OBJ", "Output/Shape/Box2x2x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 1, 2);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box1x1x2.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x1x2.OBJ", "Output/Shape/Box1x1x2.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 2, 1);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box1x2x1.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box1x2x1.OBJ", "Output/Shape/Box1x2x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 1, 1);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Box2x1x1.OBJ");
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertFile("Input/Shape/Box2x1x1.OBJ", "Output/Shape/Box2x1x1.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>(&sXDirection, &sYDirection, &sZDirection, &sStart, 1, 1, 1);

	cMeshEditor.Collapse();

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
	AssertInt(6, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());

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
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sStart;
	Ptr<CMeshDiscEditor>	pDiscEditor;
	CMeshEditor				cMeshEditor;

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pDiscEditor = cMeshEditor.AddModifier<CMeshDiscEditor>(&sZDirection, &sStart, 0.0f, 1.0f, 3, 1);

	cMeshEditor.Collapse();

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

	pDiscEditor = cMeshEditor.AddModifier<CMeshDiscEditor>(&sZDirection, &sStart, 0.0f, 1.0f, 3, 2);

	cMeshEditor.Collapse();

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
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sStart;
	Ptr<CMeshRingEditor>	pRingEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pRingEditor = cMeshEditor.AddModifier<CMeshRingEditor>(&sZDirection, &sStart, 0.5f, 1.0f, 1.0f, 3, 2);

	cMeshEditor.Collapse();

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(4, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	AssertInt(6, cMesh.NumCorners());
	AssertInt(6, cMesh.NumFaces());
	AssertInt(6, cMesh.TestNumPositions());
	AssertInt(3, cMeshEditor.NumPolygons());
	AssertInt(9, cMeshEditor.NumVisibleEdges(1));

	//The bottom radius is at the start and the top radius is at the end.
	AssertFloat3(SFloat32Vec3(0.0f, 1.0f, 0.0f), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(0.0f, 0.5f, 1.0f), cMesh.GetPosition(3), 3);

	cMesh.Kill();
	cMeshEditor.Kill();

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pRingEditor = cMeshEditor.AddModifier<CMeshRingEditor>(&sZDirection, &sStart, 0.5f, 1.0f, 3.0f, 12, 8);

	cMeshEditor.Collapse();

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
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sStart;
	Ptr<CMeshConeEditor>	pConeEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	pConeEditor = cMeshEditor.AddModifier<CMeshConeEditor>(&sZDirection, &sStart, 1.0f, 3.0f, 6.0f, 20, 3, 4, 11);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Cone20x3x4x11.OBJ");

	//I don't know why I'm testing this.
	AssertInt(72, cMesh.CornerChunkSize());
	AssertInt(108, cMesh.FaceChunkSize());
	AssertInt(72, cMesh.TestPositionChunkSize());

	AssertInt(322, cMesh.NumCorners());
	AssertInt(640, cMesh.NumFaces());
	AssertInt(322, cMesh.TestNumPositions());
	AssertInt(340, cMeshEditor.NumPolygons());
	AssertInt(3, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Cone20x3x4x11.OBJ", "Output/Shape/Cone20x3x4x11.OBJ");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	pConeEditor = cMeshEditor.AddModifier<CMeshConeEditor>(&sZDirection, &sStart, 1.0f, 1.0f, 2.0f, 3, 1, 1, 2);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Cone3x1x1x2.OBJ");

	//I don't know why I'm testing this.
	AssertInt(4, cMesh.CornerChunkSize());
	AssertInt(8, cMesh.FaceChunkSize());
	AssertInt(4, cMesh.TestPositionChunkSize());

	AssertInt(8, cMesh.NumCorners());
	AssertInt(12, cMesh.NumFaces());
	AssertInt(8, cMesh.TestNumPositions());
	AssertInt(9, cMeshEditor.NumPolygons());
	AssertInt(3, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Cone3x1x1x2.OBJ", "Output/Shape/Cone3x1x1x2.OBJ");

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCylinder(void)
{
	CMesh						cMesh;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	Ptr<CMeshCylinderEditor>	pCylinderEditor;
	CMeshEditor					cMeshEditor;
	CFileUtil					cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	pCylinderEditor = cMeshEditor.AddModifier<CMeshCylinderEditor>(&sZDirection, &sStart, 1.0f, 2.0f, 8, 3, 4);

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Cylinder8x3x4x11.OBJ");

	//The checks the shapes below are put to hold for a cylinder already.
	AssertShapeClosed(&cMesh, 2);
	AssertCylindricalFacesOutward(&cMesh, &sZDirection, 2.0f);

	cMeshEditor.RemoveModifiers();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Cylinder8x3x4x11.OBJ", "Output/Shape/Cylinder8x3x4x11.OBJ");
	cMeshEditor.Kill();

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
// The disc, ring and cone (and so the cylinder) are the same shape whichever
// way their axis points: along Z, along Y, along -X or between Y and Z.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeAxes(void)
{
	CMesh			cMesh;
	CMeshEditor		cMeshEditor;
	SFloat32Vec3	asZDirection[4];
	SFloat32Vec3	sStart;
	SFloat32Vec3	sAcross;
	SFloat32Vec3*	psZDirection;
	float32			fAlong;
	float32			fRadius;
	float32			fMaxRadius;
	bool			bFlat;
	bool			bOnRing;
	int				i;
	int				j;

	asZDirection[0].Init(0, 0, 1);
	asZDirection[1].Init(0, 1, 0);
	asZDirection[2].Init(-1, 0, 0);
	asZDirection[3].Init(0, 1, 1);
	asZDirection[3].Normalize();
	sStart.Init(0, 0, 0);

	for (i = 0; i < 4; i++)
	{
		psZDirection = &asZDirection[i];

		//A disc lies across its axis, out to its radius.
		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		cMeshEditor.AddModifier<CMeshDiscEditor>(psZDirection, &sStart, 0.0f, 2.0f, 8, 3);
		cMeshEditor.Collapse();

		AssertInt(25, cMesh.NumCorners());
		bFlat = true;
		fMaxRadius = 0.0f;
		for (j = 0; j < cMesh.NumCorners(); j++)
		{
			SplitAlongAxis(cMesh.GetPosition(j), psZDirection, &fAlong, &sAcross);
			bFlat &= fabsf(fAlong) < 0.0001f;
			fRadius = sAcross.Magnitude();
			if (fRadius > fMaxRadius)
			{
				fMaxRadius = fRadius;
			}
		}
		AssertTrue(bFlat);
		AssertFloat(2.0f, fMaxRadius, 3);

		cMesh.Kill();
		cMeshEditor.Kill();

		//A ring goes from its bottom radius at the start to its top radius at the end.
		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		cMeshEditor.AddModifier<CMeshRingEditor>(psZDirection, &sStart, 0.5f, 1.0f, 3.0f, 12, 8);
		cMeshEditor.Collapse();

		AssertInt(96, cMesh.NumCorners());
		bOnRing = true;
		for (j = 0; j < cMesh.NumCorners(); j++)
		{
			SplitAlongAxis(cMesh.GetPosition(j), psZDirection, &fAlong, &sAcross);
			bOnRing &= (fAlong > -0.0001f) && (fAlong < 3.0001f);
			bOnRing &= fabsf(sAcross.Magnitude() - InterpolateFloat(1.0f, 0.5f, fAlong / 3.0f)) < 0.0001f;
		}
		AssertTrue(bOnRing);

		cMesh.Kill();
		cMeshEditor.Kill();

		//A cone is closed and faces out, with its ends across its axis.
		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		cMeshEditor.AddModifier<CMeshConeEditor>(psZDirection, &sStart, 1.0f, 3.0f, 6.0f, 20, 3, 4, 11);
		cMeshEditor.Collapse();

		AssertInt(322, cMesh.NumCorners());
		AssertInt(640, cMesh.NumFaces());
		AssertShapeClosed(&cMesh, 2);
		AssertCylindricalFacesOutward(&cMesh, psZDirection, 6.0f);

		cMesh.Kill();
		cMeshEditor.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSphere(void)
{
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sCentre;
	Ptr<CMeshSphereEditor>	pSphereEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sCentre.Init(1, 2, 3);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pSphereEditor = cMeshEditor.AddModifier<CMeshSphereEditor>(&sZDirection, &sCentre, 2.0f, 8, 4);
	AssertInt(26, pSphereEditor->ExpectedCornerChunkSize());
	AssertInt(48, pSphereEditor->ExpectedFaceChunkSize());

	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Sphere8x4.OBJ");

	AssertInt(26, cMesh.NumCorners());
	AssertInt(48, cMesh.NumFaces());
	AssertInt(72, cMesh.NumEdges());
	AssertInt(16 + 16, cMeshEditor.NumPolygons());
	AssertInt(1, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	AssertFloat3(SFloat32Vec3(1, 2, 1), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(1, 2 + sqrtf(2.0f), 3 - sqrtf(2.0f)), cMesh.GetPosition(1), 3);
	AssertFloat3(SFloat32Vec3(1, 2, 5), cMesh.GetPosition(25), 3);
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 2.0f, false);

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Sphere8x4.OBJ", "Output/Shape/Sphere8x4.OBJ");

	//The fewest bands: two cones of three faces, base to base.
	sCentre.Init(0, 0, 0);
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	cMeshEditor.AddModifier<CMeshSphereEditor>(&sZDirection, &sCentre, 1.0f, 3, 2);
	cMeshEditor.Collapse();
	AssertInt(5, cMesh.NumCorners());
	AssertInt(6, cMesh.NumFaces());
	AssertInt(6, cMeshEditor.NumPolygons());
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 1.0f, false);
	cMeshEditor.Kill();
	cMesh.Kill();

	//Along Y, which a cone's wedges can't be turned about, and flipped.
	sZDirection.Init(0, 2, 0);
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pSphereEditor = cMeshEditor.AddModifier<CMeshSphereEditor>(&sZDirection, &sCentre, 1.0f, 6, 3);
	pSphereEditor->SetFlipFaces(true);
	cMeshEditor.Collapse();
	AssertInt(14, cMesh.NumCorners());
	AssertInt(24, cMesh.NumFaces());
	AssertFloat3(SFloat32Vec3(0, -1, 0), cMesh.GetPosition(0), 3);
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 1.0f, true);
	cMeshEditor.Kill();
	cMesh.Kill();

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGeosphere(void)
{
	CMesh						cMesh;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sCentre;
	Ptr<CMeshGeosphereEditor>	pGeosphereEditor;
	CMeshEditor					cMeshEditor;
	CFileUtil					cFileUtil;
	CMeshEdge*					pcEdge;
	SFloat32Vec3				sEdge;
	float32						fShortest;
	float32						fLongest;
	float32						fLength;
	int							i;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sCentre.Init(0, 0, 0);

	//An icosahedron.
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	cMeshEditor.AddModifier<CMeshGeosphereEditor>(&sZDirection, &sCentre, 1.0f, 1);
	cMeshEditor.Collapse();
	AssertInt(12, cMesh.NumCorners());
	AssertInt(20, cMesh.NumFaces());
	AssertInt(30, cMesh.NumEdges());
	AssertInt(20, cMeshEditor.NumPolygons());
	AssertFloat3(SFloat32Vec3(0, 0, 1), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(0, 0, -1), cMesh.GetPosition(11), 3);
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 1.0f, false);
	cMeshEditor.Kill();
	cMesh.Kill();

	sCentre.Init(-1, 0.5f, 2);
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pGeosphereEditor = cMeshEditor.AddModifier<CMeshGeosphereEditor>(&sZDirection, &sCentre, 3.0f, 3);
	AssertInt(92, pGeosphereEditor->ExpectedCornerChunkSize());
	AssertInt(180, pGeosphereEditor->ExpectedFaceChunkSize());
	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Geosphere3.OBJ");

	AssertInt(92, cMesh.NumCorners());
	AssertInt(180, cMesh.NumFaces());
	AssertInt(270, cMesh.NumEdges());
	AssertInt(180, cMeshEditor.NumPolygons());
	AssertInt(270, cMeshEditor.NumVisibleEdges(1));
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 3.0f, false);

	//Its triangles are nearly all the same size.
	fShortest = 1000.0f;
	fLongest = 0.0f;
	for (i = 0; i < cMesh.NumEdges(); i++)
	{
		pcEdge = cMesh.GetEdge(i);
		sEdge = *cMesh.GetPosition(pcEdge->maiCorner[0]) - *cMesh.GetPosition(pcEdge->maiCorner[1]);
		fLength = sEdge.Magnitude();
		fShortest = (fLength < fShortest) ? fLength : fShortest;
		fLongest = (fLength > fLongest) ? fLength : fLongest;
	}
	AssertTrue(fShortest / fLongest > 0.8f);

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Geosphere3.OBJ", "Output/Shape/Geosphere3.OBJ");

	//On a slant, and flipped.
	sZDirection.Init(1, 1, 1);
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pGeosphereEditor = cMeshEditor.AddModifier<CMeshGeosphereEditor>(&sZDirection, &sCentre, 0.5f, 2);
	pGeosphereEditor->SetFlipFaces(true);
	pGeosphereEditor->SetFaceName(5);
	cMeshEditor.Collapse();
	AssertInt(42, cMesh.NumCorners());
	AssertInt(80, cMesh.NumFaces());
	AssertInt(5, cMesh.GetFaceName(79));
	AssertFloat3(SFloat32Vec3(-1 + 0.5f / sqrtf(3.0f), 0.5f + 0.5f / sqrtf(3.0f), 2 + 0.5f / sqrtf(3.0f)), cMesh.GetPosition(0), 3);
	AssertShapeClosed(&cMesh, 2);
	AssertBallShape(&cMesh, &sCentre, 0.5f, true);
	cMeshEditor.Kill();
	cMesh.Kill();

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTorus(void)
{
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sCentre;
	Ptr<CMeshTorusEditor>	pTorusEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;
	SFloat32Vec3			sNormal;
	SFloat32Vec3			sMiddle;
	SFloat32Vec3			sOut;
	SFloat32Vec3*			psPosition;
	float32					fAcross;
	bool					bOnSurface;
	bool					bFacing;
	int						i;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sCentre.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pTorusEditor = cMeshEditor.AddModifier<CMeshTorusEditor>(&sZDirection, &sCentre, 3.0f, 1.0f, 8, 6);
	AssertInt(48, pTorusEditor->ExpectedCornerChunkSize());
	AssertInt(96, pTorusEditor->ExpectedFaceChunkSize());
	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Torus8x6.OBJ");

	AssertInt(48, cMesh.NumCorners());
	AssertInt(96, cMesh.NumFaces());
	AssertInt(144, cMesh.NumEdges());
	AssertInt(48, cMeshEditor.NumPolygons());
	AssertFloat3(SFloat32Vec3(0, 4, 0), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(0, 3.5f, sqrtf(0.75f)), cMesh.GetPosition(8), 3);
	AssertShapeClosed(&cMesh, 0);

	//Every corner is the tube's radius from the circle through its middle, and every face faces away from that.
	bOnSurface = true;
	for (i = 0; i < cMesh.NumCorners(); i++)
	{
		psPosition = cMesh.GetPosition(i);
		fAcross = sqrtf(psPosition->x * psPosition->x + psPosition->y * psPosition->y) - 3.0f;
		bOnSurface &= fabsf(sqrtf(fAcross * fAcross + psPosition->z * psPosition->z) - 1.0f) < 0.0001f;
	}
	AssertTrue(bOnSurface);

	bFacing = true;
	for (i = 0; i < cMesh.NumFaces(); i++)
	{
		TestShapeFaceNormal(&cMesh, i, &sNormal, &sMiddle);
		sOut.Init(sMiddle.x, sMiddle.y, 0);
		sOut.Normalize();
		sOut *= -3.0f;
		sOut.Add(&sMiddle);
		bFacing &= Float32Vec3Dot(&sNormal, &sOut) > 0.0f;
	}
	AssertTrue(bFacing);

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Torus8x6.OBJ", "Output/Shape/Torus8x6.OBJ");

	cFileUtil.RemoveDir("Output/Shape");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTube(void)
{
	CMesh					cMesh;
	SFloat32Vec3			sZDirection;
	SFloat32Vec3			sStart;
	Ptr<CMeshTubeEditor>	pTubeEditor;
	CMeshEditor				cMeshEditor;
	CFileUtil				cFileUtil;
	SFloat32Vec3*			psPosition;
	float32					fRadius;
	bool					bInside;
	int						i;

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	sZDirection.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	pTubeEditor = cMeshEditor.AddModifier<CMeshTubeEditor>(&sZDirection, &sStart, 1.0f, 2.0f, 3.0f, 8, 2, 3);
	AssertInt(64, pTubeEditor->ExpectedCornerChunkSize());
	AssertInt(128, pTubeEditor->ExpectedFaceChunkSize());
	cMeshEditor.Collapse();
	WriteMesh(&cMesh, "Output/Shape/Tube8x2x3.OBJ");

	AssertInt(64, cMesh.NumCorners());
	AssertInt(128, cMesh.NumFaces());
	AssertInt(192, cMesh.NumEdges());
	AssertInt(64, cMeshEditor.NumPolygons());
	AssertInt(4, cMeshEditor.mpMesh->mcPolygons.NumUniqueNames());
	AssertInt(32, CountFacesNamed(&cMesh, 1));
	AssertInt(32, CountFacesNamed(&cMesh, 2));
	AssertInt(32, CountFacesNamed(&cMesh, 3));
	AssertInt(32, CountFacesNamed(&cMesh, 4));
	AssertFloat3(SFloat32Vec3(0, 2, 0), cMesh.GetPosition(0), 3);
	AssertFloat3(SFloat32Vec3(0, 1, 3), cMesh.GetPosition(32), 3);
	AssertShapeClosed(&cMesh, 0);
	AssertCylindricalFacesOutward(&cMesh, &sZDirection, 3.0f);

	bInside = true;
	for (i = 0; i < cMesh.NumCorners(); i++)
	{
		psPosition = cMesh.GetPosition(i);
		fRadius = sqrtf(psPosition->x * psPosition->x + psPosition->y * psPosition->y);
		bInside &= (fRadius > 0.9999f) && (fRadius < 2.0001f) && (psPosition->z > -0.0001f) && (psPosition->z < 3.0001f);
	}
	AssertTrue(bInside);

	cMeshEditor.RemoveModifiers();
	cMeshEditor.Kill();
	cMesh.Kill();
	AssertOBJShape("Input/Shape/Tube8x2x3.OBJ", "Output/Shape/Tube8x2x3.OBJ");

	//The fewest segments: a square ring of four quads around, turned.
	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	cMeshEditor.AddModifier<CMeshTubeEditor>(&sZDirection, &sStart, 0.5f, 1.0f, 1.0f, 3, 1, 2);
	cMeshEditor.Collapse();
	AssertInt(12, cMesh.NumCorners());
	AssertInt(24, cMesh.NumFaces());
	AssertShapeClosed(&cMesh, 0);
	AssertCylindricalFacesOutward(&cMesh, &sZDirection, 1.0f);
	cMeshEditor.Kill();
	cMesh.Kill();

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
	TestShapeAxes();
	TestTube();
	TestSphere();
	TestGeosphere();
	TestTorus();

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
	AssertInt(iCorner1, pcMeshFace->maiCorner[0]);
	AssertInt(iCorner2, pcMeshFace->maiCorner[1]);
	AssertInt(iCorner3, pcMeshFace->maiCorner[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerCount(CMeshCorner* pcMeshCorner, int iEdgeCount, int iFaceCount)
{
	AssertSize(iEdgeCount, pcMeshCorner->maiEdge.NumElements());
	AssertSize(iFaceCount, pcMeshCorner->maiFace.NumElements());
}

