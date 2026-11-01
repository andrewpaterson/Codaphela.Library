#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "AssertImage.h"


//////////////////////////////////////////////////////////////////////////
//
// Input/PNG/szName-expected.png has the same pixels as Input/PNG/szName.png
// but is not interlaced, uses no filters and has palettes expanded.
//
//////////////////////////////////////////////////////////////////////////
void AssertPNGMatchesExpected(const char* szName)
{
	CChars	szPNG;
	CChars	szExpected;

	szPNG.Init("Input" _FS_ "PNG" _FS_);
	szPNG.Append(szName);
	szPNG.Append(".png");
	szExpected.Init("Input" _FS_ "PNG" _FS_);
	szExpected.Append(szName);
	szExpected.Append("-expected.png");

	Ptr<CImage> pcPNG = ReadImage(szPNG.Text());
	Ptr<CImage> pcExpected = ReadImage(szExpected.Text());
	AssertTrue(pcPNG.IsNotNull());
	AssertSameImage(pcExpected, pcPNG);

	pcPNG = NULL;
	pcExpected = NULL;
	szExpected.Kill();
	szPNG.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGInterlaced(void)
{
	AssertPNGMatchesExpected("interlacedgrey1");
	AssertPNGMatchesExpected("interlacedgrey2");
	AssertPNGMatchesExpected("interlacedgrey4");
	AssertPNGMatchesExpected("interlacedgrey8");
	AssertPNGMatchesExpected("interlacedgrey16");
	AssertPNGMatchesExpected("interlacedrgb8");
	AssertPNGMatchesExpected("interlacedrgb16");
	AssertPNGMatchesExpected("interlacedgreyalpha8");
	AssertPNGMatchesExpected("interlacedrgba8");
	AssertPNGMatchesExpected("interlacedrgba16");
	AssertPNGMatchesExpected("interlaced1x1");
	AssertPNGMatchesExpected("interlaced3x2");
	AssertPNGMatchesExpected("paletteinterlaced");
}


//////////////////////////////////////////////////////////////////////////
//
// The rows cycle through all five filter types.
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGFilters(void)
{
	AssertPNGMatchesExpected("filtersgrey2");
	AssertPNGMatchesExpected("filtersgrey8");
	AssertPNGMatchesExpected("filtersgrey16");
	AssertPNGMatchesExpected("filtersrgb8");
	AssertPNGMatchesExpected("filtersrgba16");
	AssertPNGMatchesExpected("filtersgreyalpha16");
}


//////////////////////////////////////////////////////////////////////////
//
// Palettes that are all grey are read as grey, tRNS alpha as opacity.
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGPalettes(void)
{
	CArrayChannel	acChannels;

	AssertPNGMatchesExpected("palette1");
	AssertPNGMatchesExpected("palette2");
	AssertPNGMatchesExpected("palette4");
	AssertPNGMatchesExpected("palette8");
	AssertPNGMatchesExpected("palettetrns");
	AssertPNGMatchesExpected("paletteshorttrns");
	AssertPNGMatchesExpected("paletteopaquetrns");
	AssertPNGMatchesExpected("palettegrey");
	AssertPNGMatchesExpected("palettegreytrns");
	AssertPNGMatchesExpected("paletteindexpastend");

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "PNG" _FS_ "palettetrns.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(4, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(2));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(3));
	acChannels.Kill();

	pcImage = ReadImage("Input" _FS_ "PNG" _FS_ "palettegrey.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Image data split over many (some empty) IDAT chunks, ancillary chunks
// that are skipped, and tRNS on an RGB image, which isn't used.
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGChunks(void)
{
	AssertPNGMatchesExpected("splitidat");
	AssertPNGMatchesExpected("ancillary");
	AssertPNGMatchesExpected("rgbtrns");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertPNGRejected(const char* szName)
{
	CChars	szPNG;

	szPNG.Init("Input" _FS_ "PNG" _FS_ "Bad" _FS_);
	szPNG.Append(szName);

	Ptr<CImage> pcImage = ReadImage(szPNG.Text());
	AssertTrue(pcImage.IsNull());

	pcImage = NULL;
	szPNG.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Malformed PNGs must be rejected without reading past the file.
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGBadFiles(void)
{
	SLogConfig	sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertPNGRejected("badsignature.png");
	AssertPNGRejected("badcrc.png");
	AssertPNGRejected("truncated.png");
	AssertPNGRejected("headeronly.png");
	AssertPNGRejected("noidat.png");
	AssertPNGRejected("noiend.png");
	AssertPNGRejected("ihdrnotfirst.png");
	AssertPNGRejected("unknowncritical.png");
	AssertPNGRejected("badrgbdepth.png");
	AssertPNGRejected("badcolourtype.png");
	AssertPNGRejected("badcompression.png");
	AssertPNGRejected("badinterlace.png");
	AssertPNGRejected("zerowidth.png");
	AssertPNGRejected("hugeimage.png");
	AssertPNGRejected("nopalette.png");
	AssertPNGRejected("trnsbeforeplte.png");
	AssertPNGRejected("badfilter.png");
	AssertPNGRejected("toolittledata.png");
	AssertPNGRejected("toomuchdata.png");
	AssertPNGRejected("badzlib.png");
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads szInput and writes it out again, which must give exactly
// Input/PNG/Written/szName.png.
//
//////////////////////////////////////////////////////////////////////////
void AssertPNGWrittenAsExpected(const char* szInput, const char* szName)
{
	CChars	szOutput;
	CChars	szExpected;

	szOutput.Init("Output" _FS_ "Written");
	szOutput.Append(szName);
	szOutput.Append(".png");
	szExpected.Init("Input" _FS_ "PNG" _FS_ "Written" _FS_);
	szExpected.Append(szName);
	szExpected.Append(".png");

	Ptr<CImage> pcImage = ReadImage((char*)szInput);
	AssertTrue(pcImage.IsNotNull());
	AssertTrue(WriteImage(pcImage, szOutput.Text(), IT_PNG));
	AssertFile(szExpected.Text(), szOutput.Text());

	pcImage = NULL;
	szExpected.Kill();
	szOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each row is written with whichever filter suits it best, so changes to
// how filters are chosen must not change the file.  The expected files
// were written by the byte at a time filter code.
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNGWrite(void)
{
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "filtersgrey8-expected.png", "grey8");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "filtersgrey16-expected.png", "grey16");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "filtersgreyalpha16-expected.png", "greyalpha16");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "interlacedgreyalpha8-expected.png", "greyalpha8");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "filtersrgb8-expected.png", "rgb8");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "interlacedrgb16-expected.png", "rgb16");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "interlacedrgba8-expected.png", "rgba8");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "filtersrgba16-expected.png", "rgba16");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "interlaced1x1-expected.png", "1x1");
	AssertPNGWrittenAsExpected("Input" _FS_ "PNG" _FS_ "interlaced3x2-expected.png", "3x2");
	AssertPNGWrittenAsExpected("Input" _FS_ "Adelle.png", "Adelle");
	AssertPNGWrittenAsExpected("Input" _FS_ "Heightmap.png", "HeightMap");
	AssertPNGWrittenAsExpected("Input" _FS_ "MakiStand.png", "MakiStand");
	AssertPNGWrittenAsExpected("Input" _FS_ "Fighting320.png", "Fighting320");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImagePNG(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImagePNGInterlaced();
	TestImagePNGFilters();
	TestImagePNGPalettes();
	TestImagePNGChunks();
	TestImagePNGBadFiles();
	TestImagePNGWrite();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
