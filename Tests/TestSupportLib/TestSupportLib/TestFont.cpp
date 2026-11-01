#include <string.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Timer.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/Font.h"
#include "SupportLib/FontFactory.h"
#include "SupportLib/Text.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/TextUTF16Short.h"
#include "SupportLib/TextUTF16Long.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/GlyphListFontDraw.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/Maps.h"
#include "SupportLib/MapViewport.h"
#include "SupportLib/TrueTypeFont.h"
#include "SupportLib/TrueTypeFontFactory.h"
#include "SupportLib/FontWriter.h"
#include "SupportLib/FontReader.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertUTFChar(char* szExpected, char* szActual, size uiActualLength)
{
	size	uiExpectedLength;

	uiExpectedLength = strlen(szExpected);
	AssertSize(uiExpectedLength, uiActualLength);
	AssertMemory(szExpected, szActual, uiActualLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontCreateFont(void)
{
	ObjectsInit();
	{
		CFontFactory				cFactory;
		CFontImportParameters		cImportParams;
		CFontCreationParameters		cCreateParams;
		CChars						szCharacterFile;
		CChars						szImageFile;
		Ptr<CFont>					pFont;
		char						szDirectory[] = { "Input" _FS_ "Font Pack" _FS_ };
		bool						bExists;
		SIndexTreeMemoryIterator	sIter;
		char						szCharacter[5];
		size						uiLength;

		cCreateParams.Init(8, 8);

		szCharacterFile.Init(szDirectory);
		szCharacterFile.Append("anuvverbubbla_8x8.txt");
		szImageFile.Init(szDirectory);
		szImageFile.Append("anuvverbubbla_8x8.png");

		bExists = cImportParams.Init(szImageFile.Text(), szCharacterFile.Text(), 8, 8);
		AssertTrue(bExists);
		cFactory.Init();
		pFont = cFactory.Generate(&cImportParams, &cCreateParams);
		AssertTrue(pFont.IsNotNull());

		AssertSize(73, pFont->NumGlyphs());

		memset(szCharacter, 0, 5);
		memset(szCharacter, 0x7F, 4);
		bExists = pFont->StartIteration(&sIter, (uint8*)szCharacter, &uiLength, 4);
		AssertUTFChar("\x5&", szCharacter, uiLength);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		bExists = pFont->Iterate(&sIter, (uint8*)szCharacter, &uiLength, 4);
		sIter.Kill();

		cFactory.Kill();
		cImportParams.Kill();

		szCharacterFile.Kill();
		szImageFile.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CreateFont(char* szFontName, int iCharWidth, int iCharHeight)
{
	CFontFactory				cFactory;
	CFontImportParameters		cImportParams;
	CFontCreationParameters		cCreateParams;
	CChars						szCharacterFile;
	CChars						szImageFile;
	Ptr<CFont>					pFont;
	char						szDirectory[] = { "Input" _FS_ "Font Pack" _FS_ };
	bool						bExists;

	cCreateParams.Init(iCharWidth, iCharHeight);

	szCharacterFile.Init(szDirectory);
	szCharacterFile.Append(szFontName);
	szCharacterFile.Append(".txt");
	szImageFile.Init(szDirectory);
	szImageFile.Append(szFontName);
	szImageFile.Append(".png");

	bExists = cImportParams.Init(szImageFile.Text(), szCharacterFile.Text(), iCharWidth, iCharHeight);
	if (!bExists)
	{
		szCharacterFile.Kill();
		szImageFile.Kill();
		cImportParams.Kill();
		return NULL;
	}
	
	cFactory.Init();
	pFont = cFactory.Generate(&cImportParams, &cCreateParams);
	szCharacterFile.Kill();
	szImageFile.Kill();
	cFactory.Kill();
	cImportParams.Kill();

	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontLookupText(void)
{
	ObjectsInit();
	{
		Ptr<CFont>			pFont;
		Ptr<CText>			pText;
		CSimpleTextReader	cTextReader;
		size				uiNumRuns;
		size				uiNumElements;
		CTextRun*			pcRun;
		CTextElement*		pcElement;
		CTextUTF16Short*	pcUTF16Short;
		CChars				sz;

		pFont = CreateFont("anuvverbubbla_8x8", 8, 8);
		cTextReader.Init(&pFont);
		pText = cTextReader.Read("Input" _FS_ "Text" _FS_ "Test1 anuvverbubbla.txt");
		cTextReader.Kill();

		uiNumRuns = pText->NumRuns();
		AssertSize(3, uiNumRuns);

		pcRun = pText->GetRun(0);
		uiNumElements = pcRun->NumElements();
		AssertSize(2, uiNumElements);
		pcElement = pcRun->GetElement(0);
		AssertString("CTextUTF16Short", pcElement->ClassName());
		pcUTF16Short = (CTextUTF16Short*)pcElement;
		sz.Init();
		pcUTF16Short->PrintAsASCII(&sz);
		AssertString("¿BACK IN THE DÄY?  ! ", sz.Text());
		sz.Kill();
		pcElement = pcRun->GetElement(1);
		AssertString("CTextNewLine", pcElement->ClassName());

		pcRun = pText->GetRun(1);
		uiNumElements = pcRun->NumElements();
		AssertSize(2, uiNumElements);
		pcElement = pcRun->GetElement(0);
		AssertString("CTextUTF16Short", pcElement->ClassName());
		pcUTF16Short = (CTextUTF16Short*)pcElement;
		sz.Init();
		pcUTF16Short->PrintAsASCII(&sz);
		AssertString(" Ñ = (5   3 + 7 / 2)", sz.Text());
		sz.Kill();
		pcElement = pcRun->GetElement(1);
		AssertString("CTextNewLine", pcElement->ClassName());

		pcRun = pText->GetRun(2);
		uiNumElements = pcRun->NumElements();
		AssertSize(2, uiNumElements);
		pcElement = pcRun->GetElement(0);
		AssertString("CTextUTF16Short", pcElement->ClassName());
		pcUTF16Short = (CTextUTF16Short*)pcElement;
		sz.Init();
		pcUTF16Short->PrintAsASCII(&sz);
		AssertString("  ©< >© UNICORNS  ", sz.Text());
		sz.Kill();
		pcElement = pcRun->GetElement(1);
		AssertString("CTextNewLine", pcElement->ClassName());

		pText->Kill();
		pFont = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGlyph(int iXExpected, int iYExpected, char* szExpected, SGlyphPosition* psGlyph)
{
	CChars						sz;

	AssertInt((int32)iXExpected, psGlyph->sPosition.x);
	AssertInt((int32)iYExpected, psGlyph->sPosition.y);
	sz.Init();
	psGlyph->pcGlyph->Print(&sz);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontGetGlyphsFromText(void)
{
	ObjectsInit();
	{
		Ptr<CFont>					pFont;
		Ptr<CText>					pText;
		CSimpleTextReader			cTextReader;
		CTextLayout					cLayout;
		Ptr<CGlyphListFontDraw>		pFontDraw;
		CArrayGlyphPosition* pacGlyphs;

		pFont = CreateFont("anuvverbubbla_8x8", 8, 8);
		cTextReader.Init(&pFont);
		pText = cTextReader.Read("Input" _FS_ "Text" _FS_ "Test1 anuvverbubbla.txt");
		cTextReader.Kill();

		pFontDraw = OMalloc<CGlyphListFontDraw>();

		cLayout.Init();
		cLayout.Layout(pFontDraw, pText);
		cLayout.Kill();

		pacGlyphs = pFontDraw->GetGlyphs();
		AssertSize(59, pacGlyphs->NumElements());
		AssertGlyph(0, 0,
			"    ··  \n"
			"   ·••· \n"
			"   ···  \n"
			" ···OO· \n"
			"·•@@@•· \n"
			"·OO·····\n"
			"··ooooo·\n"
			" ·······\n", pacGlyphs->Get(0));
		AssertGlyph(8, 0,
			"······· \n"
			"·•••••··\n"
			"·oo··o•·\n"
			"·OOOOo· \n"
			"·@@··@o·\n"
			"·OOOOOO·\n"
			"·ooooo··\n"
			"······· \n", pacGlyphs->Get(1));
		AssertGlyph(16, 0,
			" ······ \n"
			"··••••··\n"
			"·oo···o·\n"
			"·OOOOOO·\n"
			"·@@···@·\n"
			"·OO· ·O·\n"
			"·oo· ·o·\n"
			"···· ···\n", pacGlyphs->Get(2));
		AssertGlyph(24, 0,
			" ·······\n"
			"··•••••·\n"
			"·oo·····\n"
			"·OO·    \n"
			"·@@·····\n"
			"·OOOOOO·\n"
			"··ooooo·\n"
			" ·······\n", pacGlyphs->Get(3));

		AssertGlyph(144, 0,
			"", pacGlyphs->Get(18));
		AssertGlyph(152, 0,
			" ··· \n"
			"·•••·\n"
			"·oo··\n"
			"·O·· \n"
			" ··  \n"
			"·OO· \n"
			"·oo· \n"
			" ··  \n", pacGlyphs->Get(19));
		AssertGlyph(160, 0,
			" ·· ·· \n"
			"··•·•··\n"
			"·OO•OO·\n"
			" ·@@@· \n"
			"  ·O·  \n"
			"   ·   \n", pacGlyphs->Get(20));

		AssertGlyph(24, 8,
			"······\n"
			"·oooo·\n"
			"······\n"
			"·@@@@·\n"
			"······\n", pacGlyphs->Get(24));
		AssertGlyph(64, 8,
			"   ··   \n"
			"  ····  \n"
			" ··oo·· \n"
			"··OOOO··\n"
			" ·•@@•· \n"
			"··OOOO··\n"
			"·o····o·\n"
			"···  ···\n", pacGlyphs->Get(29));

		pFontDraw->Kill();
		pText->Kill();
		pFont = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontMapFontWriteToImage(void)
{
	char		szDirectory[] = "Output" _FS_ "MapFontDraw";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CFont>			pFont;
		Ptr<CRoot>			pRoot;
		Ptr<CText>			pText;
		CSimpleTextReader	cTextReader;
		Ptr<CTextLayout>	pLayout;
		Ptr<CMapFontDraw>	pFontDraw;
		Ptr<CSpriteMap>		pSpriteMap;
		Ptr<CImage>			pImage;
		CChars				szOutputFilename;
		CChars				szInputFilename;
		bool				bWritten;

		pRoot = ORoot();
		pFont = CreateFont("anuvverbubbla_8x8", 8, 8);
		pRoot->Add(pFont);
		cTextReader.Init(&pFont);
		pText = cTextReader.Read("Input" _FS_ "Text" _FS_ "Test1 anuvverbubbla.txt");
		pRoot->Add(pText);
		cTextReader.Kill();

		pSpriteMap = OMalloc<CSpriteMap>();
		pFontDraw = OMalloc<CMapFontDraw>(pSpriteMap);
		pRoot->Add(pSpriteMap);
		pRoot->Add(pFontDraw);

		pSpriteMap->BeginChange();
		pLayout = OMalloc<CTextLayout>();
		pLayout->Layout(pFontDraw, pText);
		pLayout = NULL;
		pSpriteMap->EndChange();

		pImage = pSpriteMap->WriteToImage();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "anuvverbubbla.txt.png");
		bWritten = WriteImage(pImage, szOutputFilename.Text());
		AssertTrue(bWritten);

		pFontDraw->Kill();
		pText->Kill();
		pFont = NULL;

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontMapFontViewportLayout(void)
{
	char		szDirectory[] = "Output" _FS_ "MapFontDraw";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CFont>					pFont;
		Ptr<CText>					pText;
		CSimpleTextReader			cTextReader;
		Ptr<CTextLayout>			pLayout;
		Ptr<CMapFontDraw>			pFontDraw;
		Ptr<CSpriteMap>				pSpriteMap;
		CChars						szOutputFilename;
		CChars						szInputFilename;
		bool						bWritten;
		Ptr<CMaps>					pMaps;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		bool						bResult;
		CTimer						cTimer;
		int64						iMilliseconds;

		pRoot = ORoot();
		pFont = CreateFont("nuskool_krome_64x64", 64, 64);
		pRoot->Add(pFont);
		cTextReader.Init(&pFont);
		pText = cTextReader.Read("Input" _FS_ "Text" _FS_ "Test1 nuskool_krome_64x64.txt");
		cTextReader.Kill();

		pDestImage = OMalloc<CImage>(1280, 720, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->Black();

		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		pRoot->Add(pBlitterCache);

		pMaps = ONMalloc<CMaps>("Maps", pBlitterCache, pDestImage);
		pRoot->Add(pMaps);

		pSpriteMap = OMalloc<CSpriteMap>();
		pFontDraw = OMalloc<CMapFontDraw>(pSpriteMap);
		pRoot->Add(pFontDraw);

		gcObjects.DisableValidation();

		pSpriteMap->BeginChange();
		pLayout = OMalloc<CTextLayout>();
		pRoot->Add(pLayout);
		pLayout->Layout(pFontDraw, pText);
		pSpriteMap->EndChange();
		pMaps->AddMap(pSpriteMap);
		pLayout = NULL;

		bResult = pMaps->CreateCelBlitters();
		AssertTrue(bResult);

		cTimer.Init();
		pMaps->SetViewportPosition(0, 0);
		bResult = pMaps->Blit(false);
		AssertTrue(bResult);
		cTimer.Update();
		iMilliseconds = cTimer.GetTotalTimeInMillieconds();
		AssertTrue(iMilliseconds < 10);

		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "nuskool_krome_64x64.txt.png");
		bWritten = WriteImage(pDestImage, szOutputFilename.Text());
		AssertTrue(bWritten);

		pFontDraw->Kill();
		pText = NULL;
		pFont = NULL;
		pMaps = NULL;

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");

		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CreateTrueTypeFont(Ptr<CRoot> pRoot, char* szFontFile, float32 fPixelSize, char* szCharacterFile)
{
	CTrueTypeFont			cTrueTypeFont;
	CTrueTypeFontFactory	cFactory;
	CTextFile				cTextFile;
	CImageColourRGB			cWhite;
	CChars					szFilename;
	Ptr<CFontDescription>	pDescription;
	Ptr<CImage>				pImage;
	Ptr<CFont>				pFont;

	szFilename.Init("Input" _FS_ "Fonts" _FS_);
	szFilename.Append(szFontFile);
	cTrueTypeFont.Init();
	if (!cTrueTypeFont.Load(szFilename.Text()))
	{
		szFilename.Kill();
		cTrueTypeFont.Kill();
		return NULL;
	}
	szFilename.Kill();

	cTextFile.Init();
	if (!cTextFile.Read(szCharacterFile))
	{
		cTextFile.Kill();
		cTrueTypeFont.Kill();
		return NULL;
	}

	cWhite.Init(1.0f, 1.0f, 1.0f);
	cFactory.Init(&cTrueTypeFont, fPixelSize);
	pDescription = cFactory.Describe(szFontFile, cTextFile.Text());
	pImage = cFactory.Rasterise(pDescription, &cWhite);
	if (pImage.IsNotNull())
	{
		pRoot->Add(pImage);
		pFont = OMalloc<CFont>(pDescription, pImage);
		pRoot->Add(pFont);
	}
	cFactory.Kill();
	cTextFile.Kill();
	cTrueTypeFont.Kill();

	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontTrueTypeCreateFont(void)
{
	char		szDirectory[] = "Output" _FS_ "TrueTypeFont";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CRoot>		pRoot;
		Ptr<CFont>		pFont;
		Ptr<CImage>		pImage;
		Ptr<CGlyph>		pGlyph;
		CSubImage*		pcSubImage;
		CChars			szOutputFilename;
		CChars			szInputFilename;

		gcObjects.DisableValidation();

		pRoot = ORoot();
		//Validating every object each time a glyph is added is too slow for hundreds of glyphs; validate once afterwards.
		gcObjects.DisableValidation();
		pFont = CreateTrueTypeFont(pRoot, "LiberationSans-Regular.ttf", 24.0f, "Input" _FS_ "Text" _FS_ "Test1 LiberationSans-Regular.txt");
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();
		AssertTrue(pFont.IsNotNull());
		pImage = pFont->GetImage();
		AssertTrue(pImage.IsNotNull());

		AssertSize(658, pFont->NumGlyphs());
		AssertInt(22, pFont->GetAscent());
		AssertInt(6, pFont->GetDescent());
		AssertInt(28, pFont->GetLineHeight());
		AssertInt(7, pFont->GetSpaceWidth());
		AssertInt((int32)512, pImage->GetWidth());
		AssertInt((int32)452, pImage->GetHeight());

		//Liberation Sans 'A' advances 1366 units (16 pixels) and is 1409 units high (17 rows), starting 5 rows below the top of the line.
		pGlyph = pFont->GetGlyph((uint16)'A', 1);
		AssertTrue(pGlyph.IsNotNull());
		AssertInt(16, pGlyph->GetFullWidth());
		AssertInt(28, pGlyph->GetFullHeight());
		pcSubImage = pGlyph->GetSubImage();
		AssertInt((int32)0, pcSubImage->GetImageLeft());
		AssertInt((int32)5, pcSubImage->GetImageTop());
		AssertInt((int32)16, pcSubImage->GetImageWidth());
		AssertInt((int32)17, pcSubImage->GetImageHeight());

		//Space has no image, only an advance.
		pGlyph = pFont->GetGlyph((uint16)' ', 1);
		AssertTrue(pGlyph.IsNotNull());
		AssertInt(7, pGlyph->GetFullWidth());
		AssertInt(28, pGlyph->GetFullHeight());
		AssertInt((int32)0, pGlyph->GetSubImage()->GetImageWidth());

		//Code points below 0xFF are keyed with one byte, others with more.
		pGlyph = pFont->GetGlyph((uint16)0x00C1, 1);
		AssertTrue(pGlyph.IsNotNull());
		AssertInt(16, pGlyph->GetFullWidth());
		pGlyph = pFont->GetGlyph((uint32)0x0416, 2);
		AssertTrue(pGlyph.IsNotNull());
		pGlyph = pFont->GetGlyph((uint32)0x4E00, 2);
		AssertTrue(pGlyph.IsNull());

		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "LiberationSans-Regular.png");
		AssertTrue(WriteImage(pImage, szOutputFilename.Text()));

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontTrueTypeWriteText(void)
{
	char		szDirectory[] = "Output" _FS_ "TrueTypeFont";
	char		szTextFile[] = "Input" _FS_ "Text" _FS_ "Test1 LiberationSans-Regular.txt";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CRoot>					pRoot;
		Ptr<CFont>					pFont;
		Ptr<CText>					pText;
		CSimpleTextReader			cTextReader;
		Ptr<CTextLayout>			pLayout;
		Ptr<CMapFontDraw>			pFontDraw;
		Ptr<CSpriteMap>				pSpriteMap;
		Ptr<CMaps>					pMaps;
		Ptr<CImage>					pDestImage;
		Ptr<CImageCelBlitterCache>	pBlitterCache;
		CChars						szOutputFilename;
		CChars						szInputFilename;

		gcObjects.DisableValidation();

		pRoot = ORoot();
		//Validating every object each time a glyph is added is too slow for hundreds of glyphs; validate once afterwards.
		gcObjects.DisableValidation();
		pFont = CreateTrueTypeFont(pRoot, "LiberationSans-Regular.ttf", 24.0f, szTextFile);
		AssertTrue(pFont.IsNotNull());

		cTextReader.Init(&pFont);
		pText = cTextReader.Read(szTextFile);
		cTextReader.Kill();
		AssertTrue(pText.IsNotNull());
		pRoot->Add(pText);
		AssertSize(17, pText->NumRuns());

		pDestImage = OMalloc<CImage>(1024, 480, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pDestImage->Black();
		pRoot->Add(pDestImage);

		pBlitterCache = OMalloc<CImageCelBlitterCache>(pDestImage);
		pRoot->Add(pBlitterCache);

		pMaps = ONMalloc<CMaps>("TrueType Maps", pBlitterCache, pDestImage);
		pRoot->Add(pMaps);

		pSpriteMap = OMalloc<CSpriteMap>();
		pRoot->Add(pSpriteMap);
		pFontDraw = OMalloc<CMapFontDraw>(pSpriteMap);
		pRoot->Add(pFontDraw);
		pLayout = OMalloc<CTextLayout>();
		pRoot->Add(pLayout);

		pSpriteMap->BeginChange();
		pLayout->Layout(pFontDraw, pText);
		pSpriteMap->EndChange();
		pMaps->AddMap(pSpriteMap);

		AssertTrue(pMaps->CreateCelBlitters());
		pMaps->SetViewportPosition(0, 0);
		AssertTrue(pMaps->Blit(false));
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "Test1 LiberationSans-Regular.txt.png");
		AssertTrue(WriteImage(pDestImage, szOutputFilename.Text()));

		szInputFilename.Init(szOutputFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename, szOutputFilename);

		szOutputFilename.Kill();
		szInputFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFontGlyphsEqual(Ptr<CFont> pExpected, Ptr<CFont> pActual)
{
	SIndexTreeMemoryIterator	sIter;
	uint8						auiKey[5];
	size						uiLength;
	bool						bExists;
	Ptr<CGlyph>					pExpectedGlyph;
	Ptr<CGlyph>					pActualGlyph;
	CSubImage*					pcExpected;
	CSubImage*					pcActual;
	size						uiCount;

	AssertSize(pExpected->NumGlyphs(), pActual->NumGlyphs());

	uiCount = 0;
	memset(auiKey, 0, 5);
	bExists = pExpected->StartIteration(&sIter, auiKey, &uiLength, 4);
	while (bExists)
	{
		pExpectedGlyph = pExpected->GetGlyph(auiKey, uiLength);
		pActualGlyph = pActual->GetGlyph(auiKey, uiLength);
		AssertTrue(pActualGlyph.IsNotNull());
		AssertInt(pExpectedGlyph->GetStep(), pActualGlyph->GetStep());
		AssertTrue(pActual->GetImage() == pActualGlyph->GetSourceImage());

		pcExpected = pExpectedGlyph->GetSubImage();
		pcActual = pActualGlyph->GetSubImage();
		AssertInt((int32)pcExpected->mcImageRect.msTopLeft.x, pcActual->mcImageRect.msTopLeft.x);
		AssertInt((int32)pcExpected->mcImageRect.msTopLeft.y, pcActual->mcImageRect.msTopLeft.y);
		AssertInt((int32)pcExpected->mcImageRect.msBottomRight.x, pcActual->mcImageRect.msBottomRight.x);
		AssertInt((int32)pcExpected->mcImageRect.msBottomRight.y, pcActual->mcImageRect.msBottomRight.y);
		AssertInt((int32)pcExpected->msOffsetTopLeft.x, pcActual->msOffsetTopLeft.x);
		AssertInt((int32)pcExpected->msOffsetTopLeft.y, pcActual->msOffsetTopLeft.y);
		AssertInt((int32)pcExpected->msOffsetBottomRight.x, pcActual->msOffsetBottomRight.x);
		AssertInt((int32)pcExpected->msOffsetBottomRight.y, pcActual->msOffsetBottomRight.y);
		AssertInt((int32)pcExpected->msAlignment.x, pcActual->msAlignment.x);
		AssertInt((int32)pcExpected->msAlignment.y, pcActual->msAlignment.y);

		uiCount++;
		bExists = pExpected->Iterate(&sIter, auiKey, &uiLength, 4);
	}
	AssertSize(pExpected->NumGlyphs(), uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Inter's variable font gives its default instance, Regular.
//
//////////////////////////////////////////////////////////////////////////
void TestFontTrueTypeWriteAndRead(void)
{
	char		szDirectory[] = "Output" _FS_ "TrueTypeFont";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		Ptr<CRoot>		pRoot;
		Ptr<CFont>		pFont;
		Ptr<CFont>		pLoadedFont;
		Ptr<CImage>		pImage;
		Ptr<CImage>		pLoadedImage;
		CChars			szImageFilename;
		CChars			szCelsFilename;
		CChars			szFontFilename;
		CChars			szLoadedImageFilename;
		CChars			szInputFilename;

		pRoot = ORoot();
		//Validating every object each time a glyph is added is too slow for hundreds of glyphs; validate once afterwards.
		gcObjects.DisableValidation();
		//14 points on a 96 DPI screen.
		pFont = CreateTrueTypeFont(pRoot, "InterVariable.ttf", 14.0f * 96.0f / 72.0f, "Input" _FS_ "Text" _FS_ "Inter-Regular characters.txt");
		AssertTrue(pFont.IsNotNull());
		AssertSize(586, pFont->NumGlyphs());
		pImage = pFont->GetImage();
		AssertTrue(pImage.IsNotNull());

		szImageFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szImageFilename, "Inter-Regular-14pt.png");
		szCelsFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szCelsFilename, "Inter-Regular-14pt.cels");
		szFontFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szFontFilename, "Inter-Regular-14pt.font");

		AssertTrue(WriteImage(pImage, szImageFilename.Text()));
		AssertTrue(FontWriter(pFont, szImageFilename.Text(), szCelsFilename.Text(), szFontFilename.Text()));

		pLoadedFont = FontReader(szFontFilename.Text());
		AssertTrue(pLoadedFont.IsNotNull());
		pRoot->Add(pLoadedFont);

		AssertString("InterVariable.ttf", pLoadedFont->GetFontName());
		AssertInt(pFont->GetSpaceWidth(), pLoadedFont->GetSpaceWidth());
		AssertInt(pFont->GetAscent(), pLoadedFont->GetAscent());
		AssertInt(pFont->GetDescent(), pLoadedFont->GetDescent());
		//Comparing makes every glyph of both fonts so is also done without validating each one.
		AssertFontGlyphsEqual(pFont, pLoadedFont);
		gcObjects.EnableValidation();
		gcObjects.ValidateObjectsConsistency();

		pLoadedImage = pLoadedFont->GetImage();
		AssertTrue(pLoadedImage.IsNotNull());
		AssertTrue(pLoadedImage != pImage);
		AssertInt((int32)pImage->GetWidth(), pLoadedImage->GetWidth());
		AssertInt((int32)pImage->GetHeight(), pLoadedImage->GetHeight());

		szInputFilename.Init(szImageFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename, szImageFilename);
		szInputFilename.Kill();

		szInputFilename.Init(szCelsFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename, szCelsFilename);
		szInputFilename.Kill();

		szInputFilename.Init(szFontFilename);
		szInputFilename.Replace("Output", "Input");
		AssertFile(szInputFilename, szFontFilename);
		szInputFilename.Kill();

		//The loaded image must write the same file as the one it was read from.
		szLoadedImageFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szLoadedImageFilename, "Inter-Regular-14pt Loaded.png");
		AssertTrue(WriteImage(pLoadedImage, szLoadedImageFilename.Text()));
		AssertFile(szImageFilename.Text(), szLoadedImageFilename.Text());
		szLoadedImageFilename.Kill();

		szFontFilename.Kill();
		szCelsFilename.Kill();
		szImageFilename.Kill();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFont(void)
{
	BeginTests();

	DataIOInit();

	TestFontCreateFont();
	TestFontLookupText();
	TestFontGetGlyphsFromText();
	TestFontMapFontWriteToImage();
	TestFontMapFontViewportLayout();
	TestFontTrueTypeCreateFont();
	TestFontTrueTypeWriteText();
	TestFontTrueTypeWriteAndRead();

	DataIOKill();

	TestStatistics();
}

