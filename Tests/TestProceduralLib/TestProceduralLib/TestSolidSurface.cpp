#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Box.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/Sphere.h"
#include "ProceduralLib/SolidBricks.h"
#include "ProceduralLib/SolidCavity.h"
#include "ProceduralLib/SolidMaterials.h"
#include "ProceduralLib/SolidRamp.h"
#include "ProceduralLib/SolidSlope.h"
#include "TestLib/Assert.h"
#include "TextureGallery.h"


//////////////////////////////////////////////////////////////////////////
//
// A texture's value at a point of a surface whose normal is (nx, ny, nz).
//
//////////////////////////////////////////////////////////////////////////
static float32 TestSolidSurfaceValue(Ptr<CSolidTexture> pTexture, float32 x, float32 y, float32 z, float32 nx, float32 ny, float32 nz)
{
	SSolidSample	sSample;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNormal;

	sPoint.Init(x, y, z);
	sNormal.Init(nx, ny, nz);
	pTexture->Sample(&sPoint, &sNormal, 0.0f, &sSample);
	return sSample.Value();
}


//////////////////////////////////////////////////////////////////////////
//
// Bricks 1 long (x), 0.5 deep (y) and 0.25 high (z) with 0.1 of mortar.
// Inside a brick the value is how far in from its nearest face, less half
// the mortar: 0.075 at the middle of one, and 0 in the mortar.  A brick's
// number is the same all through it and differs from the next's.  Each
// row is half a brick along from the one below, and each layer in depth
// from the one in front, so x = 0.5 is the middle of a brick in the first
// row and a joint in the second.  With no offset they're in a grid.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidBricks(void)
{
	Ptr<CSolidBricks>	pInside;
	Ptr<CSolidBricks>	pBrick;
	SFloat32Vec3		sSize;
	float32				fNumber;

	sSize.Init(1.0f, 0.5f, 0.25f);
	pInside = OMalloc<CSolidBricks>(SB_Inside, &sSize, 0.1f, 0.5f, 7);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, 0.5f, 0.25f, 0.125f, 0, -1, 0), 5);
	AssertFloat(0.025f, TestSolidSurfaceValue(pInside, 0.5f, 0.25f, 0.075f, 0, -1, 0), 5);
	AssertFloat(0.05f, TestSolidSurfaceValue(pInside, 0.1f, 0.25f, 0.125f, 0, -1, 0), 5);
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.05f, 0.25f, 0.125f, 0, -1, 0), 5);
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.02f, 0.25f, 0.125f, 0, -1, 0), 6);
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.5f, 0.25f, 0.24f, 0, -1, 0), 6);
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.5f, 0.48f, 0.125f, 0, -1, 0), 6);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, -3.0f, -0.25f, 0.125f, 0, -1, 0), 5);  //A layer in front: half a brick back.
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, -3.5f, -0.75f, 0.125f, 0, -1, 0), 5);  //Two in front: a whole one.

	//The second row.
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.5f, 0.25f, 0.375f, 0, -1, 0), 6);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, 1.0f, 0.25f, 0.375f, 0, -1, 0), 5);
	//The second layer of the first row, and of the second: twice half a brick along, as the first.
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 0.5f, 0.75f, 0.125f, 0, -1, 0), 6);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, 0.5f, 0.75f, 0.375f, 0, -1, 0), 5);

	pBrick = OMalloc<CSolidBricks>(SB_Brick, &sSize, 0.1f, 0.5f, 7);
	fNumber = TestSolidSurfaceValue(pBrick, 0.1f, 0.1f, 0.1f, 0, -1, 0);
	AssertTrue((fNumber >= 0.0f) && (fNumber < 1.0f));
	AssertFloat(fNumber, TestSolidSurfaceValue(pBrick, 0.9f, 0.4f, 0.2f, 0, -1, 0), 6);
	AssertFloat(fNumber, TestSolidSurfaceValue(pBrick, 0.02f, 0.25f, 0.01f, 0, -1, 0), 6);
	AssertTrue(fabsf(fNumber - TestSolidSurfaceValue(pBrick, 1.1f, 0.1f, 0.1f, 0, -1, 0)) > 0.0001f);
	AssertTrue(fabsf(fNumber - TestSolidSurfaceValue(pBrick, 0.1f, 0.6f, 0.1f, 0, -1, 0)) > 0.0001f);
	AssertTrue(fabsf(fNumber - TestSolidSurfaceValue(pBrick, 0.1f, 0.1f, 0.3f, 0, -1, 0)) > 0.0001f);
	//In the second row 0.4 and 0.6 are either side of a joint; in the first they were one brick.
	AssertFloat(TestSolidSurfaceValue(pBrick, 0.4f, 0.1f, 0.1f, 0, -1, 0), TestSolidSurfaceValue(pBrick, 0.6f, 0.1f, 0.1f, 0, -1, 0), 6);
	AssertTrue(fabsf(TestSolidSurfaceValue(pBrick, 0.4f, 0.1f, 0.3f, 0, -1, 0) - TestSolidSurfaceValue(pBrick, 0.6f, 0.1f, 0.3f, 0, -1, 0)) > 0.0001f);
	AssertTrue(fabsf(fNumber - TestSolidSurfaceValue(OMalloc<CSolidBricks>(SB_Brick, &sSize, 0.1f, 0.5f, 8), 0.1f, 0.1f, 0.1f, 0, -1, 0)) > 0.0001f);

	pInside = OMalloc<CSolidBricks>(SB_Inside, &sSize, 0.1f, 0.0f, 7);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, 0.5f, 0.25f, 0.375f, 0, -1, 0), 5);
	AssertFloat(0.075f, TestSolidSurfaceValue(pInside, 0.5f, 0.75f, 0.125f, 0, -1, 0), 5);
	AssertFloat(0.0f, TestSolidSurfaceValue(pInside, 1.0f, 0.25f, 0.375f, 0, -1, 0), 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A slope is 1 where the surface faces its direction, a half where it's
// square to it and 0 where it faces away, wherever the point is.  It
// starts up (z); a direction given is made a unit; turned with its
// texture, it's the texture's own direction that counts.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidSlope(void)
{
	Ptr<CSolidSlope>	pSlope;
	SFloat32Vec3		sDirection;
	SFloat32Vec3		sOrigin;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;

	pSlope = OMalloc<CSolidSlope>();
	AssertFloat(1.0f, TestSolidSurfaceValue(pSlope, 3, -4, 5, 0, 0, 1), 6);
	AssertFloat(0.0f, TestSolidSurfaceValue(pSlope, 3, -4, 5, 0, 0, -1), 6);
	AssertFloat(0.5f, TestSolidSurfaceValue(pSlope, 3, -4, 5, 1, 0, 0), 6);
	AssertFloat(0.5f, TestSolidSurfaceValue(pSlope, 0, 0, 0, 0, -1, 0), 6);
	AssertFloat(0.75f, TestSolidSurfaceValue(pSlope, 0, 0, 0, 0.8660254f, 0, 0.5f), 5);  //A third of a right angle up from flat.

	sDirection.Init(3, 0, 4);
	pSlope = OMalloc<CSolidSlope>(&sDirection);
	AssertFloat(1.0f, pSlope->msDirection.Magnitude(), 6);
	AssertFloat(1.0f, TestSolidSurfaceValue(pSlope, 1, 2, 3, 0.6f, 0, 0.8f), 5);
	AssertFloat(0.9f, TestSolidSurfaceValue(pSlope, 1, 2, 3, 0, 0, 1), 5);
	AssertFloat(0.0f, TestSolidSurfaceValue(pSlope, 1, 2, 3, -0.6f, 0, -0.8f), 5);

	//Turned so the texture's z is the world's x, up is along x.
	pSlope = OMalloc<CSolidSlope>();
	sOrigin.Init(0, 0, 0);
	sX.Init(0, 1, 0);
	sY.Init(0, 0, 1);
	pSlope->Place(&sOrigin, &sX, &sY, 1.0f);
	AssertFloat(1.0f, TestSolidSurfaceValue(pSlope, 1, 2, 3, 1, 0, 0), 6);
	AssertFloat(0.5f, TestSolidSurfaceValue(pSlope, 1, 2, 3, 0, 0, 1), 6);
}


//////////////////////////////////////////////////////////////////////////
//
// A box two units each way, with a reach of 0.2.  On the middle of a face
// nothing else is near, out or in: a half.  0.05 from an edge the next
// face is only 0.05 from the point 0.2 in, a quarter of the reach: three
// quarters edged, 0.875.  At a corner two more faces are that near, but
// it's the nearest that counts: the same.  Further than the reach from
// every edge it's a half again.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidCavityEdges(void)
{
	Ptr<CSolidCavity>	pCavity;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		sOrigin;

	sCentre.Init(0, 0, 0);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.0f, &asAxes[1], 1.0f, &asAxes[2], 1.0f);
	pCavity = OMalloc<CSolidCavity>(OMalloc<CBox>(&sBox), 0.2f);

	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0, 0, 1, 0, 0, 1), 5);
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0.3f, -0.4f, -1, 0, 0, -1), 5);
	AssertFloat(0.875f, TestSolidSurfaceValue(pCavity, 0.95f, 0, 1, 0, 0, 1), 5);
	AssertFloat(0.875f, TestSolidSurfaceValue(pCavity, 0.95f, -0.95f, 1, 0, 0, 1), 5);
	AssertFloat(0.75f, TestSolidSurfaceValue(pCavity, 1, 0.1f, -0.9f, 1, 0, 0), 5);
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0.79f, 0, 1, 0, 0, 1), 5);
	AssertFloat(1.0f, TestSolidSurfaceValue(pCavity, 1, 0, 1, 0, 0, 1), 5);  //On the edge itself.

	//The shape stays where it is when the texture is placed elsewhere.
	sOrigin.Init(5, 6, 7);
	pCavity->Place(&sOrigin, &asAxes[1], &asAxes[2], 3.0f);
	AssertFloat(0.875f, TestSolidSurfaceValue(pCavity, 0.95f, 0, 1, 0, 0, 1), 5);
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0, 0, 1, 0, 0, 1), 5);

	//Without a shape, or a reach, everywhere is flat.
	pCavity->mfReach = 0.0f;
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0.95f, 0, 1, 0, 0, 1), 6);
	pCavity->mfReach = 0.2f;
	pCavity->mpShape = NULL;
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, 0.95f, 0, 1, 0, 0, 1), 6);
}


