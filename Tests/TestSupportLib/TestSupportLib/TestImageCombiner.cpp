#include "BaseLib/FileUtil.h"
#include "BaseLib/Mallocator.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/ImageCelsSource.h"
#include "SupportLib/ImageRGBToGrey.h"
#include "SupportLib/ImageCelSourceBorders.h"
#include "SupportLib/ImageSourceDiskFile.h"
#include "SupportLib/ImageCombiner.h"
#include "SupportLib/ImageCelSourceSingle.h"
#include "SupportLib/ImageCelsWriter.h"
#include "SupportLib/ImageCelsReader.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombinerMask(void)
{
	CImageCelSourceBorders	cBorder;
	CImageCombiner			cCombiner;
	Ptr<CImage>				pcImage;
	CImageCelsSource		cSource;
	SImageColour			sColour;

	sColour.c[0] = 0;
	sColour.c[1] = 0;
	sColour.c[2] = 0;
	sColour.c[3] = 0;

	cBorder.Init(&sColour);

	cSource.Init();
	cSource.AddDiskFileSource("Input" _FS_ "cel4.png", "Cel 4", &cBorder);
	cSource.AddDiskFileSource("Input" _FS_ "cel6.png", "Cel 6", &cBorder);

	cSource.Load();

	cCombiner.Init(ICL_Best, ICS_Arbitrary, ICC_FromCels);
	cCombiner.AddCels(cSource.GetImageCels());
	pcImage = cCombiner.Combine();

	WriteImage(&pcImage, "Output" _FS_ "CombineMask.png");
	WriteImage(&pcImage, "Output" _FS_ "CombineMask.raw");
	AssertFileMemory("Input" _FS_ "CombineMask.raw", pcImage->GetChannels()->GetData(), pcImage->GetByteSize());

	pcImage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombinerStarControl(void)
{
	CImageCelSourceSingle	cSingle;
	Ptr<CImageCombiner>		pCombiner;
	Ptr<CImage>				pcImage;
	Ptr<CImageCelsSource>	pSource;
	CFileUtil				cFileUtil;
	CArrayChars				aszDirectories;
	size					i;
	bool					bResult;
	Ptr<CArrayImageCel>		pacCels;
	Ptr<CArrayImageCel>		pacLoadedCels;
	size					uiMaxDebugAllocatorSize;
	int32					iWidth;
	int32					iHeight;

	//The combined image is about 18MB, larger than the debug allocation limit set in main.
	uiMaxDebugAllocatorSize = guiMaxDebugAllocatorSize;
	InitAllocatorDebug(32 MB);

	//Validating every object each time one is added is too slow for hundreds of cels; validate once at the end.
	gcObjects.DisableValidation();

	//The objects are rooted so that pointing to them doesn't make the objects work out again which are on the stack.
	cSingle.Init();
	pSource = OMalloc<CImageCelsSource>(true);
	ORoot()->Add(pSource);

	aszDirectories.Init();
	cFileUtil.FindAllDirectories("Input" _FS_ "StarControl", &aszDirectories);
	aszDirectories.QuickSort();
	AssertSize(5, aszDirectories.NumElements());
	for (i = 0; i < aszDirectories.NumElements(); i++)
	{
		pSource->AddDiskFileSources(aszDirectories.Get(i)->Text(), "frame", NULL, &cSingle);
	}
	aszDirectories.Kill();

	pSource->Load();
	AssertSize(360, pSource->GetImageCels()->NumElements());

	AssertSize(360, pSource->GetCelFileNames()->NumElements());

	pCombiner = OMalloc<CImageCombiner>(ICL_Best, ICS_Arbitrary, ICC_FromCels, 0, 0, true);
	ORoot()->Add(pCombiner);
	pCombiner->AddCels(pSource->GetImageCels(), pSource->GetCelFileNames());
	pcImage = pCombiner->Combine();
	AssertTrue(pcImage.IsNotNull());

	pacCels = pCombiner->GetCels();
	AssertSize(360, pacCels->NumElements());
	AssertTrue(pacCels->Get(0)->GetSourceImage() == pcImage);
	AssertString("Input/StarControl/Earthling Cruiser/frame0000", pacCels->Get(0)->GetName());
	AssertString("Input/StarControl/Earthling Cruiser/frame0071", pacCels->Get(71)->GetName());
	AssertString("Input/StarControl/Ur-Quan Dreadnought/frame0071", pacCels->Get(359)->GetName());

	bResult = WriteImage(pcImage, "Output" _FS_ "StarControl.png", IT_PNG);
	AssertTrue(bResult);
	bResult = CelsWriter(pacCels, "Output" _FS_ "StarControl.png", "Output" _FS_ "StarControl.cels");
	AssertTrue(bResult);

	iWidth = pcImage->GetWidth();
	iHeight = pcImage->GetHeight();

	//Release the combined cels so the loaded cels can take the same names.
	pacCels = NULL;
	pcImage = NULL;
	ORoot()->Remove(pCombiner);
	ORoot()->Remove(pSource);
	pCombiner = NULL;
	pSource = NULL;

	pacLoadedCels = CelsReader("Output" _FS_ "StarControl.cels");
	AssertTrue(pacLoadedCels.IsNotNull());
	AssertSize(360, pacLoadedCels->NumElements());
	AssertInt((int32)iWidth, pacLoadedCels->Get(0)->GetSourceImage()->GetWidth());
	AssertInt((int32)iHeight, pacLoadedCels->Get(0)->GetSourceImage()->GetHeight());
	AssertString("Input/StarControl/Earthling Cruiser/frame0000", pacLoadedCels->Get(0)->GetName());
	AssertString("Input/StarControl/Ur-Quan Dreadnought/frame0071", pacLoadedCels->Get(359)->GetName());

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	pacLoadedCels = NULL;

	InitAllocatorDebug(uiMaxDebugAllocatorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombinerEmptyCels(void)
{
	CImageCombiner	cCombiner;
	Ptr<CImage>		pcSource;
	Ptr<CImage>		pcImage;

	pcSource = OMalloc<CImage>(4, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);

	//Every cel is empty so nothing is packed.
	cCombiner.Init(ICL_Best, ICS_Arbitrary, ICC_FromCels);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 0, 0));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 0, 0));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNull());
	cCombiner.Kill();

	cCombiner.Init(ICL_Best, ICS_PowerOf2, ICC_FromCels);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 0, 0));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNull());
	cCombiner.Kill();

	pcSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombinerUserSpecifiedSize(void)
{
	CImageCombiner		cCombiner;
	Ptr<CImage>			pcSource;
	Ptr<CImage>			pcImage;
	Ptr<CArrayImageCel>	pacCels;
	CInt32Rect*			pcRect;
	size				i;

	pcSource = OMalloc<CImage>(12, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);

	//Three 4x4 cels in a row with a one pixel outside edge exactly fill 14x6.
	cCombiner.Init(ICL_Best, 14, 6, ICC_FromCels, 1, 0);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 4, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 4, 0, 8, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 8, 0, 12, 4));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNotNull());
	AssertInt((int32)14, pcImage->GetWidth());
	AssertInt((int32)6, pcImage->GetHeight());

	pacCels = cCombiner.GetCels();
	AssertSize(3, pacCels->NumElements());
	for (i = 0; i < pacCels->NumElements(); i++)
	{
		pcRect = &pacCels->Get(i)->GetSubImage()->mcImageRect;
		AssertInt((int32)4, pcRect->GetWidth());
		AssertInt((int32)4, pcRect->GetHeight());
		AssertInt((int32)1, pcRect->msTopLeft.y);
		AssertTrue(pcRect->msTopLeft.x >= 1);
		AssertTrue(pcRect->msBottomRight.x <= 13);
	}
	pacCels = NULL;
	pcImage = NULL;
	cCombiner.Kill();

	//Inside the outside edge 13x6 is only 11 wide so the third cel needs a second row, which is too high.
	cCombiner.Init(ICL_Best, 13, 6, ICC_FromCels, 1, 0);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 4, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 4, 0, 8, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 8, 0, 12, 4));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNull());
	cCombiner.Kill();

	//No attempt can hold a 4 wide cel in 3 pixels.
	cCombiner.Init(ICL_Best, 3, 8, ICC_FromCels, 0, 0);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 4, 4));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNull());
	cCombiner.Kill();

	pcSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertCombinedCelRect(Ptr<CArrayImageCel> pacCels, size iIndex, int iLeft, int iTop, int iRight, int iBottom)
{
	CInt32Rect*	pcRect;

	pcRect = &pacCels->Get(iIndex)->GetSubImage()->mcImageRect;
	AssertInt((int32)iLeft, pcRect->msTopLeft.x);
	AssertInt((int32)iTop, pcRect->msTopLeft.y);
	AssertInt((int32)iRight, pcRect->msBottomRight.x);
	AssertInt((int32)iBottom, pcRect->msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombinerLineLayoutGaps(void)
{
	CImageCombiner		cCombiner;
	Ptr<CImage>			pcSource;
	Ptr<CImage>			pcImage;
	Ptr<CArrayImageCel>	pacCels;

	pcSource = OMalloc<CImage>(8, 4, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);

	//A 3x4 and a 4x4 cel side by side with a one pixel outside edge and a one pixel gap.
	cCombiner.Init(ICL_LeftToRight, ICS_Arbitrary, ICC_FromCels, 1, 1);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 3, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 4, 0, 8, 4));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNotNull());
	AssertInt((int32)10, pcImage->GetWidth());
	AssertInt((int32)6, pcImage->GetHeight());

	pacCels = cCombiner.GetCels();
	AssertSize(2, pacCels->NumElements());
	AssertCombinedCelRect(pacCels, 0, 1, 1, 4, 5);
	AssertCombinedCelRect(pacCels, 1, 5, 1, 9, 5);
	pacCels = NULL;
	pcImage = NULL;
	cCombiner.Kill();

	//The same cels one above the other.
	cCombiner.Init(ICL_TopToBottom, ICS_Arbitrary, ICC_FromCels, 1, 1);
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 0, 0, 3, 4));
	cCombiner.AddCel(OMalloc<CImageCel>(pcSource, 4, 0, 8, 4));
	pcImage = cCombiner.Combine();
	AssertTrue(pcImage.IsNotNull());
	AssertInt((int32)6, pcImage->GetWidth());
	AssertInt((int32)11, pcImage->GetHeight());

	pacCels = cCombiner.GetCels();
	AssertSize(2, pacCels->NumElements());
	AssertCombinedCelRect(pacCels, 0, 1, 1, 4, 5);
	AssertCombinedCelRect(pacCels, 1, 1, 6, 5, 10);
	pacCels = NULL;
	pcImage = NULL;
	cCombiner.Kill();

	pcSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestImageCombiner(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestImageCombinerMask();
	TestImageCombinerStarControl();
	TestImageCombinerEmptyCels();
	TestImageCombinerUserSpecifiedSize();
	TestImageCombinerLineLayoutGaps();

	ObjectsFlush();
	ObjectsKill(false);
	DataIOKill();
	
	TestStatistics();
}

