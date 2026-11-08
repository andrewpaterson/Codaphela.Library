#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageReader.h"
#include "SupportLib/ObjectConverterImage.h"
#include "TestLib/Assert.h"
#include "AssertImage.h"


//////////////////////////////////////////////////////////////////////////
//
// An image source's object is named its file's relative path, and is the
// image ReadImage reads from that file.
//
//////////////////////////////////////////////////////////////////////////
void AssertObjectConverterImage(const char* szFilename)
{
	Ptr<CImage>	pImage;
	Ptr<CImage>	pAgain;
	Ptr<CImage>	pExpected;

	pImage = gcObjects.Get<CImage>(szFilename);
	AssertTrue(pImage.IsNotNull());
	AssertTrue(pImage->IsNamed());
	AssertString(szFilename, pImage->GetName());

	pExpected = ReadImage((char*)szFilename);
	AssertFalse(pExpected->IsNamed());
	AssertSameImage(pExpected, pImage);

	pAgain = gcObjects.Get<CImage>(szFilename);
	AssertPointer(pImage.Object(), pAgain.Object());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterImageSources(void)
{
	CObjectConverterImage	cConverter;
	CObjectSource*			pcSource;
	Ptr<CImage>				pImage;
	SLogConfig				sLogConfig;

	cConverter.Init(&gcObjects);
	pcSource = cConverter.CreateSource(NULL, "Input/Tiny.png");
	AssertNotNull(pcSource);
	AssertTrue(pcSource->Contains("Input/Tiny.png"));
	AssertFalse(pcSource->Contains("Input/Tiny"));
	AssertFalse(pcSource->Contains(""));
	pcSource->Kill();

	AssertNull(cConverter.CreateSource(NULL, NULL));
	AssertNull(cConverter.CreateSource(NULL, ""));
	cConverter.Kill();

	AssertNotNull(gcObjects.AddSource<CObjectConverterImage>(NULL, "Input/Tiny.png"));
	AssertNotNull(gcObjects.AddSource<CObjectConverterImage>(NULL, "Input/Opacity.png"));
	AssertNotNull(gcObjects.AddSource<CObjectConverterImage>(NULL, "Input/BMP/pal4.bmp"));
	AssertNotNull(gcObjects.AddSource<CObjectConverterImage>(NULL, "Input/DoesNotExist.png"));

	AssertObjectConverterImage("Input/Tiny.png");
	AssertObjectConverterImage("Input/Opacity.png");
	AssertObjectConverterImage("Input/BMP/pal4.bmp");

	//The missing file is an error (logged), not a crash.
	sLogConfig = gcLogger.SetSilent();
	pImage = gcObjects.Get<CImage>("Input/DoesNotExist.png");
	gcLogger.SetConfig(&sLogConfig);
	AssertTrue(pImage.IsNull());
	pImage = gcObjects.Get<CImage>("Input/NotASource.png");
	AssertTrue(pImage.IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
// The readers name the image when they are given an object name.
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterImageNamedRead(void)
{
	Ptr<CImage>	pImage;

	pImage = ReadImage("Input/Tiny.png", IT_Unknown, false, "Tiny");
	AssertTrue(pImage.IsNotNull());
	AssertString("Tiny", pImage->GetName());
	AssertPointer(pImage.Object(), gcObjects.Get<CImage>("Tiny").Object());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestObjectConverterImage(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestObjectConverterImageSources();
	TestObjectConverterImageNamedRead();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

