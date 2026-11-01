#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontFamily.h"
#include "SupportLib/TrueTypeFontFactory.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "TestLib/Assert.h"


#define FAMILY_FONTS	"Input" _FS_ "Fonts" _FS_


//////////////////////////////////////////////////////////////////////////
//
// Liberation Sans in its four faces.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontFamily> CreateLiberationFamily(float32 fDefaultPixelSize, EColourOrder eColourOrder)
{
	Ptr<CFontFamily>	pFamily;

	pFamily = OMalloc<CFontFamily>("Liberation Sans", "AB ab", fDefaultPixelSize, eColourOrder);
	AssertTrue(pFamily->SetFace(FS_Regular, FAMILY_FONTS "LiberationSans-Regular.ttf"));
	AssertTrue(pFamily->SetFace(FS_Bold, FAMILY_FONTS "LiberationSans-Bold.ttf"));
	AssertTrue(pFamily->SetFace(FS_Italic, FAMILY_FONTS "LiberationSans-Italic.ttf"));
	AssertTrue(pFamily->SetFace(FS_BoldItalic, FAMILY_FONTS "LiberationSans-BoldItalic.ttf"));
	return pFamily;
}


//////////////////////////////////////////////////////////////////////////
//
// Each face and size is made once, the first time it is asked for.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyFacesAndSizes(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pFamily;
		Ptr<CFontColours>	pRegular;
		Ptr<CFontColours>	pBold;
		Ptr<CFontColours>	pItalic;
		Ptr<CFontColours>	pBoldItalic;
		Ptr<CFontColours>	pBig;
		STextStyle			sStyle;
		ARGB32				sRed;

		pFamily = CreateLiberationFamily(16.0f, CCO_Unknown);
		AssertString("Liberation Sans", pFamily->GetFamilyName());
		AssertTrue(pFamily->GetDefaultPixelSize() == 16.0f);
		AssertSize(0, pFamily->NumSizes());

		//A size of 0 is the default size.  Sizes are told apart to 1/64 of a pixel.
		pRegular = pFamily->GetColours(FS_Regular, 0.0f);
		AssertTrue(pRegular.IsNotNull());
		AssertString("Liberation Sans Regular 16px", pRegular->GetWhite()->GetFontName());
		AssertTrue(pFamily->GetColours(FS_Regular, 16.0f) == pRegular);
		AssertTrue(pFamily->GetColours(FS_Regular, 16.004f) == pRegular);
		AssertSize(1, pFamily->NumSizes());

		pBold = pFamily->GetColours(FS_Bold, 16.0f);
		pItalic = pFamily->GetColours(FS_Italic, 16.0f);
		pBoldItalic = pFamily->GetColours(FS_BoldItalic, 16.0f);
		AssertString("Liberation Sans Bold 16px", pBold->GetWhite()->GetFontName());
		AssertString("Liberation Sans Italic 16px", pItalic->GetWhite()->GetFontName());
		AssertString("Liberation Sans Bold Italic 16px", pBoldItalic->GetWhite()->GetFontName());
		AssertSize(4, pFamily->NumSizes());

		//Bold A is wider than regular A.
		AssertInt(11, pRegular->GetWhite()->GetGlyph((uint32)'A', 1)->GetStep());
		AssertInt(12, pBold->GetWhite()->GetGlyph((uint32)'A', 1)->GetStep());

		pBig = pFamily->GetColours(FS_Regular, 32.0f);
		AssertString("Liberation Sans Regular 32px", pBig->GetWhite()->GetFontName());
		AssertTrue(pBig->GetWhite()->GetLineHeight() > pRegular->GetWhite()->GetLineHeight());
		AssertSize(5, pFamily->NumSizes());

		//A style's font is its face and size in its colour.
		sRed = Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00);
		sStyle.Init(true, false, 16.0f, sRed);
		AssertTrue(pFamily->GetFont(&sStyle) == pBold->GetFont(sRed));
		sStyle.Init(false, false, 0.0f, FONT_COLOURS_WHITE);
		AssertTrue(pFamily->GetFont(&sStyle) == pRegular->GetWhite());
		AssertSize(5, pFamily->NumSizes());

		pBig = NULL;
		pBoldItalic = NULL;
		pItalic = NULL;
		pBold = NULL;
		pRegular = NULL;
		pFamily = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A style with no face is an error, as is a face that can't be loaded.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyMissingFace(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pFamily;
		Ptr<CFontColours>	pColours;
		SLogConfig			sLogConfig;
		STextStyle			sStyle;

		pFamily = OMalloc<CFontFamily>("Liberation Sans", "AB", 16.0f, CCO_Unknown);
		AssertTrue(pFamily->SetFace(FS_Regular, FAMILY_FONTS "LiberationSans-Regular.ttf"));
		AssertTrue(pFamily->HasFace(FS_Regular));
		AssertFalse(pFamily->HasFace(FS_Bold));

		sLogConfig = gcLogger.SetSilent();
		pColours = pFamily->GetColours(FS_Bold, 16.0f);
		AssertTrue(pColours.IsNull());
		sStyle.Init(false, true, 16.0f, FONT_COLOURS_WHITE);
		AssertTrue(pFamily->GetFont(&sStyle).IsNull());
		AssertFalse(pFamily->SetFace(FS_Bold, FAMILY_FONTS "Missing.ttf"));
		gcLogger.SetConfig(&sLogConfig);
		AssertFalse(pFamily->HasFace(FS_Bold));
		AssertSize(0, pFamily->NumSizes());

		pColours = NULL;
		pFamily = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A family of one font already made has only that style and size.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyOneFont(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pLiberation;
		Ptr<CFontColours>	pColours;
		Ptr<CFontFamily>	pFamily;
		SLogConfig			sLogConfig;

		pLiberation = CreateLiberationFamily(16.0f, CCO_Unknown);
		pColours = pLiberation->GetColours(FS_Regular, 0.0f);

		pFamily = OMalloc<CFontFamily>(pColours);
		AssertString("Liberation Sans Regular 16px", pFamily->GetFamilyName());
		AssertTrue(pFamily->GetColours(FS_Regular, 0.0f) == pColours);
		AssertSize(1, pFamily->NumSizes());

		sLogConfig = gcLogger.SetSilent();
		AssertTrue(pFamily->GetColours(FS_Bold, 0.0f).IsNull());
		AssertTrue(pFamily->GetColours(FS_Regular, 16.0f).IsNull());
		gcLogger.SetConfig(&sLogConfig);
		AssertSize(1, pFamily->NumSizes());

		pFamily = NULL;
		pColours = NULL;
		pLiberation = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Sizes are made in the family's colour order.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyColourOrder(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pFamily;
		Ptr<CFontColours>	pColours;

		pFamily = CreateLiberationFamily(16.0f, CCO_BGR);
		pColours = pFamily->GetColours(FS_Italic, 0.0f);
		AssertInt(CCO_BGR, pColours->GetWhite()->GetImage()->GetColourOrder());
		AssertInt(CCO_BGR, pColours->GetFont(Set32BitColour((uint8)0x10, (uint8)0x20, (uint8)0x30))->GetImage()->GetColourOrder());

		pColours = NULL;
		pFamily = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Superscript and subscript are the face at the size times the font's
// scale, their baselines moved by the font's shift.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilySuperscriptAndSubscript(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pFamily;
		Ptr<CFontFamily>	pOneFont;
		Ptr<CFont>			pFont;
		STextStyle			sStyle;
		int16				iShift;
		ARGB32				sRed;
		SLogConfig			sLogConfig;

		pFamily = CreateLiberationFamily(16.0f, CCO_Unknown);
		sRed = Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00);

		//Not moved.
		sStyle.Init(false, false, 0.0f, FONT_COLOURS_WHITE);
		pFont = pFamily->GetRunFont(&sStyle, &iShift);
		AssertString("Liberation Sans Regular 16px", pFont->GetFontName());
		AssertInt(0, iShift);

		//16 * 1331 / 2048 = 10.4 pixels, raised 977 units at 16 pixels.
		sStyle.ePosition = TP_Superscript;
		pFont = pFamily->GetRunFont(&sStyle, &iShift);
		AssertString("Liberation Sans Regular 10px", pFont->GetFontName());
		AssertInt(8, iShift);
		AssertTrue(pFont == pFamily->GetColours(FS_Regular, 16.0f * 1331.0f / 2048.0f)->GetWhite());

		//Lowered 283 units at 16 pixels.
		sStyle.ePosition = TP_Subscript;
		pFont = pFamily->GetRunFont(&sStyle, &iShift);
		AssertString("Liberation Sans Regular 10px", pFont->GetFontName());
		AssertInt(-2, iShift);

		//In the style's face, size and colour.
		sStyle.Init(true, false, 32.0f, sRed);
		sStyle.ePosition = TP_Superscript;
		pFont = pFamily->GetRunFont(&sStyle, &iShift);
		AssertString("Liberation Sans Bold 21px", pFont->GetFontName());
		AssertInt(15, iShift);
		AssertTrue(pFont == pFamily->GetColours(FS_Bold, 32.0f * 1331.0f / 2048.0f)->GetFont(sRed));
		AssertTrue(pFamily->GetFont(&sStyle) == pFont);

		//A family of a font already made can't make the smaller size.
		pOneFont = OMalloc<CFontFamily>(pFamily->GetColours(FS_Regular, 0.0f));
		sStyle.Init(false, false, 0.0f, FONT_COLOURS_WHITE);
		sStyle.ePosition = TP_Subscript;
		sLogConfig = gcLogger.SetSilent();
		AssertTrue(pOneFont->GetRunFont(&sStyle, &iShift).IsNull());
		gcLogger.SetConfig(&sLogConfig);
		AssertInt(0, iShift);

		pOneFont = NULL;
		pFont = NULL;
		pFamily = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The number of pixels with any opacity in the rectangle (right and bottom
// exclusive).
//
//////////////////////////////////////////////////////////////////////////
size CountOpaquePixels(Ptr<CImage> pImage, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	uint8					uiOpacity;
	int32					x;
	int32					y;
	size					uiCount;

	cCreator.Init(&pImage);
	cCreator.AddAccess(IMAGE_OPACITY, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	uiCount = 0;
	for (y = iTop; y < iBottom; y++)
	{
		for (x = iLeft; x < iRight; x++)
		{
			pcAccessor->Get(x, y, &uiOpacity);
			if (uiOpacity != 0)
			{
				uiCount++;
			}
		}
	}
	pcAccessor->Kill();
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountGlyphOpaquePixels(Ptr<CFont> pFont, SGlyphDescription* psDescription)
{
	CInt32Rect*	pcRect;

	pcRect = &psDescription->cSubImage.mcImageRect;
	return CountOpaquePixels(pFont->GetImage(), pcRect->msTopLeft.x, pcRect->msTopLeft.y, pcRect->msBottomRight.x, pcRect->msBottomRight.y);
}


//////////////////////////////////////////////////////////////////////////
//
// A family's sizes start with empty images.  A glyph's pixels are drawn
// when it is first asked for, a coloured font's from its white font's.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyLazyGlyphs(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>		pFamily;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pWhite;
		Ptr<CFont>				pRed;
		Ptr<CFontDescription>	pDescription;
		SGlyphDescription*		psA;
		SGlyphDescription*		psB;
		SGlyphDescription*		psLowerA;
		SGlyphDescription*		psLowerB;
		Ptr<CImage>				pImage;
		ARGB32					sRed;

		pFamily = CreateLiberationFamily(16.0f, CCO_Unknown);
		ORoot()->Add(pFamily);
		sRed = Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00);

		pColours = pFamily->GetColours(FS_Regular, 0.0f);
		pWhite = pColours->GetWhite();
		pDescription = pWhite->GetDescription();
		AssertTrue(pWhite->IsLazy());
		AssertTrue(pDescription->GetRasteriser().IsNotNull());
		psA = pDescription->GetGlyph((uint8*)"A", 1);
		psB = pDescription->GetGlyph((uint8*)"B", 1);
		psLowerA = pDescription->GetGlyph((uint8*)"a", 1);
		psLowerB = pDescription->GetGlyph((uint8*)"b", 1);
		AssertTrue(psA->iSourceGlyph > 0);

		//Nothing is drawn until it is asked for.
		pImage = pWhite->GetImage();
		AssertSize(0, CountOpaquePixels(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight()));
		AssertFalse(pWhite->HasPixels(psA));
		AssertFalse(pWhite->HasPixels(psB));

		pWhite->GetGlyph((uint32)'A', 1);
		AssertTrue(pWhite->HasPixels(psA));
		AssertFalse(pWhite->HasPixels(psB));
		AssertTrue(CountGlyphOpaquePixels(pWhite, psA) > 0);
		AssertSize(0, CountGlyphOpaquePixels(pWhite, psB));
		AssertSize(CountGlyphOpaquePixels(pWhite, psA), CountOpaquePixels(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight()));

		//A colour of a lazy font is lazy too.  Its b draws the white font's b first.
		pRed = pColours->GetFont(sRed);
		AssertTrue(pRed->IsLazy());
		AssertSize(0, CountOpaquePixels(pRed->GetImage(), 0, 0, pImage->GetWidth(), pImage->GetHeight()));
		pRed->GetGlyph((uint32)'b', 1);
		AssertTrue(pRed->HasPixels(psLowerB));
		AssertTrue(pWhite->HasPixels(psLowerB));
		AssertFalse(pRed->HasPixels(psA));
		AssertSize(CountGlyphOpaquePixels(pWhite, psLowerB), CountGlyphOpaquePixels(pRed, psLowerB));
		AssertSize(0, CountGlyphOpaquePixels(pRed, psA));

		//A cel made for writing the font draws the glyph too.
		AssertFalse(pWhite->HasPixels(psLowerA));
		pWhite->CreateCel(psLowerA);
		AssertTrue(pWhite->HasPixels(psLowerA));

		pImage = NULL;
		pDescription = NULL;
		pRed = NULL;
		pWhite = NULL;
		pColours = NULL;
		pFamily = NULL;
		ORoot()->RemoveAll();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// With every glyph asked for, a lazy font's image is exactly the image
// CTrueTypeFontFactory::Rasterise makes, and a lazy colour has the opacity
// and, where there is any, the colour that recolouring the whole image
// gives.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyLazyMatchesRasterise(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>			pFamily;
		Ptr<CFontColours>			pLazyColours;
		Ptr<CFontColours>			pEagerColours;
		Ptr<CFont>					pLazy;
		Ptr<CFont>					pEager;
		Ptr<CFont>					pLazyRed;
		Ptr<CFont>					pEagerRed;
		CTrueTypeFont				cFont;
		CTrueTypeFontFactory		cFactory;
		CImageColourRGB				cWhite;
		Ptr<CFontDescription>		pDescription;
		SIndexTreeMemoryIterator	sIter;
		uint8						auiKey[64];
		size						uiKeyLength;
		bool						bExists;
		lsize						uiBytes;
		uint8*						puiLazy;
		uint8*						puiEager;
		lsize						i;
		size						uiDifferences;
		ARGB32						sRed;

		sRed = Set32BitColour((uint8)0xC0, (uint8)0x40, (uint8)0x20, (uint8)0x90);
		pFamily = CreateLiberationFamily(20.0f, CCO_Unknown);
		ORoot()->Add(pFamily);
		pLazyColours = pFamily->GetColours(FS_Italic, 0.0f);
		pLazy = pLazyColours->GetWhite();
		pLazyRed = pLazyColours->GetFont(sRed);

		cFont.Init();
		AssertTrue(cFont.Load((char*)FAMILY_FONTS "LiberationSans-Italic.ttf"));
		cFactory.Init(&cFont, 20.0f);
		pDescription = cFactory.Describe((char*)"Eager", (char*)"AB ab");
		cWhite.Init(1.0f, 1.0f, 1.0f);
		pEager = OMalloc<CFont>(pDescription, cFactory.Rasterise(pDescription, &cWhite));
		cFactory.Kill();
		cFont.Kill();
		pEagerColours = OMalloc<CFontColours>(pEager);
		ORoot()->Add(pEagerColours);
		pEagerRed = pEagerColours->GetFont(sRed);
		AssertFalse(pEager->IsLazy());
		AssertFalse(pEagerRed->IsLazy());

		bExists = pLazy->StartIteration(&sIter, auiKey, &uiKeyLength, 64);
		while (bExists)
		{
			pLazyRed->GetGlyph(auiKey, uiKeyLength);
			bExists = pLazy->Iterate(&sIter, auiKey, &uiKeyLength, 64);
		}

		AssertInt((int32)pEager->GetImage()->GetWidth(), pLazy->GetImage()->GetWidth());
		AssertInt((int32)pEager->GetImage()->GetHeight(), pLazy->GetImage()->GetHeight());
		uiBytes = (lsize)pLazy->GetImage()->GetWidth() * pLazy->GetImage()->GetHeight() * 4;
		AssertMemory(pEager->GetImage()->GetData(), pLazy->GetImage()->GetData(), uiBytes);

		//Opacity, red, green, blue.  Eager recolouring also colours pixels with no opacity.
		puiLazy = (uint8*)pLazyRed->GetImage()->GetData();
		puiEager = (uint8*)pEagerRed->GetImage()->GetData();
		uiDifferences = 0;
		for (i = 0; i < uiBytes; i += 4)
		{
			if ((puiLazy[i] != puiEager[i]) || ((puiLazy[i] != 0) && (memcmp(&puiLazy[i + 1], &puiEager[i + 1], 3) != 0)))
			{
				uiDifferences++;
			}
		}
		AssertSize(0, uiDifferences);
		AssertTrue(CountOpaquePixels(pLazyRed->GetImage(), 0, 0, pLazyRed->GetImage()->GetWidth(), pLazyRed->GetImage()->GetHeight()) > 0);

		pEagerRed = NULL;
		pEagerColours = NULL;
		pEager = NULL;
		pDescription = NULL;
		pLazyRed = NULL;
		pLazy = NULL;
		pLazyColours = NULL;
		pFamily = NULL;
		ORoot()->RemoveAll();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The opacity of a pixel of a font's image, which is opacity first.
//
//////////////////////////////////////////////////////////////////////////
uint8 GetFontImageOpacity(Ptr<CFont> pFont, int32 x, int32 y)
{
	return ((uint8*)pFont->GetImage()->GetData())[((lsize)y * pFont->GetImage()->GetWidth() + x) * 4];
}


//////////////////////////////////////////////////////////////////////////
//
// A clockwise font is its own size of the family.  Each glyph is the
// upright glyph turned a quarter clockwise about the pen: same step, the
// image turned and the offsets moved round with it.
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamilyClockwise(void)
{
	ObjectsInit();
	{
		Ptr<CFontFamily>	pFamily;
		Ptr<CFontColours>	pUprightColours;
		Ptr<CFontColours>	pClockwiseColours;
		Ptr<CFont>			pUpright;
		Ptr<CFont>			pClockwise;
		Ptr<CGlyph>			pUprightGlyph;
		Ptr<CGlyph>			pClockwiseGlyph;
		CSubImage*			pcUp;
		CSubImage*			pcTurned;
		const char*			szCharacters;
		size				i;
		int32				x;
		int32				y;
		size				uiDifferences;
		size				uiOpaque;

		pFamily = CreateLiberationFamily(20.0f, CCO_Unknown);
		ORoot()->Add(pFamily);
		pUprightColours = pFamily->GetColours(FS_Regular, 0.0f);
		pClockwiseColours = pFamily->GetColours(FS_Regular, 0.0f, FT_Clockwise);
		AssertTrue(pUprightColours != pClockwiseColours);
		AssertTrue(pFamily->GetColours(FS_Regular, 20.0f, FT_Clockwise) == pClockwiseColours);
		AssertSize(2, pFamily->NumSizes());

		pUpright = pUprightColours->GetWhite();
		pClockwise = pClockwiseColours->GetWhite();
		AssertString("Liberation Sans Regular 20px clockwise", pClockwise->GetFontName());
		AssertInt(FT_Upright, pUpright->GetDescription()->GetTurn());
		AssertInt(FT_Clockwise, pClockwise->GetDescription()->GetTurn());
		AssertInt(pUpright->GetAscent(), pClockwise->GetAscent());
		AssertInt(pUpright->GetDescent(), pClockwise->GetDescent());
		AssertSize(pUpright->CalculateWidth((char*)"AB ab"), pClockwise->CalculateWidth((char*)"AB ab"));

		szCharacters = "AB ab";
		uiOpaque = 0;
		for (i = 0; szCharacters[i] != '\0'; i++)
		{
			pUprightGlyph = pUpright->GetGlyph((uint32)szCharacters[i], 1);
			pClockwiseGlyph = pClockwise->GetGlyph((uint32)szCharacters[i], 1);
			AssertInt(pUprightGlyph->GetStep(), pClockwiseGlyph->GetStep());

			pcUp = pUprightGlyph->GetSubImage();
			pcTurned = pClockwiseGlyph->GetSubImage();
			AssertInt((int32)pUpright->GetLineHeight(), pcTurned->GetFullWidth());
			AssertInt((int32)pUprightGlyph->GetStep(), pcTurned->GetFullHeight());
			AssertInt((int32)pcUp->GetImageHeight(), pcTurned->GetImageWidth());
			AssertInt((int32)pcUp->GetImageWidth(), pcTurned->GetImageHeight());
			if (pcUp->GetImageWidth() == 0)
			{
				continue;
			}
			AssertInt((int32)pcUp->msOffsetBottomRight.y, pcTurned->msOffsetTopLeft.x);
			AssertInt((int32)pcUp->msOffsetTopLeft.x, pcTurned->msOffsetTopLeft.y);
			AssertInt((int32)pcUp->msOffsetTopLeft.y, pcTurned->msOffsetBottomRight.x);
			AssertInt((int32)pcUp->msOffsetBottomRight.x, pcTurned->msOffsetBottomRight.y);

			//The upright glyph's top row is the turned glyph's right column.
			uiDifferences = 0;
			for (y = 0; y < pcTurned->GetImageHeight(); y++)
			{
				for (x = 0; x < pcTurned->GetImageWidth(); x++)
				{
					if (GetFontImageOpacity(pClockwise, pcTurned->mcImageRect.msTopLeft.x + x, pcTurned->mcImageRect.msTopLeft.y + y) !=
						GetFontImageOpacity(pUpright, pcUp->mcImageRect.msTopLeft.x + y, pcUp->mcImageRect.msTopLeft.y + pcTurned->GetImageWidth() - 1 - x))
					{
						uiDifferences++;
					}
					if (GetFontImageOpacity(pClockwise, pcTurned->mcImageRect.msTopLeft.x + x, pcTurned->mcImageRect.msTopLeft.y + y) != 0)
					{
						uiOpaque++;
					}
				}
			}
			AssertSize(0, uiDifferences);
		}
		AssertTrue(uiOpaque > 100);

		pClockwiseGlyph = NULL;
		pUprightGlyph = NULL;
		pClockwise = NULL;
		pUpright = NULL;
		pClockwiseColours = NULL;
		pUprightColours = NULL;
		pFamily = NULL;
		ORoot()->RemoveAll();
	}
	ObjectsFlush();
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFontFamily(void)
{
	BeginTests();

	DataIOInit();

	TestFontFamilyFacesAndSizes();
	TestFontFamilyMissingFace();
	TestFontFamilyOneFont();
	TestFontFamilyColourOrder();
	TestFontFamilySuperscriptAndSubscript();
	TestFontFamilyLazyGlyphs();
	TestFontFamilyLazyMatchesRasterise();
	TestFontFamilyClockwise();

	DataIOKill();

	TestStatistics();
}

