#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageModifierStack.h"
#include "SupportLib/ImageModifierDrawBox.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageModifierDrawBox1(void)
{

	CImage					cImage;
	CImageModifierDrawBox	cBox;
	CInt32Rect				cRect;
	CImageColourRGB			cRGB;

	cImage.Init();
	cImage.BeginChange();
	cImage.AddChannel(IMAGE_NORMAL_X, PT_uint16);
	cImage.AddChannel(IMAGE_NORMAL_Y, PT_uint16);
	cImage.AddChannel(IMAGE_NORMAL_Z, PT_uint16);
	cImage.AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
	cImage.AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
	cImage.AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
	cImage.SetSize(4, 4);
	cImage.EndChange();
	cImage.Black();

	cRect.Init(1, 1, 2, 2);  //End inclusive.
	cRGB.Init(0.9f, 0.2f, 0.25f);
	cBox.Init(&cRect, &cRGB);
	cBox.Modify(&cImage);

	WriteImage(&cImage, "Output" _FS_ "Box1.png");
	WriteImage(&cImage, "Output" _FS_ "Box1.raw");
	AssertFileMemory("Input" _FS_ "Box1.raw", cImage.GetData(), cImage.GetByteSize());

	cImage.BeginChange();
	cImage.SetSize(8, 8);  //Oops, Set size does not preserve the image.  Also Write Image cannot write out non *4 sized pngs.
	cImage.EndChange();
	cImage.Black();
	cBox.Modify(&cImage);
	cBox.Kill();

	cRect.Init(1, 1, 3, 3);  //End inclusive.
	cRGB.Init(0.5f, 0.6f, 0.7f);
	cBox.Init(&cRect, &cRGB, false);
	cBox.Modify(&cImage);
	cBox.Kill();

	WriteImage(&cImage, "Output" _FS_ "BoxBorder.png");
	WriteImage(&cImage, "Output" _FS_ "BoxBorder.raw");
	AssertFileMemory("Input" _FS_ "BoxBorder.raw", cImage.GetData(), cImage.GetByteSize());

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageModifierDrawBox2(void)
{
	CInt32Rect				cRect;
	CImageColourRGB			cRed;
	CImageColourRGB			cGreen;
	CImageColourRGB			cBlue;
	CImageColourRGB			cGrey;
	CImageModifierStack		cStack;
	Ptr<CImage>				pImage;

	pImage = ONMalloc<CImage>("Picture", 32, 32);
	pImage->Black();

	cStack.Init(pImage);

	cGrey.Init(0.3f, 0.3f, 0.3f);
	cStack.AddModifier<CImageModifierDrawBox>((CInt32Rect*)NULL, &cGrey);

	cRect.Init(12, 0, 30, 21);
	cRed.Init(1.0f, 0.2f, 0.2f);
	cStack.AddModifier<CImageModifierDrawBox>(&cRect, &cRed);

	cRect.Init(3, 15, 18, 27);
	cGreen.Init(0.2f, 1.0f, 0.2f);
	cStack.AddModifier<CImageModifierDrawBox>(&cRect, &cGreen);

	cRect.Init(6, 6, 24, 11);
	cBlue.Init(0.2f, 0.2f, 1.0f);
	cStack.AddModifier<CImageModifierDrawBox>(&cRect, &cBlue);

	cStack.ApplyAll();
	cStack.Kill();

	WriteImage(pImage, "Output" _FS_ "Box2.png");
	WriteImage(pImage, "Output" _FS_ "Box2.raw");
	AssertFileMemory("Input" _FS_ "Box2.raw", pImage->GetData(), pImage->GetByteSize());

	pImage->Kill();

	cRect.Kill();
	cRed.Kill();
	cGreen.Kill();
	cBlue.Kill();
	cGrey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageModifierDrawBox(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageModifierDrawBox1();
	TestImageModifierDrawBox2();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

