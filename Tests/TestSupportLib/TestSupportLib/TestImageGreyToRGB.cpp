#include "BaseLib/NaiveFile.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageGreyToRGB.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageGreyToRGB1(void)
{

	CImageGreyToRGB		cGreyToRGB;

	Ptr<CImage> pcImage = ReadImage("Input/basn0g08.png");

	cGreyToRGB.Init();
	cGreyToRGB.Modify(pcImage);
	cGreyToRGB.Kill();

	WriteImage(pcImage, "Output/GreyToRGB.raw");
	AssertFileMemory("Input/GreyToRGB.raw", pcImage->GetData(), pcImage->GetByteSize());

	cGreyToRGB.Kill();
	pcImage->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageGreyToRGB(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageGreyToRGB1();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

