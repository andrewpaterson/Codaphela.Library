#include <stdlib.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "RasterLib/Rasteriser.h"
#include "RasterLib/RasterPosed.h"
#include "RasterRenderLib/RasterMeshBuild.h"
#include "TestLib/Assert.h"


#define TEST_POSED_SCALE	1000
#define TEST_POSED_BLACK	0xFF000000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddTestPosedWeight(CMesh* pcMesh, int iCorner, int iMatrix, float32 fWeight)
{
	SSkinWeight*	psWeight;

	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(iCorner)->mcWeights.Add();
	psWeight->iMatrix = iMatrix;
	psWeight->fWeight = fWeight;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle with corners at 0 and 1 along x and y, skinned by three
// matrices: its first corner all the first, its second mostly the first and
// some the second, and its third a third each.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestPosedTriangle(void)
{
	Ptr<CMeshEditor>	pEditor;
	CMesh*				pcMesh;
	SFloat4x4			sIdentity;
	int					i;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcSkin.mbInUse = true;
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddFace(0, 1, 2);

	AddTestPosedWeight(pcMesh, 0, 0, 1.0f);
	AddTestPosedWeight(pcMesh, 1, 1, 0.3f);
	AddTestPosedWeight(pcMesh, 1, 0, 0.7f);
	AddTestPosedWeight(pcMesh, 2, 0, 1.0f / 3.0f);
	AddTestPosedWeight(pcMesh, 2, 1, 1.0f / 3.0f);
	AddTestPosedWeight(pcMesh, 2, 2, 1.0f / 3.0f);
	sIdentity.Identity();
	for (i = 0; i < 3; i++)
	{
		pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);
	}
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestPosedWeights(CRasterMesh* pcRaster, size uiVertex, int iMatrix0, int iWeight0, int iMatrix1, int iWeight1, int iMatrix2, int iWeight2)
{
	SRasterSkinVertex*	psSkin;
	SRasterSkinWeight*	psWeight;
	int					aiMatrices[3];
	int					aiWeights[3];
	uint32				j;
	int32				iTotal;

	aiMatrices[0] = iMatrix0;	aiWeights[0] = iWeight0;
	aiMatrices[1] = iMatrix1;	aiWeights[1] = iWeight1;
	aiMatrices[2] = iMatrix2;	aiWeights[2] = iWeight2;
	psSkin = pcRaster->masSkinVertices.Get(uiVertex);
	iTotal = 0;
	for (j = 0; j < psSkin->uiCount; j++)
	{
		psWeight = pcRaster->masSkinWeights.Get(psSkin->uiFirst + j);
		AssertInt(aiMatrices[j], (int)psWeight->uiMatrix);
		AssertInt(aiWeights[j], (int)psWeight->uiWeight);
		iTotal += psWeight->uiWeight;
	}
	AssertInt(RASTER_SKIN_ONE, iTotal);
	AssertInt((aiWeights[2] != 0) ? 3 : ((aiWeights[1] != 0) ? 2 : 1), (int)psSkin->uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Each vertex's weights are its share of RASTER_SKIN_ONE, most first, and
// add up to exactly it: what rounding leaves over goes to the largest.
// They're saved and read back with the mesh.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterPosedWeights(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CRasterMesh>	pRaster;
	Ptr<CRasterMesh>	pLoaded;
	CFileUtil			cFileUtil;

	pEditor = TestPosedTriangle();
	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &pEditor->mpMesh, TEST_POSED_SCALE);
	AssertTrue(pRaster->IsSkinned());
	AssertInt(3, (int)pRaster->muiNumMatrices);
	AssertSize(3, pRaster->masVertices.NumElements());
	AssertSize(3, pRaster->masSkinVertices.NumElements());

	//0.7 and 0.3 of 32768 are 22937.6 and 9830.4.  A third is 10922.67, three of which round to one too many.
	AssertTestPosedWeights(&pRaster, 0, 0, RASTER_SKIN_ONE, 0, 0, 0, 0);
	AssertTestPosedWeights(&pRaster, 1, 0, 22938, 1, 9830, 0, 0);
	AssertTestPosedWeights(&pRaster, 2, 1, 10923, 2, 10923, 0, 10922);

	cFileUtil.TouchDir("Output");
	AssertTrue(pRaster->WriteFile("Output" _FS_ "Skinned.rastermesh", 42));
	pLoaded = OMalloc<CRasterMesh>();
	AssertTrue(pLoaded->ReadFile("Output" _FS_ "Skinned.rastermesh", 42));
	AssertInt(3, (int)pLoaded->muiNumMatrices);
	AssertTestPosedWeights(&pLoaded, 1, 0, 22938, 1, 9830, 0, 0);
	AssertTestPosedWeights(&pLoaded, 2, 1, 10923, 2, 10923, 0, 10922);
	cFileUtil.Delete("Output" _FS_ "Skinned.rastermesh");

	//A rigid mesh has no skin.
	pEditor->mpMesh->mcSkin.mbInUse = false;
	BuildRasterMesh(&pRaster, &pEditor->mpMesh, TEST_POSED_SCALE);
	AssertFalse(pRaster->IsSkinned());
	AssertInt(0, (int)pRaster->muiNumMatrices);
}


//////////////////////////////////////////////////////////////////////////
//
// Bones that move by x, turn a quarter about z, and stay where they are.
//
//////////////////////////////////////////////////////////////////////////
static void TestPosedBones(SRasterBone* asBones, int32 iMove)
{
	int32	iOne;
	int		i;

	iOne = 1 << RASTER_AXIS_BITS;
	for (i = 0; i < 3; i++)
	{
		asBones[i].asAxes[0].Init(iOne, 0, 0);
		asBones[i].asAxes[1].Init(0, iOne, 0);
		asBones[i].asAxes[2].Init(0, 0, iOne);
		asBones[i].sOffset.Init(0, 0, 0);
	}
	asBones[0].sOffset.Init(iMove, 0, 0);

	//The new x is the old -y, and the new y the old x: x goes to y.
	asBones[1].asAxes[0].Init(0, -iOne, 0);
	asBones[1].asAxes[1].Init(iOne, 0, 0);
	for (i = 0; i < 3; i++)
	{
		asBones[i].asLightAxes[0] = asBones[i].asAxes[0];
		asBones[i].asLightAxes[1] = asBones[i].asAxes[1];
		asBones[i].asLightAxes[2] = asBones[i].asAxes[2];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex all of one bone is exactly where that bone puts it.  One of two
// is between where they'd put it, by their weights, and its normal turned by
// each and made a unit again.  The bounds are of what's posed.  It can't be
// posed with too few bones, nor can a rigid mesh.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterPosedPose(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CRasterMesh>	pRaster;
	Ptr<CRasterMesh>	pRigid;
	CRasterPosed		cPosed;
	SRasterBone			asBones[3];
	SRasterVertex*		psVertex;
	int64				iLength;

	pEditor = TestPosedTriangle();
	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &pEditor->mpMesh, TEST_POSED_SCALE);
	pRaster->masVertices.Get(1)->sNormal.Init(SRasterUnit::FromRaw(1 << SRasterUnit::Bits), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
	TestPosedBones(asBones, 100);

	cPosed.Init();
	AssertTrue(cPosed.Pose(&pRaster, asBones, 3));
	AssertSize(3, cPosed.masVertices.NumElements());

	psVertex = cPosed.masVertices.Get(0);
	AssertInt(100, psVertex->sPosition.x);
	AssertInt(0, psVertex->sPosition.y);

	//0.7 of (1100, 0, 0) and 0.3 of (0, 1000, 0).
	psVertex = cPosed.masVertices.Get(1);
	AssertTrue(abs(psVertex->sPosition.x - 770) <= 1);
	AssertTrue(abs(psVertex->sPosition.y - 300) <= 1);
	AssertInt(0, psVertex->sPosition.z);
	iLength = (int64)psVertex->sNormal.x.miRaw * psVertex->sNormal.x.miRaw + (int64)psVertex->sNormal.y.miRaw * psVertex->sNormal.y.miRaw;
	AssertTrue(llabs(iLength - ((int64)1 << (SRasterUnit::Bits * 2))) < ((int64)1 << (SRasterUnit::Bits * 2)) / 500);
	AssertTrue(psVertex->sNormal.x.miRaw > 2 * psVertex->sNormal.y.miRaw);
	AssertTrue(psVertex->sNormal.y.miRaw > 0);

	//A third each of (100, 1000, 0), (-1000, 0, 0) and (0, 1000, 0).
	psVertex = cPosed.masVertices.Get(2);
	AssertTrue(abs(psVertex->sPosition.x + 300) <= 1);
	AssertTrue(abs(psVertex->sPosition.y - 667) <= 1);

	AssertInt(cPosed.masVertices.Get(2)->sPosition.x, cPosed.msMin.x);
	AssertInt(cPosed.masVertices.Get(1)->sPosition.x, cPosed.msMax.x);
	AssertInt(0, cPosed.msMin.y);
	AssertInt(cPosed.masVertices.Get(2)->sPosition.y, cPosed.msMax.y);

	AssertFalse(cPosed.Pose(&pRaster, asBones, 2));
	pRigid = OMalloc<CRasterMesh>();
	pEditor->mpMesh->mcSkin.mbInUse = false;
	BuildRasterMesh(&pRigid, &pEditor->mpMesh, TEST_POSED_SCALE);
	AssertFalse(cPosed.Pose(&pRigid, asBones, 3));
	cPosed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An instance with posed vertices draws what a rigid mesh of those
// vertices does, its own bounds lighting it.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterPosedDraw(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CRasterMesh>	pRaster;
	Ptr<CRasterMesh>	pMade;
	CRasterPosed		cPosed;
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	CRasterTarget		cPosedTarget;
	CRasterTarget		cMadeTarget;
	SRasterInstance		sPosed;
	SRasterInstance		sMade;
	SRasterBone			asBones[3];
	uint32*				puiPosed;
	uint32*				puiMade;
	size				i;
	size				uiDrawn;

	pEditor = TestPosedTriangle();
	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &pEditor->mpMesh);
	TestPosedBones(asBones, 1 << 18);
	cPosed.Init();
	AssertTrue(cPosed.Pose(&pRaster, asBones, 3));

	pMade = OMalloc<CRasterMesh>();
	for (i = 0; i < cPosed.masVertices.NumElements(); i++)
	{
		*pMade->masVertices.Add() = *cPosed.masVertices.Get(i);
	}
	for (i = 0; i < pRaster->masOpaque.NumElements(); i++)
	{
		*pMade->masOpaque.Add() = *pRaster->masOpaque.Get(i);
	}
	pMade->UpdateBounds();

	cRasteriser.Init();
	cCamera.Init();
	cCamera.Fit(64, 64);
	puiPosed = (uint32*)malloc(64 * 64 * sizeof(uint32));
	puiMade = (uint32*)malloc(64 * 64 * sizeof(uint32));
	cPosedTarget.Init(puiPosed, 64, 64, 64);
	cMadeTarget.Init(puiMade, 64, 64, 64);

	sPosed.Init(&pRaster);
	sPosed.pcPosed = &cPosed;
	sMade.Init(&pMade);
	cPosedTarget.Clear(TEST_POSED_BLACK);
	cRasteriser.Draw(&sPosed, 1, &cCamera, &cPosedTarget);
	cMadeTarget.Clear(TEST_POSED_BLACK);
	cRasteriser.Draw(&sMade, 1, &cCamera, &cMadeTarget);
	AssertTrue(memcmp(puiPosed, puiMade, 64 * 64 * sizeof(uint32)) == 0);
	AssertTrue(memcmp(cPosedTarget.mpiDepths, cMadeTarget.mpiDepths, 64 * 64 * sizeof(int32)) == 0);
	uiDrawn = 0;
	for (i = 0; i < 64 * 64; i++)
	{
		uiDrawn += (puiPosed[i] != TEST_POSED_BLACK) ? 1 : 0;
	}
	AssertTrue(uiDrawn > 0);

	cMadeTarget.Kill();
	cPosedTarget.Kill();
	free(puiMade);
	free(puiPosed);
	cRasteriser.Kill();
	cPosed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterPosed(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterPosedWeights();
	TestRasterPosedPose();
	TestRasterPosedDraw();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

