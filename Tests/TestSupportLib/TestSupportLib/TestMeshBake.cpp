#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/Material.h"
#include "SupportLib/SolidChecker.h"
#include "SupportLib/MeshBake.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A baked image's texel as an ARGB32, opaque.
//
//////////////////////////////////////////////////////////////////////////
static ARGB32 TestBakeTexel(CImage* pcImage, int x, int y)
{
	uint8*	puiData;
	size	uiTexel;

	puiData = (uint8*)pcImage->GetData();
	uiTexel = ((size)y * pcImage->GetWidth()) + (size)x;
	return Set32BitColour(puiData[uiTexel * 3], puiData[(uiTexel * 3) + 1], puiData[(uiTexel * 3) + 2]);
}


//////////////////////////////////////////////////////////////////////////
//
// The cube at the origin's positive corner is the first colour, its
// neighbours along each axis the second.  Averaged over many cubes it's
// half each; over a texel straddling a face between two cubes evenly, half
// each too.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBakeChecker(void)
{
	Ptr<CSolidChecker>	pChecker;
	SFloat32Vec3		sPoint;

	pChecker = OMalloc<CSolidChecker>(0.5f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)200, (uint8)100, (uint8)50));
	sPoint.Init(0.25f, 0.25f, 0.25f);
	AssertFloat(0.0f, pChecker->Mix(&sPoint, 0.0f), 5);
	sPoint.Init(0.75f, 0.25f, 0.25f);
	AssertFloat(1.0f, pChecker->Mix(&sPoint, 0.0f), 5);
	sPoint.Init(0.75f, 0.75f, 0.25f);
	AssertFloat(0.0f, pChecker->Mix(&sPoint, 0.0f), 5);
	sPoint.Init(-0.25f, 0.25f, 0.25f);
	AssertFloat(1.0f, pChecker->Mix(&sPoint, 0.0f), 5);
	sPoint.Init(0.37f, 0.11f, 0.29f);
	AssertFloat(0.5f, pChecker->Mix(&sPoint, 20.0f), 2);
	sPoint.Init(0.5f, 0.25f, 0.25f);
	AssertFloat(0.5f, pChecker->Mix(&sPoint, 0.2f), 5);
	sPoint.Init(0.75f, 0.25f, 0.25f);
	AssertInt(Set32BitColour((uint8)200, (uint8)100, (uint8)50), pChecker->Colour(&sPoint, &sPoint, 0.0001f));
}


