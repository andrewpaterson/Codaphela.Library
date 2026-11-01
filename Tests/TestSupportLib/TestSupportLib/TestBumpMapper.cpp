#include "BaseLib/NaiveFile.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/BumpMapper.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBumpMapper1(void)
{
	Ptr<CImage> pcSource = ReadImage("Input" _FS_ "Heightmap.png");
	Ptr<CImage>	pcDest = OMalloc<CImage>();

	pcDest = ConvertHeightMapTo(true, true, pcSource, IMAGE_DIFFUSE_RED);

	pcDest->BeginChange();
	pcDest->RenameChannel(IMAGE_NORMAL_X, IMAGE_DIFFUSE_RED);
	pcDest->RenameChannel(IMAGE_NORMAL_Y, IMAGE_DIFFUSE_GREEN);
	pcDest->RenameChannel(IMAGE_NORMAL_Z, IMAGE_DIFFUSE_BLUE);
	pcDest->EndChange();

	WriteImage(pcDest, "Output" _FS_ "NormalMap.png", IT_PNG);

	pcDest->BeginChange();
	pcDest->RenameChannel(IMAGE_DIFFUSE_RED, IMAGE_NORMAL_X);
	pcDest->RenameChannel(IMAGE_DIFFUSE_GREEN, IMAGE_NORMAL_Y);

	pcDest->RenameChannel(IMAGE_BUMP_U, IMAGE_DIFFUSE_RED);
	pcDest->RenameChannel(IMAGE_BUMP_V, IMAGE_DIFFUSE_GREEN);
	pcDest->EndChange();

	WriteImage(pcDest, "Output" _FS_ "BumpMap.png", IT_PNG);

	AssertFile("Input" _FS_ "NormalMap.png", "Output" _FS_ "NormalMap.png");
	AssertFile("Input" _FS_ "BumpMap.png", "Output" _FS_ "BumpMap.png");

	//These are probably unnecessay.
	pcDest->Kill();
	pcSource->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBumpMapperHeightToBumpUVs(void)
{
	CImageHeightToBumpUVs	cToUVs;
	Ptr<CImage>				pFlat;
	Ptr<CImage>				pRamp;
	Ptr<CImage>				pFlatUVs;
	Ptr<CImage>				pRampUVs;
	uint8*					puiFlat;
	uint8*					puiRamp;
	int32					x;
	int32					y;
	size					uiFlatDifferent;
	size					uiRampUFlat;
	size					uiRampVSloped;

	//A height map becomes a two channel U V bump map of the same size.  A flat height map bumps nowhere so every
	//pixel is the same.  A height map that only rises across x bumps U but not V away from its edges.
	pFlat = OMalloc<CImage>(6, 6, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	pRamp = OMalloc<CImage>(6, 6, PT_uint8, IMAGE_DIFFUSE_GREY, CHANNEL_STOP);
	puiFlat = (uint8*)pFlat->GetData();
	puiRamp = (uint8*)pRamp->GetData();
	for (y = 0; y < 6; y++)
	{
		for (x = 0; x < 6; x++)
		{
			puiFlat[x + y * 6] = 128;
			puiRamp[x + y * 6] = (uint8)(x * 40);
		}
	}

	cToUVs.Init(IMAGE_DIFFUSE_GREY);
	pFlatUVs = cToUVs.Modify(pFlat);
	pRampUVs = cToUVs.Modify(pRamp);
	cToUVs.Kill();

	AssertInt((int32)6, pFlatUVs->GetWidth());
	AssertInt((int32)6, pFlatUVs->GetHeight());
	AssertTrue(pFlatUVs->HasChannels(IMAGE_BUMP_U, IMAGE_BUMP_V, CHANNEL_STOP));
	AssertSize(72, pFlatUVs->GetByteSize());
	AssertSize(72, pRampUVs->GetByteSize());

	puiFlat = (uint8*)pFlatUVs->GetData();
	puiRamp = (uint8*)pRampUVs->GetData();
	uiFlatDifferent = 0;
	for (x = 0; x < 36; x++)
	{
		if ((puiFlat[x * 2] != puiFlat[0]) || (puiFlat[x * 2 + 1] != puiFlat[1]))
		{
			uiFlatDifferent++;
		}
	}
	AssertSize(0, uiFlatDifferent);

	uiRampUFlat = 0;
	uiRampVSloped = 0;
	for (y = 1; y < 5; y++)
	{
		for (x = 1; x < 5; x++)
		{
			if (puiRamp[(x + y * 6) * 2] == puiFlat[0])
			{
				uiRampUFlat++;
			}
			if (puiRamp[(x + y * 6) * 2 + 1] != puiFlat[1])
			{
				uiRampVSloped++;
			}
		}
	}
	AssertSize(0, uiRampUFlat);
	AssertSize(0, uiRampVSloped);

	pFlat = NULL;
	pRamp = NULL;
	pFlatUVs = NULL;
	pRampUVs = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBumpMapper(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestBumpMapper1();
	TestBumpMapperHeightToBumpUVs();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
