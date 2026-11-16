#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "ProceduralLib/SolidFlat.h"
#include "ProceduralLib/SolidGradient.h"
#include "ProceduralLib/SolidMaterials.h"
#include "ProceduralLib/SolidNoise.h"
#include "ProceduralLib/SolidRamp.h"
#include "ProceduralLib/SolidWarp.h"
#include "TestLib/Assert.h"
#include "TextureGallery.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 TestSolidValueAt(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z)
{
	SSolidSample	sSample;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;

	sPoint.Init(x, y, z);
	sNormal.Init(0, 0, 1);
	pTexture->Sample(&sPoint, &sNormal, 0.0f, &sSample);
	return sSample.Value();
}


//////////////////////////////////////////////////////////////////////////
//
// An operator's input is sampled at the operator's own point, so the
// input's placement is inside the operator's: a ramp twice the size from
// (1, 0, 0), over a gradient a quarter the size from (0.5, 0, 0) in the
// ramp's space, has the gradient's 0 at the world's x = 2 and its 1 half
// a unit on.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidPlacementInside(void)
{
	Ptr<CSolidRamp>		pRamp;
	Ptr<CSolidGradient>	pGradient;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	pGradient = OMalloc<CSolidGradient>(SG_Plane, SR_Once);
	sOrigin.Init(0.5f, 0, 0);
	pGradient->Place(&sOrigin, &sX, &sY, 0.25f);
	pRamp = OMalloc<CSolidRamp>(pGradient);
	pRamp->AddKey(0.0f, Set32BitColour((uint8)0, (uint8)0, (uint8)0));
	pRamp->AddKey(1.0f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
	sOrigin.Init(1.0f, 0, 0);
	pRamp->Place(&sOrigin, &sX, &sY, 2.0f);

	AssertFloat(0.0f, TestSolidValueAt(pRamp, 1.9f, 3, 4), 5);
	AssertFloat(0.0f, TestSolidValueAt(pRamp, 2.0f, 3, 4), 5);
	AssertFloat(0.5f, TestSolidValueAt(pRamp, 2.25f, 3, 4), 5);
	AssertFloat(1.0f, TestSolidValueAt(pRamp, 2.5f, 3, 4), 5);

	//Turned a quarter about z, the ramp's x is the world's y.
	sOrigin.Init(0, 0, 0);
	sX.Init(0, 1, 0);
	sY.Init(-1, 0, 0);
	pRamp->Place(&sOrigin, &sX, &sY, 1.0f);
	AssertFloat(0.5f, TestSolidValueAt(pRamp, 7.0f, 0.625f, 4), 5);
}


//////////////////////////////////////////////////////////////////////////
//
// A warp by nothing, by none, or by a half grey is its input.  By white
// it's the input a whole amount on along every axis, by black a whole
// amount back.  By a noise it's the input somewhere within the amount.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidWarp(void)
{
	Ptr<CSolidWarp>		pWarp;
	Ptr<CSolidNoise>	pNoise;
	SSolidSample		sGrey;
	float32				fValue;
	int					i;

	pNoise = OMalloc<CSolidNoise>(FN_Sum, 3, 5);
	fValue = TestSolidValueAt(pNoise, 0.3f, 0.6f, -1.2f);

	pWarp = OMalloc<CSolidWarp>(pNoise, Ptr<CSolidTexture>(), 0.5f);
	AssertFloat(fValue, TestSolidValueAt(pWarp, 0.3f, 0.6f, -1.2f), 6);

	pWarp = OMalloc<CSolidWarp>(pNoise, OMalloc<CSolidNoise>(FN_Sum, 3, 9), 0.0f);
	AssertFloat(fValue, TestSolidValueAt(pWarp, 0.3f, 0.6f, -1.2f), 6);

	sGrey.InitValue(0.5f);
	pWarp = OMalloc<CSolidWarp>(pNoise, OMalloc<CSolidFlat>(&sGrey), 0.5f);
	AssertFloat(fValue, TestSolidValueAt(pWarp, 0.3f, 0.6f, -1.2f), 6);

	pWarp = OMalloc<CSolidWarp>(pNoise, OMalloc<CSolidFlat>(Set32BitColour((uint8)255, (uint8)255, (uint8)255)), 0.25f);
	AssertFloat(fValue, TestSolidValueAt(pWarp, 0.05f, 0.35f, -1.45f), 5);

	pWarp = OMalloc<CSolidWarp>(pNoise, OMalloc<CSolidFlat>(Set32BitColour((uint8)0, (uint8)0, (uint8)0)), 0.25f);
	AssertFloat(fValue, TestSolidValueAt(pWarp, 0.55f, 0.85f, -0.95f), 5);

	//A gradient along x warped by a noise: each value is the gradient's within the amount either side.
	pWarp = OMalloc<CSolidWarp>(OMalloc<CSolidGradient>(SG_Plane, SR_Once), OMalloc<CSolidNoise>(FN_Sum, 3, 9), 0.1f);
	for (i = 0; i < 20; i++)
	{
		fValue = TestSolidValueAt(pWarp, 0.5f, (float32)i * 0.31f, (float32)i * -0.17f);
		AssertTrue((fValue >= 0.4f) && (fValue <= 0.6f));
	}
	AssertTrue(fabsf(TestSolidValueAt(pWarp, 0.5f, 0.31f, -0.17f) - TestSolidValueAt(pWarp, 0.5f, 2.17f, -1.19f)) > 0.001f);
}


//////////////////////////////////////////////////////////////////////////
//
// A material is drawn for the gallery, is the same for the same seed and
// different for another, and its colours and other channels are within
// what a sample's are.
//
//////////////////////////////////////////////////////////////////////////
static void TestSolidMaterial(const char* szName, Ptr<CSolidTexture> (*fMake)(uint32 uiSeed))
{
	Ptr<CSolidTexture>	pMaterial;
	Ptr<CSolidTexture>	pSame;
	Ptr<CSolidTexture>	pOther;
	SSolidSample		sSample;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sNormal;
	float32*			pafChannels;
	float32				fDifferent;
	int					i;
	size				j;

	pMaterial = fMake(1);
	pSame = fMake(1);
	pOther = fMake(2);
	sNormal.Init(0, 0, 1);
	fDifferent = 0.0f;
	for (i = 0; i < 50; i++)
	{
		sPoint.Init(((float32)i * 0.137f) - 3.0f, ((float32)i * 0.071f) + 0.2f, (float32)i * -0.093f);
		pMaterial->Sample(&sPoint, &sNormal, 0.01f, &sSample);
		pafChannels = sSample.Channels();
		for (j = 0; j < SOLID_SAMPLE_CHANNELS; j++)
		{
			AssertTrue((pafChannels[j] >= 0.0f) && (pafChannels[j] <= 1.0f));
		}
		AssertFloat(1.0f, sSample.sColour.w, 6);
		AssertFloat(sSample.Value(), TestSolidValueAt(pSame, sPoint.x, sPoint.y, sPoint.z), 2);
		fDifferent += fabsf(sSample.Value() - TestSolidValueAt(pOther, sPoint.x, sPoint.y, sPoint.z));
	}
	AssertTrue(fDifferent > 0.5f);
	TextureGallery(szName, pMaterial);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidMaterials(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestSolidPlacementInside();
	TestSolidWarp();
	TestSolidMaterial("Marble", MakeSolidMarble);
	TestSolidMaterial("Wood", MakeSolidWood);
	TestSolidMaterial("Stone", MakeSolidStone);
	TestSolidMaterial("Rust", MakeSolidRust);
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

