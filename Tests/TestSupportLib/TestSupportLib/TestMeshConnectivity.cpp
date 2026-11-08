#include "BaseLib/GlobalDataTypesIO.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
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
	AssertSize(9, pcConn->mcEdges.NumElements());
	AssertSize(6, pcConn->mcCorners.NumElements());
	AssertSize(6, pcConn->mcCornerEdgesMap.NumElements());
	AssertSize(4, pcConn->mcFaces.NumElements());

	AssertCornerFaces(pcConn->mcCorners.Get(0),  1,  0);
	AssertCornerFaces(pcConn->mcCorners.Get(1),  2,  0, 1);
	AssertCornerFaces(pcConn->mcCorners.Get(2),  3,  0, 1, 2);
	AssertCornerFaces(pcConn->mcCorners.Get(3),  3,  1, 2, 3);
	AssertCornerFaces(pcConn->mcCorners.Get(4),  2,  2, 3);
	AssertCornerFaces(pcConn->mcCorners.Get(5),  1,  3);

	AssertCornerEdges(pcConn->mcCorners.Get(0),  2,  0, 2);
	AssertCornerEdges(pcConn->mcCorners.Get(1),  3,  0, 1, 3);
	AssertCornerEdges(pcConn->mcCorners.Get(2),  4,  1, 2, 4, 6);
	AssertCornerEdges(pcConn->mcCorners.Get(3),  4,  3, 4, 5, 7);
	AssertCornerEdges(pcConn->mcCorners.Get(4),  3,  5, 6, 8);
	AssertCornerEdges(pcConn->mcCorners.Get(5),  2,  7, 8);

	for (i = 0; i < pcConn->mcCornerEdgesMap.NumElements(); i++)
	{
		pcCornerEdgesMap = pcConn->mcCornerEdgesMap.Get(i);
		AssertSize(pcCornerEdgesMap->maiEdgeIndex.NumElements(), pcCornerEdgesMap->maiLargerCornerIndex.NumElements());
	}

	AssertEdgeCorners(pcConn->mcEdges.Get(0), 0, 1);
	AssertEdgeCorners(pcConn->mcEdges.Get(1), 1, 2);
	AssertEdgeCorners(pcConn->mcEdges.Get(2), 0, 2);
	AssertEdgeCorners(pcConn->mcEdges.Get(3), 1, 3);
	AssertEdgeCorners(pcConn->mcEdges.Get(4), 2, 3);
	AssertEdgeCorners(pcConn->mcEdges.Get(5), 3, 4);
	AssertEdgeCorners(pcConn->mcEdges.Get(6), 2, 4);
	AssertEdgeCorners(pcConn->mcEdges.Get(7), 3, 5);
	AssertEdgeCorners(pcConn->mcEdges.Get(8), 4, 5);

	AssertEdgeFaces(pcConn->mcEdges.Get(0), 0, -1);
	AssertEdgeFaces(pcConn->mcEdges.Get(1), 0, 1);
	AssertEdgeFaces(pcConn->mcEdges.Get(2), 0, -1);
	AssertEdgeFaces(pcConn->mcEdges.Get(3), 1, -1);
	AssertEdgeFaces(pcConn->mcEdges.Get(4), 1, 2);
	AssertEdgeFaces(pcConn->mcEdges.Get(5), 2, 3);
	AssertEdgeFaces(pcConn->mcEdges.Get(6), 2, -1);
	AssertEdgeFaces(pcConn->mcEdges.Get(7), 3, -1);
	AssertEdgeFaces(pcConn->mcEdges.Get(8), 3, -1);
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
// A face's matrix count is the most of any of its three corners and reading it leaves the corners as they were.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFaceTypesFromSkin(void)
{
	CMesh			cMesh;
	CMeshFace*		pcFace;
	SSkinWeight*	psWeight;
	int				i;

	cMesh.Init();
	cMesh.mcSkin.mbInUse = true;
	for (i = 0; i < 3; i++)
	{
		cMesh.AddCorner((float32)i, 0, 0);
	}
	cMesh.AddFace(0, 1, 2);

	for (i = 0; i < 2; i++)
	{
		psWeight = cMesh.mcSkin.mcSkinVerts.Get(2)->mcWeights.Add();
		psWeight->iMatrix = i;
		psWeight->fWeight = 0.5f;
	}

	cMesh.mcFaceTypes.ClearFaceTypes(1);
	cMesh.mcFaceTypes.GenerateFromSkin(&cMesh);

	pcFace = cMesh.GetFace(0);
	AssertInt(0, pcFace->maiCorner[0]);
	AssertInt(1, pcFace->maiCorner[1]);
	AssertInt(2, pcFace->maiCorner[2]);
	AssertInt(2 << MESH_FACE_MATRIX_SHIFT, (int)cMesh.mcFaceTypes.mcFaces.Get(0)->iFlags1);
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

	r = cMeshEditor.AddQuad(0, 1, 2, 3, 0, true);
	AssertInt(2, r.NumFaces());

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
void TestMeshConnectivity(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshEdgeConnectivity();
	TestMeshFaceTypesFromSkin();
	TestMeshEditorAddFlippedQuad();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

