#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshEditorWrapper.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "TestLib/Assert.h"


void AssertCornerFaces(CMeshCorner* pcCorner, size iNumFaces, ...);
void AssertCornerEdges(CMeshCorner* pcCorner, size iNumEdges, ...);
void AssertEdgeFaces(CMeshEdge* pcEdge, size iFace1, size iFace2);
void AssertEdgeCorners(CMeshEdge* pcEdge, size iCorner1, size iCorner2);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeConnectivity(void)
{
	CMesh				cMesh;
	SFloat32Vec3				sXDirection;
	SFloat32Vec3				sYDirection;
	SFloat32Vec3				sZDirection;
	SFloat32Vec3				sStart;
	CMeshPlaneEditor*	pcPlaneEditor;
	CMeshEditor			cMeshEditor;
	CMeshConnectivity*	pcConn;
	size				i;
	CCornerEdgesMap*	pcCornerEdgesMap;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);

	sXDirection.Init(1, 0, 0);
	sYDirection.Init(0, 1, 0);
	sZDirection.Init(0, 0, 1);
	sStart.Init(-0.5f, -0.5f, -0.5f);

	pcPlaneEditor = cMeshEditor.AddModifier<CMeshPlaneEditor>();
	pcPlaneEditor->Init(&sXDirection, &sYDirection, &sStart, 1, 2);

	cMeshEditor.Collapse();

	pcConn = cMesh.GetConnectivity();
	AssertLSize(9, pcConn->NumEdges());
	AssertLSize(6, pcConn->NumCorners());
	AssertLSize(4, pcConn->mcFaces.NumElements());

	AssertCornerFaces(pcConn->GetCorner(0),  1,  0);
	AssertCornerFaces(pcConn->GetCorner(1),  2,  0, 1);
	AssertCornerFaces(pcConn->GetCorner(2),  3,  0, 1, 2);
	AssertCornerFaces(pcConn->GetCorner(3),  3,  1, 2, 3);
	AssertCornerFaces(pcConn->GetCorner(4),  2,  2, 3);
	AssertCornerFaces(pcConn->GetCorner(5),  1,  3);

	AssertCornerEdges(pcConn->GetCorner(0),  2,  0, 2);
	AssertCornerEdges(pcConn->GetCorner(1),  3,  0, 1, 3);
	AssertCornerEdges(pcConn->GetCorner(2),  4,  1, 2, 4, 6);
	AssertCornerEdges(pcConn->GetCorner(3),  4,  3, 4, 5, 7);
	AssertCornerEdges(pcConn->GetCorner(4),  3,  5, 6, 8);
	AssertCornerEdges(pcConn->GetCorner(5),  2,  7, 8);

	for (i = 0; i < pcConn->NumCorners(); i++)
	{
		pcCornerEdgesMap = pcConn->GetCornerEdgesMap((int)i);
		AssertSize(pcCornerEdgesMap->maiEdgeIndex.NumElements(), pcCornerEdgesMap->maiLargerCornerIndex.NumElements());
	}

	AssertEdgeCorners(pcConn->GetEdge(0), 0, 1);
	AssertEdgeCorners(pcConn->GetEdge(1), 1, 2);
	AssertEdgeCorners(pcConn->GetEdge(2), 0, 2);
	AssertEdgeCorners(pcConn->GetEdge(3), 1, 3);
	AssertEdgeCorners(pcConn->GetEdge(4), 2, 3);
	AssertEdgeCorners(pcConn->GetEdge(5), 3, 4);
	AssertEdgeCorners(pcConn->GetEdge(6), 2, 4);
	AssertEdgeCorners(pcConn->GetEdge(7), 3, 5);
	AssertEdgeCorners(pcConn->GetEdge(8), 4, 5);

	AssertEdgeFaces(pcConn->GetEdge(0), 0, -1);
	AssertEdgeFaces(pcConn->GetEdge(1), 0, 1);
	AssertEdgeFaces(pcConn->GetEdge(2), 0, -1);
	AssertEdgeFaces(pcConn->GetEdge(3), 1, -1);
	AssertEdgeFaces(pcConn->GetEdge(4), 1, 2);
	AssertEdgeFaces(pcConn->GetEdge(5), 2, 3);
	AssertEdgeFaces(pcConn->GetEdge(6), 2, -1);
	AssertEdgeFaces(pcConn->GetEdge(7), 3, -1);
	AssertEdgeFaces(pcConn->GetEdge(8), 3, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerFaces(CMeshCorner* pcCorner, size iNumFaces, ...)
{
	va_list		vaMarker;
	size			i;
	size			iFaceNum;

	AssertSize(iNumFaces, pcCorner->maiFace.NumElements());
	if (iNumFaces != pcCorner->maiFace.NumElements())
	{
		return;
	}

	va_start(vaMarker, iNumFaces);
	for (i = 0; i < iNumFaces; i++)
	{
		iFaceNum = (size)va_arg(vaMarker, int);  //The face numbers are passed as int literals.
		AssertSize(iFaceNum, pcCorner->maiFace.GetValue(i));
	}
	va_end(vaMarker);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCornerEdges(CMeshCorner* pcCorner, size iNumEdges, ...)
{
	va_list		vaMarker;
	size			i;
	size			iEdgeNum;

	AssertSize(iNumEdges, pcCorner->maiEdge.NumElements());
	if (iNumEdges != pcCorner->maiEdge.NumElements())
	{
		return;
	}

	va_start(vaMarker, iNumEdges);
	for (i = 0; i < iNumEdges; i++)
	{
		iEdgeNum = (size)va_arg(vaMarker, int);  //The edge numbers are passed as int literals.
		AssertSize(iEdgeNum, pcCorner->maiEdge.GetValue(i));
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEdgeFaces(CMeshEdge* pcEdge, size iFace1, size iFace2)
{
	AssertSize(iFace1, pcEdge->maiFace[0]);
	AssertSize(iFace2, pcEdge->maiFace[1]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEdgeCorners(CMeshEdge* pcEdge, size iCorner1, size iCorner2)
{
	AssertSize(iCorner1, pcEdge->maiCorner[0]);
	AssertSize(iCorner2, pcEdge->maiCorner[1]);
}


//////////////////////////////////////////////////////////////////////////
//
// A flipped quad's return says which of the quad's corners are in each of its two faces.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorAddFlippedQuad(void)
{
	CMesh		cMesh;
	CMeshEditor	cMeshEditor;
	CMFRet		r;
	int			i;
	int			iCorner1;
	int			iCorner2;
	int			iCorner3;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	for (i = 0; i < 4; i++)
	{
		cMeshEditor.AddCorner((float32)i, 0, 0);
	}

	r = cMeshEditor.AddQuad(0, 1, 2, 3, 5, true);
	AssertInt(2, r.NumFaces());
	AssertInt(5, cMesh.GetFaceName(0));
	AssertInt(5, cMesh.GetFaceName(1));

	r.Unpack1(&iCorner1, &iCorner2, &iCorner3);
	AssertInt(0, iCorner1);
	AssertInt(3, iCorner2);
	AssertInt(1, iCorner3);

	r.Unpack2(&iCorner1, &iCorner2, &iCorner3);
	AssertInt(1, iCorner1);
	AssertInt(3, iCorner2);
	AssertInt(2, iCorner3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivitySaveLoad(void)
{
	CMesh				cMesh;
	CMemoryFile			cMemory;
	CObjectWriter		cWriter;
	CObjectReader		cReader;
	CMeshConnectivity	cLoaded;
	int					i;

	cMesh.Init();
	for (i = 0; i < 4; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(2, 1, 3);

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	AssertTrue(cMesh.GetConnectivity()->Save(&cWriter));
	cMemory.Close();
	cWriter.Kill();

	cMemory.Open(EFM_Read);
	cReader.Init(&cMemory, NULL);
	AssertTrue(cLoaded.Load(&cReader));
	cMemory.Close();
	cReader.Kill();
	cMemory.Kill();

	AssertFalse(cLoaded.IsGenerated());
	AssertLSize(4, cLoaded.NumCorners());
	AssertLSize(2, cLoaded.NumFaces());
	AssertInt(3, cLoaded.GetFaceCorners(1)[2]);
	AssertFalse(cLoaded.IsGenerated());

	AssertLSize(5, cLoaded.NumEdges());
	AssertTrue(cLoaded.IsGenerated());
	AssertCornerFaces(cLoaded.GetCorner(1),  2,  0, 1);
	AssertCornerEdges(cLoaded.GetCorner(1),  3,  0, 1, 3);
	AssertEdgeFaces(cLoaded.GetEdge(1), 0, 1);

	cLoaded.AddFace(0, 3, 1);
	AssertLSize(6, cLoaded.NumEdges());
	AssertCornerFaces(cLoaded.GetCorner(3),  2,  1, 2);

	cLoaded.Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Adding only records, and reading a face's corners or counting corners and faces doesn't need anything generated.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivityGeneratedWhenNeeded(void)
{
	CMesh				cMesh;
	CMeshConnectivity*	pcConn;
	int					i;

	cMesh.Init();
	pcConn = cMesh.GetConnectivity();
	AssertTrue(pcConn->IsGenerated());

	for (i = 0; i < 4; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	AssertFalse(pcConn->IsGenerated());
	AssertInt(4, cMesh.NumCorners());
	AssertInt(1, cMesh.NumFaces());
	AssertInt(2, cMesh.GetFaceCorners(0)[2]);
	AssertFalse(pcConn->IsGenerated());

	AssertInt(3, cMesh.NumEdges());
	AssertTrue(pcConn->IsGenerated());

	cMesh.AddFace(2, 1, 3);
	AssertFalse(pcConn->IsGenerated());
	AssertCornerFaces(cMesh.GetCorner(1),  2,  0, 1);
	AssertTrue(pcConn->IsGenerated());
	AssertEdgeFaces(cMesh.GetEdge(1), 0, 1);
	AssertInt(1, cMesh.GetFace(1)->maiEdge[0]);

	cMesh.ClearConnectivity();
	AssertTrue(pcConn->IsGenerated());
	AssertInt(0, cMesh.NumCorners());
	AssertInt(0, cMesh.NumFaces());
	AssertInt(0, cMesh.NumEdges());

	cMesh.AddCorner(0, 0, 0);
	cMesh.AddCorner(1, 0, 0);
	cMesh.AddCorner(2, 0, 0);
	cMesh.AddFace(0, 1, 2);
	AssertInt(3, cMesh.NumEdges());
}


//////////////////////////////////////////////////////////////////////////
//
// Generating after each face gives the same connectivity as generating once after them all.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivityGeneratedInSteps(void)
{
	CMesh	cSteps;
	CMesh	cOnce;
	int		i;
	int		j;
	int		iFaces[4][3] = {{0, 1, 2}, {2, 1, 3}, {2, 3, 4}, {0, 2, 4}};

	cSteps.Init();
	cOnce.Init();
	for (i = 0; i < 5; i++)
	{
		cSteps.AddCorner((float32)i, 0, 0);
		cOnce.AddCorner((float32)i, 0, 0);
	}
	for (i = 0; i < 4; i++)
	{
		cSteps.AddFace(iFaces[i][0], iFaces[i][1], iFaces[i][2]);
		cOnce.AddFace(iFaces[i][0], iFaces[i][1], iFaces[i][2]);
		cSteps.NumEdges();
	}

	AssertInt(cOnce.NumEdges(), cSteps.NumEdges());
	for (i = 0; i < cOnce.NumEdges(); i++)
	{
		AssertEdgeCorners(cSteps.GetEdge(i), cOnce.GetEdge(i)->maiCorner[0], cOnce.GetEdge(i)->maiCorner[1]);
		AssertEdgeFaces(cSteps.GetEdge(i), cOnce.GetEdge(i)->maiFace[0], cOnce.GetEdge(i)->maiFace[1]);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < NUM_FACE_EDGES; j++)
		{
			AssertInt(cOnce.GetFace(i)->maiEdge[j], cSteps.GetFace(i)->maiEdge[j]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The editor still says how many edges each face added, and faces added directly to the mesh aren't counted as its own.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorNewEdges(void)
{
	CMesh		cMesh;
	CMeshEditor	cMeshEditor;
	int			i;

	cMesh.Init();
	cMeshEditor.Init(&cMesh);
	for (i = 0; i < 5; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	AssertInt(2, cMeshEditor.AddFace(2, 1, 3).NewEdges());
	AssertInt(2, cMeshEditor.AddFace(3, 1, 4).NewEdges());
	AssertInt(7, cMesh.NumEdges());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshCutAddCorners(CMesh* pcMesh, float32* afXYZ, int iNumCorners)
{
	int		i;

	for (i = 0; i < iNumCorners; i++)
	{
		pcMesh->AddCorner(afXYZ[i * 3], afXYZ[i * 3 + 1], afXYZ[i * 3 + 2]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A face's normal, twice its area long.
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshCutFaceNormal(SFloat64Vec3* psNormal, CMesh* pcMesh, int iFace)
{
	int*			aiCorners;
	SFloat32Vec3*	ps1;
	SFloat32Vec3*	ps2;
	SFloat32Vec3*	ps3;
	SFloat64Vec3	s12;
	SFloat64Vec3	s13;

	aiCorners = pcMesh->GetFaceCorners(iFace);
	ps1 = pcMesh->GetPosition(aiCorners[0]);
	ps2 = pcMesh->GetPosition(aiCorners[1]);
	ps3 = pcMesh->GetPosition(aiCorners[2]);
	s12.Init((float64)ps2->x - ps1->x, (float64)ps2->y - ps1->y, (float64)ps2->z - ps1->z);
	s13.Init((float64)ps3->x - ps1->x, (float64)ps3->y - ps1->y, (float64)ps3->z - ps1->z);
	Float64Vec3Cross(psNormal, &s12, &s13);
}


//////////////////////////////////////////////////////////////////////////
//
// Each of a face's corners is the mesh's corner its place in aiQuad says.
//
//////////////////////////////////////////////////////////////////////////
static void AssertMeshCutFace(CMesh* pcMesh, int iFace, int* aiQuad, int iQuad1, int iQuad2, int iQuad3)
{
	int*	aiCorners;

	aiCorners = pcMesh->GetFaceCorners(iFace);
	AssertInt(aiQuad[iQuad1], aiCorners[0]);
	AssertInt(aiQuad[iQuad2], aiCorners[1]);
	AssertInt(aiQuad[iQuad3], aiCorners[2]);
}


//////////////////////////////////////////////////////////////////////////
//
// What's returned for a quad says the corners its two faces have.
//
//////////////////////////////////////////////////////////////////////////
static void AssertMeshCutQuadReturn(CMesh* pcMesh, CMFRet r, int* aiQuad)
{
	int		i1;
	int		i2;
	int		i3;

	AssertInt(2, r.NumFaces());
	r.Unpack1(&i1, &i2, &i3);
	AssertMeshCutFace(pcMesh, r.miFirstFace, aiQuad, i1, i2, i3);
	r.Unpack2(&i1, &i2, &i3);
	AssertMeshCutFace(pcMesh, r.miFirstFace + 1, aiQuad, i1, i2, i3);
}


//////////////////////////////////////////////////////////////////////////
//
// A flat convex quad is cut from its second corner to its fourth, as it
// always was.  A bent one is cut the way it folds least, and a concave one
// through its inward corner, so both of its faces face as it does.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshAddQuadCuts(void)
{
	CMesh			cMesh;
	CMFRet			r;
	SFloat64Vec3	sNormal;
	int				aiQuad[4] = { 0, 1, 2, 3 };
	float32			afSquare[] = { 0, 0, 0,  1, 0, 0,  1, 1, 0,  0, 1, 0 };
	float32			afBent[] = { 0, 0, 0,  1, 0, 0.5f,  1, 1, 0,  0, 1, 0 };
	float32			afInwardFirst[] = { 1, 1, 0,  2, 0, 0,  1, 3, 0,  0, 0, 0 };
	float32			afInwardSecond[] = { 0, 0, 0,  1, 1, 0,  2, 0, 0,  1, 3, 0 };

	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afSquare, 4);
	r = cMesh.AddQuad(0, 1, 2, 3);
	AssertMeshCutQuadReturn(&cMesh, r, aiQuad);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 1, 3);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 1, 2, 3);
	cMesh.Kill();

	//Corner 1 is lifted, so cutting from corner 0 to 2 leaves the other face flat.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afBent, 4);
	r = cMesh.AddQuad(0, 1, 2, 3);
	AssertMeshCutQuadReturn(&cMesh, r, aiQuad);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 1, 2);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 0, 2, 3);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 1);
	AssertDouble(0.0, sNormal.x, 6);
	AssertDouble(0.0, sNormal.y, 6);
	cMesh.Kill();

	//Corner 0 points inwards, so the cut from 1 to 3 would make a face outside the quad.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afInwardFirst, 4);
	r = cMesh.AddQuad(0, 1, 2, 3);
	AssertMeshCutQuadReturn(&cMesh, r, aiQuad);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 1, 2);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 0, 2, 3);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 0);
	AssertTrue(sNormal.z > 0.0);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 1);
	AssertTrue(sNormal.z > 0.0);
	cMesh.Kill();

	//Corner 1 points inwards, so the usual cut goes through it.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afInwardSecond, 4);
	r = cMesh.AddQuad(0, 1, 2, 3);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 1, 3);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 1, 2, 3);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 0);
	AssertTrue(sNormal.z > 0.0);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 1);
	AssertTrue(sNormal.z > 0.0);
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An editor and a wrapper on a bare mesh cut a bent quad the same way,
// flipped or not, and say so in what they return.  The editor's quad is one
// polygon.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEditorAddQuadCuts(void)
{
	CMesh				cMesh;
	CMeshEditor			cEditor;
	CMeshEditorWrapper	cWrapper;
	Ptr<CMesh>			pMesh;
	CMFRet				r;
	SFloat64Vec3		sNormal;
	int					aiQuad[4] = { 0, 1, 2, 3 };
	float32				afBent[] = { 0, 0, 0,  1, 0, 0.5f,  1, 1, 0,  0, 1, 0 };

	cMesh.Init();
	cEditor.Init(&cMesh);
	TestMeshCutAddCorners(&cMesh, afBent, 4);
	r = cEditor.AddQuad(0, 1, 2, 3, 7, false);
	AssertMeshCutQuadReturn(&cMesh, r, aiQuad);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 1, 2);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 0, 2, 3);
	AssertLSize(1, cMesh.mcPolygons.mcPolygons.NumElements());
	cEditor.Kill();
	cMesh.Kill();

	cMesh.Init();
	cEditor.Init(&cMesh);
	TestMeshCutAddCorners(&cMesh, afBent, 4);
	r = cEditor.AddQuad(0, 1, 2, 3, 7, true);
	AssertMeshCutQuadReturn(&cMesh, r, aiQuad);
	AssertMeshCutFace(&cMesh, 0, aiQuad, 0, 2, 1);
	AssertMeshCutFace(&cMesh, 1, aiQuad, 0, 3, 2);
	TestMeshCutFaceNormal(&sNormal, &cMesh, 1);
	AssertTrue(sNormal.z < 0.0);
	AssertLSize(1, cMesh.mcPolygons.mcPolygons.NumElements());
	cEditor.Kill();
	cMesh.Kill();

	pMesh = OMalloc<CMesh>();
	TestMeshCutAddCorners(&pMesh, afBent, 4);
	cWrapper.Init(pMesh);
	r = cWrapper.AddQuad(0, 1, 2, 3, 0, true);
	AssertMeshCutQuadReturn(&pMesh, r, aiQuad);
	TestMeshCutFaceNormal(&sNormal, &pMesh, 0);
	AssertTrue(sNormal.z < 0.0);
	TestMeshCutFaceNormal(&sNormal, &pMesh, 1);
	AssertTrue(sNormal.z < 0.0);
	cWrapper.Kill();
	pMesh = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A polygon of five or more corners is cut into faces where it was dropped:
// a flat one into faces that cover it facing its way, a concave one too, and
// a bent one with its bend in one face.  In an editor its faces are one
// polygon with only its outline visible, and flipped they face the other
// way.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshAddPolygonCuts(void)
{
	CMesh				cMesh;
	CMeshEditor			cEditor;
	CMeshEditorWrapper	cWrapper;
	Ptr<CMesh>			pMesh;
	CMFRet				r;
	CArrayInt			aiCorners;
	CArrayInt			aiCorners6;
	SFloat64Vec3		sNormal;
	float64				fArea;
	int					i;
	int					j;
	int					iFacesOnLifted;
	int*				aiFaceCorners;
	bool				abVisible[NUM_FACE_EDGES];
	int					iVisible;
	float32				afPentagon[] = { 2, 0, 0,  4, 1, 0,  3, 3, 0,  1, 3, 0,  0, 1, 0 };
	float32				afL[] = { 0, 0, 0,  2, 0, 0,  2, 1, 0,  1, 1, 0,  1, 2, 0,  0, 2, 0 };
	float32				afBent[] = { 2, 0, 0,  4, 1, 0,  3, 3, 1.5f,  1, 3, 0,  0, 1, 0 };

	aiCorners.Init();
	aiCorners6.Init();
	for (i = 0; i < 6; i++)
	{
		if (i < 5)
		{
			aiCorners.Add(i);
		}
		aiCorners6.Add(i);
	}

	//Flat and convex, area 8.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afPentagon, 5);
	r = cMesh.AddPolygon(&aiCorners);
	AssertInt(3, r.NumFaces());
	AssertInt((int32)0, r.miFirstFace);
	AssertInt(3, cMesh.NumFaces());
	fArea = 0.0;
	for (i = 0; i < 3; i++)
	{
		TestMeshCutFaceNormal(&sNormal, &cMesh, i);
		AssertTrue(sNormal.z > 0.0);
		fArea += sNormal.z / 2.0;
	}
	AssertDouble(8.0, fArea, 6);
	cMesh.Kill();

	//An L, concave at corner 3, area 3.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afL, 6);
	r = cMesh.AddPolygon(&aiCorners6);
	AssertInt(4, cMesh.NumFaces());
	fArea = 0.0;
	for (i = 0; i < 4; i++)
	{
		TestMeshCutFaceNormal(&sNormal, &cMesh, i);
		AssertTrue(sNormal.z > 0.0);
		fArea += sNormal.z / 2.0;
	}
	AssertDouble(3.0, fArea, 6);
	cMesh.Kill();

	//Corner 2 is lifted.  Only one face reaches it, and the others are flat.
	cMesh.Init();
	TestMeshCutAddCorners(&cMesh, afBent, 5);
	cMesh.AddPolygon(&aiCorners);
	iFacesOnLifted = 0;
	for (i = 0; i < 3; i++)
	{
		aiFaceCorners = cMesh.GetFaceCorners(i);
		if ((aiFaceCorners[0] == 2) || (aiFaceCorners[1] == 2) || (aiFaceCorners[2] == 2))
		{
			iFacesOnLifted++;
		}
		else
		{
			TestMeshCutFaceNormal(&sNormal, &cMesh, i);
			AssertDouble(0.0, sNormal.x, 6);
			AssertDouble(0.0, sNormal.y, 6);
			AssertTrue(sNormal.z > 0.0);
		}
	}
	AssertInt(1, iFacesOnLifted);
	cMesh.Kill();

	cMesh.Init();
	cEditor.Init(&cMesh);
	TestMeshCutAddCorners(&cMesh, afPentagon, 5);
	r = cEditor.AddPolygon(&aiCorners, 4, false);
	AssertInt(3, r.NumFaces());
	AssertInt(3, cMesh.NumFaces());
	AssertLSize(1, cMesh.mcPolygons.mcPolygons.NumElements());
	AssertSize(3, cMesh.mcPolygons.Get(0)->maiFaces.NumElements());
	iVisible = 0;
	for (i = 0; i < 3; i++)
	{
		AssertInt(4, cMesh.GetFaceName(i));
		cMesh.mcPolygons.GetFaceEdges(cMesh.GetConnectivity(), i, abVisible);
		for (j = 0; j < NUM_FACE_EDGES; j++)
		{
			iVisible += abVisible[j] ? 1 : 0;
		}
	}
	AssertInt(5, iVisible);

	cEditor.Kill();
	cMesh.Kill();

	cMesh.Init();
	cEditor.Init(&cMesh);
	TestMeshCutAddCorners(&cMesh, afPentagon, 5);
	r = cEditor.AddPolygon(&aiCorners, 4, true);
	AssertInt((int32)0, r.miFirstFace);
	AssertLSize(1, cMesh.mcPolygons.mcPolygons.NumElements());
	for (i = 0; i < 3; i++)
	{
		TestMeshCutFaceNormal(&sNormal, &cMesh, i);
		AssertTrue(sNormal.z < 0.0);
	}
	cEditor.Kill();
	cMesh.Kill();

	pMesh = OMalloc<CMesh>();
	TestMeshCutAddCorners(&pMesh, afPentagon, 5);
	cWrapper.Init(pMesh);
	r = cWrapper.AddPolygon(&aiCorners, 0, true);
	AssertInt(3, pMesh->NumFaces());
	for (i = 0; i < 3; i++)
	{
		TestMeshCutFaceNormal(&sNormal, &pMesh, i);
		AssertTrue(sNormal.z < 0.0);
	}
	cWrapper.Kill();
	pMesh = NULL;

	aiCorners6.Kill();
	aiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshConnectivity(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshEdgeConnectivity();
	TestMeshEditorAddFlippedQuad();
	TestMeshConnectivitySaveLoad();
	TestMeshConnectivityGeneratedWhenNeeded();
	TestMeshConnectivityGeneratedInSteps();
	TestMeshEditorNewEdges();
	TestMeshAddQuadCuts();
	TestMeshEditorAddQuadCuts();
	TestMeshAddPolygonCuts();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