//////////////////////////////////////////////////////////////////////////
//
// Two unit spheres 1.6 apart meet in a groove round x = 0.  On the first,
// 0.05 from the groove, the point 0.2 out is 0.058 from the second sphere
// (0.71 hollowed) and the point 0.2 in is 0.2 deep in the first: 0.146.
// On top of the first, far from the groove, it's a half, as a sphere is
// evenly curved.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidCavityHollows(void)
{
	Ptr<CSolidCavity>	pCavity;
	Ptr<CShapeUnion>	pSpheres;
	SFloat32Vec3		sCentre;
	float32				fGroove;

	pSpheres = OMalloc<CShapeUnion>();
	sCentre.Init(-0.8f, 0, 0);
	pSpheres->Add(OMalloc<CSphere>(&sCentre, 1.0f));
	sCentre.Init(0.8f, 0, 0);
	pSpheres->Add(OMalloc<CSphere>(&sCentre, 1.0f));
	pCavity = OMalloc<CSolidCavity>(pSpheres, 0.2f);

	fGroove = TestSolidSurfaceValue(pCavity, -0.05f, 0, 0.6614378f, 0.75f, 0, 0.6614378f);
	AssertFloat(0.14575f, fGroove, 4);
	AssertFloat(fGroove, TestSolidSurfaceValue(pCavity, 0.05f, 0, 0.6614378f, -0.75f, 0, 0.6614378f), 4);  //And on the second.
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, -0.8f, 0, 1, 0, 0, 1), 5);
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, -1.8f, 0, 0, -1, 0, 0), 5);
	AssertFloat(0.364f, TestSolidSurfaceValue(pCavity, -0.1f, 0, 0.7141428f, 0.7f, 0, 0.7141428f), 2);  //Twice as far from it, less hollowed.
	AssertFloat(0.5f, TestSolidSurfaceValue(pCavity, -0.2f, 0, 0.8f, 0.6f, 0, 0.8f), 5);  //Four times: out of reach.
}


