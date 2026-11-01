#include "BaseLib/NaiveFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageModifierStack.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageRGBToGrey.h"
#include "SupportLib/BumpMapper.h"
#include "SupportLib/ImageResampler.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageModifierStack1(void)
{
	CImageModifierStack		cStack;
	
	Ptr<CImage> pcBak = ReadImage("Input/Adelle.png", IT_Unknown, true);
	AssertTrue(pcBak.IsNotNull());

	Ptr<CImage> pcImage = OMallocNoI<CImage>();
	pcImage->Copy(pcBak);
	cStack.Init(pcImage);

	cStack.AddModifier<CImageRGBToGrey>(RGBTGS_UseRed);
	cStack.ApplyAll();

	WriteImage(pcImage, "Output/AdelleGrey.raw");
	AssertFileMemory("Input/AdelleGrey.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;

	pcImage = OMallocNoI<CImage>();
	cStack.AddModifier<CImageHeightToNormals>(IMAGE_DIFFUSE_GREY);
	pcImage->Copy(pcBak);
	cStack.SetImage(pcImage);
	cStack.ApplyAll();

	WriteImage(pcImage, "Output/AdelleNormal.raw");
	AssertFileMemory("Input/AdelleNormal.raw", pcImage->GetData(), pcImage->GetByteSize());
	pcImage = NULL;

	pcImage = OMallocNoI<CImage>();
	cStack.AddModifier<CImageResampler>(IR_NearestNeighbour, 21, 16);
	pcImage->Copy(pcBak);
	cStack.SetImage(pcImage);
	cStack.ApplyAll();

	WriteImage(pcImage, "Output/AdelleSmall.raw");
	AssertFileMemory("Input/AdelleSmall.raw", pcImage->GetData(), pcImage->GetByteSize());
	AssertInt(3, cStack.NumModifiers());

	cStack.Kill();
	pcImage = NULL;

	pcBak = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageResamplerPowerOf2(void)
{
	CImageResampler		cResampler;
	Ptr<CImage>			pSource;
	Ptr<CImage>			pSquare;
	Ptr<CImage>			pDest;

	//21 x 12 rounds up to 32 x 16 and down to 16 x 8.  Auto scales by 1.5 then rounds down, giving 16 x 16.
	pSource = OMalloc<CImage>(21, 12, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	pSource->Black();

	cResampler.Init(IR_NearestNeighbour, RS_Up);
	pDest = cResampler.Modify(pSource);
	AssertInt((int32)32, pDest->GetWidth());
	AssertInt((int32)16, pDest->GetHeight());
	cResampler.Kill();

	cResampler.Init(IR_NearestNeighbour, RS_Down);
	pDest = cResampler.Modify(pSource);
	AssertInt((int32)16, pDest->GetWidth());
	AssertInt((int32)8, pDest->GetHeight());
	cResampler.Kill();

	cResampler.Init(IR_NearestNeighbour, RS_Auto);
	pDest = cResampler.Modify(pSource);
	AssertInt((int32)16, pDest->GetWidth());
	AssertInt((int32)16, pDest->GetHeight());
	cResampler.Kill();

	//An image that is already a power of two in size is returned as it is.
	pSquare = OMalloc<CImage>(16, 8, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	cResampler.Init(IR_NearestNeighbour, RS_Up);
	pDest = cResampler.Modify(pSquare);
	AssertTrue(pDest.Object() == pSquare.Object());
	cResampler.Kill();

	pDest = NULL;
	pSquare = NULL;
	pSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageModifierStack(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	ImageChannelDescriptorInit();

	TestImageModifierStack1();
	TestImageResamplerPowerOf2();

	ImageChannelDescriptorKill();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

