#include "BaseLib/NaiveFile.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageRGBToGrey.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageRGBToGrey1(void)
{
	CImageRGBToGrey		cRGBToGrey;
	CArrayChannel		acChannels;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "RGBToGrey.png");
	
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();
	AssertInt((int32)32, pcImage->GetWidth());
	AssertInt((int32)32, pcImage->GetHeight());
	AssertSize(3072, pcImage->GetByteSize());

	cRGBToGrey.Init(RGBTGS_OnlyIfChannelsSame);
	pcImage = cRGBToGrey.Modify(pcImage);
	cRGBToGrey.Kill();
	AssertTrue(pcImage.IsNotNull());

	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();
	AssertInt((int32)32, pcImage->GetWidth());
	AssertInt((int32)32, pcImage->GetHeight());
	AssertSize(1024, pcImage->GetChannels()->GetSize());
	AssertSize(1024, pcImage->GetByteSize());

	WriteImage(pcImage, "Output" _FS_ "RGBToGrey.raw");
	AssertFileMemory("Input" _FS_ "RGBToGrey.raw", pcImage->GetData(), pcImage->GetByteSize());

	//This is probably unnecessay.
	pcImage->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageRGBToGrey(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageRGBToGrey1();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