//////////////////////////////////////////////////////////////////////////
//
// The materials of bricks and of tiles, and a stone weathered over the
// gallery's own shape: dirt in the hollow where its sphere and box meet,
// wear along the box's edges and moss on what faces up.
//
//////////////////////////////////////////////////////////////////////////
void TestSolidSurfaceMaterials(void)
{
	Ptr<CSolidTexture>	pMaterial;
	SSolidSample		sBrick;
	SSolidSample		sMortar;
	SFloat32Vec3		sPoint;
	SFloat32Vec3		sNormal;

	pMaterial = MakeSolidBricks(1);
	TextureGalleryAlign(pMaterial);
	TextureGallery("Bricks", pMaterial);
	TextureGallery("WeatheredBricks", MakeSolidWeathered(pMaterial, TextureGalleryShape(), 1));

	pMaterial = MakeSolidTiles(1);
	TextureGalleryAlign(pMaterial);
	TextureGallery("Tiles", pMaterial);

	TextureGallery("Slope", OMalloc<CSolidSlope>());
	TextureGallery("Cavity", OMalloc<CSolidCavity>(TextureGalleryShape(), 0.15f));
	TextureGallery("WeatheredStone", MakeSolidWeathered(MakeSolidStone(1), TextureGalleryShape(), 1));

	//A brick stands proud of its mortar.
	sNormal.Init(0, -1, 0);
	sPoint.Init(0.15f, 0.075f, 0.05f);
	MakeSolidBricks(1)->Sample(&sPoint, &sNormal, 0.0f, &sBrick);
	sPoint.Init(0.001f, 0.075f, 0.05f);
	MakeSolidBricks(1)->Sample(&sPoint, &sNormal, 0.0f, &sMortar);
	AssertTrue(sBrick.fHeight > sMortar.fHeight);
	AssertTrue(sBrick.sColour.x > sBrick.sColour.z);  //Red.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSolidSurface(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestSolidBricks();
	TestSolidSlope();
	TestSolidCavityEdges();
	TestSolidCavityHollows();
	TestSolidSurfaceMaterials();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

