#include "BaseLib/NaiveFile.h"
#include "BaseLib/PointerRemapper.h"
#include <string.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Objects.h"
#include "StandardLib/Array.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageModifierDrawBox.h"
#include "SupportLib/ImageCopier.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"
#include "TestReadImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> ReallocateImageAndReturn(Ptr<CImage> pImage)
{
	Ptr<CImage>	pNewImage;

	pNewImage = OMalloc<CImage>(pImage);
	pNewImage->White();
	return pNewImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArray<CImage>> SomethingWeirdIsGoingOn(void)
{
	Ptr<CImage>				pImage;
	Ptr<CArray<CImage>>		pArray;

	pArray = OMalloc<CArray<CImage>>();
	pImage = OMalloc<CImage>(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	pImage = ReallocateImageAndReturn(pImage);

	pArray->Add(pImage);

	return pArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImagePassOnstack(void)
{
	ObjectsInit();
	{
		Ptr<CRoot>				pRoot;
		Ptr<CArray<CImage>>		pArray;
		Ptr<CImage>				pImage;
		uint32*					piData;

		pRoot = ORoot();
		pArray = SomethingWeirdIsGoingOn();
		pRoot->Add(pArray);

		pImage = pArray->Get(0);
		AssertSize(3, pImage->GetWidth());
		AssertSize(2, pImage->GetHeight());

		piData = (uint32*)pImage->GetData();
		AssertInt(0xffffffff, *piData);

		pArray = NULL;
		pRoot->RemoveAll();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageKillChannels(void)
{
	ObjectsInit();
	{
		CChannels* pcChannelsSource;
		CChannels* pcChannelsDest;

		AssertSize(0, gcObjects.NumMemoryIndexes());

		Ptr<CImage> pImageSource = OMalloc<CImage>(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImageSource->Black();
		Ptr<CImage> pImageDest = OMalloc<CImage>(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pImageDest->Black();

		AssertSize(2, gcObjects.NumMemoryIndexes());

		pcChannelsSource = &pImageSource->mcChannels;
		pcChannelsDest = &pImageDest->mcChannels;

		AssertSize(6, pcChannelsSource->GetSize());
		AssertSize(2, pImageSource.NumEmbedded());  //Two because the Image is considered to be embedded in itself along with the Channels.
		pImageSource = NULL;
		AssertSize(0, pImageSource.NumEmbedded());

		AssertSize(6, pcChannelsDest->GetSize());
		AssertSize(2, pImageDest.NumEmbedded());
		pImageDest = NULL;
		AssertSize(0, pImageDest.NumEmbedded());

		AssertSize(0, gcObjects.NumMemoryIndexes());
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCopier(void)
{
	ObjectsInit();
	{
		CImage						cImageDest;
		CImageModifierDrawBox		cBox;
		CImage						cImageSource;
		char						szSourceRGB[18] = "ABCDEFGHIJKLMNOPQ";
		char						szSourceRB[12] = "ACDFGIJLMOP";
		char						szSourceR2G2B2[36] = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqR";
		char*						pcData;
		bool						bResult;
		char						szDestSameType[18] = "BACEDFHGIKJLNMOQP";
		char						szDestRGBMissingType[18] = "A\0CD\0FG\0IJ\0LM\0OP\0";
		char						szDestRGMissingType[13] = "ABDEGHJKMNPQ";
		char						szDestRGBDifferentType[36] = "AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQ";
		char						szDestRGBDifferentType2[18] = "abcdefghijklmnopq";
		CImageColourRGB				cRGB;
		CChannels*					pcChannelsSource;
		CChannels*					pcChannelsDest;

		//Same format, Same types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;
		cImageDest.Black();

		cImageSource.Init(3, 2, szSourceRGB, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertTrue(bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0, NULL);
		pcData = (char*)cImageDest.GetData();
		AssertString(szSourceRGB, pcData);

		AssertSize(0, gcObjects.NumMemoryIndexes());

		AssertSize(6, pcChannelsSource->GetSize());
		AssertSize(2, cImageSource.NumEmbedded());
		cImageSource.Kill();
		AssertSize(0, pcChannelsSource->GetSize());
		AssertSize(6, pcChannelsDest->GetSize());
		AssertSize(2, cImageDest.NumEmbedded());
		cImageDest.Kill();
		AssertSize(0, pcChannelsDest->GetSize());
		AssertSize(0, gcObjects.NumMemoryIndexes());

		//Different format, Same types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint8, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;
		cImageDest.Black();

		cImageSource.Init(3, 2, szSourceRGB, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertBool(false, bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0, NULL);
		pcData = (char*)cImageDest.GetData();
		AssertString(szDestSameType, pcData);

		AssertSize(6, pcChannelsSource->GetSize());
		AssertSize(2, cImageSource.NumEmbedded());
		cImageSource.Kill();
		AssertSize(0, pcChannelsSource->GetSize());
		AssertSize(6, pcChannelsDest->GetSize());
		AssertSize(2, cImageDest.NumEmbedded());
		cImageDest.Kill();
		AssertSize(0, pcChannelsDest->GetSize());

		//Different format, Missing source types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;

		cRGB.Init(0, 0, 0);
		cBox.Init(NULL, &cRGB);
		cBox.Modify(&cImageDest);
		cBox.Kill();

		cImageSource.Init(3, 2, szSourceRB, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertBool(false, bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0, NULL);
		pcData = (char*)cImageDest.GetData();
		AssertMemory(szDestRGBMissingType, pcData, 18);

		cImageSource.Kill();
		cImageDest.Kill();

		//Different format, Missing dest types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;
		cImageDest.Black();

		cImageSource.Init(3, 2, szSourceRGB, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertBool(false, bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0 , NULL);
		pcData = (char*)cImageDest.GetData();
		AssertMemory(szDestRGMissingType, pcData, 12);

		cImageSource.Kill();
		cImageDest.Kill();

		//Different types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint16, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;
		cImageDest.Black();
		AssertSize(3*2 * 3*2, cImageDest.GetByteSize());

		cImageSource.Init(3, 2, szSourceRGB, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertBool(false, bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0, NULL);
		pcData = (char*)cImageDest.GetData();
		AssertString(szDestRGBDifferentType, pcData);

		cImageSource.Kill();
		cImageDest.Kill();

		//Different types ------------------------------------------
		cImageDest.Init(3, 2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsDest = &cImageDest.mcChannels;
		cImageDest.Black();
		AssertSize(3*2 * 3, cImageDest.GetByteSize());

		cImageSource.Init(3, 2, szSourceR2G2B2, PT_uint16, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcChannelsSource = &cImageSource.mcChannels;

		bResult = cImageSource.IsSameFormat(&cImageDest);
		AssertBool(false, bResult);

		CImageCopier::Copy(&cImageSource, &cImageDest, 0, 0, NULL);
		pcData = (char*)cImageDest.GetData();

		AssertString(szDestRGBDifferentType2, pcData);

		cImageSource.Kill();
		cImageDest.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCopierRealPNG(void)
{
	char				szDirectory[] = "Output" _FS_ "CopierRealPNG";
	CIndexTreeMemory	cMemory;
	CFileUtil			cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CImage>		pImage;
		CArrayChannel	asChannels;
		SChannel*		psChannel;
		Ptr<CImage>		pDestImage;
		size			uiWidth;
		size			uiHeight;
		CImageCopier	cCopier;
		CChars			szOutputFilename;
		CChars			szInputFilename;
		bool			bWritten;

		pImage = TestReadImage("SpaceCrusade", "SoulSucker.png");

		asChannels.Init();
		pImage->GetAllChannels(&asChannels);

		AssertSize(4, asChannels.NumElements());
		psChannel = asChannels.Get(0);
		AssertSize(IMAGE_OPACITY, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);
		AssertFalse(psChannel->bReverse);

		psChannel = asChannels.Get(1);
		AssertSize(IMAGE_DIFFUSE_RED, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);

		psChannel = asChannels.Get(2);
		AssertSize(IMAGE_DIFFUSE_GREEN, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);

		psChannel = asChannels.Get(3);
		AssertSize(IMAGE_DIFFUSE_BLUE, psChannel->iChannel);
		AssertInt(PT_uint8, psChannel->eType);

		asChannels.Kill();

		uiWidth = pImage->GetWidth() / 2;
		uiHeight = pImage->GetHeight();
		pDestImage = OMalloc<CImage>((uint32)uiWidth, (uint32)uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		AssertFalse(pDestImage.IsNull());

		pDestImage->Black();
		cCopier.Init(pImage, pDestImage);
		cCopier.Copy(0, 0);
		cCopier.Kill();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "SoulSucker.png");

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
void AssertImageSize(int32 iWidth, int32 iHeight, int32 iExpectedWidth, int32 iExpectedHeight, lsize uiExpectedByteSize)
{
	CImage	cImage;

	cImage.Init(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	AssertInt(iExpectedWidth, cImage.GetWidth());
	AssertInt(iExpectedHeight, cImage.GetHeight());
	AssertLSize(uiExpectedByteSize, cImage.GetByteSize());
	if (uiExpectedByteSize == 0)
	{
		AssertNull(cImage.GetData());
	}
	else
	{
		AssertNotNull(cImage.GetData());
	}
	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Sizes that can't be addressed give an empty (0 x 0) image rather than a
// wrapped size with too little data.
//
//////////////////////////////////////////////////////////////////////////
void TestImageSizeLimits(void)
{
	SLogConfig	sLogConfig;

	ObjectsInit();

	AssertImageSize(13, 7, 13, 7, 13 * 7 * 4);

	sLogConfig = gcLogger.SetSilent();
	AssertImageSize(-5, 10, 0, 0, 0);			//Negative width.
#ifndef BUILD_64
	//These can be addressed in a 64 bit build, where they would allocate 6.4GB to 19.6GB.
	AssertImageSize(65536, 65536, 0, 0, 0);		//Width * height overflows 32 bits.
	AssertImageSize(70000, 70000, 0, 0, 0);		//Width * height overflows 32 bits.
	AssertImageSize(40000, 40000, 0, 0, 0);		//Width * height fits but the bytes (6.4GB) can't be addressed.
#endif // BUILD_64
	gcLogger.SetConfig(&sLogConfig);

	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// An image can draw onto memory it doesn't own (e.g. a WinGDI DIB
// section).  Black and White must still clear it.
//
//////////////////////////////////////////////////////////////////////////
void TestImageBlackWhiteUserData(void)
{
	Ptr<CImage>	pImage;
	uint8		auiData[3 * 2 * 4 + 1];
	size		i;

	ObjectsInit();

	memset(auiData, 0x5A, sizeof(auiData));
	pImage = OMalloc<CImage>(3, 2, (void*)auiData, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_STOP);
	AssertPointer(auiData, pImage->GetData());

	pImage->Black();
	for (i = 0; i < 3 * 2 * 4; i++)
	{
		AssertInt(0, auiData[i]);
	}
	AssertInt(0x5A, auiData[3 * 2 * 4]);

	pImage->White();
	for (i = 0; i < 3 * 2 * 4; i++)
	{
		AssertInt(0xFF, auiData[i]);
	}
	AssertInt(0x5A, auiData[3 * 2 * 4]);

	pImage = NULL;

	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImage(void)
{
	BeginTests();

	DataIOInit();

	TestImagePassOnstack();
	TestImageKillChannels();
	TestImageCopier();
	TestImageCopierRealPNG();
	TestImageSizeLimits();
	TestImageBlackWhiteUserData();

	DataIOKill();

	TestStatistics();
}


