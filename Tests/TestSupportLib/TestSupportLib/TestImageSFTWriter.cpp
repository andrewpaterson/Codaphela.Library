#include "SupportLib/Image.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NaiveFile.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/SFTReader.h"
#include "SupportLib/SFTWriter.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/DiskFile.h"
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
// Rows without a pixel (the first, two in the middle and the last) are
// still rows: what is under them stays where it was.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterTransparentBlankRows(void)
{
	static const char*	aszRows[6] =
	{
		"....",
		".##.",
		"....",
		"....",
		"#..#",
		"....",
	};
	bool		bResult;
	uint32*		puiPixels;
	uint32		auiExpected[6 * 4];
	size		x;
	size		y;

	Ptr<CImage> pImage = OMalloc<CImage>(4, 6, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	AssertTrue(pImage.IsNotNull());
	puiPixels = (uint32*)pImage->GetData();
	for (y = 0; y < 6; y++)
	{
		for (x = 0; x < 4; x++)
		{
			auiExpected[y * 4 + x] = (aszRows[y][x] == '#') ? 0xFFFFFFFF : 0;
			puiPixels[y * 4 + x] = auiExpected[y * 4 + x];
		}
	}

	bResult = SaveSFT(pImage, "Output" _FS_ "BlankRows.sft");
	AssertTrue(bResult);
	pImage = NULL;

	pImage = LoadSFT("Output" _FS_ "BlankRows.sft");
	AssertTrue(pImage.IsNotNull());
	AssertSize(4, pImage->GetWidth());
	AssertSize(6, pImage->GetHeight());
	AssertMemory(auiExpected, pImage->GetData(), sizeof(auiExpected));
	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs of a row no more than the merge gap apart are one mixed run, whose
// gaps are transparent pixels: the picture read back is the same.  A run
// with a black pixel stays by itself, and so does one further away.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTWriterTransparentMixedRuns(void)
{
	static const char*	aszRows[5] =
	{
		"#.#..#......",		//One mixed run of six: gaps of one and two.
		"............",
		".##...#.....",		//Two runs: the gap is three.
		"#.B.#.......",		//The black pixel's run is by itself, and so the runs either side of it are.
		"...........#",
	};
	static const uint8	auiExpectedFile[] =
	{
		0x53, 0x46, 1, 2, 12, 0, 5, 0,
		0xe5, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff,
		0xc1, 0x80, 0x02, 0xff, 0xff, 0x82, 0x01, 0xff,
		0xc0, 0x01, 0xff, 0x80, 0x01, 0x00, 0x80, 0x01, 0xff,
		0xc0, 0x8a, 0x01, 0xff,
		0x00,
	};
	bool		bResult;
	uint32*		puiPixels;
	uint32		auiExpected[5 * 12];
	uint8		auiFile[sizeof(auiExpectedFile) + 1];
	CFileBasic	cFile;
	size		x;
	size		y;

	Ptr<CImage> pImage = OMalloc<CImage>(12, 5, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	AssertTrue(pImage.IsNotNull());
	puiPixels = (uint32*)pImage->GetData();
	for (y = 0; y < 5; y++)
	{
		for (x = 0; x < 12; x++)
		{
			auiExpected[y * 12 + x] = (aszRows[y][x] == '#') ? 0xFFFFFFFF : ((aszRows[y][x] == 'B') ? 0xFF000000 : 0);
			puiPixels[y * 12 + x] = auiExpected[y * 12 + x];
		}
	}

	bResult = SaveSFT(pImage, "Output" _FS_ "MixedRuns.sft", true, 2);
	AssertTrue(bResult);
	pImage = NULL;

	cFile.Init(DiskFile("Output" _FS_ "MixedRuns.sft"));
	AssertTrue(cFile.Open(EFM_Read));
	AssertSize(sizeof(auiExpectedFile), cFile.Read(auiFile, 1, sizeof(auiFile)));
	cFile.Close();
	cFile.Kill();
	AssertMemory((void*)auiExpectedFile, auiFile, sizeof(auiExpectedFile));

	pImage = LoadSFT("Output" _FS_ "MixedRuns.sft");
	AssertTrue(pImage.IsNotNull());
	AssertSize(12, pImage->GetWidth());
	AssertSize(5, pImage->GetHeight());
	AssertMemory(auiExpected, pImage->GetData(), sizeof(auiExpected));
	pImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A palette's hash is FNV-1a of its first index, its count and its
// colours.  An image read with a palette has its colours, and fails on a
// pixel that isn't in it; a transparent image's mixed run leaves its 0s
// out.  A palette whose hash isn't its colours' isn't read, nor one past
// index 255.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTPalette(void)
{
	static const uint8	auiRed[3] = { 255, 0, 0 };
	static const uint8	auiGreen[3] = { 0, 255, 0 };
	static const uint8	auiBlue[3] = { 0, 0, 255 };
	static const uint8	auiOpaque[3] = { 1, 2, 3 };
	static const uint8	auiNotInPalette[1] = { 4 };
	static const uint8	auiTransparent[6] = { SFT_RUN_MIXED + 2, 3, SFT_PIXEL_TRANSPARENT, 1, SFT_RUN_END };
	static const uint8	auiExpected[9] = { 255, 0, 0, 0, 255, 0, 0, 0, 255 };
	CMemoryFile			cMemory;
	CFileBasic			cFile;
	SSFTImage			sImage;
	SSFTPalette			sPalette;
	SSFTColours*		psColours;
	uint32*				puiPixels;

	AssertIntHex(0x236CA970u, SFTPaletteHash(1, 3, auiRed, auiGreen, auiBlue));

	cMemory.Init();
	cFile.Init(&cMemory);
	AssertTrue(cFile.Open(EFM_ReadWrite_Create));
	AssertTrue(SaveSFTPalette(&cFile, 1, 3, auiRed, auiGreen, auiBlue));
	sImage.Init(SFT_TYPE_OPAQUE, 3, 1);
	AssertTrue(sImage.Write(&cFile));
	AssertTrue(cFile.WriteData(auiOpaque, 3));
	sImage.Init(SFT_TYPE_OPAQUE, 1, 1);
	AssertTrue(sImage.Write(&cFile));
	AssertTrue(cFile.WriteData(auiNotInPalette, 1));
	sImage.Init(SFT_TYPE_TRANSPARENT, 3, 1);
	AssertTrue(sImage.Write(&cFile));
	AssertTrue(cFile.WriteData(auiTransparent, 5));
	sPalette.Init(1, 3, 0x12345678);
	AssertTrue(sPalette.Write(&cFile));
	AssertTrue(cFile.WriteData(auiRed, 3));
	AssertTrue(cFile.WriteData(auiGreen, 3));
	AssertTrue(cFile.WriteData(auiBlue, 3));
	AssertFalse(SaveSFTPalette(&cFile, 255, 2, auiRed, auiGreen, auiBlue));
	AssertSize(SFT_PALETTE_SIZE + 9 + SFT_IMAGE_SIZE + 3 + SFT_IMAGE_SIZE + 1 + SFT_IMAGE_SIZE + 5 + SFT_PALETTE_SIZE + 9, (size)cFile.Size());

	AssertTrue(cFile.Seek(0));
	psColours = (SSFTColours*)malloc(sizeof(SSFTColours));
	psColours->Init();
	AssertTrue(psColours->Read(&cFile));
	AssertInt(1, psColours->uiFirst);
	AssertInt(3, psColours->uiCount);

	Ptr<CImage> pImage = LoadSFTOpaque(&cFile, false, psColours);
	AssertTrue(pImage.IsNotNull());
	AssertSize(3, pImage->GetWidth());
	AssertMemory(auiExpected, pImage->GetData(), 9);
	pImage = LoadSFTOpaque(&cFile, false, psColours);
	AssertTrue(pImage.IsNull());

	pImage = LoadSFTTransparent(&cFile, false, psColours);
	AssertTrue(pImage.IsNotNull());
	puiPixels = (uint32*)pImage->GetData();
	AssertIntHex(0xFFFF0000u, puiPixels[0]);
	AssertIntHex((uint32)0, puiPixels[1]);
	AssertIntHex(0xFF0000FFu, puiPixels[2]);
	pImage = NULL;

	AssertFalse(psColours->Read(&cFile));
	free(psColours);
	cFile.Close();
	cFile.Kill();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A container's palette is the palette of the images after it.  A record
// with another magic number or version isn't read.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTPaletteContainer(void)
{
	static const uint8	auiRed[3] = { 255, 0, 0 };
	static const uint8	auiGreen[3] = { 0, 255, 0 };
	static const uint8	auiBlue[3] = { 0, 0, 255 };
	static const uint8	auiPixels[8] = { 1, 2, 3, 1, 3, 2, 1, 2 };
	static const uint8	auiExpected[6] = { 255, 0, 0, 0, 255, 0 };
	CFileBasic			cFile;
	SSFTContainer		sContainer;
	SSFTContained		sContained;
	SSFTImage			sImage;
	uint8				uiVersion;

	cFile.Init(DiskFile("Output" _FS_ "Palette.sft"));
	AssertTrue(cFile.Open(EFM_Write_Replace));
	sContainer.Init(2);
	AssertTrue(sContainer.Write(&cFile));
	sContained.Init((uint16)(sizeof(uint16) + SFT_PALETTE_SIZE + 9));
	AssertTrue(sContained.Write(&cFile));
	AssertTrue(SaveSFTPalette(&cFile, 1, 3, auiRed, auiGreen, auiBlue));
	sContained.Init();
	AssertTrue(sContained.Write(&cFile));
	sImage.Init(SFT_TYPE_OPAQUE, 4, 2);
	AssertTrue(sImage.Write(&cFile));
	AssertTrue(cFile.WriteData(auiPixels, 8));
	cFile.Close();
	cFile.Kill();

	Ptr<CImage> pImage = LoadSFT("Output" _FS_ "Palette.sft");
	AssertTrue(pImage.IsNotNull());
	AssertSize(4, pImage->GetWidth());
	AssertSize(2, pImage->GetHeight());
	AssertMemory(auiExpected, pImage->GetData(), 6);
	pImage = NULL;

	//The version after this layout's.
	cFile.Init(DiskFile("Output" _FS_ "Palette.sft"));
	AssertTrue(cFile.Open(EFM_ReadWrite));
	AssertTrue(cFile.Seek(2));
	uiVersion = SFT_VERSION + 1;
	AssertTrue(cFile.WriteData(&uiVersion, 1));
	cFile.Close();
	cFile.Kill();
	pImage = LoadSFT("Output" _FS_ "Palette.sft");
	AssertTrue(pImage.IsNull());
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
	TestImageSFTWriterTransparentBlankRows();
	TestImageSFTWriterTransparentMixedRuns();
	TestImageSFTWriterOpaque();
	TestImageSFTWriterTransparentCelContainer();
	TestImageSFTWriterMultiSourceCelContainer();
	TestImageSFTPalette();
	TestImageSFTPaletteContainer();

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

