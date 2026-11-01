#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/TGAWriter.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"


//////////////////////////////////////////////////////////////////////////
//
// Input/TGA/szName.png holds the pixels Input/TGA/szName.tga was made from.
//
//////////////////////////////////////////////////////////////////////////
void AssertTGAMatchesPNG(const char* szName)
{
	CChars	szTGA;
	CChars	szPNG;

	szTGA.Init("Input" _FS_ "TGA" _FS_);
	szTGA.Append(szName);
	szTGA.Append(".tga");
	szPNG.Init("Input" _FS_ "TGA" _FS_);
	szPNG.Append(szName);
	szPNG.Append(".png");

	Ptr<CImage> pcTGA = ReadImage(szTGA.Text());
	Ptr<CImage> pcPNG = ReadImage(szPNG.Text());
	AssertSameImage(pcPNG, pcTGA);

	pcTGA = NULL;
	pcPNG = NULL;
	szPNG.Kill();
	szTGA.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageTGAReader(void)
{
	AssertTGAMatchesPNG("tc24");
	AssertTGAMatchesPNG("tc24topdown");
	AssertTGAMatchesPNG("tc24righttoleft");
	AssertTGAMatchesPNG("tc24topright");
	AssertTGAMatchesPNG("tc32alpha");
	AssertTGAMatchesPNG("tc32noalpha");
	AssertTGAMatchesPNG("tc16");
	AssertTGAMatchesPNG("tc15");
	AssertTGAMatchesPNG("cm8map24");
	AssertTGAMatchesPNG("cm8map16");
	AssertTGAMatchesPNG("cm8map32alpha");
	AssertTGAMatchesPNG("cm8grey");
	AssertTGAMatchesPNG("grey8");
	AssertTGAMatchesPNG("grey16alpha");
	AssertTGAMatchesPNG("tc24idcolourmap");
	AssertTGAMatchesPNG("rle24");
	AssertTGAMatchesPNG("rle32alpha");
	AssertTGAMatchesPNG("rle24topdown");
	AssertTGAMatchesPNG("rlecm8");
	AssertTGAMatchesPNG("rlegrey8");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTGARejected(const char* szName)
{
	CChars	szTGA;

	szTGA.Init("Input" _FS_ "TGA" _FS_ "Bad" _FS_);
	szTGA.Append(szName);

	Ptr<CImage> pcImage = ReadImage(szTGA.Text());
	AssertTrue(pcImage.IsNull());

	pcImage = NULL;
	szTGA.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Malformed Targas must be rejected without reading past the file.
//
//////////////////////////////////////////////////////////////////////////
void TestImageTGABadFiles(void)
{
	SLogConfig	sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertTGARejected("truncated.tga");
	AssertTGARejected("headeronly.tga");
	AssertTGARejected("badtype.tga");
	AssertTGARejected("nopixels.tga");
	AssertTGARejected("badgreybits.tga");
	AssertTGARejected("badtruecolourbits.tga");
	AssertTGARejected("zerowidth.tga");
	AssertTGARejected("colourmappast.tga");
	AssertTGARejected("colourmappednomap.tga");
	AssertTGARejected("badmapbits.tga");
	AssertTGARejected("rletruncated.tga");
	AssertTGARejected("hugerle.tga");
	AssertTGARejected("hugeraw.tga");
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads szSource, writes it to Output/szDest (optionally RLE compressed)
// and reads that back.
//
//////////////////////////////////////////////////////////////////////////
void AssertTGARoundTrip(const char* szSource, const char* szDest, bool bRLE)
{
	CChars	szOutput;
	bool	bResult;

	szOutput.Init("Output" _FS_);
	szOutput.Append(szDest);

	Ptr<CImage> pcImage = ReadImage((char*)szSource);
	AssertTrue(pcImage.IsNotNull());
	bResult = SaveTGA(pcImage, szOutput.Text(), bRLE);
	AssertTrue(bResult);
	Ptr<CImage> pcComp = ReadImage(szOutput.Text());
	AssertSameImage(pcImage, pcComp);

	pcImage = NULL;
	pcComp = NULL;
	szOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageTGAWriter2(void)
{
	CFileUtil	cFileUtil;
	filePos		iUncompressed;
	filePos		iCompressed;

	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "tc24.png", "tc24.tga", false);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "tc24.png", "tc24rle.tga", true);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "tc32alpha.png", "tc32alpha.tga", false);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "tc32alpha.png", "tc32alpharle.tga", true);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "grey8.png", "grey8.tga", false);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "grey8.png", "grey8rle.tga", true);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "grey16alpha.png", "grey16alpha.tga", false);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "grey16alpha.png", "grey16alpharle.tga", true);
	AssertTGARoundTrip("Input" _FS_ "basn2c08.png", "basn2c08.tga", false);
	AssertTGARoundTrip("Input" _FS_ "basn6a08.png", "basn6a08rle.tga", true);
	AssertTGARoundTrip("Input" _FS_ "basn4a08.png", "basn4a08.tga", false);

	//rle24 has runs of identical pixels so it compresses.
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "rle24.png", "runs.tga", false);
	AssertTGARoundTrip("Input" _FS_ "TGA" _FS_ "rle24.png", "runsrle.tga", true);
	iUncompressed = cFileUtil.Size("Output" _FS_ "runs.tga");
	iCompressed = cFileUtil.Size("Output" _FS_ "runsrle.tga");
	AssertTrue(iCompressed < iUncompressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageTGA(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageTGAReader();
	TestImageTGABadFiles();
	TestImageTGAWriter2();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
