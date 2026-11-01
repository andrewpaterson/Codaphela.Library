#include "BaseLib/FileUtil.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageRGBToGrey.h"
#include "SupportLib/ImageModifierDrawBox.h"
#include "SupportLib/ImageModifierStack.h"
#include "SupportLib/ImageChannelRename.h"
#include "SupportLib/ImageChannelAdd.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSwizzle1(void)
{
	CImageModifierStack		cStack;
	Ptr<CImageChannelAdd>	pChannelAdd;
	CImageColourRGB			cRGB;

	Ptr<CImage> pcImage = ReadImage("Input" _FS_ "swizzel.png");
	AssertInt((int32)25, pcImage->GetWidth());
	AssertInt((int32)25, pcImage->GetHeight());

	cStack.Init(pcImage);
	
	cStack.AddModifier<CImageRGBToGrey>(RGBTGS_OnlyIfChannelsSame);

	cStack.AddModifier<CImageChannelRename>(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY);

	pChannelAdd = cStack.AddModifier<CImageChannelAdd>(IMAGE_DIFFUSE_RED, PT_uint8);
	pChannelAdd->AddChannel(IMAGE_DIFFUSE_GREEN);
	pChannelAdd->AddChannel(IMAGE_DIFFUSE_BLUE);
	pChannelAdd = NULL;

	cRGB.Init(1.0f, 1.0f, 1.0f);
	cStack.AddModifier<CImageModifierDrawBox>((CInt32Rect*)NULL, &cRGB, true);

	cStack.ApplyAll();
	cStack.Kill();

	WriteImage(pcImage, "Output" _FS_ "swizzle.raw");
	AssertFileMemory("Input" _FS_ "swizzle.raw", pcImage->GetData(), pcImage->GetByteSize());

	pcImage->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageSwizzle(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageSwizzle1();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}



