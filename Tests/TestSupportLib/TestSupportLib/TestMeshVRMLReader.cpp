#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include <string.h>
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshReader.h"
#include "SupportLib/VRMLReader.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertVRMLVec3(float32 x, float32 y, float32 z, SFloat32Vec3* psActual)
{
	AssertFloat(x, psActual->x, 3);
	AssertFloat(y, psActual->y, 3);
	AssertFloat(z, psActual->z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertVRMLFaceCorners(int iCorner0, int iCorner1, int iCorner2, CMeshFace* pcFace)
{
	AssertInt(iCorner0, pcFace->maiCorner[0]);
	AssertInt(iCorner1, pcFace->maiCorner[1]);
	AssertInt(iCorner2, pcFace->maiCorner[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshVRMLReaderFile(void)
{
	CMesh				cMesh;
	SMeshNormalFace*	psNormalFace;
	SMeshFaceColour*	psColourFace;
	ARGB32				uiColour;

	cMesh.Init();
	AssertTrue(ReadMesh(&cMesh, "Input/Shape/KicadExport.wrl"));

	//Two triangles, then those again by USE, then a quad and a shape on its own.
	AssertInt(4 + 4 + 4 + 3, cMesh.NumCorners());
	AssertInt(2 + 2 + 2 + 1, cMesh.NumFaces());
	AssertFalse(cMesh.GetConnectivity()->IsGenerated());

	//Scaled by 2, rotated a quarter turn around Z and translated.
	AssertVRMLVec3(10.0f, 20.0f, 30.0f, cMesh.GetPosition(0));
	AssertVRMLVec3(10.0f, 22.0f, 30.0f, cMesh.GetPosition(1));
	AssertVRMLVec3(7.0f, 20.0f, 30.0f, cMesh.GetPosition(2));
	AssertVRMLVec3(10.5f, 20.5f, 30.0f, cMesh.GetPosition(3));

	//The second instance is the first translated by its parent.
	AssertVRMLVec3(110.0f, 22.0f, 30.0f, cMesh.GetPosition(5));

	//Mirrored and not uniformly scaled.
	AssertVRMLVec3(-2.0f, 1.0f, 0.0f, cMesh.GetPosition(10));

	//Faces.  The last corner of the first shape's second face was left unterminated by -1.
	AssertVRMLFaceCorners(0, 1, 2, cMesh.GetFace(0));
	AssertVRMLFaceCorners(0, 3, 1, cMesh.GetFace(1));
	AssertVRMLFaceCorners(4, 5, 6, cMesh.GetFace(2));
	AssertVRMLFaceCorners(4, 7, 5, cMesh.GetFace(3));

	//The quad is a fan of two triangles with their order reversed because the transform mirrors.
	AssertVRMLFaceCorners(8, 10, 9, cMesh.GetFace(4));
	AssertVRMLFaceCorners(8, 11, 10, cMesh.GetFace(5));
	AssertVRMLFaceCorners(12, 13, 14, cMesh.GetFace(6));

	//A flat normal for each of the seven faces, then the corner normals of the three shapes that have them.
	AssertSize(7 + 4 + 4 + 4, cMesh.mcNormals.mcNormals.NumElements());
	AssertVRMLVec3(0.0f, 0.0f, 1.0f, cMesh.mcNormals.GetFaceNormal(0));
	AssertVRMLVec3(0.0f, 0.0f, 1.0f, cMesh.mcNormals.GetFaceNormal(4));

	psNormalFace = cMesh.mcNormals.mcFaces.Get(1);
	AssertInt(7 + 0, psNormalFace->aiCornerNormals[0]);
	AssertInt(7 + 3, psNormalFace->aiCornerNormals[1]);
	AssertInt(7 + 1, psNormalFace->aiCornerNormals[2]);

	//Rotated by the same quarter turn but not scaled.
	AssertVRMLVec3(0.0f, 1.0f, 0.0f, cMesh.mcNormals.GetCornerNormal(7 + 3));

	psNormalFace = cMesh.mcNormals.mcFaces.Get(3);
	AssertInt(7 + 4 + 0, psNormalFace->aiCornerNormals[0]);
	AssertInt(7 + 4 + 3, psNormalFace->aiCornerNormals[1]);
	AssertInt(7 + 4 + 1, psNormalFace->aiCornerNormals[2]);

	//A normal under scale -2 1 1 stays perpendicular: (1, 1, 0) goes to (-0.5, 1, 0) normalised.
	psNormalFace = cMesh.mcNormals.mcFaces.Get(4);
	AssertInt(7 + 8 + 0, psNormalFace->aiCornerNormals[0]);
	AssertInt(7 + 8 + 2, psNormalFace->aiCornerNormals[1]);
	AssertInt(7 + 8 + 1, psNormalFace->aiCornerNormals[2]);
	AssertVRMLVec3(-0.447f, 0.894f, 0.0f, cMesh.mcNormals.GetCornerNormal(7 + 8));

	//A shape with no normals has only its flat one.
	psNormalFace = cMesh.mcNormals.mcFaces.Get(6);
	AssertInt(-1, psNormalFace->aiCornerNormals[0]);
	AssertInt(-1, psNormalFace->aiCornerNormals[1]);
	AssertInt(-1, psNormalFace->aiCornerNormals[2]);
	AssertVRMLVec3(0.0f, 0.0f, 1.0f, cMesh.mcNormals.GetFaceNormal(6));

	//One material used by USE and a shape without one.
	AssertSize(1, cMesh.mcColours.mcColours.NumElements());
	uiColour = cMesh.mcColours.mcColours.GetValue(0);
	AssertInt(255, Get8BitRedColour(uiColour));
	AssertInt(0, Get8BitGreenColour(uiColour));
	AssertInt(0, Get8BitBlueColour(uiColour));
	AssertInt(204, Get8BitAlphaColour(uiColour));

	psColourFace = cMesh.mcColours.mcFaces.Get(5);
	AssertInt(0, psColourFace->aiCornerColours[0]);
	AssertInt(0, psColourFace->aiCornerColours[1]);
	AssertInt(0, psColourFace->aiCornerColours[2]);

	psColourFace = cMesh.mcColours.mcFaces.Get(6);
	AssertInt(-1, psColourFace->aiCornerColours[0]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertVRMLText(bool bExpected, int iExpectedCorners, const char* szText)
{
	CMesh		cMesh;
	CVRMLReader	cReader;

	cMesh.Init();
	cReader.Init(&cMesh, "Test.wrl");
	AssertBool(bExpected, cReader.Read(szText, (lsize)strlen(szText)));
	cReader.Kill();
	AssertInt(iExpectedCorners, cMesh.NumCorners());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshVRMLReaderText(void)
{
	SLogConfig	sLogConfig;

	AssertVRMLText(true, 0, "#VRML V2.0 utf8\n");
	AssertVRMLText(true, 3, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coordIndex [ 0 1 2 ] coord Coordinate { point [ 0 0 0 1 0 0 0 1 0 ] } } }");
	AssertVRMLText(true, 3, "#VRML V2.0 utf8\nDEF A Shape { geometry IndexedFaceSet { coord Coordinate { point [ -9.4382988e-05 0 0, 1 0 0, 0 1 0 ] } coordIndex [ 0,1,2,-1 ] } }");
	AssertVRMLText(true, 6, "#VRML V2.0 utf8\nDEF A Shape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0, 1 0 0, 0 1 0 ] } coordIndex [ 0,1,2,-1 ] } } USE A");

	sLogConfig = gcLogger.SetSilent();

	AssertVRMLText(false, 0, "");
	AssertVRMLText(false, 0, "#VRML V1.0 ascii\n");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nGroup { }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nTransform { bboxCenter 0 0 0 }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nUSE Missing");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nTransform { children [ ");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nDEF A Transform { children [ USE A ] }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 ] } } }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0 ] } coordIndex [ 0 0 1 ] } }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0 ] } coordIndex [ 0 0 -2 ] } }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0 ] } coordIndex [ 0 0 -1 ] } }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0 ] } normal Normal { vector [ 0 0 1 0 0 1 ] } } }");
	AssertVRMLText(false, 0, "#VRML V2.0 utf8\nShape { geometry IndexedFaceSet { normalPerVertex FALSE } }");

	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshVRMLReader(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestMeshVRMLReaderFile();
	TestMeshVRMLReaderText();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

