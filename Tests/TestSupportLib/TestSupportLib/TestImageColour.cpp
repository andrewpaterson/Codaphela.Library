#include "BaseLib/GeometricTypes.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Unknowns.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageColour.h"
#include "SupportLib/ImageAccessor.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/Image.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageColourAccessorBytes(void)
{
	CImageColourRGB			cRGB; 
	CImageColourOpacity		cAlpha;
	CImageColourCombo2		cColour;
	CChannels*				pcChannels;
	CImageAccessor*			pcAccessor;
	CImage					cImage;
	SImageColour			sDest;
	bool					bResult;
	char*					pvData;

	cRGB.Init(1.0f, 0.5f, 0.25f);
	cAlpha.Init(0.333f);
	cColour.Init(&cRGB, &cAlpha);

	cImage.Init(10, 10, PT_uint8,	IMAGE_DIFFUSE_GREEN,	//0x7f
									IMAGE_NORMAL_Y,		
									IMAGE_DIFFUSE_BLUE,		//0x3f
									IMAGE_OPACITY,			//0x54
									IMAGE_DIFFUSE_RED,		//0xff
									IMAGE_MASK, 
									CHANNEL_STOP);
	cImage.Black();

	pcAccessor = CImageAccessorCreator::Create(&cImage, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);

	sDest.Full();
	bResult = pcAccessor->MakeColour(&sDest, &cColour);
	AssertTrue(bResult);

	//Remember ints have reverse endiannes which is why the test number below looks backwards.
	AssertIntHex(0x3f7fff54, *(int32*)((void*)sDest.c));

	pcAccessor->Set(1, 0, &sDest);
	pcAccessor->Kill();

	pcChannels = &cImage.mcChannels;

	pvData = pcChannels->GetData();

	//First pixel.  Didn't touch it.
	AssertChar(0x00, pvData[0x0]);
	AssertChar(0x00, pvData[0x1]);
	AssertChar(0x00, pvData[0x2]);
	AssertChar(0x00, pvData[0x3]);
	AssertChar(0x00, pvData[0x4]);
	AssertChar(0x00, pvData[0x5]);

	//Second pixel.
	AssertChar(0x7f, pvData[0x6]);
	AssertChar(0x00, pvData[0x7]);
	AssertChar(0x3f, pvData[0x8]);
	AssertChar(0x54, pvData[0x9]);
	AssertChar((char)0xff, pvData[0xa]);
	AssertChar(0x00, pvData[0xb]);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageColourAccessorFloats(void)
{
	CImageColourNormal	cNormal;
	CChannels*			pcChannels;
	CImageAccessor*		pcAccessor;
	CImage				cImage;
	SImageColour		sDest;
	bool				bResult;

	cNormal.Init(0.7f, 0.6f, 0.5f);
	cImage.Init();
	cImage.BeginChange();
	cImage.AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
	cImage.AddChannel(IMAGE_NORMAL_Y, PT_float32);
	cImage.AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
	cImage.AddChannel(IMAGE_OPACITY, PT_int16);
	cImage.AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
	cImage.AddChannel(IMAGE_MASK, PT_int32);
	cImage.AddChannel(IMAGE_NORMAL_Z, PT_float32);
	cImage.AddChannel(IMAGE_NORMAL_X, PT_float32);
	cImage.SetSize(1, 2);
	cImage.EndChange();
	cImage.Black();

	pcAccessor = CImageAccessorCreator::Create(&cImage, PT_float32, IMAGE_NORMAL_X, IMAGE_NORMAL_Y, IMAGE_NORMAL_Z, CHANNEL_STOP);

	sDest.Full();
	bResult = pcAccessor->MakeColour(&sDest, &cNormal);
	AssertTrue(bResult);

	AssertFloat(0.7f, *((float*)&((sDest).c)[0x0]), 3);
	AssertFloat(0.6f, *((float*)&((sDest).c)[0x4]), 3);
	AssertFloat(0.5f, *((float*)&((sDest).c)[0x8]), 3);

	pcAccessor->Set(0, 1, &sDest);
	pcAccessor->Kill();

	pcChannels = &cImage.mcChannels;
	
	AssertFloat(0.6f, *(float*)&(pcChannels->GetData()[22]), 3);
	AssertFloat(0.7f, *(float*)&(pcChannels->GetData()[38]), 3);
	AssertFloat(0.5f, *(float*)&(pcChannels->GetData()[34]), 3);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageColourMultiAccessor(void)
{
	CImageColourNormal		cNormal;
	CImageColourOpacity		cAlpha;
	CImageColourCombo2		cColour;
	CChannels*				pcChannels;
	CImageAccessor*			pcAccessor;
	CImage					cImage;
	SImageColour			sDest;
	bool					bResult;

	cNormal.Init(0.7f, -0.6f, -0.5f);
	cAlpha.Init(0.333f);
	cColour.Init(&cNormal, &cAlpha);

	cImage.Init();
	cImage.BeginChange();
	cImage.AddChannel(IMAGE_DIFFUSE_GREEN, PT_uint8);
	cImage.AddChannel(IMAGE_NORMAL_Y, PT_float32);
	cImage.AddChannel(IMAGE_DIFFUSE_BLUE, PT_uint8);
	cImage.AddChannel(IMAGE_OPACITY, PT_uint16);
	cImage.AddChannel(IMAGE_DIFFUSE_RED, PT_uint8);
	cImage.AddChannel(IMAGE_MASK, PT_int32);
	cImage.AddChannel(IMAGE_NORMAL_Z, PT_float32);
	cImage.AddChannel(IMAGE_NORMAL_X, PT_float32);
	cImage.SetSize(1, 2);
	cImage.EndChange();
	cImage.Black();

	pcAccessor = CImageAccessorCreator::Create(&cImage, IMAGE_NORMAL_X, IMAGE_NORMAL_Y, IMAGE_NORMAL_Z, IMAGE_OPACITY, CHANNEL_STOP);

	sDest.Full();
	bResult = pcAccessor->MakeColour(&sDest, &cColour);
	AssertTrue(bResult);

	AssertFloat(0.7f, *((float*)&((sDest).c)[0x0]), 3);
	AssertFloat(-0.6f, *((float*)&((sDest).c)[0x4]), 3);
	AssertFloat(-0.5f, *((float*)&((sDest).c)[0x8]), 3);
	AssertShortHex((uint16)0x553f, *((uint16*)&((sDest).c)[0xc]));

	pcAccessor->Set(0, 1, &sDest);
	pcAccessor->Kill();

	pcChannels = &cImage.mcChannels;
	AssertFloat(-0.6f, *(float*)&(pcChannels->GetData()[22]), 3);
	AssertFloat( 0.7f, *(float*)&(pcChannels->GetData()[38]), 3);
	AssertFloat(-0.5f, *(float*)&(pcChannels->GetData()[34]), 3);
	AssertShortHex((uint16)0x553f, *(uint16*)&(pcChannels->GetData()[27]));

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageColourStruct(void)
{
	SImageColour	sColour;
	int				i;

	sColour.Full();

	sColour.c[0] = 0;
	AssertBool(true, sColour.IsZero(1));

	sColour.c[0] = 3;
	AssertBool(false, sColour.IsZero(1));

	sColour.c[0] = 0xff;

	for (i = 0; i < MAX_IMAGE_COLOUR_BYTES; i++)
	{
		sColour.c[i] = 0;
		AssertBool(true, sColour.IsZero(i+1));
	}

	for (i = MAX_IMAGE_COLOUR_BYTES-1; i >= 0; i--)
	{
		sColour.c[i] = 0xff;
		AssertBool(false, sColour.IsZero(i+1));
	}

	sColour.Zero();
	AssertBool(true, sColour.IsZero(8));

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageColour(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageColourStruct();
	TestImageColourAccessorBytes();
	TestImageColourAccessorFloats();
	TestImageColourMultiAccessor();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

