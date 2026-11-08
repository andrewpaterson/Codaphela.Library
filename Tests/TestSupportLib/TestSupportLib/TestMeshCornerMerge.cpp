#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshSimplify.h"
#include "TestLib/Assert.h"


#define TEST_MESH_SIMPLIFY_COLOUR_GREY	0
#define TEST_MESH_SIMPLIFY_COLOUR_DARK	1
#define TEST_MESH_SIMPLIFY_COLOUR_GREY2	2  //The same value as grey.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshSimplifyNewMesh(void)
{
	Ptr<CMesh>	pMesh;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF808080);
	pMesh->mcColours.mcColours.Add(0xFF404040);
	pMesh->mcColours.mcColours.Add(0xFF808080);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshSimplifyAddFace(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3, int iColour)
{
	CMFRet	r;
	int		i;

	r = pcMesh->AddFace(iCorner1, iCorner2, iCorner3);
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		pcMesh->mcColours.mcFaces.Get(r.miFirstFace)->aiCornerColours[i] = iColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshSimplifyOutput(CMeshModifierStack* pcStack, Ptr<CMesh> pSource, float32 fCellSize)
{
	CMeshSimplify*	pcSimplify;

	pcStack->Init(pSource);
	pcSimplify = pcStack->AddModifier<CMeshSimplify>();
	pcSimplify->Init(fCellSize);
	return pcStack->GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
// A 30 x 30 grid of squares, each 10 units and two faces, simplified to
// cells of 100: its corners stay about where they were, with no holes, and
// nothing changes in the source.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplifyGrid(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psA;
	SFloat32Vec3*		psB;
	SFloat32Vec3*		psC;
	SFloat32Vec3*		psNormal;
	int*				piCorners;
	bool				abUsed[16];
	float64				fArea;
	float64				fFaceArea;
	int					x;
	int					y;
	int					i;
	int					k;

	pSource = TestMeshSimplifyNewMesh();
	for (y = 0; y <= 30; y++)
	{
		for (x = 0; x <= 30; x++)
		{
			pSource->AddCorner((float32)(x * 10), (float32)(y * 10), 5);
		}
	}
	for (y = 0; y < 30; y++)
	{
		for (x = 0; x < 30; x++)
		{
			TestMeshSimplifyAddFace(&pSource, y * 31 + x, y * 31 + x + 1, y * 31 + x + 32, TEST_MESH_SIMPLIFY_COLOUR_GREY);
			TestMeshSimplifyAddFace(&pSource, y * 31 + x, y * 31 + x + 32, y * 31 + x + 31, TEST_MESH_SIMPLIFY_COLOUR_GREY);
		}
	}

	pOutput = TestMeshSimplifyOutput(&cStack, pSource, 100);

	AssertInt(961, pSource->NumCorners());
	AssertInt(1800, pSource->NumFaces());
	AssertFloat(10, pSource->GetPosition(1)->x, 3);
	AssertFalse(pSource->GetConnectivity()->IsGenerated());

	//4 x 4 cells at most; each square left is two faces.
	AssertTrue(pOutput->NumCorners() <= 16);
	AssertTrue(pOutput->NumFaces() <= 18);
	AssertTrue(pOutput->NumFaces() >= 9);
	AssertFalse(pOutput->GetConnectivity()->IsGenerated());

	for (i = 0; i < 16; i++)
	{
		abUsed[i] = false;
	}

	//Every face faces up, they still cover the grid but for up to about half a cell around its edge, and their corner
	//normals are up too.
	fArea = 0.0;
	for (i = 0; i < pOutput->NumFaces(); i++)
	{
		piCorners = pOutput->GetFaceCorners(i);
		psA = pOutput->GetPosition(piCorners[0]);
		psB = pOutput->GetPosition(piCorners[1]);
		psC = pOutput->GetPosition(piCorners[2]);
		fFaceArea = ((float64)psB->x - psA->x) * ((float64)psC->y - psA->y) - ((float64)psB->y - psA->y) * ((float64)psC->x - psA->x);
		AssertTrue(fFaceArea > 0.0);
		fArea += fFaceArea / 2.0;

		psNormalFace = pOutput->mcNormals.mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			abUsed[piCorners[k]] = true;
			AssertTrue(psNormalFace->aiCornerNormals[k] >= pOutput->NumFaces());
			psNormal = pOutput->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[k]);
			AssertFloat(1.0f, psNormal->z, 3);
		}
		AssertInt(TEST_MESH_SIMPLIFY_COLOUR_GREY, pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[0]);
	}
	AssertTrue(fArea >= 200.0 * 200.0);
	AssertTrue(fArea <= 300.0 * 300.0);

	//Corners stay in the box and every one is used.
	for (i = 0; i < pOutput->NumCorners(); i++)
	{
		AssertTrue(abUsed[i]);
		psA = pOutput->GetPosition(i);
		AssertTrue((psA->x >= 0.0f) && (psA->x <= 300.0f));
		AssertFloat(5, psA->z, 3);
	}

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A sharp edge stays sharp: two faces meeting in one cell share a corner but
// keep their own normals.  Repeated faces go; ones wound the other way or
// of another colour don't.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplifyKeepsEdges(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psNormal;
	int					i;
	int					iNormal;
	int					iCorner;
	bool				bUp;
	bool				bSide;

	pSource = TestMeshSimplifyNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(1000, 0, 0);
	pSource->AddCorner(0, 1000, 0);
	pSource->AddCorner(1, 1, 0);  //In corner 0's cell.
	pSource->AddCorner(1000, 1000, 0);

	TestMeshSimplifyAddFace(&pSource, 0, 1, 2, TEST_MESH_SIMPLIFY_COLOUR_GREY);
	TestMeshSimplifyAddFace(&pSource, 1, 2, 0, TEST_MESH_SIMPLIFY_COLOUR_GREY);  //The same, starting elsewhere.
	TestMeshSimplifyAddFace(&pSource, 0, 1, 2, TEST_MESH_SIMPLIFY_COLOUR_GREY2);  //The same, in another of the colour.
	TestMeshSimplifyAddFace(&pSource, 0, 2, 1, TEST_MESH_SIMPLIFY_COLOUR_GREY);  //Facing the other way.
	TestMeshSimplifyAddFace(&pSource, 0, 1, 2, TEST_MESH_SIMPLIFY_COLOUR_DARK);  //Another colour.
	TestMeshSimplifyAddFace(&pSource, 3, 1, 4, TEST_MESH_SIMPLIFY_COLOUR_GREY);

	pSource->AddNormal(0, 0, 1);
	pSource->AddNormal(1, 0, 0);
	for (i = 0; i < 6; i++)
	{
		psNormalFace = pSource->mcNormals.mcFaces.Get(i);
		iNormal = (i == 5) ? 7 : 6;
		psNormalFace->aiCornerNormals[0] = iNormal;
		psNormalFace->aiCornerNormals[1] = iNormal;
		psNormalFace->aiCornerNormals[2] = iNormal;
	}

	pSource->SetFaceName(3, 8);

	pOutput = TestMeshSimplifyOutput(&cStack, pSource, 100);

	AssertInt(4, pOutput->NumFaces());
	AssertInt(4, pOutput->NumCorners());
	AssertInt(0, pOutput->GetFaceName(0));
	AssertInt(8, pOutput->GetFaceName(1));  //Faces 1 and 2 were repeats of 0.

	bUp = false;
	bSide = false;
	for (i = 0; i < pOutput->NumFaces(); i++)
	{
		psNormalFace = pOutput->mcNormals.mcFaces.Get(i);
		iCorner = pOutput->GetFaceCorners(i)[0];
		if (pOutput->GetPosition(iCorner)->x < 1.0f)
		{
			psNormal = pOutput->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[0]);
			bUp |= psNormal->z > 0.99f;
			bSide |= psNormal->x > 0.99f;
		}
	}
	AssertTrue(bUp);
	AssertTrue(bSide);

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two layers a hair apart, of different colours as a board's copper and
// solder mask are, stay that hair apart.  The same two in one colour merge.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplifyKeepsLayers(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	int					iLayers;
	int					i;
	float32				fLow;
	float32				fHigh;

	for (iLayers = 0; iLayers < 2; iLayers++)
	{
		pSource = TestMeshSimplifyNewMesh();
		pSource->AddCorner(0, 0, 0);
		pSource->AddCorner(1000, 0, 0);
		pSource->AddCorner(0, 1000, 0);
		pSource->AddCorner(0, 0, 2);
		pSource->AddCorner(1000, 0, 2);
		pSource->AddCorner(0, 1000, 2);
		TestMeshSimplifyAddFace(&pSource, 0, 1, 2, TEST_MESH_SIMPLIFY_COLOUR_GREY);
		TestMeshSimplifyAddFace(&pSource, 3, 4, 5, (iLayers == 0) ? TEST_MESH_SIMPLIFY_COLOUR_DARK : TEST_MESH_SIMPLIFY_COLOUR_GREY2);

		pOutput = TestMeshSimplifyOutput(&cStack, pSource, 100);

		fLow = pOutput->GetPosition(0)->z;
		fHigh = fLow;
		for (i = 1; i < pOutput->NumCorners(); i++)
		{
			fLow = (pOutput->GetPosition(i)->z < fLow) ? pOutput->GetPosition(i)->z : fLow;
			fHigh = (pOutput->GetPosition(i)->z > fHigh) ? pOutput->GetPosition(i)->z : fHigh;
		}
		if (iLayers == 0)
		{
			AssertInt(2, pOutput->NumFaces());
			AssertFloat(0, fLow, 3);
			AssertFloat(2, fHigh, 3);
		}
		else
		{
			AssertInt(1, pOutput->NumFaces());
			AssertFloat(1, fLow, 3);
			AssertFloat(1, fHigh, 3);
		}
		cStack.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each time starts from the source, so the output is the same every time.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplifyAgain(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pFirst;
	Ptr<CMesh>			pSecond;
	int					i;

	pSource = TestMeshSimplifyNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(90, 0, 0);
	pSource->AddCorner(110, 0, 0);
	pSource->AddCorner(0, 110, 0);
	TestMeshSimplifyAddFace(&pSource, 0, 2, 3, TEST_MESH_SIMPLIFY_COLOUR_GREY);
	TestMeshSimplifyAddFace(&pSource, 1, 2, 3, TEST_MESH_SIMPLIFY_COLOUR_GREY);

	pFirst = TestMeshSimplifyOutput(&cStack, pSource, 100);
	cStack.Touch();
	pSecond = cStack.GetOutput();

	AssertFalse(pFirst == pSecond);
	AssertInt(pFirst->NumCorners(), pSecond->NumCorners());
	for (i = 0; i < pFirst->NumCorners(); i++)
	{
		AssertFloat(pFirst->GetPosition(i)->x, pSecond->GetPosition(i)->x, 3);
	}
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With no faces, or what a new mesh wouldn't keep, the mesh itself comes back.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplifyLeavesMesh(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	SLogConfig			sLogConfig;

	pSource = TestMeshSimplifyNewMesh();
	AssertTrue(TestMeshSimplifyOutput(&cStack, pSource, 100) == pSource);
	cStack.Kill();

	pSource = TestMeshSimplifyNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(100, 0, 0);
	pSource->AddCorner(0, 100, 0);
	TestMeshSimplifyAddFace(&pSource, 0, 1, 2, TEST_MESH_SIMPLIFY_COLOUR_GREY);
	pSource->mcSkin.mbInUse = true;
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(TestMeshSimplifyOutput(&cStack, pSource, 100) == pSource);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSimplify(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshSimplifyGrid();
	TestMeshSimplifyKeepsEdges();
	TestMeshSimplifyKeepsLayers();
	TestMeshSimplifyAgain();
	TestMeshSimplifyLeavesMesh();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
