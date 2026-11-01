#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageDivider.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageCelMask.h"
#include "SupportLib/ImageCelTransparent.h"
#include "SupportLib/ImageCelMaskTransparent.h"
#include "SupportLib/ImageCelsWriter.h"
#include "TestLib/Assert.h"
#include "TestReadImage.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerGenerateFromBorder(void)
{
	CImageDivider		cImageDivider;
	Ptr<CImageCelMask>	pcRectangle;

	Ptr<CImage> pcImage = TestReadImage(NULL, "splitter.png");
	Ptr<CImage> pcMask = OMalloc<CImage>();

	cImageDivider.Init(&pcImage);
	cImageDivider.GenerateFromBorder(pcMask);
	AssertSize(20, cImageDivider.GetDestImageCels()->NumElements());
	pcRectangle = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)1, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)1, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)13, pcRectangle->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)13, pcRectangle->GetSubImage()->mcImageRect.GetHeight());

	pcRectangle = cImageDivider.GetDestImageCels()->Get(1);
	AssertInt((int32)15, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)1, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)13, pcRectangle->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)13, pcRectangle->GetSubImage()->mcImageRect.GetHeight());

	pcRectangle = cImageDivider.GetDestImageCels()->Get(17);
	AssertInt((int32)21, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)39, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)3, pcRectangle->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)3, pcRectangle->GetSubImage()->mcImageRect.GetHeight());

	WriteImage(pcMask, "Output" _FS_ "splitter.raw");
	AssertFileMemory("Input" _FS_ "splitter.raw", pcMask->mcChannels.GetData(), pcMask->GetByteSize());

	cImageDivider.Kill();
	pcMask = NULL;
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerGenerateFromNumbers(void)
{
	CImageDivider			cImageDivider;
	Ptr<CImageCel>			pcImageCel;
	CImageDividerNumbers	cNumbers;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");


	//int iCelWidth, int iCelHeight, int iColumnCount, int iRowCount, int iLeftOffset, int iTopOffset, int iWidthSpacing, int iHeightSpacing

	cNumbers.InitGeneral(-1, -1, 4, 3, 0, 0, 0, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(12, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(11);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(16, 16, -1, -1, 0, 0, 0, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(12, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(11);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(16, 16, -1, -1, 0, 0, 16, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(6, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(5);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(16, 16, -1, -1, 0, 0, 0, 16);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(8, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(7);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(16, 16, -1, -1, 16, 0, 0, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(9, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(8);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(16, 16, -1, -1, 16, 0, 16, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(6, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(5);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(-1, -1, 3, 3, 16, 0, 0, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(9, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(8);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	cNumbers.InitGeneral(-1, -1, 2, 3, 16, 0, 16, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(6, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)0, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(5);
	AssertInt((int32)48, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)16, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	pcImage = NULL;;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerGenerateFromNumbersWithTransparent(void)
{
	CImageDivider			cImageDivider;
	Ptr<CImageCel>			pcImageCel;
	uint32					uiTransparent;
	CImageDividerNumbers	cNumbers;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");

	uiTransparent = 0x212121ff;
	cImageDivider.Init(&pcImage, (SImageColour*)&uiTransparent);

	cNumbers.InitGeneral(-1, -1, 4, 3, 0, 0, 0, 0);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(12, cImageDivider.GetDestImageCels()->NumElements());
	pcImageCel = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)1, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)1, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)14, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)14, pcImageCel->GetSubImage()->mcImageRect.GetHeight());

	pcImageCel = cImageDivider.GetDestImageCels()->Get(11);
	AssertInt((int32)49, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)32, pcImageCel->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)14, pcImageCel->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)15, pcImageCel->GetSubImage()->mcImageRect.GetHeight());
	cImageDivider.Kill();

	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerGenerateFromBorderWithTransparent(void)
{

	CImageDivider		cImageDivider;
	Ptr<CImageCelMask>	pcRectangle;
	uint32				uiTransparent;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "cel6.png");
	Ptr<CImage> pcMask = OMalloc<CImage>();

	uiTransparent = 0x000000;
	cImageDivider.Init(&pcImage, (SImageColour*)&uiTransparent);
	cImageDivider.GenerateFromBorder(&pcMask);
	AssertSize(2, cImageDivider.GetDestImageCels()->NumElements());
	pcRectangle = cImageDivider.GetDestImageCels()->Get(0);
	AssertInt((int32)4, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)4, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)18, pcRectangle->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)28, pcRectangle->GetSubImage()->mcImageRect.GetHeight());

	pcRectangle = cImageDivider.GetDestImageCels()->Get(1);
	AssertInt((int32)9, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.x);
	AssertInt((int32)18, pcRectangle->GetSubImage()->mcImageRect.msTopLeft.y);
	AssertInt((int32)11, pcRectangle->GetSubImage()->mcImageRect.GetWidth());
	AssertInt((int32)25, pcRectangle->GetSubImage()->mcImageRect.GetHeight());

	WriteImage(&pcMask, "Output" _FS_ "cel6.raw");
	AssertFileMemory("Input" _FS_ "cel6.raw", pcMask->mcChannels.GetData(), pcMask->GetByteSize());

	cImageDivider.Kill();
	pcMask = NULL;
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertImageDividerCels(CImageDivider* pcImageDivider, char* szImageFileName, char* szName)
{
	CChars	szImagePathName;
	CChars	szExpected;
	CChars	szOutput;
	bool	bResult;

	szImagePathName.Init("Input" _FS_);
	szImagePathName.Append(szImageFileName);
	szExpected.Init("Input" _FS_);
	szExpected.Append(szName);
	szExpected.Append("-expected.cels");
	szOutput.Init("Output" _FS_);
	szOutput.Append(szName);
	szOutput.Append(".cels");

	bResult = CelsWriter(pcImageDivider->GetDestImageCels(), szImagePathName.Text(), szOutput.Text());
	AssertTrue(bResult);
	AssertFile(szExpected.Text(), szOutput.Text());

	szOutput.Kill();
	szExpected.Kill();
	szImagePathName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteBorder(void)
{
	CImageDivider	cImageDivider;

	Ptr<CImage> pcImage = TestReadImage(NULL, "splitter.png");
	Ptr<CImage> pcMask = OMalloc<CImage>();

	cImageDivider.Init(&pcImage);
	cImageDivider.GenerateFromBorder(pcMask);
	AssertSize(20, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "splitter.png", "DividerBorder");

	cImageDivider.Kill();
	pcMask = NULL;
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteBorderWithTransparent(void)
{
	CImageDivider	cImageDivider;
	uint32			uiTransparent;

	Ptr<CImage> pcImage = TestReadImage(NULL, "cel6.png");
	Ptr<CImage> pcMask = OMalloc<CImage>();

	uiTransparent = 0x000000;
	cImageDivider.Init(&pcImage, (SImageColour*)&uiTransparent);
	cImageDivider.GenerateFromBorder(&pcMask);
	AssertSize(2, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "cel6.png", "DividerBorderTransparent");

	cImageDivider.Kill();
	pcMask = NULL;
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteNumbers(void)
{
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");

	cNumbers.InitGeneral(16, 16, -1, -1, 16, 0, 16, 0);
	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(6, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "grid.png", "DividerNumbers");

	cImageDivider.Kill();
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteNumbersWithTransparent(void)
{
	CImageDivider			cImageDivider;
	CImageDividerNumbers	cNumbers;
	uint32					uiTransparent;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");

	uiTransparent = 0x212121ff;
	cNumbers.InitGeneral(-1, -1, 4, 3, 0, 0, 0, 0);
	cImageDivider.Init(&pcImage, (SImageColour*)&uiTransparent);
	cImageDivider.GenerateFromNumbers(&cNumbers);
	AssertSize(12, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "grid.png", "DividerNumbersTransparent");

	cImageDivider.Kill();
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteRectangles(void)
{
	CImageDivider	cImageDivider;
	CArrayInt32Rect	acRectangles;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");

	acRectangles.Init();
	acRectangles.Add()->Init(0, 0, 16, 16);
	acRectangles.Add()->Init(16, 16, 48, 32);
	acRectangles.Add()->Init(8, 24, 40, 48);

	cImageDivider.Init(&pcImage, NULL);
	cImageDivider.GenerateFromRectangles(&acRectangles);
	AssertSize(3, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "grid.png", "DividerRectangles");

	cImageDivider.Kill();
	acRectangles.Kill();
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDividerWriteRectanglesWithTransparent(void)
{
	CImageDivider	cImageDivider;
	CArrayInt32Rect	acRectangles;
	uint32			uiTransparent;

	Ptr<CImage> pcImage = TestReadImage(NULL, "grid.png");

	acRectangles.Init();
	acRectangles.Add()->Init(0, 0, 16, 16);
	acRectangles.Add()->Init(16, 16, 48, 32);
	acRectangles.Add()->Init(8, 24, 40, 48);

	uiTransparent = 0x212121ff;
	cImageDivider.Init(&pcImage, (SImageColour*)&uiTransparent);
	cImageDivider.GenerateFromRectangles(&acRectangles);
	AssertSize(3, cImageDivider.GetDestImageCels()->NumElements());
	AssertImageDividerCels(&cImageDivider, "grid.png", "DividerRectanglesTransparent");

	cImageDivider.Kill();
	acRectangles.Kill();
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageDivider(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageDividerGenerateFromBorder();
	TestImageDividerGenerateFromNumbers();
	TestImageDividerGenerateFromNumbersWithTransparent();
	TestImageDividerGenerateFromBorderWithTransparent();
	TestImageDividerWriteBorder();
	TestImageDividerWriteBorderWithTransparent();
	TestImageDividerWriteNumbers();
	TestImageDividerWriteNumbersWithTransparent();
	TestImageDividerWriteRectangles();
	TestImageDividerWriteRectanglesWithTransparent();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

