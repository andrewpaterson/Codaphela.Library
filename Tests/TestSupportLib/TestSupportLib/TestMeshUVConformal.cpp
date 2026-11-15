#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshUVConformal.h"
#include "SupportLib/MeshUVMeasures.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A flat chart, a 4 by 4 grid of squares a quarter of a unit across on
// a tilted plane, started from its UVs skewed, squeezed and bent: by
// LSCM it's its own shape again, at 10 texels to the unit, anticlockwise.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVConformalFlat(void)
{
	SFloat32Vec3	asPositions[25];
	SFloat32Vec2	asUVs[25];
	SFloat32Vec3	sAcross;
	SFloat32Vec3	sUp;
	SFloat32Vec3	sOrigin;
	SFloat32Vec2	sDiagonal;
	SMeshUVStretch	sStretch;
	int				aiCorners[16 * 2 * 3];
	int				i;
	int				j;
	int				iCorner;
	int*			piCorner;

	sAcross.Init(0.6f, 0, 0.8f);
	sUp.Init(0, 1, 0);
	sOrigin.Init(0.3f, -0.2f, 1.1f);
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < 5; i++)
		{
			iCorner = (j * 5) + i;
			asPositions[iCorner] = sOrigin + (sAcross * (0.25f * i)) + (sUp * (0.25f * j));
			asUVs[iCorner].Init((0.5f * i) + (0.3f * j), (1.5f * j) + (0.05f * i * j));
		}
	}
	piCorner = aiCorners;
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 4; i++)
		{
			iCorner = (j * 5) + i;
			piCorner[0] = iCorner;
			piCorner[1] = iCorner + 1;
			piCorner[2] = iCorner + 6;
			piCorner[3] = iCorner;
			piCorner[4] = iCorner + 6;
			piCorner[5] = iCorner + 5;
			piCorner += 6;
		}
	}

	MeasureUVTriangleStretch(&sStretch, asUVs, asPositions, aiCorners, 32, 10.0f);
	AssertTrue(sStretch.fWorstAngle > 2.0f);

	AssertTrue(FlattenConformal(asUVs, asPositions, 25, aiCorners, 32, 10.0f));
	AssertInt(0, CountUVTrianglesTurned(asUVs, aiCorners, 32));
	MeasureUVTriangleStretch(&sStretch, asUVs, asPositions, aiCorners, 32, 10.0f);
	AssertInt(32, sStretch.iNumFaces);
	AssertFloat(1.0f, sStretch.fWorstAngle, 3);
	AssertFloat(1.0f, sStretch.fLeastArea, 3);
	AssertFloat(1.0f, sStretch.fMostArea, 3);
	sDiagonal = asUVs[24] - asUVs[0];
	AssertFloat(10.0f * 1.4142136f, sDiagonal.Magnitude(), 3);
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere's upper half, started from its UVs straight down (stretched
// near its rim, where its faces stand up): by LSCM a disc, nothing turned
// over or overlapping, its area the hemisphere's.  Its angles are nearly
// kept (up to 1.23 times, 1.11 on average, as its triangles are coarse
// and those round the pole thin), and so its areas aren't: a conformal
// map of a hemisphere onto a disc is four times as dense at its rim as at
// its pole (it's 0.51 to 2.04 times the density).
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVConformalHemisphere(void)
{
	Ptr<CMeshEditor>		pEditor;
	Ptr<CMesh>				pMesh;
	CArrayLargeInt			aiCornerUVs;
	CArrayLargeInt			aiCorners;
	CArrayLargeFloat32Vec3	asPositions;
	CArrayLargeFloat32Vec2	asUVs;
	SFloat32Vec3			sAxis;
	SFloat32Vec3			sCentre;
	SFloat32Vec3*			psPosition;
	SMeshUVStretch			sStretch;
	int*					piCorners;
	int						iFace;
	int						iNumTriangles;
	int						k;
	bool					bUpper;
	float64					fArea;
	float64					fUVArea;
	SFloat32Vec2			sSide1;
	SFloat32Vec2			sSide2;
	SFloat32Vec2*			pasUVs;
	SFloat32Vec3			sEdge1;
	SFloat32Vec3			sEdge2;
	SFloat32Vec3			sNormal;

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.3f, -0.2f, 0.1f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, 24, 12);
	pMesh = pEditor->GetOutput();

	aiCornerUVs.Init();
	aiCornerUVs.Resize((lsize)pMesh->NumCorners());
	aiCornerUVs.SetArrayValues(-1);
	aiCorners.Init();
	asPositions.Init();
	asUVs.Init();
	iNumTriangles = 0;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		bUpper = true;
		for (k = 0; k < 3; k++)
		{
			bUpper &= (pMesh->GetPosition(piCorners[k])->z >= sCentre.z - 0.0001f);
		}
		if (!bUpper)
		{
			continue;
		}
		for (k = 0; k < 3; k++)
		{
			if (aiCornerUVs.GetValue((lsize)piCorners[k]) == -1)
			{
				aiCornerUVs.SetValue((lsize)piCorners[k], (int)asUVs.NumElements());
				psPosition = pMesh->GetPosition(piCorners[k]);
				asPositions.Add(psPosition);
				asUVs.Add()->Init(psPosition->x * 20.0f, psPosition->y * 20.0f);
			}
			aiCorners.Add(aiCornerUVs.GetValue((lsize)piCorners[k]));
		}
		iNumTriangles++;
	}
	AssertInt(24 + (24 * 5 * 2), iNumTriangles);  //A fan round the pole and five rings of squares.

	MeasureUVTriangleStretch(&sStretch, asUVs.Get(0), asPositions.Get(0), aiCorners.Get(0), iNumTriangles, 20.0f);
	AssertTrue(sStretch.fWorstAngle > 3.0f);

	AssertTrue(FlattenConformal(asUVs.Get(0), asPositions.Get(0), (int)asUVs.NumElements(), aiCorners.Get(0), iNumTriangles, 20.0f));
	AssertInt(0, CountUVTrianglesTurned(asUVs.Get(0), aiCorners.Get(0), iNumTriangles));
	AssertInt(0, CountUVTriangleOverlaps(asUVs.Get(0), aiCorners.Get(0), iNumTriangles));
	MeasureUVTriangleStretch(&sStretch, asUVs.Get(0), asPositions.Get(0), aiCorners.Get(0), iNumTriangles, 20.0f);
	AssertTrue(sStretch.fWorstAngle < 1.3f);
	AssertTrue(sStretch.fMeanAngle < 1.15f);
	AssertTrue(sStretch.fMostArea > 1.5f);

	fUVArea = 0.0;
	fArea = 0.0;
	pasUVs = asUVs.Get(0);
	piCorners = aiCorners.Get(0);
	for (iFace = 0; iFace < iNumTriangles; iFace++)
	{
		sSide1 = pasUVs[piCorners[(iFace * 3) + 1]] - pasUVs[piCorners[iFace * 3]];
		sSide2 = pasUVs[piCorners[(iFace * 3) + 2]] - pasUVs[piCorners[iFace * 3]];
		fUVArea += Float32Vec2Cross(&sSide1, &sSide2) * 0.5;
		sEdge1 = *asPositions.Get((lsize)piCorners[(iFace * 3) + 1]) - *asPositions.Get((lsize)piCorners[iFace * 3]);
		sEdge2 = *asPositions.Get((lsize)piCorners[(iFace * 3) + 2]) - *asPositions.Get((lsize)piCorners[iFace * 3]);
		Float32Vec3Cross(&sNormal, &sEdge1, &sEdge2);
		fArea += sNormal.Magnitude() * 0.5;
	}
	AssertFloat(1.0f, (float32)(fUVArea / (fArea * 400.0)), 3);

	asUVs.Kill();
	asPositions.Kill();
	aiCorners.Kill();
	aiCornerUVs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUVConformal(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshUVConformalFlat();
	TestMeshUVConformalHemisphere();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

