#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageRecolour.h"
#include "SupportLib/ImageRecolourInvertBlackAndWhite.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageRecolour1(void)
{
	CImageRecolour		cRecolour;
	CInt32Rect			cRect;
	CImageColourRGB		o1, o2, o3, n1, n2, n3;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "recolour.png");
	cRect.Init(0, 4, 8, 8);
	o1.Init(1.0f, 0.0f, 0.0f); n1.Init(0.2f, 1.0f, 0.3f);
	o2.Init(0.0f, 0.0f, 0.0f); n2.Init(0.9f, 0.0f, 0.8f);
	o3.Init(1.0f, 1.0f, 1.0f); n3.Init(0.0f, 0.0f, 0.0f);

	cRecolour.Init(&o1, &n1, &cRect);
	cRecolour.AddRecolour(&o2, &n2);
	cRecolour.AddRecolour(&o3, &n3);
	cRecolour.Modify(pcImage);
	cRecolour.Kill();

	WriteImage(pcImage, "Output" _FS_ "recolour.raw");
	AssertFileMemory("Input" _FS_ "recolour.raw", pcImage->GetData(), pcImage->GetByteSize());

	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageRecolourInvertBlackAndWhite(void)
{
	CImageRecolourInvertBlackAndWhite	cInvert;
	Ptr<CImage>							pImage;
	uint8								auiPixels[16] = { 0, 0, 0, 0,  255, 255, 255, 0,  255, 255, 255, 255,  200, 30, 40, 255 };
	uint8								auiExpected[16] = { 255, 255, 255, 255,  0, 0, 0, 0,  255, 255, 255, 255,  200, 30, 40, 255 };

	//Transparent black becomes opaque white and transparent white becomes transparent black.  Other pixels are not touched.
	pImage = OMalloc<CImage>(4, 1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	memcpy(pImage->GetData(), auiPixels, 16);

	cInvert.Init();
	pImage = cInvert.Modify(pImage);
	cInvert.Kill();

	AssertSize(16, pImage->GetByteSize());
	AssertMemory(auiExpected, pImage->GetData(), 16);
	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageRecolour(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageRecolour1();
	TestImageRecolourInvertBlackAndWhite();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

