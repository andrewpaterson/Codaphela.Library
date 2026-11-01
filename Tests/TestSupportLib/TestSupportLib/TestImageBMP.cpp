#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"


//////////////////////////////////////////////////////////////////////////
//
// Input/BMP/szName.png was decoded from Input/BMP/szName.bmp by another
// library (PIL).
//
//////////////////////////////////////////////////////////////////////////
void AssertBMPMatchesPNG(const char* szName)
{
	CChars	szBMP;
	CChars	szPNG;

	szBMP.Init("Input" _FS_ "BMP" _FS_);
	szBMP.Append(szName);
	szBMP.Append(".bmp");
	szPNG.Init("Input" _FS_ "BMP" _FS_);
	szPNG.Append(szName);
	szPNG.Append(".png");

	Ptr<CImage> pcBMP = ReadImage(szBMP.Text());
	Ptr<CImage> pcPNG = ReadImage(szPNG.Text());
	AssertSameImage(pcPNG, pcBMP);

	pcBMP = NULL;
	pcPNG = NULL;
	szPNG.Kill();
	szBMP.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageBMPReader(void)
{
	AssertBMPMatchesPNG("pal1");
	AssertBMPMatchesPNG("grey1");
	AssertBMPMatchesPNG("pal4");
	AssertBMPMatchesPNG("pal8");
	AssertBMPMatchesPNG("grey8");
	AssertBMPMatchesPNG("rle4");
	AssertBMPMatchesPNG("rle8");
	AssertBMPMatchesPNG("os2pal8");
	AssertBMPMatchesPNG("os2rgb24");
	AssertBMPMatchesPNG("rgb555");
	AssertBMPMatchesPNG("rgb565");
	AssertBMPMatchesPNG("rgb24");
	AssertBMPMatchesPNG("rgb24topdown");
	AssertBMPMatchesPNG("rgb32");
	AssertBMPMatchesPNG("rgba32v4");
	AssertBMPMatchesPNG("rgba32v5");
	AssertBMPMatchesPNG("argb1555");
	AssertBMPMatchesPNG("alphabitfields");
	AssertBMPMatchesPNG("bitfields32");
	AssertBMPMatchesPNG("bitfields32rgbx");
	AssertBMPMatchesPNG("clrused8");
	AssertBMPMatchesPNG("rleclipped");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertBMPRejected(const char* szName)
{
	CChars	szBMP;

	szBMP.Init("Input" _FS_ "BMP" _FS_ "Bad" _FS_);
	szBMP.Append(szName);

	Ptr<CImage> pcImage = ReadImage(szBMP.Text());
	AssertTrue(pcImage.IsNull());

	pcImage = NULL;
	szBMP.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Malformed bitmaps must be rejected without reading past the file.
//
//////////////////////////////////////////////////////////////////////////
void TestImageBMPBadFiles(void)
{
	SLogConfig	sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertBMPRejected("badmagic.bmp");
	AssertBMPRejected("truncated.bmp");
	AssertBMPRejected("headeronly.bmp");
	AssertBMPRejected("headerpastend.bmp");
	AssertBMPRejected("badbitdepth.bmp");
	AssertBMPRejected("badcompression.bmp");
	AssertBMPRejected("zerowidth.bmp");
	AssertBMPRejected("palettepastend.bmp");
	AssertBMPRejected("hugergb.bmp");
	AssertBMPRejected("hugerle.bmp");
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the PNG szSource, writes it to Output/szDest and reads that back.
//
//////////////////////////////////////////////////////////////////////////
void AssertBMPRoundTrip(const char* szSource, const char* szDest)
{
	CChars	szOutput;
	bool	bResult;

	szOutput.Init("Output" _FS_);
	szOutput.Append(szDest);

	Ptr<CImage> pcImage = ReadImage((char*)szSource);
	AssertTrue(pcImage.IsNotNull());
	bResult = WriteImage(pcImage, szOutput.Text());
	AssertTrue(bResult);
	Ptr<CImage> pcComp = ReadImage(szOutput.Text());
	AssertSameImage(pcImage, pcComp);

	pcImage = NULL;
	pcComp = NULL;
	szOutput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Bitmaps have no grey with alpha format so grey with alpha is written as
// RGBA with the grey copied into red, green and blue.
//
//////////////////////////////////////////////////////////////////////////
void TestImageBMPWriterGreyAlpha(void)
{
	bool		bResult;
	uint8*		puiSource;
	uint8*		puiDest;
	size		uiSourceStride;
	size		uiDestStride;
	size		uiGrey;
	size		uiOpacity;
	size		uiRed;
	size		uiGreen;
	size		uiBlue;
	size		uiAlpha;
	int			iNumPixels;
	int			i;
	int			iMismatches;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "basn4a08.png");
	AssertTrue(pcImage->HasChannels(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_STOP));
	bResult = WriteImage(pcImage, "Output" _FS_ "basn4a08.bmp");
	AssertTrue(bResult);
	Ptr<CImage> pcComp = ReadImage("Output" _FS_ "basn4a08.bmp");
	AssertTrue(pcComp->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP));
	AssertInt((int32)pcImage->GetWidth(), pcComp->GetWidth());
	AssertInt((int32)pcImage->GetHeight(), pcComp->GetHeight());

	iNumPixels = pcImage->GetWidth() * pcImage->GetHeight();
	uiSourceStride = pcImage->GetByteSize() / iNumPixels;
	uiDestStride = pcComp->GetByteSize() / iNumPixels;
	uiGrey = pcImage->GetChannel(IMAGE_DIFFUSE_GREY)->miByteOffset;
	uiOpacity = pcImage->GetChannel(IMAGE_OPACITY)->miByteOffset;
	uiRed = pcComp->GetChannel(IMAGE_DIFFUSE_RED)->miByteOffset;
	uiGreen = pcComp->GetChannel(IMAGE_DIFFUSE_GREEN)->miByteOffset;
	uiBlue = pcComp->GetChannel(IMAGE_DIFFUSE_BLUE)->miByteOffset;
	uiAlpha = pcComp->GetChannel(IMAGE_OPACITY)->miByteOffset;

	puiSource = (uint8*)pcImage->GetData();
	puiDest = (uint8*)pcComp->GetData();
	iMismatches = 0;
	for (i = 0; i < iNumPixels; i++)
	{
		if ((puiDest[i * uiDestStride + uiRed] != puiSource[i * uiSourceStride + uiGrey]) ||
			(puiDest[i * uiDestStride + uiGreen] != puiSource[i * uiSourceStride + uiGrey]) ||
			(puiDest[i * uiDestStride + uiBlue] != puiSource[i * uiSourceStride + uiGrey]) ||
			(puiDest[i * uiDestStride + uiAlpha] != puiSource[i * uiSourceStride + uiOpacity]))
		{
			iMismatches++;
		}
	}
	AssertInt(0, iMismatches);

	pcImage = NULL;
	pcComp = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageBMPWriter(void)
{
	AssertBMPRoundTrip("Input" _FS_ "BMP" _FS_ "rgb24.png", "rgb24.bmp");
	AssertBMPRoundTrip("Input" _FS_ "BMP" _FS_ "rgba32v4.png", "rgba32.bmp");
	AssertBMPRoundTrip("Input" _FS_ "BMP" _FS_ "grey8.png", "grey8.bmp");
	AssertBMPRoundTrip("Input" _FS_ "basn2c08.png", "basn2c08.bmp");
	AssertBMPRoundTrip("Input" _FS_ "basn6a08.png", "basn6a08.bmp");
	AssertBMPRoundTrip("Input" _FS_ "basn0g08.png", "basn0g08.bmp");
	TestImageBMPWriterGreyAlpha();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageBMP(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageBMPReader();
	TestImageBMPBadFiles();
	TestImageBMPWriter();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
