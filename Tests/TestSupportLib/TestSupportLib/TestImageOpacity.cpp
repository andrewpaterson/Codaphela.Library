#include "BaseLib/NaiveFile.h"
#include "BaseLib/PointerRemapper.h"
#include <string.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ChannelsAccessorContiguous.h"
#include "StandardLib/ChannelsAccessorCreatorParams.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageModifierDrawBox.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImageBlitter.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageRowBlitterFactory.h"
#include "SupportLib/ImagePremultipliedBlend.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> TestImageOpacityReadImage(char* szFilename)
{
	CFileUtil			cFileUtil;
	CChars				szInputFilename;
	Ptr<CImage>			pBackground;

	szInputFilename.Init();
	cFileUtil.CurrentDirectory(&szInputFilename);
	cFileUtil.AppendToPath(&szInputFilename, "Input");
	cFileUtil.AppendToPath(&szInputFilename, szFilename);
	AssertTrue(cFileUtil.Exists(szInputFilename.Text()));

	pBackground = ReadImage(szInputFilename.Text(), IT_Unknown, true);
	AssertTrue(pBackground.IsNotNull());
	szInputFilename.Kill();

	return pBackground;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityBlitter(char* szDirectory, char* szInputPathName, char* szTestFilename, int32 x, int32 y)
{
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CImage>		pBackgroundImage;
		Ptr<CImage>		pMakiImage;
		Ptr<CImageCel>	pBackgroundCel;
		Ptr<CImageCel>	pMakiCel;
		CArrayChannel	asChannels;
		SChannel*		psChannel;
		Ptr<CImage>		pDestImage;
		size			uiWidth;
		size			uiHeight;
		CImageBlitter	cBlitter;
		CChars			szOutputFilename;
		CChars			szInputFilename;
		bool			bWritten;
		bool			bResult;

		pBackgroundImage = TestImageOpacityReadImage("Fighting320.png");
		pMakiImage = TestImageOpacityReadImage(szInputPathName);

		asChannels.Init();
		pBackgroundImage->GetAllChannels(&asChannels);
		AssertSize(3, asChannels.NumElements());
		psChannel = asChannels.Get(0);
		AssertSize(IMAGE_DIFFUSE_RED, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);
		AssertFalse(psChannel->bReverse);
		asChannels.Kill();

		asChannels.Init();
		pMakiImage->GetAllChannels(&asChannels);
		AssertSize(4, asChannels.NumElements());
		psChannel = asChannels.Get(0);
		AssertSize(IMAGE_OPACITY, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);
		AssertFalse(psChannel->bReverse);
		asChannels.Kill();


		uiWidth = pBackgroundImage->GetWidth();
		uiHeight = pBackgroundImage->GetHeight();
		pDestImage = OMalloc<CImage>((uint32)uiWidth, (uint32)uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->Black();

		pBackgroundCel = OMalloc<CImageCel>(pBackgroundImage);
		pMakiCel = OMalloc<CImageCel>(pMakiImage, true);

		cBlitter.Init(pBackgroundCel, pDestImage);
		bResult = cBlitter.Configure(&gcImageRowBlitterFactory, false);
		AssertTrue(bResult);
		cBlitter.Blit(0, 0);
		cBlitter.Kill();

		cBlitter.Init(pMakiCel, pDestImage);
		bResult = cBlitter.Configure(&gcImageRowBlitterFactory, false);
		AssertTrue(bResult);
		cBlitter.Blit(x, y);
		cBlitter.Kill();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, szTestFilename);

		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);


		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();

	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacitySizingBlitter(char* szDirectory, char* szInputPathName, char* szTestFilename, int32 x, int32 y, int32 iWidth, int32 iHeight)
{
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CImage>		pBackgroundImage;
		Ptr<CImage>		pMakiImage;
		Ptr<CImageCel>	pBackgroundCel;
		Ptr<CImageCel>	pMakiCel;
		CArrayChannel	asChannels;
		SChannel*		psChannel;
		Ptr<CImage>		pDestImage;
		size			uiWidth;
		size			uiHeight;
		CImageBlitter	cBlitter;
		CChars			szOutputFilename;
		CChars			szInputFilename;
		bool			bWritten;
		bool			bResult;

		pBackgroundImage = TestImageOpacityReadImage("Fighting320.png");
		pMakiImage = TestImageOpacityReadImage(szInputPathName);

		asChannels.Init();
		pBackgroundImage->GetAllChannels(&asChannels);
		AssertSize(3, asChannels.NumElements());
		psChannel = asChannels.Get(0);
		AssertSize(IMAGE_DIFFUSE_RED, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);
		AssertFalse(psChannel->bReverse);
		asChannels.Kill();

		asChannels.Init();
		pMakiImage->GetAllChannels(&asChannels);
		AssertSize(4, asChannels.NumElements());
		psChannel = asChannels.Get(0);
		AssertSize(IMAGE_OPACITY, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);
		AssertFalse(psChannel->bReverse);
		asChannels.Kill();


		uiWidth = pBackgroundImage->GetWidth();
		uiHeight = pBackgroundImage->GetHeight();
		pDestImage = OMalloc<CImage>((uint32)uiWidth, (uint32)uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->Black();

		pBackgroundCel = OMalloc<CImageCel>(pBackgroundImage);
		pMakiCel = OMalloc<CImageCel>(pMakiImage, true);

		cBlitter.Init(pBackgroundCel, pDestImage);
		bResult = cBlitter.Configure(&gcImageRowBlitterFactory, false);
		AssertTrue(bResult);
		cBlitter.Blit(0, 0);
		cBlitter.Kill();

		cBlitter.Init(pMakiCel, pDestImage);
		bResult = cBlitter.Configure(&gcImageRowBlitterFactory, true);
		AssertTrue(bResult);
		cBlitter.Blit(x, y, iWidth, iHeight);
		cBlitter.Kill();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, szTestFilename);

		bWritten = WriteImage(pDestImage, szOutputFilename.Text(), IT_PNG);
		AssertTrue(bWritten);


		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();

	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityOpaqueBlitter(void)
{
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "Fighting.png", 100, 87);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "FightingZero.png", 0, 0);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "FightingMinus10Y.png", 120, -10);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "FightingPlus210Y.png", 77, 120);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "FightingMinus18X.png", -18, 60);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "FightingPlus295X.png", 295, 68);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiWalkWide.png", "FightingPlusWide.png", -170, -3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityTranslucentBlitter(void)
{
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingFeathered.png", 100, 87);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingFeathered.png", 100, 87);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingZeroFeathered.png", 0, 0);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingMinus10YFeathered.png", 120, -10);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingPlus210YFeathered.png", 77, 120);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingMinus18XFeathered.png", -18, 60);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "FightingPlus295XFeathered.png", 295, 68);
	TestImageOpacityBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiWalkWideTranslucent.png", "FightingPlusWideTranslucent.png", -170, -3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityOpaqueSizingBlitter(void)
{
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingNoScale.png", 100, 87, 42, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingPlusOneX.png", 100, 87, 43, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingPlusTwoX.png", 100, 87, 44, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingPlusThreeX.png", 100, 87, 45, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingPlusFourX.png", 100, 87, 46, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingPlusFiveX.png", 100, 87, 47, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingDoubleX.png", 100, 87, 82, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingDoublePlusOneX.png", 100, 87, 83, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingOneX.png", 100, 87, 1, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingZeroX.png", 100, 87, 0, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingTwoX.png", 100, 87, 2, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingFourX.png", 100, 87, 4, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingEightX.png", 100, 87, 8, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "MakiStand.png", "SizingFifteenX.png", 100, 87, 15, 91);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityTranslucentSizingBlitter(void)
{
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingNoScaleFeathered.png", 100, 87, 42, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingPlusOneXFeathered.png", 100, 87, 43, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingPlusTwoXFeathered.png", 100, 87, 44, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingPlusThreeXFeathered.png", 100, 87, 45, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingPlusFourXFeathered.png", 100, 87, 46, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingPlusFiveXFeathered.png", 100, 87, 47, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingDoubleXFeathered.png", 100, 87, 82, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingDoublePlusOneXFeathered.png", 100, 87, 83, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingOneXFeathered.png", 100, 87, 1, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingZeroXFeathered.png", 100, 87, 0, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingTwoXFeathered.png", 100, 87, 2, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingFourXFeathered.png", 100, 87, 4, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingEightXFeathered.png", 100, 87, 8, 91);
	TestImageOpacitySizingBlitter("Output" _FS_ "ImageOpacityCopier", "ImageOpacityCopier" _FS_ "MakiStandFeathered.png", "SizingFifteenXFeathered.png", 100, 87, 15, 91);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertRGBAPixel(uint8 r, uint8 g, uint8 b, uint8 a, uint8* puiPixel)
{
	AssertInt(r, puiPixel[0]);
	AssertInt(g, puiPixel[1]);
	AssertInt(b, puiPixel[2]);
	AssertInt(a, puiPixel[3]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacityPremultipliedBlend(void)
{
	ObjectsInit();
	{
		Ptr<CImage>		pSource;
		Ptr<CImage>		pDest;
		Ptr<CImage>		pOpaque;
		uint8			auiSource[8] = { 0, 0, 128, 128,  255, 255, 255, 255 };  //Half transparent blue then white, multiplied by their opacity.
		uint8			auiDest[12] = { 200, 0, 0, 255,  0, 0, 0, 0,  10, 20, 30, 64 };
		uint8*			puiDest;
		uint8*			puiOpaque;
		CInt32Rect		cClip;

		pSource = OMalloc<CImage>(2, 1, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
		memcpy(pSource->GetData(), auiSource, 8);
		pDest = OMalloc<CImage>(3, 1, CFT_RGBA, CCO_RGB, CRGB_24bit, ARGB_8bit);
		puiDest = (uint8*)pDest->GetData();

		//Over opaque red, over nothing (the source is unchanged) and clipped off the right.
		memcpy(puiDest, auiDest, 12);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 0, 0));
		AssertRGBAPixel(100, 0, 128, 255, &puiDest[0]);
		AssertRGBAPixel(255, 255, 255, 255, &puiDest[4]);
		AssertRGBAPixel(10, 20, 30, 64, &puiDest[8]);

		memcpy(puiDest, auiDest, 12);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 1, 0));
		AssertRGBAPixel(200, 0, 0, 255, &puiDest[0]);
		AssertRGBAPixel(0, 0, 128, 128, &puiDest[4]);
		AssertRGBAPixel(255, 255, 255, 255, &puiDest[8]);

		memcpy(puiDest, auiDest, 12);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 2, 0));
		AssertRGBAPixel(200, 0, 0, 255, &puiDest[0]);
		AssertRGBAPixel(0, 0, 0, 0, &puiDest[4]);
		AssertRGBAPixel(5, 10, 143, 160, &puiDest[8]);

		//Clipped off the left and the top.
		memcpy(puiDest, auiDest, 12);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, -1, 0));
		AssertRGBAPixel(255, 255, 255, 255, &puiDest[0]);
		AssertRGBAPixel(0, 0, 0, 0, &puiDest[4]);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 0, -1));
		AssertRGBAPixel(255, 255, 255, 255, &puiDest[0]);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 3, 0));
		AssertRGBAPixel(10, 20, 30, 64, &puiDest[8]);

		//Only the destination pixels inside the clip are blended.
		memcpy(puiDest, auiDest, 12);
		cClip.Init(1, 0, 2, 1);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 0, 0, &cClip));
		AssertRGBAPixel(200, 0, 0, 255, &puiDest[0]);
		AssertRGBAPixel(255, 255, 255, 255, &puiDest[4]);
		AssertRGBAPixel(10, 20, 30, 64, &puiDest[8]);
		memcpy(puiDest, auiDest, 12);
		cClip.Init(-5, -5, 1, 5);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 0, 0, &cClip));
		AssertRGBAPixel(100, 0, 128, 255, &puiDest[0]);
		AssertRGBAPixel(0, 0, 0, 0, &puiDest[4]);
		memcpy(puiDest, auiDest, 12);
		cClip.Init(0, 1, 3, 2);
		AssertTrue(BlendPremultipliedImage(pSource, pDest, 0, 0, &cClip));
		AssertRGBAPixel(200, 0, 0, 255, &puiDest[0]);
		AssertRGBAPixel(0, 0, 0, 0, &puiDest[4]);

		//A destination without opacity only has its colour blended.
		pOpaque = OMalloc<CImage>(1, 1, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		puiOpaque = (uint8*)pOpaque->GetData();
		puiOpaque[0] = 200;
		puiOpaque[1] = 0;
		puiOpaque[2] = 0;
		AssertTrue(BlendPremultipliedImage(pSource, pOpaque, 0, 0));
		AssertInt(100, puiOpaque[0]);
		AssertInt(0, puiOpaque[1]);
		AssertInt(128, puiOpaque[2]);

		//The source must have opacity.
		AssertFalse(BlendPremultipliedImage(pOpaque, pDest, 0, 0));
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageOpacity(void)
{
	BeginTests();

	DataIOInit();
	
	TestImageOpacityOpaqueBlitter();
	TestImageOpacityTranslucentBlitter();
	TestImageOpacityOpaqueSizingBlitter();
	TestImageOpacityTranslucentSizingBlitter();
	TestImageOpacityPremultipliedBlend();

	DataIOKill();

	TestStatistics();
}

