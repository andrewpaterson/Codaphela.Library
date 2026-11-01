#include "BaseLib/NaiveFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageReaderRAD(void)
{
	CArrayChannel	acChannels;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "readrad.rad");
	AssertTrue(pcImage.IsNotNull());
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "readrad.raw");
	AssertFileMemory("Input" _FS_ "readrad.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	//Read raw is a special case.  Because the size and the channels aren't known the image must be initialised before hand.
	pcImage = OMalloc<CImage>(32, 48, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	ReadRawImage(pcImage, "Input" _FS_ "readrad.raw");
	AssertTrue(pcImage.IsNotNull());
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "readraw.raw");
	AssertFileMemory("Input" _FS_ "readraw.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageReaderPNG(void)
{
	CArrayChannel	acChannels;	

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "basn0g01.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn0g01.raw");
	AssertFileMemory("Input" _FS_ "basn0g01.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn0g02.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn0g02.raw");
	AssertFileMemory("Input" _FS_ "basn0g02.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn0g04.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn0g04.raw");
	AssertFileMemory("Input" _FS_ "basn0g04.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn0g08.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(0));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn0g08.raw");
	AssertFileMemory("Input" _FS_ "basn0g08.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn0g16.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(1, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint16, acChannels.Get(0));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn0g16.raw");
	AssertFileMemory("Input" _FS_ "basn0g16.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn2c08.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn2c08.raw");
	AssertFileMemory("Input" _FS_ "basn2c08.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn2c16.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint16, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint16, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint16, acChannels.Get(2));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn2c16.raw");
	AssertFileMemory("Input" _FS_ "basn2c16.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn4a08.png");
	AssertInt((int32)32, pcImage->GetWidth());
	AssertInt((int32)32, pcImage->GetHeight());
	AssertSize(2048, pcImage->GetByteSize());
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(2, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint8, acChannels.Get(1));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn4a08.raw");
	AssertFileMemory("Input" _FS_ "basn4a08.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn4a16.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(2, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint16, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Intensity, PT_uint16, acChannels.Get(1));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn4a16.raw");
	AssertFileMemory("Input" _FS_ "basn4a16.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn6a08.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(4, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(2));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(3));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn6a08.raw");
	AssertFileMemory("Input" _FS_ "basn6a08.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "basn6a16.png");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(4, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint16, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Red, PT_uint16, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint16, acChannels.Get(2));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint16, acChannels.Get(3));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "basn6a16.raw");
	AssertFileMemory("Input" _FS_ "basn6a16.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageReaderTGA(void)
{
	CArrayChannel	acChannels;	

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "tar32un.tga");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(4, acChannels.NumElements());
	AssertChannel(IP_Opacity, CT_Intensity, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(2));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(3));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "tar32un.raw");
	AssertFileMemory("Input" _FS_ "tar32un.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "tar24un.tga");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();


	WriteImage(pcImage, "Output" _FS_ "tar24un.raw");
	AssertFileMemory("Input" _FS_ "tar24un.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;


	pcImage = ReadImage("Input" _FS_ "tar16un.tga");
	acChannels.Init();
	pcImage->GetAllChannels(&acChannels);
	AssertSize(3, acChannels.NumElements());
	AssertChannel(IP_Diffuse, CT_Red, PT_uint8, acChannels.Get(0));
	AssertChannel(IP_Diffuse, CT_Green, PT_uint8, acChannels.Get(1));
	AssertChannel(IP_Diffuse, CT_Blue, PT_uint8, acChannels.Get(2));
	acChannels.Kill();

	WriteImage(pcImage, "Output" _FS_ "tar16un.raw");
	AssertFileMemory("Input" _FS_ "tar16un.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageReader(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageReaderRAD();
	TestImageReaderPNG();
	TestImageReaderTGA();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

