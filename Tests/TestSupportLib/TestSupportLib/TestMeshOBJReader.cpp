#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshReader.h"
#include "SupportLib/MeshWriter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshOBJReader(void)
{
	//This test can't be run because I lost the Smoothing.OBJ file.
	NumberInit();
	DataIOInit();
	ObjectsInit();
	{
		CMesh				cMesh;
		bool				bResult;
		int					i;
		CMeshFace*			pcFace;
		bool				bOutOfBounds;
		int					iNumCorners;
		int					iNumNormals;
		SMeshNormalFace*	psNormalFace;
		CFileUtil			cFileUtil;

		cFileUtil.RemoveDir("Output/Shape");
		cFileUtil.MakeDir("Output/Shape");

		cMesh.Init();
		bResult = ReadMesh(&cMesh, "Input/Shape/Smoothing.OBJ");

		iNumCorners = cMesh.NumCorners();
		iNumNormals = (int)cMesh.mcNormals.mcNormals.NumElements();

		AssertTrue(bResult);
		AssertInt(162, iNumCorners);
		AssertInt(320, cMesh.NumFaces());
		AssertInt(320 + 394, iNumNormals);

		bOutOfBounds = false;
		for (i = 0; i < cMesh.NumFaces(); i++)
		{
			pcFace = cMesh.GetFace(i);
			if (pcFace->asCorner[0] >= iNumCorners)
			{
				bOutOfBounds = true;
				break;
			}
			if (pcFace->asCorner[1] >= iNumCorners)
			{
				bOutOfBounds = true;
				break;
			}
			if (pcFace->asCorner[2] >= iNumCorners)
			{
				bOutOfBounds = true;
				break;
			}
		}
		AssertBool(false, bOutOfBounds);

		bOutOfBounds = false;
		for (i = 0; i < cMesh.NumFaces(); i++)
		{
			psNormalFace = cMesh.mcNormals.mcFaces.Get(i);
			if (psNormalFace->aiCornerNormals[0] >= iNumNormals)
			{
				bOutOfBounds = true;
				break;
			}
			psNormalFace = cMesh.mcNormals.mcFaces.Get(i);
			if (psNormalFace->aiCornerNormals[1] >= iNumNormals)
			{
				bOutOfBounds = true;
				break;
			}
			psNormalFace = cMesh.mcNormals.mcFaces.Get(i);
			if (psNormalFace->aiCornerNormals[2] >= iNumNormals)
			{
				bOutOfBounds = true;
				break;
			}
		}
		AssertBool(false, bOutOfBounds);

		WriteMesh(&cMesh, "Output/Shape/Reader.OBJ");

		cFileUtil.RemoveDir("Output/Shape");
	}
	ObjectsKill(false);
	DataIOKill();
	NumberKill();
}

