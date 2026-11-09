#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshReader.h"
#include "SupportLib/MeshSmoothGroups.h"
#include "SupportLib/MeshWriter.h"
#include "SupportLib/OBJReader.h"
#include "SupportLib/OBJWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertVec3(float32 x, float32 y, float32 z, SFloat32Vec3* psActual)
{
	AssertFloat(x, psActual->x, 4);
	AssertFloat(y, psActual->y, 4);
	AssertFloat(z, psActual->z, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertFaceCorners(CMesh* pcMesh, int iFace, int iCorner1, int iCorner2, int iCorner3)
{
	int*	piCorners;

	piCorners = pcMesh->GetFaceCorners(iFace);
	AssertInt(iCorner1, piCorners[0]);
	AssertInt(iCorner2, piCorners[1]);
	AssertInt(iCorner3, piCorners[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertFaceUVs(CMesh* pcMesh, int iFace, int iUV1, int iUV2, int iUV3)
{
	SMeshUVFace*	psFaceUV;

	psFaceUV = pcMesh->mcUVs.mcLayers.Get(0)->mcFaces.Get(iFace);
	AssertInt(iUV1, psFaceUV->aiUVs[0]);
	AssertInt(iUV2, psFaceUV->aiUVs[1]);
	AssertInt(iUV3, psFaceUV->aiUVs[2]);
}


//////////////////////////////////////////////////////////////////////////
//
// What a corner's normal is, whether it has its own or uses the face's.
//
//////////////////////////////////////////////////////////////////////////
static SFloat32Vec3* GetFaceCornerNormal(CMesh* pcMesh, int iFace, int iFaceCorner)
{
	SMeshNormalFace*	psFaceNormal;
	int					iNormal;

	psFaceNormal = pcMesh->mcNormals.mcFaces.Get(iFace);
	iNormal = psFaceNormal->aiCornerNormals[iFaceCorner];
	if (iNormal < 0)
	{
		iNormal = psFaceNormal->iFaceNormal;
	}
	return pcMesh->mcNormals.mcNormals.Get(iNormal);
}


//////////////////////////////////////////////////////////////////////////
//
// A two by two by two box smoothed by its sides' names, with UVs on every
// other face.
//
//////////////////////////////////////////////////////////////////////////
static void MakeSmoothedBox(CMesh* pcMesh, CMeshEditor* pcEditor)
{
	SFloat32Vec3		sXDirection;
	SFloat32Vec3		sYDirection;
	SFloat32Vec3		sZDirection;
	SFloat32Vec3		sStart;
	CMeshBoxEditor*		pcBoxEditor;
	CMeshSmoothGroups*	pcSmoothing;
	SFloat32Vec3*		psPosition;
	SMeshUVFace*		psFaceUV;
	int*				piCorners;
	int					i;
	int					k;

	pcMesh->Init();
	pcMesh->mcNormals.mbInUse = true;
	pcEditor->Init(pcMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sZDirection.Init(0, 0, 1);
	sStart.Init(-0.5f, -0.5f, -0.5f);

	pcBoxEditor = pcEditor->AddModifier<CMeshBoxEditor>();
	pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 2);

	pcSmoothing = pcEditor->AddModifier<CMeshSmoothGroups>();
	pcSmoothing->Init();
	pcSmoothing->meGenerationStyle = SGS_Name;

	pcEditor->Touch();

	pcMesh->mcUVs.mbInUse = true;
	for (i = 0; i < pcMesh->NumCorners(); i++)
	{
		psPosition = pcMesh->GetPosition(i);
		pcMesh->AddUV(psPosition->x + 0.5f, psPosition->y + 0.5f);
	}
	for (i = 0; i < pcMesh->NumFaces(); i += 2)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		psFaceUV = pcMesh->mcUVs.mcLayers.Get(0)->mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psFaceUV->aiUVs[k] = piCorners[k];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// This replaces a Smoothing.OBJ that was lost: what's written is compared
// with the file, what's read from the file is compared with what was
// written, and writing that gives the same file again.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderRoundTrip(void)
{
	ObjectsInit();
	{
		CMesh			cMesh;
		CMeshEditor		cMeshEditor;
		CMesh			cRead;
		SFloat32Vec3*	psExpected;
		SMeshUVFace*	psFaceUV;
		SMeshUVFace*	psReadFaceUV;
		SFloat32Vec2*	psUV;
		SFloat32Vec2*	psReadUV;
		int				i;
		int				k;

		MakeSmoothedBox(&cMesh, &cMeshEditor);
		AssertInt(26, cMesh.NumCorners());
		AssertInt(48, cMesh.NumFaces());
		AssertSize(48 + 54, cMesh.mcNormals.mcNormals.NumElements());

		AssertTrue(WriteMesh(&cMesh, "Output/Shape/Smoothing.OBJ"));
		AssertFile("Input/Shape/Smoothing.OBJ", "Output/Shape/Smoothing.OBJ");

		cRead.Init();
		AssertTrue(ReadMesh(&cRead, "Input/Shape/Smoothing.OBJ"));
		AssertInt(26, cRead.NumCorners());
		AssertInt(48, cRead.NumFaces());
		AssertSize(48 + 54, cRead.mcNormals.mcNormals.NumElements());
		AssertInt(26, cRead.mcUVs.NumUVs());

		for (i = 0; i < cMesh.NumCorners(); i++)
		{
			psExpected = cMesh.GetPosition(i);
			AssertVec3(psExpected->x, psExpected->y, psExpected->z, cRead.GetPosition(i));

			psUV = cMesh.mcUVs.mcLayers.Get(0)->mcUVs.Get(i);
			psReadUV = cRead.mcUVs.mcLayers.Get(0)->mcUVs.Get(i);
			AssertFloat(psUV->x, psReadUV->x, 4);
			AssertFloat(psUV->y, psReadUV->y, 4);
		}

		for (i = 0; i < cMesh.NumFaces(); i++)
		{
			AssertFaceCorners(&cRead, i, cMesh.GetFaceCorners(i)[0], cMesh.GetFaceCorners(i)[1], cMesh.GetFaceCorners(i)[2]);
			psFaceUV = cMesh.mcUVs.mcLayers.Get(0)->mcFaces.Get(i);
			psReadFaceUV = cRead.mcUVs.mcLayers.Get(0)->mcFaces.Get(i);
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				AssertInt(psFaceUV->aiUVs[k], psReadFaceUV->aiUVs[k]);

				psExpected = GetFaceCornerNormal(&cMesh, i, k);
				AssertVec3(psExpected->x, psExpected->y, psExpected->z, GetFaceCornerNormal(&cRead, i, k));
			}
		}

		AssertTrue(WriteMesh(&cRead, "Output/Shape/SmoothingAgain.OBJ"));
		AssertFile("Input/Shape/Smoothing.OBJ", "Output/Shape/SmoothingAgain.OBJ");

		cRead.Kill();
		cMeshEditor.RemoveModifiers();
		cMeshEditor.Kill();
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Faces that only have flat normals still get them from a file: each of
// their corners has a normal, and a mesh read back doesn't keep growing.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderFlatNormals(void)
{
	ObjectsInit();
	{
		CMesh	cMesh;
		CMesh	cRead;
		CMesh	cReadAgain;

		cMesh.Init();
		cMesh.mcNormals.mbInUse = true;
		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(1, 0, 0);
		cMesh.AddCorner(0, 1, 0);
		cMesh.AddCorner(0, 0, 1);
		cMesh.AddFace(0, 1, 2);
		cMesh.AddFace(0, 3, 1);
		AssertSize(2, cMesh.mcNormals.mcNormals.NumElements());

		AssertTrue(WriteMesh(&cMesh, "Output/Shape/Flat.OBJ"));

		cRead.Init();
		AssertTrue(ReadMesh(&cRead, "Output/Shape/Flat.OBJ"));
		AssertInt(2, cRead.NumFaces());
		AssertSize(2 + 2, cRead.mcNormals.mcNormals.NumElements());
		AssertVec3(cMesh.mcNormals.GetFaceNormal(0)->x, cMesh.mcNormals.GetFaceNormal(0)->y, cMesh.mcNormals.GetFaceNormal(0)->z, GetFaceCornerNormal(&cRead, 0, 2));
		AssertVec3(cMesh.mcNormals.GetFaceNormal(1)->x, cMesh.mcNormals.GetFaceNormal(1)->y, cMesh.mcNormals.GetFaceNormal(1)->z, GetFaceCornerNormal(&cRead, 1, 0));

		AssertTrue(WriteMesh(&cRead, "Output/Shape/FlatAgain.OBJ"));
		AssertFile("Output/Shape/Flat.OBJ", "Output/Shape/FlatAgain.OBJ");

		cRead.Kill();
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertOBJReaderMesh(CMesh* pcMesh)
{
	AssertInt(5, pcMesh->NumCorners());
	AssertVec3(0, 0, 0.15f, pcMesh->GetPosition(4));

	//The quad is two faces, and each of its corners keeps its own UV and normal.
	AssertInt(4, pcMesh->NumFaces());
	AssertFaceCorners(pcMesh, 0, 0, 1, 3);
	AssertFaceCorners(pcMesh, 1, 1, 2, 3);
	AssertFaceUVs(pcMesh, 0, 0, 1, 3);
	AssertFaceUVs(pcMesh, 1, 1, 2, 3);
	AssertVec3(0, 0, 1, GetFaceCornerNormal(pcMesh, 0, 0));
	AssertVec3(0, 0, 1, GetFaceCornerNormal(pcMesh, 1, 2));

	//Negative indices count back from what had been read when the face was.
	AssertFaceCorners(pcMesh, 2, 0, 1, 4);
	AssertFaceUVs(pcMesh, 2, 0, 1, -1);
	AssertVec3(1, 0, 0, GetFaceCornerNormal(pcMesh, 2, 0));
	AssertVec3(1, 0, 0, GetFaceCornerNormal(pcMesh, 2, 1));
	AssertVec3(0, 1, 0, GetFaceCornerNormal(pcMesh, 2, 2));

	//With no normals in the file a face keeps its flat one.
	AssertFaceCorners(pcMesh, 3, 1, 2, 4);
	AssertFaceUVs(pcMesh, 3, -1, -1, -1);
	AssertInt(-1, pcMesh->mcNormals.mcFaces.Get(3)->aiCornerNormals[0]);

	AssertSize(4 + 3, pcMesh->mcNormals.mcNormals.NumElements());
	AssertInt(4, pcMesh->mcUVs.NumUVs());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderFile(void)
{
	ObjectsInit();
	{
		CMesh	cMesh;

		cMesh.Init();
		AssertTrue(ReadMesh(&cMesh, "Input/Shape/OBJReader.OBJ"));
		AssertOBJReaderMesh(&cMesh);
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderEditor(void)
{
	ObjectsInit();
	{
		Ptr<CMesh>			pMesh;
		Ptr<CMeshEditor>	pEditor;

		pMesh = OMalloc<CMesh>();
		pEditor = OMalloc<CMeshEditor>(pMesh);

		AssertTrue(LoadOBJ(pEditor, "Input/Shape/OBJReader.OBJ"));
		AssertOBJReaderMesh(&pMesh);
		AssertInt(3, pEditor->NumPolygons());
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// What OBJPolygons.OBJ cuts into, worked out by hand.
//
//////////////////////////////////////////////////////////////////////////
static void AssertOBJPolygonsMesh(CMesh* pcMesh)
{
	int		i;

	AssertInt(18, pcMesh->NumCorners());
	AssertInt(10, pcMesh->NumFaces());

	//The convex hexagon is a fan from its last corner, and each corner keeps its UV and normal.
	AssertFaceCorners(pcMesh, 0, 0, 1, 5);
	AssertFaceCorners(pcMesh, 1, 1, 2, 5);
	AssertFaceCorners(pcMesh, 2, 2, 3, 5);
	AssertFaceCorners(pcMesh, 3, 3, 4, 5);
	AssertFaceUVs(pcMesh, 0, 0, 1, 5);
	AssertFaceUVs(pcMesh, 1, 1, 2, 5);
	AssertFaceUVs(pcMesh, 2, 2, 3, 5);
	AssertFaceUVs(pcMesh, 3, 3, 4, 5);
	for (i = 0; i < 3; i++)
	{
		AssertVec3(0, 0, 1, GetFaceCornerNormal(pcMesh, 3, i));
	}

	//The concave quad's first corner points inwards, so it isn't cut across from there.
	AssertFaceCorners(pcMesh, 4, 7, 8, 6);
	AssertFaceCorners(pcMesh, 5, 6, 8, 9);
	AssertVec3(0, 0, 1, pcMesh->mcNormals.GetFaceNormal(4));

	//The upright concave polygon, whose faces all face -x.
	AssertFaceCorners(pcMesh, 6, 11, 12, 10);
	AssertFaceCorners(pcMesh, 7, 10, 12, 14);
	AssertFaceCorners(pcMesh, 8, 12, 13, 14);
	AssertVec3(-1, 0, 0, pcMesh->mcNormals.GetFaceNormal(6));
	AssertVec3(-1, 0, 0, pcMesh->mcNormals.GetFaceNormal(7));
	AssertVec3(-1, 0, 0, pcMesh->mcNormals.GetFaceNormal(8));

	AssertFaceCorners(pcMesh, 9, 15, 16, 17);
	AssertFaceUVs(pcMesh, 9, -1, -1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderPolygons(void)
{
	ObjectsInit();
	{
		CMesh				cMesh;
		Ptr<CMesh>			pMesh;
		Ptr<CMeshEditor>	pEditor;

		cMesh.Init();
		AssertTrue(ReadMesh(&cMesh, "Input/Shape/OBJPolygons.OBJ"));
		AssertOBJPolygonsMesh(&cMesh);
		cMesh.Kill();

		cMesh.Init();
		AssertTrue(ReadMesh(&cMesh, "Input/Shape/OBJPentagon.OBJ"));
		AssertInt(3, cMesh.NumFaces());
		AssertFaceCorners(&cMesh, 0, 0, 1, 4);
		AssertFaceCorners(&cMesh, 1, 1, 2, 4);
		AssertFaceCorners(&cMesh, 2, 2, 3, 4);
		cMesh.Kill();

		//An editor sees each one as one polygon, with only the outside edges showing.
		pMesh = OMalloc<CMesh>();
		pEditor = OMalloc<CMeshEditor>(pMesh);
		AssertTrue(LoadOBJ(pEditor, "Input/Shape/OBJPolygons.OBJ"));
		AssertOBJPolygonsMesh(&pMesh);
		AssertInt(4, pEditor->NumPolygons());
		AssertInt(6 + 4 + 5 + 3, pEditor->NumVisibleEdges(1));
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertFileContains(const char* szFilename, const char* szExpected)
{
	CTextFile	cTextFile;

	cTextFile.Init();
	AssertTrue(cTextFile.Read(szFilename));
	AssertTrue(strstr(cTextFile.mcText.Text(), szExpected) != NULL);
	cTextFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An editor writes its polygons as polygons, and reading that and writing it
// again gives the same file.  A mesh on its own writes its triangles.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJWriterPolygons(void)
{
	ObjectsInit();
	{
		Ptr<CMesh>			pMesh;
		Ptr<CMeshEditor>	pEditor;

		pMesh = OMalloc<CMesh>();
		pEditor = OMalloc<CMeshEditor>(pMesh);
		AssertTrue(LoadOBJ(pEditor, "Input/Shape/OBJPolygons.OBJ"));

		AssertTrue(SaveOBJ(&pEditor, "Output/Shape/Polygons.OBJ"));
		AssertFile("Input/Shape/OBJPolygonsWritten.OBJ", "Output/Shape/Polygons.OBJ");

		AssertTrue(SaveOBJ(&pMesh, "Output/Shape/PolygonTriangles.OBJ"));
		AssertFileContains("Output/Shape/PolygonTriangles.OBJ", "# 10 triangles");

		pMesh = OMalloc<CMesh>();
		pEditor = OMalloc<CMeshEditor>(pMesh);
		AssertTrue(LoadOBJ(pEditor, "Input/Shape/OBJPolygonsWritten.OBJ"));
		AssertInt(10, pMesh->NumFaces());
		AssertInt(4, pEditor->NumPolygons());

		AssertTrue(SaveOBJ(&pEditor, "Output/Shape/PolygonsAgain.OBJ"));
		AssertFile("Input/Shape/OBJPolygonsWritten.OBJ", "Output/Shape/PolygonsAgain.OBJ");
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A box's sides are polygons that an editor writes as such.  With UVs on
// every other face its faces don't agree, so it writes the same as a mesh.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJWriterBox(void)
{
	ObjectsInit();
	{
		CMesh				cMesh;
		CMeshEditor			cMeshEditor;
		CMesh				cRead;
		SFloat32Vec3		sXDirection;
		SFloat32Vec3		sYDirection;
		SFloat32Vec3		sZDirection;
		SFloat32Vec3		sStart;
		CMeshBoxEditor*		pcBoxEditor;

		MakeSmoothedBox(&cMesh, &cMeshEditor);
		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/SmoothingFromEditor.OBJ"));
		AssertFile("Input/Shape/Smoothing.OBJ", "Output/Shape/SmoothingFromEditor.OBJ");
		cMeshEditor.RemoveModifiers();
		cMeshEditor.Kill();
		cMesh.Kill();

		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		sXDirection.Init(1, 0, 0);
		sYDirection.Init(0, 1, 0);
		sZDirection.Init(0, 0, 1);
		sStart.Init(-0.5f, -0.5f, -0.5f);
		pcBoxEditor = cMeshEditor.AddModifier<CMeshBoxEditor>();
		pcBoxEditor->Init(&sXDirection, &sYDirection, &sZDirection, &sStart, 2, 2, 2);
		cMeshEditor.Touch();
		AssertInt(48, cMesh.NumFaces());
		AssertInt(24, cMeshEditor.NumPolygons());

		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/BoxPolygons.OBJ"));
		AssertFileContains("Output/Shape/BoxPolygons.OBJ", "# 24 polygons");

		cRead.Init();
		AssertTrue(ReadMesh(&cRead, "Output/Shape/BoxPolygons.OBJ"));
		AssertInt(26, cRead.NumCorners());
		AssertInt(48, cRead.NumFaces());
		cRead.Kill();

		cMeshEditor.RemoveModifiers();
		cMeshEditor.Kill();
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// What one line of a file can't say is written as triangles.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJWriterPolygonsAsTriangles(void)
{
	ObjectsInit();
	{
		CMesh			cMesh;
		CMeshEditor		cMeshEditor;
		SMeshUVFace*	psFaceUV;

		//A corner in the middle of a polygon.
		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(2, 0, 0);
		cMesh.AddCorner(2, 2, 0);
		cMesh.AddCorner(0, 2, 0);
		cMesh.AddCorner(1, 1, 0);
		cMeshEditor.AddFace(0, 1, 4, 0, true, false, false);
		cMeshEditor.AddFace(1, 2, 4, 0, true, false, false);
		cMeshEditor.AddFace(2, 3, 4, 0, true, false, false);
		cMeshEditor.AddFace(3, 0, 4, 0, true, false, false);
		AssertInt(1, cMeshEditor.NumPolygons());
		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/MiddleCorner.OBJ"));
		AssertFileContains("Output/Shape/MiddleCorner.OBJ", "f  1 2 5\n");
		AssertFileContains("Output/Shape/MiddleCorner.OBJ", "# 4 triangles");
		cMeshEditor.Kill();
		cMesh.Kill();

		//A quad's faces that agree on their UVs is one line.
		cMesh.Init();
		cMeshEditor.Init(&cMesh);
		cMesh.mcUVs.mbInUse = true;
		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(1, 0, 0);
		cMesh.AddCorner(1, 1, 0);
		cMesh.AddCorner(0, 1, 0);
		cMesh.AddUV(0, 0);
		cMesh.AddUV(1, 0);
		cMesh.AddUV(1, 1);
		cMesh.AddUV(0, 1);
		cMesh.AddUV(0.5f, 1);
		cMeshEditor.AddQuad(0, 1, 2, 3);
		psFaceUV = cMesh.mcUVs.mcLayers.Get(0)->mcFaces.Get(0);
		psFaceUV->aiUVs[0] = 0;
		psFaceUV->aiUVs[1] = 1;
		psFaceUV->aiUVs[2] = 3;
		psFaceUV = cMesh.mcUVs.mcLayers.Get(0)->mcFaces.Get(1);
		psFaceUV->aiUVs[0] = 1;
		psFaceUV->aiUVs[1] = 2;
		psFaceUV->aiUVs[2] = 3;
		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/QuadUVs.OBJ"));
		AssertFileContains("Output/Shape/QuadUVs.OBJ", "f  1/1 2/2 3/3 4/4\n");

		//With a seam across it, they don't.
		psFaceUV->aiUVs[2] = 4;
		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/QuadSeam.OBJ"));
		AssertFileContains("Output/Shape/QuadSeam.OBJ", "f  1/1 2/2 4/4\n");
		AssertFileContains("Output/Shape/QuadSeam.OBJ", "f  2/2 3/3 4/5\n");
		cMeshEditor.Kill();
		cMesh.Kill();

		//A quad that isn't flat has faces with different normals.
		cMesh.Init();
		cMesh.mcNormals.mbInUse = true;
		cMeshEditor.Init(&cMesh);
		cMesh.AddCorner(0, 0, 0);
		cMesh.AddCorner(1, 0, 0);
		cMesh.AddCorner(1, 1, 1);
		cMesh.AddCorner(0, 1, 0);
		cMeshEditor.AddQuad(0, 1, 2, 3);
		AssertTrue(SaveOBJ(&cMeshEditor, "Output/Shape/BentQuad.OBJ"));
		AssertFileContains("Output/Shape/BentQuad.OBJ", "# 2 triangles");
		cMeshEditor.Kill();
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReaderFailures(void)
{
	SLogConfig	sLogConfig;

	ObjectsInit();
	{
		CMesh	cMesh;

		sLogConfig = gcLogger.SetSilent();

		cMesh.Init();
		AssertFalse(ReadMesh(&cMesh, "Input/Shape/OBJNoSuchFile.OBJ"));
		cMesh.Kill();

		cMesh.Init();
		AssertFalse(ReadMesh(&cMesh, "Input/Shape/OBJBadIndex.OBJ"));
		AssertInt(0, cMesh.NumFaces());
		cMesh.Kill();

		cMesh.Init();
		AssertFalse(ReadMesh(&cMesh, "Input/Shape/OBJTwoCorners.OBJ"));
		AssertInt(0, cMesh.NumFaces());
		cMesh.Kill();

		cMesh.Init();
		AssertFalse(ReadMesh(&cMesh, "Input/Shape/OBJNormalAfterFace.OBJ"));
		cMesh.Kill();

		gcLogger.SetConfig(&sLogConfig);

		cMesh.Init();
		AssertFalse(WriteMesh(&cMesh, "Output/Shape/NoSuchDirectory/Empty.OBJ"));
		cMesh.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReader(void)
{
	CFileUtil	cFileUtil;

	BeginTests();

	DataIOInit();
	NumberInit();

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	TestMeshOBJReaderRoundTrip();
	TestMeshOBJReaderFlatNormals();
	TestMeshOBJReaderFile();
	TestMeshOBJReaderEditor();
	TestMeshOBJReaderPolygons();
	TestMeshOBJWriterPolygons();
	TestMeshOBJWriterBox();
	TestMeshOBJWriterPolygonsAsTriangles();
	TestMeshOBJReaderFailures();

	cFileUtil.RemoveDir("Output/Shape");

	NumberKill();
	DataIOKill();

	TestStatistics();
}

