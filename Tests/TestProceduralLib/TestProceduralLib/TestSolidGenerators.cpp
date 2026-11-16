#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Noise.h"
#include "StandardLib/Objects.h"
#include "ProceduralLib/SolidCells.h"
#include "ProceduralLib/SolidFlat.h"
#include "ProceduralLib/SolidGradient.h"
#include "ProceduralLib/SolidNoise.h"
#include "TestLib/Assert.h"
#include "TextureGallery.h"


//////////////////////////////////////////////////////////////////////////
//
// A texture that is only a value gives it as an opaque grey and as the
// height, and nothing else.
//
//////////////////////////////////////////////////////////////////////////
static void AssertSolidValue(float32 fExpected, Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z, float32 fWidth, int iDecimals)
{
	SSolidSample	sSample;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;

	sPoint.Init(x, y, z);
	sNormal.Init(0, 0, 1);
	pTexture->Sample(&sPoint, &sNormal, fWidth, &sSample);
	AssertFloat(fExpected, sSample.sColour.x, iDecimals);
	AssertFloat(fExpected, sSample.sColour.y, iDecimals);
	AssertFloat(fExpected, sSample.sColour.z, iDecimals);
	AssertFloat(1.0f, sSample.sColour.w, 6);
	AssertFloat(fExpected, sSample.fHeight, iDecimals);
	AssertFloat(fExpected, sSample.Value(), iDecimals);
	AssertFloat(0.0f, sSample.fShine, 6);
	AssertFloat(0.0f, sSample.fGloss, 6);
	AssertFloat(0.0f, sSample.fGlow, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A flat texture is its sample at every point and width.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidFlat(void)
{
	Ptr<CSolidFlat>	pFlat;
	SSolidSample	sSample;
	SSolidSample	sResult;
	SFloat32Vec3	sPoint;

	pFlat = OMalloc<CSolidFlat>(Set32BitColour((uint8)255, (uint8)102, (uint8)51));
	sPoint.Init(3.5f, -2.0f, 100.0f);
	pFlat->Sample(&sPoint, &sPoint, 4.0f, &sResult);
	AssertFloat(1.0f, sResult.sColour.x, 6);
	AssertFloat(0.4f, sResult.sColour.y, 6);
	AssertFloat(0.2f, sResult.sColour.z, 6);
	AssertFloat(1.0f, sResult.sColour.w, 6);
	AssertFloat(0.0f, sResult.fHeight, 6);
	TextureGallery("Flat", pFlat);

	sSample.InitValue(0.25f);
	sSample.fShine = 0.5f;
	sSample.fGloss = 0.75f;
	sSample.fGlow = 1.0f;
	pFlat = OMalloc<CSolidFlat>(&sSample);
	pFlat->Sample(&sPoint, &sPoint, 0.0f, &sResult);
	AssertFloat(0.25f, sResult.Value(), 6);
	AssertFloat(0.25f, sResult.fHeight, 6);
	AssertFloat(0.5f, sResult.fShine, 6);
	AssertFloat(0.75f, sResult.fGloss, 6);
	AssertFloat(1.0f, sResult.fGlow, 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A gradient is the distance from its plane, axis or point: once, held
// at 0 and 1; as a saw, from 0 again at each whole number; as a wave, up
// to 1 at each half and back.  Over a width it's the mean across it: a
// saw's fall is half way across a texel on it, and any repeat is a half
// over many of its waves.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidGradient(void)
{
	Ptr<CSolidGradient>	pGradient;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	pGradient = OMalloc<CSolidGradient>(SG_Plane, SR_Once);
	AssertSolidValue(0.0f, pGradient, -3.0f, 5.0f, 7.0f, 0.0f, 6);
	AssertSolidValue(0.25f, pGradient, 0.25f, 5.0f, 7.0f, 0.0f, 6);
	AssertSolidValue(1.0f, pGradient, 2.5f, 5.0f, 7.0f, 0.0f, 6);
	AssertSolidValue(0.25f, pGradient, 0.25f, 5.0f, 7.0f, 0.2f, 5);
	AssertSolidValue(0.125f, pGradient, 0.0f, 5.0f, 7.0f, 1.0f, 5);  //Half the texel is 0, half rises to a half.

	//Placed from the gallery's left to its right, it's black there and white here.
	sOrigin.Init(-2.3f, 0.0f, 0.0f);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	pGradient->Place(&sOrigin, &sX, &sY, 4.6f);
	AssertSolidValue(0.5f, pGradient, 0.0f, 5.0f, 7.0f, 0.0f, 5);
	TextureGallery("GradientPlaneOnce", pGradient);

	pGradient = OMalloc<CSolidGradient>(SG_Plane, SR_Saw);
	AssertSolidValue(0.25f, pGradient, 0.25f, 0.0f, 0.0f, 0.0f, 6);
	AssertSolidValue(0.75f, pGradient, 3.75f, 0.0f, 0.0f, 0.0f, 5);
	AssertSolidValue(0.75f, pGradient, -0.25f, 0.0f, 0.0f, 0.0f, 6);
	AssertSolidValue(0.5f, pGradient, 2.0f, 0.0f, 0.0f, 0.5f, 5);  //0.75 to 1 then 0 to 0.25.
	AssertSolidValue(0.5f, pGradient, 2.3f, 0.0f, 0.0f, 20.0f, 3);

	pGradient = OMalloc<CSolidGradient>(SG_Plane, SR_Wave);
	AssertSolidValue(0.0f, pGradient, 3.0f, 0.0f, 0.0f, 0.0f, 6);
	AssertSolidValue(0.5f, pGradient, 3.25f, 0.0f, 0.0f, 0.0f, 5);
	AssertSolidValue(1.0f, pGradient, 3.5f, 0.0f, 0.0f, 0.0f, 5);
	AssertSolidValue(0.5f, pGradient, 3.75f, 0.0f, 0.0f, 0.0f, 5);
	AssertSolidValue(0.75f, pGradient, 3.5f, 0.0f, 0.0f, 0.5f, 5);  //0.5 up to 1 and back.
	AssertSolidValue(0.5f, pGradient, 3.1f, 0.0f, 0.0f, 1.0f, 5);
	AssertSolidValue(0.5f, pGradient, 3.1f, 0.0f, 0.0f, 20.0f, 3);
	TextureGallery("GradientPlaneWave", pGradient);

	pGradient = OMalloc<CSolidGradient>(SG_Axis, SR_Saw);
	AssertSolidValue(0.0f, pGradient, 0.0f, 0.0f, 9.0f, 0.0f, 6);
	AssertSolidValue(0.5f, pGradient, 0.3f, 0.4f, 9.0f, 0.0f, 5);
	AssertSolidValue(0.5f, pGradient, -3.0f, 4.0f, -2.0f, 0.5f, 4);  //Five from the axis: a saw's fall.

	//Placed a quarter the size, rings are a quarter of a unit apart.
	sOrigin.Init(-1.3f, 0.0f, 0.0f);
	pGradient->Place(&sOrigin, &sX, &sY, 0.25f);
	AssertSolidValue(0.5f, pGradient, -0.925f, 0.0f, 0.0f, 0.0f, 4);
	TextureGallery("GradientAxisSaw", pGradient);

	pGradient = OMalloc<CSolidGradient>(SG_Point, SR_Wave);
	AssertSolidValue(0.0f, pGradient, 0.0f, 0.0f, 0.0f, 0.0f, 6);
	AssertSolidValue(1.0f, pGradient, 0.0f, 2.1f, 2.8f, 0.0f, 4);  //Three and a half away.
	AssertSolidValue(0.0f, pGradient, 2.0f, 3.0f, 6.0f, 0.0f, 4);  //Seven.
}


//////////////////////////////////////////////////////////////////////////
//
// A noise is the fractal sum of its kind at the point and width, a sum
// about a half; scaled by two it's the noise at half the point and width.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidNoise(void)
{
	Ptr<CSolidNoise>	pNoise;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sHalf;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	sPoint.Init(1.25f, -2.75f, 3.5f);
	sHalf = sPoint * 0.5f;

	pNoise = OMalloc<CSolidNoise>(FN_Sum, 4, 1);
	AssertSolidValue(0.5f + (0.5f * FractalNoise(FN_Sum, &sPoint, 1, 4, 0.0f)), pNoise, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	AssertSolidValue(0.5f + (0.5f * FractalNoise(FN_Sum, &sPoint, 1, 4, 0.5f)), pNoise, sPoint.x, sPoint.y, sPoint.z, 0.5f, 6);
	AssertSolidValue(0.5f, pNoise, sPoint.x, sPoint.y, sPoint.z, 10.0f, 6);
	TextureGallery("NoiseSum", pNoise);

	sOrigin.Init(0, 0, 0);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	pNoise->Place(&sOrigin, &sX, &sY, 2.0f);
	AssertSolidValue(0.5f + (0.5f * FractalNoise(FN_Sum, &sHalf, 1, 4, 0.25f)), pNoise, sPoint.x, sPoint.y, sPoint.z, 0.5f, 6);

	pNoise = OMalloc<CSolidNoise>(FN_Turbulence, 4, 1);
	AssertSolidValue(FractalNoise(FN_Turbulence, &sPoint, 1, 4, 0.0f), pNoise, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	pNoise->Place(&sOrigin, &sX, &sY, 0.5f);
	TextureGallery("NoiseTurbulence", pNoise);

	pNoise = OMalloc<CSolidNoise>(FN_Ridged, 4, 1);
	AssertSolidValue(FractalNoise(FN_Ridged, &sPoint, 1, 4, 0.0f), pNoise, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	pNoise->Place(&sOrigin, &sX, &sY, 0.5f);
	TextureGallery("NoiseRidged", pNoise);
}


//////////////////////////////////////////////////////////////////////////
//
// Cells are cellular noise's numbers at the point and width: the nearest
// point's distance, how much further the second is, or the cell's own.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidCells(void)
{
	Ptr<CSolidCells>	pCells;
	SCellularNoise		sNoise;
	SCellularNoise		sWide;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	sPoint.Init(1.25f, -2.75f, 3.5f);
	CellularNoise(&sNoise, &sPoint, 1, 0.0f);
	CellularNoise(&sWide, &sPoint, 1, 0.5f);
	sOrigin.Init(0, 0, 0);
	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);

	pCells = OMalloc<CSolidCells>(SC_Nearest, 1);
	AssertSolidValue(sNoise.fNearest, pCells, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	AssertSolidValue(sWide.fNearest, pCells, sPoint.x, sPoint.y, sPoint.z, 0.5f, 6);
	pCells->Place(&sOrigin, &sX, &sY, 0.25f);
	TextureGallery("CellsNearest", pCells);

	pCells = OMalloc<CSolidCells>(SC_Edge, 1);
	AssertSolidValue(sNoise.fSecond - sNoise.fNearest, pCells, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	pCells->Place(&sOrigin, &sX, &sY, 0.25f);
	TextureGallery("CellsEdge", pCells);

	pCells = OMalloc<CSolidCells>(SC_Cell, 1);
	AssertSolidValue(sNoise.fCell, pCells, sPoint.x, sPoint.y, sPoint.z, 0.0f, 6);
	AssertSolidValue(0.5f, pCells, sPoint.x, sPoint.y, sPoint.z, 10.0f, 6);
	pCells->Place(&sOrigin, &sX, &sY, 0.25f);
	TextureGallery("CellsCell", pCells);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidGenerators(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestSolidFlat();
	TestSolidGradient();
	TestSolidNoise();
	TestSolidCells();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

