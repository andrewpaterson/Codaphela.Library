#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/ImageCelMask.h"
#include "SupportLib/ImageCelTransparent.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageDivider.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelCropBorders(void)
{
	CImageCel	cCel;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "cel1.png");
	cCel.Init(&pcImage);
	cCel.CropTransparentBorders();

	AssertInt((int32)24, cCel.GetSubImage()->GetFullWidth());
	AssertInt((int32)24, cCel.GetSubImage()->GetImageWidth());
	AssertInt((int32)24, cCel.GetSubImage()->GetFullHeight());
	AssertInt((int32)24, cCel.GetSubImage()->GetImageHeight());

	cCel.Kill();
	pcImage->Kill();

	pcImage = ReadImage("Input" _FS_ "cel2.png");
	cCel.Init(&pcImage);
	cCel.CropTransparentBorders();

	AssertInt((int32)24, cCel.GetSubImage()->GetFullWidth());
	AssertInt((int32)21, cCel.GetSubImage()->GetImageWidth());
	AssertInt((int32)24, cCel.GetSubImage()->GetFullHeight());
	AssertInt((int32)18, cCel.GetSubImage()->GetImageHeight());
	AssertInt((int32)1, cCel.GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)3, cCel.GetSubImage()->mcImageRect.msTopLeft.y);

	cCel.Kill();
	pcImage->Kill();

	pcImage = ReadImage("Input" _FS_ "cel3.png");
	cCel.Init(&pcImage);
	cCel.CropTransparentBorders();

	AssertInt((int32)24, cCel.GetSubImage()->GetFullWidth());
	AssertInt((int32)0, cCel.GetSubImage()->GetImageWidth());
	AssertInt((int32)24, cCel.GetSubImage()->GetFullHeight());
	AssertInt((int32)0, cCel.GetSubImage()->GetImageHeight());
	AssertBool(true, cCel.GetSubImage()->IsImageEmpty());

	cCel.Kill();
	pcImage->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelMaskCropBorders(void)
{
	CImageDivider	cDivider;
	CImageCelMask*	pcRect1;
	CImageCelMask*	pcRect2;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "cel4.png");
	Ptr<CImage> pcMask = OMalloc<CImage>();

	cDivider.Init(&pcImage);
	cDivider.GenerateFromBorder(pcMask);
	
	AssertSize(2, cDivider.GetDestImageCels()->NumElements());
	pcRect1 = (CImageCelMask*)cDivider.GetDestImageCels()->UnsafeGetObject(0);
	pcRect2 = (CImageCelMask*)cDivider.GetDestImageCels()->UnsafeGetObject(1);
	AssertInt(1, pcRect1->GetMask());
	AssertInt(2, pcRect2->GetMask());

	AssertInt((int32)36, pcRect1->GetSubImage()->GetFullWidth());
	AssertInt((int32)28, pcRect1->GetSubImage()->GetImageWidth());
	AssertInt((int32)22, pcRect1->GetSubImage()->GetFullHeight());
	AssertInt((int32)18, pcRect1->GetSubImage()->GetImageHeight());

	AssertInt((int32)31, pcRect2->GetSubImage()->GetFullWidth());
	AssertInt((int32)25, pcRect2->GetSubImage()->GetImageWidth());
	AssertInt((int32)22, pcRect2->GetSubImage()->GetFullHeight());
	AssertInt((int32)11, pcRect2->GetSubImage()->GetImageHeight());

	cDivider.Kill();
	pcImage = NULL;
	pcMask = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelTransparentColourCropBorders(void)
{
	CImageCelTransparent	cCel;
	uint32					cBlack;

	cBlack = 0x5a000000;  //the 5a represents garbage.  Only the first three bytes are used.
	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "cel5.png");
	cCel.Init(&pcImage, (SImageColour*)&cBlack);
	cCel.CropTransparentBorders();

	AssertInt((int32)24, cCel.GetSubImage()->GetFullWidth());
	AssertInt((int32)18, cCel.GetSubImage()->GetImageWidth());
	AssertInt((int32)24, cCel.GetSubImage()->GetFullHeight());
	AssertInt((int32)19, cCel.GetSubImage()->GetImageHeight());
	AssertInt((int32)4, cCel.GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)2, cCel.GetSubImage()->mcImageRect.msTopLeft.y);

	cCel.Kill();
	pcImage->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCel(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageCelCropBorders();
	TestImageCelMaskCropBorders();
	TestImageCelTransparentColourCropBorders();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

