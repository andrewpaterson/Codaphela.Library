#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TextFile.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshReader.h"
#include "SupportLib/MeshWriter.h"
#include "SupportLib/OBJReader.h"
#include "SupportLib/OBJWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


void MakeSmoothedBox(CMesh* pcMesh, CMeshEditor* pcEditor);


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
void TestMeshOBJWriterFailures(void)
{
	ObjectsInit();
	{
		CMesh	cMesh;

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
void TestMeshOBJWriter(void)
{
	CFileUtil	cFileUtil;

	BeginTests();

	DataIOInit();
	NumberInit();

	cFileUtil.RemoveDir("Output/Shape");
	cFileUtil.MakeDir("Output/Shape");

	TestMeshOBJWriterPolygons();
	TestMeshOBJWriterBox();
	TestMeshOBJWriterPolygonsAsTriangles();
	TestMeshOBJWriterFailures();

	cFileUtil.RemoveDir("Output/Shape");

	NumberKill();
	DataIOKill();

	TestStatistics();
}

