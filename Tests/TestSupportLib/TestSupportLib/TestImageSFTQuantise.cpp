#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileBasic.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageChannel.h"
#include "SupportLib/SFTCommon.h"
#include "SupportLib/SFTReader.h"
#include "SupportLib/SFTWriter.h"
#include "SupportLib/SFTQuantiser.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// An image of uiWidth by uiHeight whose pixels are auiPixels' red, green,
// blue (and opacity if bOpacity) bytes.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CImage> TestQuantiseImage(uint16 uiWidth, uint16 uiHeight, bool bOpacity, const uint8* puiPixels)
{
	Ptr<CImage>	pImage;

	if (bOpacity)
	{
		pImage = OMalloc<CImage>(uiWidth, uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		pImage = OMalloc<CImage>(uiWidth, uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}
	memcpy(pImage->GetData(), puiPixels, (size)uiWidth * uiHeight * (bOpacity ? 4 : 3));
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The records of the SFT container szFilename after its palette, read in
// its colours: the uiIndex'th image (opaque, or transparent if bTransparent).
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CImage> TestQuantiseLoad(char* szFilename, size uiIndex, bool bTransparent, SSFTColours* psColours)
{
	CFileBasic		cFile;
	SSFTContainer	sContainer;
	SSFTContained	sContained;
	filePos			uiPosition;
	Ptr<CImage>		pImage;
	size			i;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return NULL;
	}
	sContainer.Read(&cFile);
	uiPosition = cFile.Tell();
	sContained.Read(&cFile);
	psColours->Init();
	psColours->Read(&cFile);
	for (i = 0; i <= uiIndex; i++)
	{
		cFile.Seek(uiPosition + sContained.uiSkipToNext);
		uiPosition = cFile.Tell();
		sContained.Read(&cFile);
	}
	pImage = bTransparent ? LoadSFTTransparent(&cFile, false, psColours) : LoadSFTOpaque(&cFile, false, psColours);
	cFile.Close();
	cFile.Kill();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Two images with four colours between them, one with transparent pixels:
// the palette is their colours exactly, from index 1 (0 is transparent),
// and the images read back from the file are the same.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTQuantiseExact(void)
{
	static const uint8	auiOpaque[4 * 2 * 3] =
	{
		255, 0, 0,  0, 255, 0,  0, 255, 0,  255, 0, 0,
		0, 0, 255,  0, 0, 255,  255, 0, 0,  0, 255, 0,
	};
	static const uint8	auiTransparent[3 * 2 * 4] =
	{
		255, 0, 0, 255,  0, 0, 0, 0,  10, 20, 30, 255,
		0, 0, 0, 0,  0, 255, 0, 200,  0, 0, 0, 0,
	};
	static const uint8	auiDrawn[3 * 2] = { 1, 0, 1, 0, 1, 0 };
	CArrayImage		acImages;
	CSFTQuantised	cQuantised;
	SSFTColours*	psColours;
	Ptr<CImage>		pImage;
	uint8*			puiRGB;
	uint32*			puiRGBA;
	uint8*			puiPixel;
	size			i;

	acImages.Init();
	acImages.Add(TestQuantiseImage(4, 2, false, auiOpaque));
	acImages.Add(TestQuantiseImage(3, 2, true, auiTransparent));
	AssertTrue(QuantiseSFT(&acImages, 16, &cQuantised));
	AssertInt(1, cQuantised.msColours.uiFirst);
	AssertInt(4, cQuantised.msColours.uiCount);
	AssertSize(2, cQuantised.muiNumImages);
	AssertFalse(cQuantised.masImages[0].bTransparent);
	AssertTrue(cQuantised.masImages[1].bTransparent);
	for (i = 0; i < 3 * 2; i++)
	{
		AssertTrue((cQuantised.masImages[1].puiPixels[i] == 0) == (auiDrawn[i] == 0));
	}
	for (i = 0; i < 4 * 2; i++)
	{
		AssertTrue((cQuantised.masImages[0].puiPixels[i] >= 1) && (cQuantised.masImages[0].puiPixels[i] <= 4));
	}

	//The same colour is the same index in both images.
	AssertInt(cQuantised.masImages[0].puiPixels[0], cQuantised.masImages[1].puiPixels[0]);
	AssertInt(cQuantised.masImages[0].puiPixels[1], cQuantised.masImages[1].puiPixels[4]);

	AssertTrue(SaveSFTQuantised(&cQuantised, "Output" _FS_ "Quantised.sft"));
	cQuantised.Kill();

	psColours = (SSFTColours*)malloc(sizeof(SSFTColours));
	pImage = TestQuantiseLoad("Output" _FS_ "Quantised.sft", 0, false, psColours);
	AssertTrue(pImage.IsNotNull());
	AssertInt(1, psColours->uiFirst);
	AssertInt(4, psColours->uiCount);
	puiRGB = (uint8*)pImage->GetData();
	AssertMemory(auiOpaque, puiRGB, 4 * 2 * 3);
	pImage = TestQuantiseLoad("Output" _FS_ "Quantised.sft", 1, true, psColours);
	AssertTrue(pImage.IsNotNull());
	puiRGBA = (uint32*)pImage->GetData();
	for (i = 0; i < 3 * 2; i++)
	{
		puiPixel = (uint8*)&puiRGBA[i];
		if (auiDrawn[i])
		{
			AssertInt(auiTransparent[i * 4], puiPixel[0]);
			AssertInt(auiTransparent[i * 4 + 1], puiPixel[1]);
			AssertInt(auiTransparent[i * 4 + 2], puiPixel[2]);
			AssertInt(255, puiPixel[3]);
		}
		else
		{
			AssertIntHex((uint32)0, puiRGBA[i]);
		}
	}
	pImage = NULL;
	free(psColours);
	acImages.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// More colours than the palette may have: a gradient of 256 colours made
// 8, each pixel the nearest of them and none far from its own; and two
// clusters of colours made 2, each the mean of its cluster, with each
// pixel its own cluster's.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTQuantiseMedianCut(void)
{
	uint8			auiGradient[16 * 16 * 3];
	uint8			auiClusters[8 * 2 * 3];
	CArrayImage		acImages;
	CSFTQuantised	cQuantised;
	uint8			uiIndex;
	int32			iRed;
	int32			iGreen;
	int32			iBlue;
	int32			iDistance;
	int32			iBest;
	size			x;
	size			y;
	size			i;
	size			j;

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			auiGradient[(y * 16 + x) * 3] = (uint8)(x * 16);
			auiGradient[(y * 16 + x) * 3 + 1] = (uint8)(y * 16);
			auiGradient[(y * 16 + x) * 3 + 2] = 128;
		}
	}
	acImages.Init();
	acImages.Add(TestQuantiseImage(16, 16, false, auiGradient));
	AssertTrue(QuantiseSFT(&acImages, 8, &cQuantised));
	AssertInt(0, cQuantised.msColours.uiFirst);
	AssertInt(8, cQuantised.msColours.uiCount);
	for (i = 0; i < 16 * 16; i++)
	{
		uiIndex = cQuantised.masImages[0].puiPixels[i];
		AssertTrue(uiIndex < 8);
		iRed = (int32)cQuantised.msColours.auiRed[uiIndex] - auiGradient[i * 3];
		iGreen = (int32)cQuantised.msColours.auiGreen[uiIndex] - auiGradient[i * 3 + 1];
		iBlue = (int32)cQuantised.msColours.auiBlue[uiIndex] - auiGradient[i * 3 + 2];
		iDistance = iRed * iRed + iGreen * iGreen + iBlue * iBlue;
		AssertTrue(iDistance <= 2 * 64 * 64);

		//It is the nearest entry.
		iBest = iDistance;
		for (j = 0; j < 8; j++)
		{
			iRed = (int32)cQuantised.msColours.auiRed[j] - auiGradient[i * 3];
			iGreen = (int32)cQuantised.msColours.auiGreen[j] - auiGradient[i * 3 + 1];
			iBlue = (int32)cQuantised.msColours.auiBlue[j] - auiGradient[i * 3 + 2];
			if (iRed * iRed + iGreen * iGreen + iBlue * iBlue < iBest)
			{
				iBest = iRed * iRed + iGreen * iGreen + iBlue * iBlue;
			}
		}
		AssertInt(iBest, iDistance);
	}
	cQuantised.Kill();
	acImages.Kill();

	//Reds round (200, 10, 10) on the top row, blues round (10, 10, 200) on the bottom.
	for (x = 0; x < 8; x++)
	{
		auiClusters[x * 3] = (uint8)(196 + x);
		auiClusters[x * 3 + 1] = (uint8)(10 + (x & 1));
		auiClusters[x * 3 + 2] = 10;
		auiClusters[(8 + x) * 3] = 10;
		auiClusters[(8 + x) * 3 + 1] = (uint8)(10 + (x & 1));
		auiClusters[(8 + x) * 3 + 2] = (uint8)(196 + x);
	}
	acImages.Init();
	acImages.Add(TestQuantiseImage(8, 2, false, auiClusters));
	AssertTrue(QuantiseSFT(&acImages, 2, &cQuantised));
	AssertInt(2, cQuantised.msColours.uiCount);
	uiIndex = cQuantised.masImages[0].puiPixels[0];
	AssertInt(200, cQuantised.msColours.auiRed[uiIndex]);
	AssertInt(11, cQuantised.msColours.auiGreen[uiIndex]);
	AssertInt(10, cQuantised.msColours.auiBlue[uiIndex]);
	for (x = 0; x < 8; x++)
	{
		AssertInt(uiIndex, cQuantised.masImages[0].puiPixels[x]);
		AssertTrue(cQuantised.masImages[0].puiPixels[8 + x] != uiIndex);
	}
	uiIndex = cQuantised.masImages[0].puiPixels[8];
	AssertInt(10, cQuantised.msColours.auiRed[uiIndex]);
	AssertInt(200, cQuantised.msColours.auiBlue[uiIndex]);
	cQuantised.Kill();
	acImages.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An image with nothing drawn has a palette of one colour; one without
// red, green and blue isn't quantised.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTQuantiseEdges(void)
{
	static const uint8	auiNothing[2 * 1 * 4] = { 1, 2, 3, 0,  4, 5, 6, 10 };
	CArrayImage		acImages;
	CSFTQuantised	cQuantised;
	Ptr<CImage>		pGrey;

	acImages.Init();
	acImages.Add(TestQuantiseImage(2, 1, true, auiNothing));
	AssertTrue(QuantiseSFT(&acImages, 16, &cQuantised));
	AssertInt(1, cQuantised.msColours.uiFirst);
	AssertInt(1, cQuantised.msColours.uiCount);
	AssertTrue(cQuantised.masImages[0].bTransparent);
	AssertInt(0, cQuantised.masImages[0].puiPixels[0]);
	AssertInt(0, cQuantised.masImages[0].puiPixels[1]);
	cQuantised.Kill();
	acImages.Kill();

	pGrey = OMalloc<CImage>(2, 2, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	acImages.Init();
	acImages.Add(pGrey);
	AssertFalse(QuantiseSFT(&acImages, 16, &cQuantised));
	acImages.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSFTQuantise(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	ImageChannelDescriptorInit();

	TestImageSFTQuantiseExact();
	TestImageSFTQuantiseMedianCut();
	TestImageSFTQuantiseEdges();

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
