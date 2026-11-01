#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageCelsReader.h"
#include "SupportLib/ImageCelsWriter.h"
#include "SupportLib/ImageDivider.h"
#include "SupportLib/ImageCel.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "TestReadImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertSubImage(CSubImage* pcExpected, CSubImage* pcActual)
{
	AssertInt((int32)pcExpected->mcImageRect.msTopLeft.x, pcActual->mcImageRect.msTopLeft.x);
	AssertInt((int32)pcExpected->mcImageRect.msTopLeft.y, pcActual->mcImageRect.msTopLeft.y);
	AssertInt((int32)pcExpected->mcImageRect.msBottomRight.x, pcActual->mcImageRect.msBottomRight.x);
	AssertInt((int32)pcExpected->mcImageRect.msBottomRight.y, pcActual->mcImageRect.msBottomRight.y);
	AssertInt((int32)pcExpected->msOffsetTopLeft.x, pcActual->msOffsetTopLeft.x);
	AssertInt((int32)pcExpected->msOffsetTopLeft.y, pcActual->msOffsetTopLeft.y);
	AssertInt((int32)pcExpected->msOffsetBottomRight.x, pcActual->msOffsetBottomRight.x);
	AssertInt((int32)pcExpected->msOffsetBottomRight.y, pcActual->msOffsetBottomRight.y);
	AssertInt((int32)pcExpected->msAlignment.x, pcActual->msAlignment.x);
	AssertInt((int32)pcExpected->msAlignment.y, pcActual->msAlignment.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelsFileWriteRead(void)
{
	bool					bResult;
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	Ptr<CArrayImageCel>		pacImageCels;
	Ptr<CArrayImageCel>		pacLoadedCels;
	Ptr<CImageCel>			pImageCel;
	Ptr<CImageCel>			pLoadedCel;
	CArraySubImage			acExpected;
	size					i;

	Ptr<CImage> pImage = TestReadImage(NULL, "MakiWalk.png");
	AssertTrue(pImage.IsNotNull());

	cNumbers.InitGeneral(-1, -1, 6, 1, 0, 0, 0, 0);
	cImageDivider.Init(&pImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	pacImageCels = cImageDivider.GetDestImageCels();
	AssertSize(6, pacImageCels->NumElements());

	pImageCel = ONMalloc<CImageCel>("MakiWalk/Whole", pImage, 2, 3, 40, 50, 1, 2, 3, 4, SUB_IMAGE_ALIGNMENT_RIGHT | SUB_IMAGE_ALIGNMENT_BOTTOM);
	pacImageCels->Add(pImageCel);
	pImageCel = NULL;

	acExpected.Init();
	for (i = 0; i < pacImageCels->NumElements(); i++)
	{
		acExpected.Add(pacImageCels->Get(i)->GetSubImage());
	}

	bResult = CelsWriter(pacImageCels, "Input" _FS_ "MakiWalk.png", "Output" _FS_ "MakiWalk.cels");
	AssertTrue(bResult);
	AssertFile("Input" _FS_ "MakiWalk-expected.cels", "Output" _FS_ "MakiWalk.cels");

	//Release the written cels so the named cel can be loaded with the same name.
	pacImageCels = NULL;
	cImageDivider.Kill();
	pImage = NULL;

	pacLoadedCels = CelsReader("Output" _FS_ "MakiWalk.cels");
	AssertTrue(pacLoadedCels.IsNotNull());
	AssertSize(7, pacLoadedCels->NumElements());

	pLoadedCel = pacLoadedCels->Get(0);
	AssertInt((int32)252, pLoadedCel->GetSourceImage()->GetWidth());
	AssertInt((int32)91, pLoadedCel->GetSourceImage()->GetHeight());
	AssertFalse(pLoadedCel->IsNamed());

	for (i = 0; i < pacLoadedCels->NumElements(); i++)
	{
		pLoadedCel = pacLoadedCels->Get(i);
		AssertSubImage(acExpected.Get(i), pLoadedCel->GetSubImage());
		AssertTrue(pacLoadedCels->Get(0)->GetSourceImage() == pLoadedCel->GetSourceImage());
	}
	pLoadedCel = pacLoadedCels->Get(6);
	AssertString("MakiWalk/Whole", pLoadedCel->GetName());

	acExpected.Kill();
	pLoadedCel = NULL;
	pacLoadedCels = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelsFileReadOptional(void)
{
	CTextFile				cTextFile;
	Ptr<CArrayImageCel>		pacLoadedCels;
	CSubImage*				pcSubImage;

	cTextFile.Init(
		"ImageCels\n"
		"{\n"
		"	Image\n"
		"	{\n"
		"		FileName	\"MakiWalk.png\";\n"
		"		Directory	\"Input\";\n"
		"	}\n"
		"	Cel\n"
		"	{\n"
		"		Rect	{1, 2, 3, 4};  //Top, left, bottom, right.\n"
		"		Offset	{5, 6, 7, 8};\n"
		"		Align	{-9, -10};\n"
		"	}\n"
		"	Cel\n"
		"	{\n"
		"		Index	1;\n"
		"		Rect	{10, 20, 30, 40};\n"
		"		Offset	{0, 0, 0, 0};\n"
		"		Align	{0, 0};\n"
		"	}\n"
		"}\n");
	cTextFile.Write("Output" _FS_ "Optional.cels");
	cTextFile.Kill();

	pacLoadedCels = CelsReader("Output" _FS_ "Optional.cels");
	AssertTrue(pacLoadedCels.IsNotNull());
	AssertSize(2, pacLoadedCels->NumElements());

	pcSubImage = pacLoadedCels->Get(0)->GetSubImage();
	AssertInt((int32)1, pcSubImage->mcImageRect.msTopLeft.y);
	AssertInt((int32)2, pcSubImage->mcImageRect.msTopLeft.x);
	AssertInt((int32)3, pcSubImage->mcImageRect.msBottomRight.y);
	AssertInt((int32)4, pcSubImage->mcImageRect.msBottomRight.x);
	AssertInt((int32)5, pcSubImage->msOffsetTopLeft.y);
	AssertInt((int32)6, pcSubImage->msOffsetTopLeft.x);
	AssertInt((int32)7, pcSubImage->msOffsetBottomRight.y);
	AssertInt((int32)8, pcSubImage->msOffsetBottomRight.x);
	AssertInt((int32)-9, pcSubImage->msAlignment.x);
	AssertInt((int32)-10, pcSubImage->msAlignment.y);

	pcSubImage = pacLoadedCels->Get(1)->GetSubImage();
	AssertInt((int32)10, pcSubImage->mcImageRect.msTopLeft.y);
	AssertInt((int32)20, pcSubImage->mcImageRect.msTopLeft.x);
	AssertInt((int32)30, pcSubImage->mcImageRect.msBottomRight.y);
	AssertInt((int32)40, pcSubImage->mcImageRect.msBottomRight.x);
	pacLoadedCels = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelsFileReadBadIndex(void)
{
	CTextFile				cTextFile;
	Ptr<CArrayImageCel>		pacLoadedCels;

	cTextFile.Init(
		"ImageCels\n"
		"{\n"
		"	Image\n"
		"	{\n"
		"		FileName	\"MakiWalk.png\";\n"
		"		Directory	\"Input\";\n"
		"	}\n"
		"	Cel\n"
		"	{\n"
		"		Index	1;\n"
		"		Rect	{0, 0, 10, 10};\n"
		"		Offset	{0, 0, 0, 0};\n"
		"		Align	{0, 0};\n"
		"	}\n"
		"}\n");
	cTextFile.Write("Output" _FS_ "BadIndex.cels");
	cTextFile.Kill();

	pacLoadedCels = CelsReader("Output" _FS_ "BadIndex.cels");
	AssertTrue(pacLoadedCels.IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCelsFile(void)
{
	BeginTests();
	DataIOInit();
	ObjectsInit();
	ImageChannelDescriptorInit();

	TestImageCelsFileWriteRead();
	TestImageCelsFileReadOptional();
	TestImageCelsFileReadBadIndex();

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

