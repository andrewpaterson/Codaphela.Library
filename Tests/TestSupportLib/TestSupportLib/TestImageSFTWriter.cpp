#include "SupportLib/Image.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NaiveFile.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/SFTReader.h"
#include "SupportLib/SFTWriter.h"
#include "SupportLib/ImageDivider.h"
#include "SupportLib/ImageCel.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "TestReadImage.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterTinyTransparent(void)
{
	bool		bResult;
	CNaiveFile	cNaiveFile;

	Ptr<CImage> pImage = TestReadImage(NULL, "Tiny.png");
	AssertTrue(pImage.IsNotNull());

	bResult = SaveSFT(pImage, "Output" _FS_ "Tiny.sft", true);
	AssertTrue(bResult);

	pImage = NULL;

	AssertFile("Input" _FS_ "Tiny-expected.sft", "Output" _FS_ "Tiny.sft")

	pImage = LoadSFT("Output" _FS_ "Tiny.sft");

	cNaiveFile.Init(pImage->GetData(), pImage->GetByteSize());
	cNaiveFile.Write("Output" _FS_ "Tiny.raw");
	cNaiveFile.Kill();

	pImage = NULL;

	AssertFile("Input" _FS_ "Tiny-expected.raw", "Output" _FS_ "Tiny.raw")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterTransparent(void)
{
	bool		bResult;

	Ptr<CImage> pImage = TestReadImage(NULL, "MakiStand.png");
	AssertTrue(pImage.IsNotNull());

	bResult = SaveSFT(pImage, "Output" _FS_ "MakiStand.sft");
	AssertTrue(bResult);

	AssertFile("Input" _FS_ "MakiStand-expected.sft", "Output" _FS_ "MakiStand.sft")

	pImage = NULL;

	pImage = LoadSFT("Output" _FS_ "MakiStand.sft");
	WriteImage(pImage, "Output" _FS_ "MakiStand.png", IT_PNG);

	pImage = NULL;

	AssertFile("Input" _FS_ "MakiStand-expected.png", "Output" _FS_ "MakiStand.png")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterOpaque(void)
{
	bool		bResult;

	Ptr<CImage> pImage = TestReadImage(NULL, "Fighting320.png");
	AssertTrue(pImage.IsNotNull());

	bResult = SaveSFT(pImage, "Output" _FS_ "Fighting320.sft");
	AssertTrue(bResult);

	AssertFile("Input" _FS_ "Fighting320-expected.sft", "Output" _FS_ "Fighting320.sft")

	pImage = NULL;

	pImage = LoadSFT("Output" _FS_ "Fighting320.sft");
	WriteImage(pImage, "Output" _FS_ "Fighting320.png", IT_PNG);

	pImage = NULL;

	AssertFile("Input" _FS_ "Fighting320-expected.png", "Output" _FS_ "Fighting320.png")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterTransparentCelContainer(void)
{
	bool					bResult;
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	Ptr<CArrayImageCel>		pacImageCels;
	Ptr<CImageCel>			pImageCel;
	CFileUtil				cFileUtil;

	Ptr<CImage> pImage = TestReadImage(NULL, "MakiWalk.png");
	AssertTrue(pImage.IsNotNull());

	cNumbers.InitGeneral(-1, -1, 6, 1, 0, 0, 0, 0);
	cImageDivider.Init(&pImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	pacImageCels = cImageDivider.GetDestImageCels();

	AssertSize(6, pacImageCels->NumElements());
	pImageCel = pacImageCels->Get(0);

	AssertSize(42, pImageCel->GetSubImage()->GetFullWidth());
	AssertSize(32, pImageCel->GetSubImage()->GetImageWidth());
	AssertSize(91, pImageCel->GetSubImage()->GetFullHeight());
	AssertSize(90, pImageCel->GetSubImage()->GetImageHeight());

	cFileUtil.Delete("Output" _FS_ "MakiWalk.sft");
	cFileUtil.Delete("Output" _FS_ "MakiWalk.png");

	bResult = SaveSFTCelArray(pacImageCels, "Output" _FS_ "MakiWalk.sft");
	AssertTrue(bResult);

	AssertFile("Input" _FS_ "MakiWalk-expected.sft", "Output" _FS_ "MakiWalk.sft")

	pImage = NULL;

	pImage = LoadSFT("Output" _FS_ "MakiWalk.sft");
	WriteImage(pImage, "Output" _FS_ "MakiWalk.png", IT_PNG);

	pImage = NULL;
	pacImageCels = NULL;

	AssertFile("Input" _FS_ "MakiWalk-expected.png", "Output" _FS_ "MakiWalk.png")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterMultiSourceCelContainer(void)
{
	bool					bResult;
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	Ptr<CArrayImageCel>		pacMakiWalkCels;
	Ptr<CImageCel>			pImageCel;
	CFileUtil				cFileUtil;
	CArrayImageCel			acImageCels;

	Ptr<CImage> pcBackground = TestReadImage(NULL, "Fighting320.png");
	AssertTrue(pcBackground.IsNotNull());

	Ptr<CImage> pcMakiStand = TestReadImage(NULL, "MakiStand.png");
	AssertTrue(pcMakiStand.IsNotNull());

	Ptr<CImage> pcMakiWalk = TestReadImage(NULL, "MakiWalk.png");
	AssertTrue(pcMakiWalk.IsNotNull());

	cNumbers.InitGeneral(-1, -1, 6, 1, 0, 0, 0, 0);
	cImageDivider.Init(&pcMakiWalk, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	pacMakiWalkCels = cImageDivider.GetDestImageCels();

	acImageCels.Init();
	pImageCel = OMalloc<CImageCel>(pcBackground);
	acImageCels.Add(pImageCel);

	pImageCel = OMalloc<CImageCel>(pcMakiStand);
	acImageCels.Add(pImageCel);
	

	acImageCels.AddAll(pacMakiWalkCels);

	cFileUtil.Delete("Output" _FS_ "Combined.sft");
	cFileUtil.Delete("Output" _FS_ "Combined.png");

	bResult = SaveSFTCelArray(&acImageCels, "Output" _FS_ "Combined.sft");
	AssertTrue(bResult);

	AssertFile("Input" _FS_ "Combined-expected.sft", "Output" _FS_ "Combined.sft")


	Ptr<CImage> pImage = LoadSFT("Output" _FS_ "Combined.sft");
	WriteImage(pImage, "Output" _FS_ "Combined.png", IT_PNG);

	pImage = NULL;

	AssertFile("Input" _FS_ "Combined-expected.png", "Output" _FS_ "Combined.png")
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriter(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	ImageChannelDescriptorInit();

	TestImageSFTWriterTinyTransparent();
	TestImageSFTWriterTransparent();
	TestImageSFTWriterOpaque();
	TestImageSFTWriterTransparentCelContainer();
	TestImageSFTWriterMultiSourceCelContainer();

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

