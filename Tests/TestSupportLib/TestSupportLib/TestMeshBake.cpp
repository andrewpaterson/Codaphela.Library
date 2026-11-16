#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/Material.h"
#include "SupportLib/SolidChecker.h"
#include "SupportLib/MeshUnwrap.h"
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
// each too.  Placed, it's moved, scaled and turned.  (Mix is in its own
// space; Colour in the world's.)
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBakeChecker(void)
{
	Ptr<CSolidChecker>	pChecker;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

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

	//Placed twice the size from 0.1 each way, its cubes are a unit, starting there.
	sOrigin.Init(0.1f, 0.1f, 0.1f);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	pChecker->Place(&sOrigin, &sX, &sY, 2.0f);
	sPoint.Init(0.6f, 0.6f, 0.6f);
	AssertInt(Set32BitColour((uint8)0, (uint8)0, (uint8)0), pChecker->Colour(&sPoint, &sPoint, 0.0001f));
	sPoint.Init(1.35f, 0.6f, 0.6f);
	AssertInt(Set32BitColour((uint8)200, (uint8)100, (uint8)50), pChecker->Colour(&sPoint, &sPoint, 0.0001f));

	//Turned a quarter about z (its x along the world's y), the cube at (0.25, 0.75, 0.25) is the first colour, not the second.
	sOrigin.Init(0, 0, 0);
	sX.Init(0, 1, 0);
	sY.Init(-1, 0, 0);
	pChecker->Place(&sOrigin, &sX, &sY, 1.0f);
	sPoint.Init(0.25f, 0.75f, 0.25f);
	AssertInt(Set32BitColour((uint8)0, (uint8)0, (uint8)0), pChecker->Colour(&sPoint, &sPoint, 0.0001f));
	AssertFloat(1.0f, pChecker->masAxes[2].z, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// A sample starts opaque black with no height, shine, gloss or glow; its
// colour as an ARGB32 is clamped and rounded.  A texture's sample is its
// own at the point in its own space: the checker's is its colour as
// floats, a width across two cubes half of each, and nothing else.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshBakeSample(void)
{
	Ptr<CSolidChecker>	pChecker;
	SSolidSample		sSample;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	sSample.Init();
	AssertFloat(0.0f, sSample.sColour.x, 5);
	AssertFloat(0.0f, sSample.sColour.y, 5);
	AssertFloat(0.0f, sSample.sColour.z, 5);
	AssertFloat(1.0f, sSample.sColour.w, 5);
	AssertFloat(0.0f, sSample.fHeight, 5);
	AssertFloat(0.0f, sSample.fShine, 5);
	AssertFloat(0.0f, sSample.fGloss, 5);
	AssertFloat(0.0f, sSample.fGlow, 5);
	AssertInt(Set32BitColour((uint8)0, (uint8)0, (uint8)0, (uint8)255), sSample.Colour());

	sSample.Init(Set32BitColour((uint8)255, (uint8)51, (uint8)0, (uint8)102));
	AssertFloat(1.0f, sSample.sColour.x, 5);
	AssertFloat(0.2f, sSample.sColour.y, 5);
	AssertFloat(0.0f, sSample.sColour.z, 5);
	AssertFloat(0.4f, sSample.sColour.w, 5);
	AssertInt(Set32BitColour((uint8)255, (uint8)51, (uint8)0, (uint8)102), sSample.Colour());

	sSample.sColour.x = 1.5f;
	sSample.sColour.y = -0.2f;
	sSample.sColour.z = 0.5f;
	sSample.sColour.w = 0.999f;
	AssertInt(Set32BitColour((uint8)255, (uint8)0, (uint8)128, (uint8)255), sSample.Colour());

	pChecker = OMalloc<CSolidChecker>(0.5f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)255, (uint8)102, (uint8)51));
	sSample.fHeight = 3.0f;
	sSample.fShine = 3.0f;
	sSample.fGloss = 3.0f;
	sSample.fGlow = 3.0f;
	sPoint.Init(0.75f, 0.25f, 0.25f);
	pChecker->Sample(&sPoint, &sPoint, 0.0001f, &sSample);
	AssertFloat(1.0f, sSample.sColour.x, 5);
	AssertFloat(0.4f, sSample.sColour.y, 5);
	AssertFloat(0.2f, sSample.sColour.z, 5);
	AssertFloat(1.0f, sSample.sColour.w, 5);
	AssertFloat(0.0f, sSample.fHeight, 5);
	AssertFloat(0.0f, sSample.fShine, 5);
	AssertFloat(0.0f, sSample.fGloss, 5);
	AssertFloat(0.0f, sSample.fGlow, 5);

	sPoint.Init(0.5f, 0.25f, 0.25f);
	pChecker->Sample(&sPoint, &sPoint, 0.2f, &sSample);
	AssertFloat(0.5f, sSample.sColour.x, 4);
	AssertFloat(0.2f, sSample.sColour.y, 4);
	AssertFloat(0.1f, sSample.sColour.z, 4);
	AssertFloat(1.0f, sSample.sColour.w, 5);

	//Placed twice the size from 0.1 each way, the world's (1.35, 0.6, 0.6) is its own (0.625, 0.25, 0.25): the second colour.
	sOrigin.Init(0.1f, 0.1f, 0.1f);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	pChecker->Place(&sOrigin, &sX, &sY, 2.0f);
	sPoint.Init(1.35f, 0.6f, 0.6f);
	pChecker->Sample(&sPoint, &sPoint, 0.0001f, &sSample);
	AssertFloat(1.0f, sSample.sColour.x, 5);
	AssertFloat(0.4f, sSample.sColour.y, 5);
	AssertFloat(0.2f, sSample.sColour.z, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// A unit square at z = 0.25 whose UVs are the middle half of a 16 by 16
// image, texels 4 to 11 each way: a texel there is the checker at its
// centre's point on the square (x = (u - 0.25) * 2), averaged over its
// width (an eighth of the square), and every texel round it is its
// nearest covered texel's.  Tinted, a face's texels are its tint times
// the texture.
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
	ARGB32				auiTints[2];
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

	//Tinted, the first face (below the diagonal) red, the second white: the first's texels keep only their red.
	pChecker = OMalloc<CSolidChecker>(0.5f, Set32BitColour((uint8)10, (uint8)20, (uint8)30), Set32BitColour((uint8)240, (uint8)200, (uint8)160));
	auiTints[0] = Set32BitColour((uint8)255, (uint8)0, (uint8)0);
	auiTints[1] = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
	pImage = BakeMeshTexture(&cMesh, 0, &pChecker, 16, 16, auiTints);
	AssertInt(Set32BitColour((uint8)240, (uint8)0, (uint8)0), TestBakeTexel(&pImage, 9, 5));
	AssertInt(Set32BitColour((uint8)240, (uint8)200, (uint8)160), TestBakeTexel(&pImage, 5, 9));
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A box unwrapped and baked: one white material on every face, its
// diffuse image the layout's size (kept on the UV layer), every mip
// level, and the UVs kept.
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
	CMaterialImage*		pcDiffuse;
	CMeshUVLayer		cLoaded;
	CFileBasic			cFile;
	int					iFace;
	int					iWidth;
	int					iHeight;
	size				i;
	bool				bAll;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 1, 1, 1);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(32.0f);
	pBake = pEditor->AddModifier<CMeshBake>(OMalloc<CSolidChecker>(0.25f, Set32BitColour((uint8)0, (uint8)0, (uint8)0), Set32BitColour((uint8)255, (uint8)255, (uint8)255)));
	pMesh = pEditor->GetOutput();

	AssertTrue(pMesh->mcUVs.InUse());
	AssertInt(pUnwrap->miLayoutWidth, pMesh->mcUVs.mcLayers.Get(0)->miLayoutWidth);
	AssertInt(pUnwrap->miLayoutHeight, pMesh->mcUVs.mcLayers.Get(0)->miLayoutHeight);
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

	//The layer saved and loaded keeps its size.
	cFile.Init(MemoryFile());
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));
	AssertTrue(pMesh->mcUVs.mcLayers.Get(0)->Save(&cFile));
	AssertTrue(cFile.Seek(0));
	AssertTrue(cLoaded.Load(&cFile));
	AssertInt(pUnwrap->miLayoutWidth, cLoaded.miLayoutWidth);
	AssertInt(pUnwrap->miLayoutHeight, cLoaded.miLayoutHeight);
	AssertLSize(pMesh->mcUVs.mcLayers.Get(0)->mcUVs.NumElements(), cLoaded.mcUVs.NumElements());
	cLoaded.Kill();
	cFile.Close();
	cFile.Kill();

	//Every mip level, each half the last (rounded up) to 1 by 1.
	pcDiffuse = pMaterial->GetImage(MI_Diffuse);
	iWidth = pUnwrap->miLayoutWidth;
	iHeight = pUnwrap->miLayoutHeight;
	bAll = true;
	for (i = 0; i < pcDiffuse->mapLevels.NumElements(); i++)
	{
		iWidth = (iWidth + 1) / 2;
		iHeight = (iHeight + 1) / 2;
		pImage = pcDiffuse->mapLevels.Get(i);
		bAll &= ((int)pImage->GetWidth() == iWidth) && ((int)pImage->GetHeight() == iHeight);
	}
	AssertTrue(bAll);
	AssertTrue(pcDiffuse->mapLevels.NumElements() > 0);
	AssertInt(1, iWidth);
	AssertInt(1, iHeight);
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
	TestMeshBakeSample();
	TestMeshBakeSquare();
	ObjectsFlush();
	TestMeshBakeModifier();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