//////////////////////////////////////////////////////////////////////////
//
// A unit square at z = 0.25 whose UVs are the middle half of a 16 by 16
// image, texels 4 to 11 each way: a texel there is the checker at its
// centre's point on the square (x = (u - 0.25) * 2), averaged over its
// width (an eighth of the square), and every texel round it is its
// nearest covered texel's.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBakeSquare(void)
{
	Ptr<CSolidChecker>	pChecker;
	Ptr<CImage>			pImage;
	CMesh				cMesh;
	CMeshUVLayer*		pcLayer;
	SMeshUVFace*		psUVFace;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sNormal;
	int					x;
	int					y;
	bool				bSame;

	cMesh.Init();
	cMesh.mcUVs.mbInUse = true;
	cMesh.mcUVs.AddLayer(cMesh.GetConnectivity());
	cMesh.AddCorner(0, 0, 0.25f);
	cMesh.AddCorner(1, 0, 0.25f);
	cMesh.AddCorner(1, 1, 0.25f);
	cMesh.AddCorner(0, 1, 0.25f);
	cMesh.AddUV(0.25f, 0.25f);
	cMesh.AddUV(0.75f, 0.25f);
	cMesh.AddUV(0.75f, 0.75f);
	cMesh.AddUV(0.25f, 0.75f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(0, 2, 3);
	pcLayer = cMesh.mcUVs.mcLayers.Get(0);
	psUVFace = pcLayer->mcFaces.Get(0);
	psUVFace->aiUVs[0] = 0;
	psUVFace->aiUVs[1] = 1;
	psUVFace->aiUVs[2] = 2;
	psUVFace = pcLayer->mcFaces.Get(1);
	psUVFace->aiUVs[0] = 0;
	psUVFace->aiUVs[1] = 2;
	psUVFace->aiUVs[2] = 3;

	pChecker = OMalloc<CSolidChecker>(0.5f, Set32BitColour((uint8)10, (uint8)20, (uint8)30), Set32BitColour((uint8)240, (uint8)200, (uint8)160));
	pImage = BakeMeshTexture(&cMesh, 0, &pChecker, 16, 16);
	AssertNotNull(&pImage);
	AssertInt(16, (int)pImage->GetWidth());
	AssertInt(16, (int)pImage->GetHeight());

	sNormal.Init(0, 0, 1);
	bSame = true;
	for (y = 4; y < 12; y++)
	{
		for (x = 4; x < 12; x++)
		{
			sPoint.Init((((x + 0.5f) / 16.0f) - 0.25f) * 2.0f, (((y + 0.5f) / 16.0f) - 0.25f) * 2.0f, 0.25f);
			bSame &= (TestBakeTexel(&pImage, x, y) == pChecker->Colour(&sPoint, &sNormal, 0.125f));
		}
	}
	AssertTrue(bSame);
	AssertInt(Set32BitColour((uint8)10, (uint8)20, (uint8)30), TestBakeTexel(&pImage, 5, 5));
	AssertInt(Set32BitColour((uint8)240, (uint8)200, (uint8)160), TestBakeTexel(&pImage, 9, 5));

	AssertInt(TestBakeTexel(&pImage, 4, 8), TestBakeTexel(&pImage, 0, 8));
	AssertInt(TestBakeTexel(&pImage, 11, 6), TestBakeTexel(&pImage, 15, 6));
	AssertInt(TestBakeTexel(&pImage, 4, 4), TestBakeTexel(&pImage, 0, 0));
	AssertInt(TestBakeTexel(&pImage, 11, 11), TestBakeTexel(&pImage, 15, 15));

	//So coarse a checker's cubes are much smaller than a texel: every texel is near the colours' average.
	pChecker = OMalloc<CSolidChecker>(0.01f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)200, (uint8)200, (uint8)200));
	pImage = BakeMeshTexture(&cMesh, 0, &pChecker, 4, 4);
	AssertInt(100, Get8BitRedColour(TestBakeTexel(&pImage, 1, 1)));
	AssertInt(100, Get8BitRedColour(TestBakeTexel(&pImage, 2, 2)));

	AssertNull(&BakeMeshTexture(&cMesh, 1, &pChecker, 4, 4));
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A box unwrapped and baked: one white material on every face, its
// diffuse image the layout's size, and the UVs kept.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBakeModifier(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMeshBake>		pBake;
	Ptr<CMesh>			pMesh;
	Ptr<CMaterial>		pMaterial;
	Ptr<CImage>			pImage;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;
	int					iFace;
	bool				bAll;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 1, 1, 1);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(32.0f);
	pBake = pEditor->AddModifier<CMeshBake>(OMalloc<CSolidChecker>(0.25f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)255, (uint8)255, (uint8)255)), pUnwrap);
	pMesh = pEditor->GetOutput();

	AssertTrue(pMesh->mcUVs.InUse());
	AssertTrue(pMesh->mcMaterials.InUse());
	AssertInt(1, (int)pMesh->mcMaterials.mapMaterials.NumElements());
	bAll = true;
	for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
	{
		bAll &= (pMesh->mcMaterials.mcFaces.GetValue((lsize)iFace) == 0);
	}
	AssertTrue(bAll);
	pMaterial = pMesh->mcMaterials.mapMaterials.Get(0);
	pImage = pMaterial->GetImage(MI_Diffuse)->mpImage;
	AssertTrue(&pImage == &pBake->mpImage);
	AssertInt(pUnwrap->miLayoutWidth, (int)pImage->GetWidth());
	AssertInt(pUnwrap->miLayoutHeight, (int)pImage->GetHeight());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBake(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshBakeChecker();
	TestMeshBakeSquare();
	ObjectsFlush();
	TestMeshBakeModifier();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

