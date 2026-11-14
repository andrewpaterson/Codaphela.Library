#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/MeshEditor.h"
#include "RenderLib/CookedMesh.h"
#include "RasterLib/RasterMesh.h"
#include "RasterRenderLib/RasterMeshBuild.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMaterial> TestCookedMaterial(float32 fAlpha, float32 fRed, float32 fBlue)
{
	Ptr<CMaterial>	pMaterial;

	pMaterial = OMalloc<CMaterial>();
	pMaterial->msProperties.mcDiffuseColour.Init(fAlpha, fRed, 0.0f, fBlue);
	return pMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
// A square of two faces, red then half transparent blue, and a red face
// beside it whose corner it shares with the square has another UV there.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestCookedSquare(void)
{
	Ptr<CMeshEditor>	pEditor;
	CMesh*				pcMesh;
	CMeshUVLayer*		pcLayer;
	int					aiFaceUVs[3][3] = { { 0, 1, 2 }, { 0, 2, 3 }, { 4, 1, 2 } };
	int					i;
	int					k;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcUVs.mbInUse = true;
	pcMesh->mcUVs.AddLayer(pcMesh->GetConnectivity());
	pcMesh->mcMaterials.mbInUse = true;
	pcMesh->mcMaterials.mapMaterials.Add(TestCookedMaterial(1.0f, 1.0f, 0.0f));
	pcMesh->mcMaterials.mapMaterials.Add(TestCookedMaterial(0.5f, 0.0f, 1.0f));

	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(1, 1, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddCorner(2, 0, 0);
	pEditor->AddFace(0, 1, 2);
	pEditor->AddFace(0, 2, 3);
	pEditor->AddFace(1, 4, 2);

	pcLayer = pcMesh->mcUVs.mcLayers.Get(0);
	for (i = 0; i < 5; i++)
	{
		pcLayer->mcUVs.Add()->Init((float32)i, 0.5f);
	}
	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			pcLayer->mcFaces.Get(i)->aiUVs[k] = aiFaceUVs[i][k];
		}
	}
	pcMesh->mcMaterials.mcFaces.SetValue(0, 0);
	pcMesh->mcMaterials.mcFaces.SetValue(1, 1);
	pcMesh->mcMaterials.mcFaces.SetValue(2, 0);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A corner is one vertex unless its faces give it different UVs.  The
// triangles are by submesh, each in the order its faces are, and a
// submesh is a material and whether it's transparent, with the material's
// diffuse colour.  A raster mesh built from it has those colours.
//
//////////////////////////////////////////////////////////////////////////
void TestCookedMeshVerticesAndSubmeshes(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CCookedMesh>	pCooked;
	Ptr<CRasterMesh>	pRaster;
	SCookedSubmesh*		psSubmesh;
	int32				aiCorners[6] = { 0, 1, 2, 3, 1, 4 };
	uint32				auiIndices[9] = { 0, 1, 2,  4, 5, 2,  0, 2, 3 };
	size				i;

	pEditor = TestCookedSquare();
	pCooked = OMalloc<CCookedMesh>();
	pCooked->Cook(&pEditor->mpMesh);

	AssertSize(6, pCooked->NumVertices());
	AssertSize(3, pCooked->NumTriangles());
	for (i = 0; i < 6; i++)
	{
		AssertInt(aiCorners[i], pCooked->maiCorners.GetValue(i));
	}
	for (i = 0; i < 9; i++)
	{
		AssertInt((int)auiIndices[i], (int)pCooked->mauiIndices.GetValue(i));
	}
	AssertTrue(pCooked->HasUVs());
	AssertFalse(pCooked->HasColours());
	AssertFalse(pCooked->IsSkinned());
	AssertFloat(4.0f, pCooked->masUVs.Get(4)->x, 3);
	AssertFloat(1.0f, pCooked->masUVs.Get(1)->x, 3);
	AssertFloat(0.0f, pCooked->masNormals.Get(0)->z, 3);
	AssertFloat(2.0f, pCooked->msMax.x, 3);
	AssertFloat(1.0f, pCooked->msMax.y, 3);
	AssertFloat(0.0f, pCooked->msMin.x, 3);

	AssertSize(2, pCooked->masSubmeshes.NumElements());
	psSubmesh = pCooked->masSubmeshes.Get(0);
	AssertInt(0, psSubmesh->iSlot);
	AssertIntHex((int32)0xFFFF0000, (int32)psSubmesh->uiColour);
	AssertFalse(psSubmesh->bTransparent);
	AssertInt(0, (int)psSubmesh->uiFirstIndex);
	AssertInt(6, (int)psSubmesh->uiNumIndices);
	psSubmesh = pCooked->masSubmeshes.Get(1);
	AssertInt(1, psSubmesh->iSlot);
	AssertIntHex((int32)0x800000FF, (int32)psSubmesh->uiColour);
	AssertTrue(psSubmesh->bTransparent);
	AssertInt(6, (int)psSubmesh->uiFirstIndex);
	AssertTrue(&pCooked->mapMaterials.Get(1) == &pEditor->mpMesh->mcMaterials.mapMaterials.Get(1));

	pRaster = OMalloc<CRasterMesh>();
	BuildRasterMesh(&pRaster, &pCooked, 100);
	AssertSize(6, pRaster->masVertices.NumElements());
	AssertSize(2, pRaster->masOpaque.NumElements());
	AssertSize(1, pRaster->masTransparent.NumElements());
	AssertIntHex((int32)0xFFFF0000, (int32)pRaster->masOpaque.Get(1)->uiColour);
	AssertIntHex((int32)0x800000FF, (int32)pRaster->masTransparent.Get(0)->uiColour);
	AssertInt(200, pRaster->msMax.x);
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex's colour is its corner's in that face, and a corner with two
// colours is two vertices.  A face whose first corner is transparent is.
// A mesh with no materials has one submesh, grey.
//
//////////////////////////////////////////////////////////////////////////
void TestCookedMeshColours(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CCookedMesh>	pCooked;
	CMesh*				pcMesh;
	int					k;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcColours.mbInUse = true;
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddCorner(1, 1, 0);
	pEditor->AddFace(0, 1, 2);
	pEditor->AddFace(2, 1, 3);
	pcMesh->mcColours.mcColours.Add(Set32BitColour(255u, 0u, 0u, 255u));
	pcMesh->mcColours.mcColours.Add(Set32BitColour(0u, 255u, 0u, 128u));
	for (k = 0; k < 3; k++)
	{
		pcMesh->mcColours.mcFaces.Get(0)->aiCornerColours[k] = 0;
		pcMesh->mcColours.mcFaces.Get(1)->aiCornerColours[k] = 1;
	}

	pCooked = OMalloc<CCookedMesh>();
	pCooked->Cook(pcMesh);
	AssertTrue(pCooked->HasColours());
	AssertSize(6, pCooked->NumVertices());
	AssertIntHex((int32)0xFFFF0000, (int32)pCooked->mauiColours.GetValue(0));
	AssertIntHex((int32)0x8000FF00, (int32)pCooked->mauiColours.GetValue(3));
	AssertSize(2, pCooked->masSubmeshes.NumElements());
	AssertInt(-1, pCooked->masSubmeshes.Get(0)->iSlot);
	AssertIntHex((int32)0xFFCCCCCC, (int32)pCooked->masSubmeshes.Get(0)->uiColour);
	AssertTrue(pCooked->masSubmeshes.Get(1)->bTransparent);
	AssertTrue(pCooked->mapMaterials.Get(0).IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex's weights are its corner's, made to add up to one, most first,
// and none of nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestCookedMeshSkin(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CCookedMesh>	pCooked;
	CMesh*				pcMesh;
	SSkinWeight*		psWeight;
	SCookedSkinVertex*	psSkin;
	SFloat4x4			sIdentity;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcSkin.mbInUse = true;
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddFace(0, 1, 2);
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(1)->mcWeights.Add();
	psWeight->iMatrix = 0;
	psWeight->fWeight = 1.0f;
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(1)->mcWeights.Add();
	psWeight->iMatrix = 1;
	psWeight->fWeight = 3.0f;
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(1)->mcWeights.Add();
	psWeight->iMatrix = 2;
	psWeight->fWeight = 0.0f;
	sIdentity.Identity();
	pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);
	pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);
	pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);

	pCooked = OMalloc<CCookedMesh>();
	pCooked->Cook(pcMesh);
	AssertTrue(pCooked->IsSkinned());
	AssertInt(3, (int)pCooked->muiNumMatrices);
	AssertSize(3, pCooked->masSkinVertices.NumElements());
	AssertInt(0, (int)pCooked->masSkinVertices.Get(0)->uiCount);
	psSkin = pCooked->masSkinVertices.Get(1);
	AssertInt(2, (int)psSkin->uiCount);
	AssertInt(1, (int)pCooked->masSkinWeights.Get(psSkin->uiFirst)->uiMatrix);
	AssertFloat(0.75f, pCooked->masSkinWeights.Get(psSkin->uiFirst)->fWeight, 3);
	AssertInt(0, (int)pCooked->masSkinWeights.Get(psSkin->uiFirst + 1)->uiMatrix);
	AssertFloat(0.25f, pCooked->masSkinWeights.Get(psSkin->uiFirst + 1)->fWeight, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCookedMesh(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestCookedMeshVerticesAndSubmeshes();
	TestCookedMeshColours();
	TestCookedMeshSkin();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

