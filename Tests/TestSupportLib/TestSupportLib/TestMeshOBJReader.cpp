#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshReader.h"
#include "SupportLib/MeshSmoothGroups.h"
#include "SupportLib/MeshWriter.h"
#include "SupportLib/OBJReader.h"
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
		AssertFalse(ReadMesh(&cMesh, "Input/Shape/OBJPentagon.OBJ"));
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
	TestMeshOBJReaderFailures();

	cFileUtil.RemoveDir("Output/Shape");

	NumberKill();
	DataIOKill();

	TestStatistics();
}

