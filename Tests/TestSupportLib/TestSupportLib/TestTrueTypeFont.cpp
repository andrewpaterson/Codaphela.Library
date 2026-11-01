#include <string.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageWriter.h"
#include "SupportLib/TrueTypeFont.h"
#include "SupportLib/TrueTypeRasteriser.h"
#include "SupportLib/TrueTypeFontFactory.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/GlyphListFontDraw.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageColourConverter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadTrueTypeFont(CTrueTypeFont* pcFont, char* szFontName)
{
	CChars	szFilename;
	bool	bResult;

	szFilename.Init("Input" _FS_ "Fonts" _FS_);
	szFilename.Append(szFontName);
	pcFont->Init();
	bResult = pcFont->Load(szFilename.Text());
	szFilename.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTrueTypeMetrics(int iAdvance, int iLeftSideBearing, int iXMin, int iYMin, int iXMax, int iYMax, CTrueTypeFont* pcFont, int iGlyph)
{
	STrueTypeGlyphMetrics	sMetrics;

	AssertTrue(pcFont->GetGlyphMetrics(iGlyph, &sMetrics));
	AssertInt(iAdvance, sMetrics.iAdvanceWidth);
	AssertInt(iLeftSideBearing, sMetrics.iLeftSideBearing);
	AssertInt(iXMin, sMetrics.iXMin);
	AssertInt(iYMin, sMetrics.iYMin);
	AssertInt(iXMax, sMetrics.iXMax);
	AssertInt(iYMax, sMetrics.iYMax);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the outline's points aren't exactly the glyph's
// bounding box.
//
//////////////////////////////////////////////////////////////////////////
bool TrueTypeOutlineMatchesBounds(CTrueTypeOutline* pcOutline, STrueTypeGlyphMetrics* psMetrics)
{
	size			i;
	STrueTypePoint*	psPoint;
	int32			iXMin;
	int32			iYMin;
	int32			iXMax;
	int32			iYMax;

	if (pcOutline->NumPoints() == 0)
	{
		return (psMetrics->iXMin == 0) && (psMetrics->iYMin == 0) && (psMetrics->iXMax == 0) && (psMetrics->iYMax == 0);
	}

	psPoint = pcOutline->GetPoint(0);
	iXMin = psPoint->x;
	iYMin = psPoint->y;
	iXMax = psPoint->x;
	iYMax = psPoint->y;
	for (i = 1; i < pcOutline->NumPoints(); i++)
	{
		psPoint = pcOutline->GetPoint(i);
		iXMin = (psPoint->x < iXMin) ? psPoint->x : iXMin;
		iYMin = (psPoint->y < iYMin) ? psPoint->y : iYMin;
		iXMax = (psPoint->x > iXMax) ? psPoint->x : iXMax;
		iYMax = (psPoint->y > iYMax) ? psPoint->y : iYMax;
	}
	return (iXMin == psMetrics->iXMin) && (iYMin == psMetrics->iYMin) && (iXMax == psMetrics->iXMax) && (iYMax == psMetrics->iYMax);
}


//////////////////////////////////////////////////////////////////////////
//
// Draws an opacity image as text, darker characters for more coverage.
//
//////////////////////////////////////////////////////////////////////////
void TrueTypeImageToString(Ptr<CImage> pcImage, CChars* psz)
{
	char	aszShades[] = " .:+*#";
	uint8*	puiData;
	uint32	x;
	uint32	y;
	uint8	uiValue;

	puiData = (uint8*)pcImage->GetData();
	for (y = 0; y < pcImage->GetHeight(); y++)
	{
		for (x = 0; x < pcImage->GetWidth(); x++)
		{
			uiValue = puiData[y * pcImage->GetWidth() + x];
			if (uiValue == 0)
			{
				psz->Append(aszShades[0]);
			}
			else if (uiValue == 255)
			{
				psz->Append(aszShades[5]);
			}
			else
			{
				psz->Append(aszShades[1 + uiValue / 64]);
			}
		}
		psz->Append('\n');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontLoad(void)
{
	CTrueTypeFont	cFont;
	CChars			sz;
	int				iXMin;
	int				iYMin;
	int				iXMax;
	int				iYMax;
	SLogConfig		sLogConfig;

	AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
	AssertInt(675, cFont.NumGlyphs());
	AssertInt(2048, cFont.GetUnitsPerEm());
	AssertInt(1854, cFont.GetAscender());
	AssertInt(-434, cFont.GetDescender());
	AssertInt(67, cFont.GetLineGap());
	cFont.GetBounds(&iXMin, &iYMin, &iXMax, &iYMax);
	AssertInt(-416, iXMin);
	AssertInt(-621, iYMin);
	AssertInt(2151, iXMax);
	AssertInt(1864, iYMax);
	sz.Init();
	AssertTrue(cFont.GetName(TRUE_TYPE_NAME_FAMILY, &sz));
	AssertString("Liberation Sans", sz.Text());
	sz.Kill();
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
	AssertInt(1590, cFont.NumGlyphs());
	AssertInt(1000, cFont.GetUnitsPerEm());
	AssertInt(1020, cFont.GetAscender());
	AssertInt(-300, cFont.GetDescender());
	AssertInt(0, cFont.GetLineGap());
	sz.Init();
	AssertTrue(cFont.GetName(TRUE_TYPE_NAME_FAMILY, &sz));
	AssertString("JetBrains Mono NL", sz.Text());
	sz.Kill();
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "InterVariable.ttf"));
	AssertInt(2937, cFont.NumGlyphs());
	AssertInt(2048, cFont.GetUnitsPerEm());
	AssertInt(1984, cFont.GetAscender());
	AssertInt(-494, cFont.GetDescender());
	AssertInt(0, cFont.GetLineGap());
	sz.Init();
	AssertTrue(cFont.GetName(TRUE_TYPE_NAME_FAMILY, &sz));
	AssertString("Inter Variable", sz.Text());
	sz.Kill();
	cFont.Kill();

	sLogConfig = gcLogger.SetSilent();
	cFont.Init();
	AssertFalse(cFont.Load("Input" _FS_ "Fonts" _FS_ "Missing.ttf"));
	AssertFalse(cFont.Load("Input" _FS_ "Adelle.png"));
	cFont.Kill();
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// Liberation Sans only has a format 4 character map; JetBrains Mono and
// Inter have format 12 maps too.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontGlyphIndex(void)
{
	CTrueTypeFont	cFont;

	AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
	AssertInt(36, cFont.GetGlyphIndex('A'));
	AssertInt(3, cFont.GetGlyphIndex(' '));
	AssertInt(74, cFont.GetGlyphIndex('g'));
	AssertInt(8, cFont.GetGlyphIndex('%'));
	AssertInt(129, cFont.GetGlyphIndex(0xC1));
	AssertInt(169, cFont.GetGlyphIndex(0xE9));
	AssertInt(0, cFont.GetGlyphIndex(0x10FFFF));
	AssertInt(0, cFont.GetGlyphIndex(0x1F600));
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
	AssertInt(1, cFont.GetGlyphIndex('A'));
	AssertInt(888, cFont.GetGlyphIndex(' '));
	AssertInt(245, cFont.GetGlyphIndex('g'));
	AssertInt(1010, cFont.GetGlyphIndex('%'));
	AssertInt(2, cFont.GetGlyphIndex(0xC1));
	AssertInt(226, cFont.GetGlyphIndex(0xE9));
	AssertInt(0, cFont.GetGlyphIndex(0x10FFFF));
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "InterVariable.ttf"));
	AssertInt(2, cFont.GetGlyphIndex('A'));
	AssertInt(1777, cFont.GetGlyphIndex(' '));
	AssertInt(658, cFont.GetGlyphIndex('g'));
	AssertInt(1744, cFont.GetGlyphIndex('%'));
	AssertInt(6, cFont.GetGlyphIndex(0xC1));
	AssertInt(618, cFont.GetGlyphIndex(0xE9));
	AssertInt(0, cFont.GetGlyphIndex(0x10FFFF));
	cFont.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontMetrics(void)
{
	CTrueTypeFont			cFont;
	STrueTypeGlyphMetrics	sMetrics;

	AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
	AssertTrueTypeMetrics(1366, 4, 4, 0, 1362, 1409, &cFont, 36);
	AssertTrueTypeMetrics(569, 0, 0, 0, 0, 0, &cFont, 3);
	AssertTrueTypeMetrics(1139, 86, 86, -425, 1007, 1099, &cFont, 74);
	AssertTrueTypeMetrics(1366, 4, 4, 0, 1362, 1776, &cFont, 129);
	AssertTrueTypeMetrics(1139, 87, 87, -20, 1048, 1508, &cFont, 169);
	AssertFalse(cFont.GetGlyphMetrics(675, &sMetrics));
	AssertFalse(cFont.GetGlyphMetrics(-1, &sMetrics));
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
	AssertTrueTypeMetrics(600, 50, 50, 0, 550, 730, &cFont, 1);
	AssertTrueTypeMetrics(600, 0, 0, 0, 0, 0, &cFont, 888);
	AssertTrueTypeMetrics(600, 88, 88, -180, 507, 560, &cFont, 245);
	AssertTrueTypeMetrics(600, 50, 50, 0, 550, 950, &cFont, 2);
	cFont.Kill();

	AssertTrue(LoadTrueTypeFont(&cFont, "InterVariable.ttf"));
	AssertTrueTypeMetrics(1413, 52, 52, 0, 1361, 1490, &cFont, 2);
	AssertTrueTypeMetrics(576, 0, 0, 0, 0, 0, &cFont, 1777);
	AssertTrueTypeMetrics(1256, 104, 104, -442, 1098, 1132, &cFont, 658);
	AssertTrueTypeMetrics(2011, 230, 230, -29, 1781, 1514, &cFont, 1744);
	AssertTrueTypeMetrics(1194, 104, 104, -24, 1094, 1558, &cFont, 618);
	cFont.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontOutline(void)
{
	CTrueTypeFont			cFont;
	CTrueTypeOutline		cOutline;
	STrueTypeGlyphMetrics	sMetrics;

	AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
	cOutline.Init();

	AssertTrue(cFont.GetGlyphOutline(36, &cOutline));
	AssertSize(2, cOutline.NumContours());
	AssertSize(21, cOutline.NumPoints());
	AssertSize(0, cOutline.GetContourStart(0));
	AssertSize(cOutline.GetContourEnd(0) + 1, cOutline.GetContourStart(1));
	AssertSize(20, cOutline.GetContourEnd(1));
	cFont.GetGlyphMetrics(36, &sMetrics);
	AssertTrue(TrueTypeOutlineMatchesBounds(&cOutline, &sMetrics));

	//Space has no outline.
	cOutline.Clear();
	AssertTrue(cFont.GetGlyphOutline(3, &cOutline));
	AssertSize(0, cOutline.NumContours());
	AssertSize(0, cOutline.NumPoints());

	//A composite of A and an acute accent.
	cOutline.Clear();
	AssertTrue(cFont.GetGlyphOutline(129, &cOutline));
	AssertTrue(cOutline.NumContours() > 2);
	cFont.GetGlyphMetrics(129, &sMetrics);
	AssertTrue(TrueTypeOutlineMatchesBounds(&cOutline, &sMetrics));

	AssertFalse(cFont.GetGlyphOutline(675, &cOutline));
	AssertSize(0, cOutline.NumPoints());

	cOutline.Kill();
	cFont.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every glyph of every font reads, and its points match its bounding box.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontAllGlyphs(void)
{
	char*					aszFonts[] = {	"InterVariable.ttf", "InterVariable-Italic.ttf",
											"JetBrainsMonoNL-Bold.ttf", "JetBrainsMonoNL-BoldItalic.ttf", "JetBrainsMonoNL-Italic.ttf",
											"JetBrainsMonoNL-Light.ttf", "JetBrainsMonoNL-LightItalic.ttf", "JetBrainsMonoNL-Regular.ttf",
											"LiberationSans-Bold.ttf", "LiberationSans-BoldItalic.ttf", "LiberationSans-Italic.ttf", "LiberationSans-Regular.ttf" };
	int						iFont;
	CTrueTypeFont			cFont;
	CTrueTypeOutline		cOutline;
	STrueTypeGlyphMetrics	sMetrics;
	int						iGlyph;
	int						iFailedOutlines;
	int						iMismatchedBounds;

	cOutline.Init();
	for (iFont = 0; iFont < 12; iFont++)
	{
		AssertTrue(LoadTrueTypeFont(&cFont, aszFonts[iFont]));

		iFailedOutlines = 0;
		iMismatchedBounds = 0;
		for (iGlyph = 0; iGlyph < cFont.NumGlyphs(); iGlyph++)
		{
			cOutline.Clear();
			if (!cFont.GetGlyphOutline(iGlyph, &cOutline) || !cFont.GetGlyphMetrics(iGlyph, &sMetrics))
			{
				iFailedOutlines++;
			}
			else if (!TrueTypeOutlineMatchesBounds(&cOutline, &sMetrics))
			{
				iMismatchedBounds++;
			}
		}
		AssertInt(0, iFailedOutlines);
		AssertInt(0, iMismatchedBounds);

		cFont.Kill();
	}
	cOutline.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeRasteriseGlyph(void)
{
	ObjectsInit();
	{
		CTrueTypeFont			cFont;
		CTrueTypeRasteriser		cRasteriser;
		Ptr<CImage>				pcImage;
		int						iLeft;
		int						iTop;
		CChars					sz;

		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
		cRasteriser.Init(&cFont, 16.0f);
		AssertFloat(16.0f / 2048.0f, cRasteriser.GetScale(), 6);

		pcImage = cRasteriser.RasteriseGlyph(cFont.GetGlyphIndex('A'), &iLeft, &iTop);
		AssertTrue(pcImage.IsNotNull());
		AssertInt(0, iLeft);
		AssertInt(-12, iTop);
		AssertInt((int32)11, pcImage->GetWidth());
		AssertInt((int32)12, pcImage->GetHeight());
		AssertSize(1, pcImage->NumChannels());
		AssertTrue(pcImage->HasChannel(IMAGE_OPACITY));
		sz.Init();
		TrueTypeImageToString(pcImage, &sz);
		AssertString(
			"    ..     \n"
			"    +#:    \n"
			"   .*+*    \n"
			"   +*.#.   \n"
			"  .*+ *+   \n"
			"  :*. :*.  \n"
			"  ++  .*:  \n"
			" .*+:::**  \n"
			" +******#. \n"
			".*+    .*+ \n"
			":#.     :*.\n"
			"**      .*:\n", sz.Text());
		sz.Kill();

		pcImage = cRasteriser.RasteriseGlyph(cFont.GetGlyphIndex('g'), &iLeft, &iTop);
		AssertTrue(pcImage.IsNotNull());
		AssertInt(0, iLeft);
		AssertInt(-9, iTop);
		sz.Init();
		TrueTypeImageToString(pcImage, &sz);
		AssertString(
			"  :+:..:\n"
			" :*++*+*\n"
			".*+  .**\n"
			".#.   **\n"
			":#.   +*\n"
			":#.   +*\n"
			".#:  .**\n"
			" **..+**\n"
			" .***:+*\n"
			"   .  ++\n"
			" ++. .*:\n"
			" .****+.\n"
			"  ..:.  \n", sz.Text());
		sz.Kill();

		pcImage = cRasteriser.RasteriseGlyph(cFont.GetGlyphIndex(' '), &iLeft, &iTop);
		AssertTrue(pcImage.IsNull());

		pcImage = NULL;
		cRasteriser.Kill();
		cFont.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeRasteriseText(void)
{
	ObjectsInit();
	{
		CTrueTypeFont			cFont;
		CTrueTypeRasteriser		cRasteriser;
		Ptr<CImage>				pcImage;
		int						iOriginX;
		int						iOriginY;
		CChars					sz;

		AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
		cRasteriser.Init(&cFont, 10.0f);
		AssertFloat(30.0f, cRasteriser.GetTextWidth("Hello"), 3);

		pcImage = cRasteriser.RasteriseText("Hi", &iOriginX, &iOriginY);
		AssertTrue(pcImage.IsNotNull());
		AssertInt(0, iOriginX);
		AssertInt(8, iOriginY);
		sz.Init();
		TrueTypeImageToString(pcImage, &sz);
		AssertString(
			".:  :.  :+  \n"
			".*  *.  .:  \n"
			".*  *. :+:  \n"
			".*..*. ::+  \n"
			".*++*.  .+  \n"
			".*  *.  .+  \n"
			".*  *.  .+  \n"
			".*  *..****:\n", sz.Text());
		sz.Kill();

		pcImage = cRasteriser.RasteriseText("   ", &iOriginX, &iOriginY);
		AssertTrue(pcImage.IsNull());

		pcImage = NULL;
		cRasteriser.Kill();
		cFont.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeDrawText(void)
{
	char		szDirectory[] = "Output" _FS_ "TrueType";
	CFileUtil	cFileUtil;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));

	ObjectsInit();
	{
		char*					aszFonts[] = { "LiberationSans-Regular.ttf", "LiberationSans-Bold.ttf", "LiberationSans-Italic.ttf", "LiberationSans-BoldItalic.ttf",
												"JetBrainsMonoNL-Light.ttf", "JetBrainsMonoNL-Regular.ttf", "JetBrainsMonoNL-Bold.ttf", "JetBrainsMonoNL-LightItalic.ttf",
												"InterVariable.ttf", "InterVariable-Italic.ttf" };
		float32					afSizes[] = { 11.0f, 16.0f, 24.0f, 40.0f };
		//Colour channels as bytes; fractions like 9 / 10.0f land on half a byte and round either way under /fp:fast.
		int						aiReds[] = { 0, 26, 51, 77, 102, 128, 153, 178, 204, 229 };
		int						aiBlues[] = { 0, 43, 85, 128 };
		CTrueTypeFont			cFont;
		CTrueTypeRasteriser		cRasteriser;
		Ptr<CImage>				pcImage;
		CImageColourRGB			cColour;
		int						iFont;
		int						iSize;
		float32					fY;
		CChars					szOutputFilename;
		CChars					szInputFilename;

		pcImage = OMalloc<CImage>(640, 1280, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
		pcImage->White();

		fY = 40.0f;
		for (iFont = 0; iFont < 10; iFont++)
		{
			AssertTrue(LoadTrueTypeFont(&cFont, aszFonts[iFont]));
			for (iSize = 0; iSize < 4; iSize++)
			{
				cRasteriser.Init(&cFont, afSizes[iSize]);
				fY += cRasteriser.GetAscent();
				cColour.Init(aiReds[iFont] / 255.0f, 0.0f, aiBlues[iSize] / 255.0f);
				AssertTrue(cRasteriser.DrawText(pcImage, 4.25f, fY, "Sphinx of black quartz, judge my vow! \xC3\x81\xC3\xA9 0123 %&@", &cColour));
				fY += cRasteriser.GetDescent();
				cRasteriser.Kill();
			}
			cFont.Kill();
		}

		//Clipped at every edge.
		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Bold.ttf"));
		cRasteriser.Init(&cFont, 64.0f);
		cColour.Init(0.0f, 0.6f, 0.0f);
		AssertTrue(cRasteriser.DrawText(pcImage, 300.0f, 20.0f, "Clipped", &cColour));
		AssertTrue(cRasteriser.DrawText(pcImage, -20.5f, 1200.0f, "Clipped", &cColour));
		AssertTrue(cRasteriser.DrawText(pcImage, 500.0f, 1290.0f, "Clipped", &cColour));
		AssertTrue(cRasteriser.DrawText(pcImage, 5000.0f, 100.0f, "Outside", &cColour));
		cRasteriser.Kill();
		cFont.Kill();

		szOutputFilename.Init(szDirectory);
		cFileUtil.AppendToPath(&szOutputFilename, "DrawText.png");
		AssertTrue(WriteImage(pcImage, szOutputFilename.Text()));
		pcImage = NULL;


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
void TestTrueTypeFontFactoryLazyGlyphs(void)
{
	ObjectsInit();
	{
		CTrueTypeFont				cFont;
		CTrueTypeFontFactory		cFactory;
		CImageColourRGB				cColour;
		Ptr<CFontDescription>		pDescription;
		Ptr<CImage>					pImage;
		Ptr<CFont>					pFont;
		Ptr<CGlyph>					pGlyphA;
		Ptr<CGlyph>					pGlyph;
		SGlyphDescription*			psDescription;
		CUTF8						cUTF8;
		CSimpleTextReader			cReader;
		Ptr<CText>					pText;
		Ptr<CTextLayout>			pLayout;
		Ptr<CGlyphListFontDraw>		pDraw;

		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
		cFactory.Init(&cFont, 16.0f);
		cColour.Init(0.0f, 0.0f, 0.0f);
		pDescription = cFactory.Describe("Liberation Sans 16px", "AB g\xC3\xA9");
		AssertTrue(pDescription.IsNotNull());
		pImage = cFactory.Rasterise(pDescription, &cColour);
		cFactory.Kill();
		AssertTrue(pImage.IsNotNull());
		pFont = OMalloc<CFont>(pDescription, pImage);
		AssertTrue(pFont->GetImage() == pImage);
		AssertTrue(pFont->GetDescription() == pDescription);

		//Every character is described but no glyph has been generated.
		AssertSize(5, pFont->NumGlyphs());
		AssertSize(0, pFont->NumGeneratedGlyphs());

		//Measuring uses the descriptions.
		AssertSize(22, pFont->CalculateWidth("AB"));
		AssertSize(0, pFont->NumGeneratedGlyphs());

		pGlyphA = pFont->GetGlyph((uint32)'A', 1);
		AssertTrue(pGlyphA.IsNotNull());
		AssertSize(1, pFont->NumGeneratedGlyphs());
		AssertInt(11, pGlyphA->GetStep());
		AssertTrue(pGlyphA->GetSourceImage() == pImage);
		psDescription = pFont->GetGlyphDescription((uint8*)"A", 1);
		AssertNotNull(psDescription);
		AssertInt((int32)psDescription->cSubImage.GetSourceLeft(), pGlyphA->GetSubImage()->GetSourceLeft());
		AssertInt((int32)psDescription->cSubImage.GetSourceTop(), pGlyphA->GetSubImage()->GetSourceTop());
		AssertInt((int32)psDescription->cSubImage.GetImageWidth(), pGlyphA->GetSubImage()->GetImageWidth());
		AssertInt((int32)psDescription->cSubImage.GetImageHeight(), pGlyphA->GetSubImage()->GetImageHeight());
		AssertInt(11, pGlyphA->GetFullWidth());
		AssertInt(pFont->GetLineHeight(), pGlyphA->GetFullHeight());

		//Asked for again it is the same glyph.
		pGlyph = pFont->GetGlyph((uint32)'A', 1);
		AssertTrue(pGlyph == pGlyphA);
		AssertSize(1, pFont->NumGeneratedGlyphs());

		cUTF8.Init("\xC3\xA9");
		pGlyph = pFont->GetGlyph(&cUTF8);
		cUTF8.Kill();
		AssertTrue(pGlyph.IsNotNull());
		AssertSize(2, pFont->NumGeneratedGlyphs());

		//A character the font doesn't have.
		pGlyph = pFont->GetGlyph((uint32)'Z', 1);
		AssertTrue(pGlyph.IsNull());
		AssertNull(pFont->GetGlyphDescription((uint8*)"Z", 1));
		AssertSize(2, pFont->NumGeneratedGlyphs());

		//Laying out text generates the glyphs drawn and no others.
		cUTF8.Init("gAg");
		cReader.Init(&pFont);
		pText = cReader.Read(&cUTF8, true);
		cReader.Kill();
		cUTF8.Kill();
		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();
		pDraw->Begin();
		pLayout->Layout(pDraw, pText);
		pDraw->End();
		AssertSize(3, pDraw->GetGlyphs()->NumElements());
		AssertSize(3, pFont->NumGeneratedGlyphs());
		AssertPointer(&pGlyphA, pDraw->GetGlyphs()->Get(1)->pcGlyph);
		AssertPointer(pDraw->GetGlyphs()->Get(0)->pcGlyph, pDraw->GetGlyphs()->Get(2)->pcGlyph);
		AssertSize(5, pFont->NumGlyphs());

		pGlyph = NULL;
		pGlyphA = NULL;
		pText = NULL;
		pLayout = NULL;
		pDraw = NULL;
		pFont = NULL;
		pImage = NULL;
		pDescription = NULL;
		cFont.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixel with the most opacity in the glyph's part of its image.
// Opacity, red, green and blue bytes.
//
//////////////////////////////////////////////////////////////////////////
uint8* GetMostOpaqueGlyphPixel(Ptr<CGlyph> pGlyph)
{
	Ptr<CImage>		pImage;
	CSubImage*		pcSubImage;
	uint8*			puiPixel;
	uint8*			puiMost;
	int32			x;
	int32			y;

	pImage = pGlyph->GetSourceImage();
	pcSubImage = pGlyph->GetSubImage();
	puiMost = NULL;
	for (y = pcSubImage->mcImageRect.msTopLeft.y; y < pcSubImage->mcImageRect.msBottomRight.y; y++)
	{
		for (x = pcSubImage->mcImageRect.msTopLeft.x; x < pcSubImage->mcImageRect.msBottomRight.x; x++)
		{
			puiPixel = &((uint8*)pImage->GetData())[(y * pImage->GetWidth() + x) * 4];
			if ((puiMost == NULL) || (puiPixel[0] > puiMost[0]))
			{
				puiMost = puiPixel;
			}
		}
	}
	return puiMost;
}


//////////////////////////////////////////////////////////////////////////
//
// Opacity, red, green and blue bytes.
//
//////////////////////////////////////////////////////////////////////////
size CountOpacityDifferences(Ptr<CImage> pImage1, Ptr<CImage> pImage2)
{
	lsize	i;
	lsize	uiNumPixels;
	size	uiCount;
	uint8*	puiData1;
	uint8*	puiData2;

	uiNumPixels = (lsize)pImage1->GetWidth() * pImage1->GetHeight();
	puiData1 = (uint8*)pImage1->GetData();
	puiData2 = (uint8*)pImage2->GetData();
	uiCount = 0;
	for (i = 0; i < uiNumPixels; i++)
	{
		if (puiData1[i * 4] != puiData2[i * 4])
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Fonts of different colours share one description, each with its own
// image and its own glyphs.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontFactoryColours(void)
{
	ObjectsInit();
	{
		CTrueTypeFont				cFont;
		CTrueTypeFontFactory		cFactory;
		CImageColourRGB				cBlack;
		CImageColourRGB				cRed;
		Ptr<CFontDescription>		pDescription;
		Ptr<CFont>					pBlack;
		Ptr<CFont>					pRed;
		Ptr<CGlyph>					pBlackA;
		Ptr<CGlyph>					pRedA;
		uint8*						puiBlackPixel;
		uint8*						puiRedPixel;

		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
		cFactory.Init(&cFont, 16.0f);
		cBlack.Init(0.0f, 0.0f, 0.0f);
		cRed.Init(1.0f, 0.0f, 0.0f);
		pDescription = cFactory.Describe("Liberation Sans 16px", "AB");
		pBlack = OMalloc<CFont>(pDescription, cFactory.Rasterise(pDescription, &cBlack));
		pRed = OMalloc<CFont>(pDescription, cFactory.Rasterise(pDescription, &cRed));
		cFactory.Kill();

		AssertTrue(pBlack->GetDescription() == pRed->GetDescription());
		AssertTrue(pBlack->GetImage() != pRed->GetImage());
		AssertInt((int32)pDescription->GetImageWidth(), pRed->GetImage()->GetWidth());
		AssertInt((int32)pDescription->GetImageHeight(), pRed->GetImage()->GetHeight());
		AssertSize(2, pDescription->NumGlyphs());
		AssertSize(0, pDescription->GetGlyph((uint8*)"A", 1)->uiIndex);
		AssertSize(1, pDescription->GetGlyph((uint8*)"B", 1)->uiIndex);

		//Each font generates its own glyphs.
		pBlackA = pBlack->GetGlyph((uint32)'A', 1);
		AssertSize(1, pBlack->NumGeneratedGlyphs());
		AssertSize(0, pRed->NumGeneratedGlyphs());
		pRedA = pRed->GetGlyph((uint32)'A', 1);
		AssertSize(1, pRed->NumGeneratedGlyphs());
		AssertTrue(pBlackA != pRedA);
		AssertTrue(pBlackA->GetSourceImage() == pBlack->GetImage());
		AssertTrue(pRedA->GetSourceImage() == pRed->GetImage());
		AssertInt((int32)pBlackA->GetSubImage()->GetSourceLeft(), pRedA->GetSubImage()->GetSourceLeft());
		AssertInt((int32)pBlackA->GetSubImage()->GetSourceTop(), pRedA->GetSubImage()->GetSourceTop());
		AssertInt(pBlackA->GetStep(), pRedA->GetStep());

		//The same coverage in each colour.
		puiBlackPixel = GetMostOpaqueGlyphPixel(pBlackA);
		puiRedPixel = GetMostOpaqueGlyphPixel(pRedA);
		AssertInt(255, puiBlackPixel[0]);
		AssertInt(0, puiBlackPixel[1]);
		AssertInt(255, puiRedPixel[0]);
		AssertInt(255, puiRedPixel[1]);
		AssertInt(0, puiRedPixel[2]);
		AssertInt(0, puiRedPixel[3]);
		AssertSize(0, CountOpacityDifferences(pBlack->GetImage(), pRed->GetImage()));

		pBlackA = NULL;
		pRedA = NULL;
		pBlack = NULL;
		pRed = NULL;
		pDescription = NULL;
		cFont.Kill();
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A glyph's full width is its advance rounded to whole pixels.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontDescriptionWidths(void)
{
	ObjectsInit();
	{
		CTrueTypeFont				cFont;
		CTrueTypeFontFactory		cFactory;
		Ptr<CFontDescription>		pDescription;
		CSubImage					cSubImage;

		//Liberation Sans is proportional: A and B 11, space 4, g and e acute 9.
		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
		cFactory.Init(&cFont, 16.0f);
		pDescription = cFactory.Describe("Liberation Sans 16px", "AB g\xC3\xA9");
		cFactory.Kill();
		cFont.Kill();
		AssertSize(44 / 5, pDescription->GetAverageWidth());
		AssertFalse(pDescription->IsFixedWidth());
		AssertSize((size)pDescription->GetLineHeight(), pDescription->GetAverageHeight());

		//Putting a glyph works the widths out again.
		cSubImage.Init(0, 0, 0, 0, 56, 0, 0, 24);
		pDescription->PutGlyph((uint32)'W', 1, &cSubImage, 56);
		AssertSize(100 / 6, pDescription->GetAverageWidth());

		//JetBrains Mono is fixed width.
		AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
		cFactory.Init(&cFont, 16.0f);
		pDescription = cFactory.Describe("JetBrains Mono 16px", "AB g\xC3\xA9");
		cFactory.Kill();
		cFont.Kill();
		AssertSize(10, pDescription->GetAverageWidth());
		AssertTrue(pDescription->IsFixedWidth());

		pDescription = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A font of characters in white, made from a TrueType font in
// Input/Fonts.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CreateWhiteTrueTypeFontFrom(char* szFontName, char* szCharacters, float32 fPixelSize)
{
	CTrueTypeFont				cFont;
	CTrueTypeFontFactory		cFactory;
	CImageColourRGB				cWhite;
	Ptr<CFontDescription>		pDescription;
	Ptr<CFont>					pFont;

	if (!LoadTrueTypeFont(&cFont, szFontName))
	{
		return NULL;
	}
	cFactory.Init(&cFont, fPixelSize);
	cWhite.Init(1.0f, 1.0f, 1.0f);
	pDescription = cFactory.Describe(szFontName, szCharacters);
	pFont = OMalloc<CFont>(pDescription, cFactory.Rasterise(pDescription, &cWhite));
	cFactory.Kill();
	cFont.Kill();
	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
// A font of characters in white, made from Liberation Sans.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CreateWhiteTrueTypeFont(char* szCharacters, float32 fPixelSize)
{
	return CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", szCharacters, fPixelSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The red, green, blue and opacity of a pixel, whatever order the image
// has them in.
//
//////////////////////////////////////////////////////////////////////////
void GetPixelRGBA(Ptr<CImage> pImage, int32 x, int32 y, uint8* puiRGBA)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;

	cCreator.Init(&pImage);
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();
	memcpy(puiRGBA, pcAccessor->Get(x, y), 4);
	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The most opaque pixel of the glyph's part of its image, as red, green,
// blue and opacity.
//
//////////////////////////////////////////////////////////////////////////
void GetMostOpaqueGlyphRGBA(Ptr<CGlyph> pGlyph, uint8* puiRGBA)
{
	Ptr<CImage>		pImage;
	CSubImage*		pcSubImage;
	uint8			auiPixel[4];
	int32			x;
	int32			y;

	pImage = pGlyph->GetSourceImage();
	pcSubImage = pGlyph->GetSubImage();
	memset(puiRGBA, 0, 4);
	for (y = pcSubImage->mcImageRect.msTopLeft.y; y < pcSubImage->mcImageRect.msBottomRight.y; y++)
	{
		for (x = pcSubImage->mcImageRect.msTopLeft.x; x < pcSubImage->mcImageRect.msBottomRight.x; x++)
		{
			GetPixelRGBA(pImage, x, y, auiPixel);
			if (auiPixel[3] > puiRGBA[3])
			{
				memcpy(puiRGBA, auiPixel, 4);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontColours(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pRed;
		Ptr<CFont>				pHalfRed;
		Ptr<CFont>				pBGRWhite;
		Ptr<CFont>				pBGRRed;
		Ptr<CFontColours>		pBGRColours;
		uint8					auiRGBA[4];

		pWhite = CreateWhiteTrueTypeFont("AB", 16.0f);
		AssertTrue(pWhite.IsNotNull());
		pColours = OMalloc<CFontColours>(pWhite);

		//White is the white font.
		AssertTrue(pColours->GetFont(FONT_COLOURS_WHITE) == pWhite);
		AssertTrue(pColours->GetWhite() == pWhite);
		AssertSize(0, pColours->NumColours());

		//A colour is made once and shares the white font's description.
		pRed = pColours->GetFont(Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00));
		AssertTrue(pRed.IsNotNull());
		AssertTrue(pRed != pWhite);
		AssertTrue(pRed->GetDescription() == pWhite->GetDescription());
		AssertTrue(pRed->GetImage() != pWhite->GetImage());
		AssertSize(1, pColours->NumColours());
		AssertTrue(pColours->GetFont(Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00)) == pRed);
		AssertSize(1, pColours->NumColours());

		//The colour is set and the coverage kept.
		GetMostOpaqueGlyphRGBA(pRed->GetGlyph((uint32)'A', 1), auiRGBA);
		AssertInt(255, auiRGBA[0]);
		AssertInt(0, auiRGBA[1]);
		AssertInt(0, auiRGBA[2]);
		AssertInt(255, auiRGBA[3]);
		AssertSize(0, CountOpacityDifferences(pWhite->GetImage(), pRed->GetImage()));

		//An alpha scales the opacity.
		pHalfRed = pColours->GetFont(Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00, (uint8)0x80));
		AssertSize(2, pColours->NumColours());
		GetMostOpaqueGlyphRGBA(pHalfRed->GetGlyph((uint32)'A', 1), auiRGBA);
		AssertInt(255, auiRGBA[0]);
		AssertInt(128, auiRGBA[3]);

		//A white font in blue, green, red order is recoloured in that order.
		pBGRWhite = OMalloc<CFont>(pWhite->GetDescription(), CImageColourConverter::ConvertColour(pWhite->GetImage(), CCO_BGR));
		AssertInt(CCO_BGR, pBGRWhite->GetImage()->GetColourOrder());
		pBGRColours = OMalloc<CFontColours>(pBGRWhite);
		pBGRRed = pBGRColours->GetFont(Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00));
		AssertInt(CCO_BGR, pBGRRed->GetImage()->GetColourOrder());
		GetMostOpaqueGlyphRGBA(pBGRRed->GetGlyph((uint32)'A', 1), auiRGBA);
		AssertInt(255, auiRGBA[0]);
		AssertInt(0, auiRGBA[1]);
		AssertInt(0, auiRGBA[2]);
		AssertInt(255, auiRGBA[3]);

		pBGRRed = NULL;
		pBGRColours = NULL;
		pBGRWhite = NULL;
		pHalfRed = NULL;
		pRed = NULL;
		pColours = NULL;
		pWhite = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A font change starts a new run.  A change in the middle of a character
// takes effect at the next character.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontColoursTextRuns(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pWhite;
		Ptr<CFontColours>		pColours;
		Ptr<CFont>				pRed;
		CArrayTextFontChange	asChanges;
		STextFontChange*		psChange;
		CSimpleTextReader		cReader;
		CUTF8					cUTF8;
		Ptr<CText>				pText;
		Ptr<CTextLayout>		pLayout;
		Ptr<CGlyphListFontDraw>	pDraw;
		CArrayGlyphPosition*	pasGlyphs;

		pWhite = CreateWhiteTrueTypeFont("abcd\xC3\xA9", 16.0f);
		pColours = OMalloc<CFontColours>(pWhite);
		pRed = pColours->GetFont(Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00));

		//"ab", e acute (2 bytes), "cd".  Red from the middle of the e acute to before the d.
		asChanges.Init();
		psChange = asChanges.Add();
		psChange->Init(3, &pRed);
		psChange = asChanges.Add();
		psChange->Init(5, &pWhite);

		cUTF8.Init("ab\xC3\xA9" "cd");
		cReader.Init(&pWhite);
		pText = cReader.Read(&cUTF8, true, &asChanges);
		cReader.Kill();
		cUTF8.Kill();
		asChanges.Kill();

		AssertSize(3, pText->NumRuns());
		AssertPointer(&pWhite, pText->GetRun(0)->GetFont());
		AssertPointer(&pRed, pText->GetRun(1)->GetFont());
		AssertPointer(&pWhite, pText->GetRun(2)->GetFont());

		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();
		pDraw->Begin();
		pLayout->Layout(pDraw, pText);
		pDraw->End();
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(5, pasGlyphs->NumElements());
		AssertPointer(&pWhite->GetImage(), &pasGlyphs->Get(2)->pcGlyph->GetSourceImage());
		AssertPointer(&pRed->GetImage(), &pasGlyphs->Get(3)->pcGlyph->GetSourceImage());
		AssertPointer(&pWhite->GetImage(), &pasGlyphs->Get(4)->pcGlyph->GetSourceImage());

		//The glyphs are placed as if the text were all one colour.
		AssertInt((int32)pasGlyphs->Get(2)->sPosition.x + pasGlyphs->Get(2)->pcGlyph->GetStep(), pasGlyphs->Get(3)->sPosition.x);
		AssertInt((int32)pasGlyphs->Get(2)->sPosition.y, pasGlyphs->Get(3)->sPosition.y);

		pDraw = NULL;
		pLayout = NULL;
		pText = NULL;
		pRed = NULL;
		pColours = NULL;
		pWhite = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out szText in pSmall with the bytes from uiBigStart to uiBigEnd in
// pBig.
//
//////////////////////////////////////////////////////////////////////////
void LayoutMixedSizes(Ptr<CTextLayout> pLayout, Ptr<CGlyphListFontDraw> pDraw, Ptr<CFont> pSmall, Ptr<CFont> pBig, char* szText, size uiBigStart, size uiBigEnd)
{
	CArrayTextFontChange	asChanges;
	STextFontChange*		psChange;
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	Ptr<CText>				pText;

	asChanges.Init();
	psChange = asChanges.Add();
	psChange->Init(uiBigStart, &pBig);
	psChange = asChanges.Add();
	psChange->Init(uiBigEnd, &pSmall);

	cUTF8.Init(szText);
	cReader.Init(&pSmall);
	pText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();

	pDraw->Begin();
	pLayout->Layout(pDraw, pText);
	pDraw->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Glyphs of different sizes on a line share a baseline.  A line is as high
// as the largest ascent plus the largest descent of the glyphs on it.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontMixedSizeLayout(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pSmall;
		Ptr<CFont>				pBig;
		Ptr<CTextLayout>		pLayout;
		Ptr<CGlyphListFontDraw>	pDraw;
		CArrayGlyphPosition*	pasGlyphs;
		SInt32Vec2				sSize;
		int32					iSmallAscent;
		int32					iSmallHeight;
		int32					iBigAscent;
		int32					iBigHeight;
		int32					iWidth;

		pSmall = CreateWhiteTrueTypeFont("ab ", 16.0f);
		pBig = CreateWhiteTrueTypeFont("AB", 32.0f);
		iSmallAscent = pSmall->GetAscent();
		iSmallHeight = pSmall->GetLineHeight();
		iBigAscent = pBig->GetAscent();
		iBigHeight = pBig->GetLineHeight();
		AssertTrue(iBigAscent > iSmallAscent);
		AssertTrue(pBig->GetDescent() > pSmall->GetDescent());

		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();

		//"aA b" then "b" on its own line.
		LayoutMixedSizes(pLayout, pDraw, pSmall, pBig, "aA b\nb", 1, 2);
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(5, pasGlyphs->NumElements());
		AssertInt((int32)iBigAscent - iSmallAscent, pasGlyphs->Get(0)->sPosition.y);
		AssertInt((int32)0, pasGlyphs->Get(1)->sPosition.y);
		AssertInt((int32)iBigAscent - iSmallAscent, pasGlyphs->Get(2)->sPosition.y);
		AssertInt((int32)iBigAscent - iSmallAscent, pasGlyphs->Get(3)->sPosition.y);
		AssertInt((int32)iBigHeight, pasGlyphs->Get(4)->sPosition.y);
		AssertInt((int32)0, pasGlyphs->Get(4)->sPosition.x);
		sSize = pLayout->GetSize();
		AssertInt(iBigHeight + iSmallHeight, sSize.y);

		//An empty line is as high as the font it is in.
		LayoutMixedSizes(pLayout, pDraw, pSmall, pBig, "a\n\nA", 3, 4);
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(2, pasGlyphs->NumElements());
		AssertInt((int32)0, pasGlyphs->Get(0)->sPosition.y);
		AssertInt((int32)iSmallHeight * 2, pasGlyphs->Get(1)->sPosition.y);
		sSize = pLayout->GetSize();
		AssertInt(iSmallHeight * 2 + iBigHeight, sSize.y);

		//A big word wrapped to the next line leaves the first line only as high as the small glyphs on it.
		//Wide enough for BB but not for aa BB.
		iWidth = (int32)pBig->GetGlyph((uint32)'B', 1)->GetFullWidth() * 2 + 1;
		pLayout->SetWidth(iWidth);
		LayoutMixedSizes(pLayout, pDraw, pSmall, pBig, "aa BB", 3, 5);
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(5, pasGlyphs->NumElements());
		AssertInt((int32)0, pasGlyphs->Get(0)->sPosition.y);
		AssertInt((int32)0, pasGlyphs->Get(3)->sPosition.x);
		AssertInt((int32)iSmallHeight, pasGlyphs->Get(3)->sPosition.y);
		AssertInt((int32)iSmallHeight, pasGlyphs->Get(4)->sPosition.y);
		sSize = pLayout->GetSize();
		AssertInt(iSmallHeight + iBigHeight, sSize.y);

		pDraw = NULL;
		pLayout = NULL;
		pBig = NULL;
		pSmall = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Different typefaces on a line share a baseline.  The line's ascent and
// descent can come from different fonts.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontMixedTypefaceLayout(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pLiberation;
		Ptr<CFont>				pMono;
		Ptr<CFont>				pInter;
		CArrayTextFontChange	asChanges;
		STextFontChange*		psChange;
		CSimpleTextReader		cReader;
		CUTF8					cUTF8;
		Ptr<CText>				pText;
		Ptr<CTextLayout>		pLayout;
		Ptr<CGlyphListFontDraw>	pDraw;
		CArrayGlyphPosition*	pasGlyphs;
		SInt32Vec2				sSize;
		int32					iRowAscent;
		int32					iRowDescent;

		pLiberation = CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", "abX", 20.0f);
		pMono = CreateWhiteTrueTypeFontFrom("JetBrainsMonoNL-Regular.ttf", "abX", 17.0f);
		pInter = CreateWhiteTrueTypeFontFrom("InterVariable.ttf", "abX", 16.0f);

		//Liberation Sans has the most ascent and JetBrains Mono the most descent.
		AssertTrue(pLiberation->GetAscent() > pMono->GetAscent());
		AssertTrue(pLiberation->GetAscent() > pInter->GetAscent());
		AssertTrue(pMono->GetDescent() > pLiberation->GetDescent());
		AssertTrue(pMono->GetDescent() > pInter->GetDescent());
		iRowAscent = pLiberation->GetAscent();
		iRowDescent = pMono->GetDescent();

		//"a" in Liberation Sans, "X" in JetBrains Mono, "b" in Inter, then "b" in Inter on its own line.
		asChanges.Init();
		psChange = asChanges.Add();
		psChange->Init(1, &pMono);
		psChange = asChanges.Add();
		psChange->Init(2, &pInter);
		cUTF8.Init("aXb\nb");
		cReader.Init(&pLiberation);
		pText = cReader.Read(&cUTF8, true, &asChanges);
		cReader.Kill();
		cUTF8.Kill();
		asChanges.Kill();
		//A line feed ends a run so the second line's b is a run of its own.
		AssertSize(4, pText->NumRuns());

		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();
		pDraw->Begin();
		pLayout->Layout(pDraw, pText);
		pDraw->End();
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(4, pasGlyphs->NumElements());
		AssertInt((int32)iRowAscent - pLiberation->GetAscent(), pasGlyphs->Get(0)->sPosition.y);
		AssertInt((int32)iRowAscent - pMono->GetAscent(), pasGlyphs->Get(1)->sPosition.y);
		AssertInt((int32)iRowAscent - pInter->GetAscent(), pasGlyphs->Get(2)->sPosition.y);
		AssertInt((int32)pasGlyphs->Get(0)->pcGlyph->GetFullWidth(), pasGlyphs->Get(1)->sPosition.x);
		AssertInt((int32)pasGlyphs->Get(1)->sPosition.x + pasGlyphs->Get(1)->pcGlyph->GetFullWidth(), pasGlyphs->Get(2)->sPosition.x);
		AssertInt((int32)iRowAscent + iRowDescent, pasGlyphs->Get(3)->sPosition.y);
		sSize = pLayout->GetSize();
		AssertInt(iRowAscent + iRowDescent + pInter->GetLineHeight(), sSize.y);

		pDraw = NULL;
		pLayout = NULL;
		pText = NULL;
		pInter = NULL;
		pMono = NULL;
		pLiberation = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The underline, strikeout, superscript and subscript from the post and
// OS/2 tables, in font units and as pixels in a description.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontScriptAndLineMetrics(void)
{
	ObjectsInit();
	{
		CTrueTypeFont			cFont;
		CTrueTypeFontFactory	cFactory;
		Ptr<CFontDescription>	pDescription;

		AssertTrue(LoadTrueTypeFont(&cFont, "LiberationSans-Regular.ttf"));
		AssertTrue(cFont.HasPost());
		AssertInt(-367, cFont.GetUnderlinePosition());
		AssertInt(150, cFont.GetUnderlineThickness());
		AssertTrue(cFont.HasOS2());
		AssertInt(530, cFont.GetStrikeoutPosition());
		AssertInt(102, cFont.GetStrikeoutSize());
		AssertInt(1331, cFont.GetSuperscriptYSize());
		AssertInt(977, cFont.GetSuperscriptYOffset());
		AssertInt(1331, cFont.GetSubscriptYSize());
		AssertInt(283, cFont.GetSubscriptYOffset());

		//At 16 pixels an em is 2048 units.
		cFactory.Init(&cFont, 16.0f);
		pDescription = cFactory.Describe("Liberation Sans 16px", "AB");
		cFactory.Kill();
		cFont.Kill();
		AssertInt(3, pDescription->GetUnderlineTop());
		AssertInt(1, pDescription->GetUnderlineThickness());
		AssertInt(-4, pDescription->GetStrikeoutTop());
		AssertInt(1, pDescription->GetStrikeoutThickness());
		AssertFloat(1331.0f / 2048.0f, pDescription->GetSuperscriptScale(), 6);
		AssertInt(8, pDescription->GetSuperscriptShift());
		AssertFloat(1331.0f / 2048.0f, pDescription->GetSubscriptScale(), 6);
		AssertInt(2, pDescription->GetSubscriptShift());

		//At 20 pixels an em is 1000 units.  A subscript shift of 1.5 pixels rounds to 2.
		AssertTrue(LoadTrueTypeFont(&cFont, "JetBrainsMonoNL-Regular.ttf"));
		cFactory.Init(&cFont, 20.0f);
		pDescription = cFactory.Describe("JetBrains Mono 20px", "AB");
		cFactory.Kill();
		cFont.Kill();
		AssertInt(3, pDescription->GetUnderlineTop());
		AssertInt(1, pDescription->GetUnderlineThickness());
		AssertInt(-6, pDescription->GetStrikeoutTop());
		AssertInt(1, pDescription->GetStrikeoutThickness());
		AssertFloat(0.6f, pDescription->GetSuperscriptScale(), 6);
		AssertInt(7, pDescription->GetSuperscriptShift());
		AssertFloat(0.6f, pDescription->GetSubscriptScale(), 6);
		AssertInt(2, pDescription->GetSubscriptShift());

		//A font without the tables, ascent 15 and descent 4.
		pDescription = OMalloc<CFontDescription>((char*)"Estimated", (int16)4, (int16)15, (int16)4, 0, 0);
		AssertInt(1, pDescription->GetUnderlineTop());
		AssertInt(1, pDescription->GetUnderlineThickness());
		AssertInt(-5, pDescription->GetStrikeoutTop());
		AssertInt(1, pDescription->GetStrikeoutThickness());
		AssertFloat(0.65f, pDescription->GetSuperscriptScale(), 6);
		AssertInt(7, pDescription->GetSuperscriptShift());
		AssertFloat(0.65f, pDescription->GetSubscriptScale(), 6);
		AssertInt(3, pDescription->GetSubscriptShift());

		pDescription = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// A run moved up (superscript) or down (subscript) sits that far from the
// line's baseline, and the line grows to fit it.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontScriptLayout(void)
{
	ObjectsInit();
	{
		Ptr<CFont>				pBig;
		Ptr<CFont>				pSmall;
		CArrayTextFontChange	asChanges;
		CSimpleTextReader		cReader;
		CUTF8					cUTF8;
		Ptr<CText>				pText;
		Ptr<CTextLayout>		pLayout;
		Ptr<CGlyphListFontDraw>	pDraw;
		CArrayGlyphPosition*	pasGlyphs;
		SInt32Vec2				sSize;
		int32					iRowAscent;
		int32					iRowDescent;

		pBig = CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", "xy2", 20.0f);
		pSmall = CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", "xy2", 13.0f);

		//Raised 10 the small 2 reaches above the big x, lowered 4 it reaches below the big y.
		AssertTrue(pSmall->GetAscent() + 10 > pBig->GetAscent());
		AssertTrue(pSmall->GetDescent() + 4 > pBig->GetDescent());
		iRowAscent = pSmall->GetAscent() + 10;
		iRowDescent = pSmall->GetDescent() + 4;

		//x, a superscript 2, y, a subscript 2.
		asChanges.Init();
		asChanges.Add()->Init(1, &pSmall, 10);
		asChanges.Add()->Init(2, &pBig);
		asChanges.Add()->Init(3, &pSmall, -4);
		cUTF8.Init("x2y2");
		cReader.Init(&pBig);
		pText = cReader.Read(&cUTF8, true, &asChanges);
		cReader.Kill();
		cUTF8.Kill();
		asChanges.Kill();
		AssertSize(4, pText->NumRuns());
		AssertInt(10, pText->GetRun(1)->GetBaselineShift());
		AssertInt(-4, pText->GetRun(3)->GetBaselineShift());

		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();
		pDraw->Begin();
		pLayout->Layout(pDraw, pText);
		pDraw->End();
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(4, pasGlyphs->NumElements());
		AssertInt((int32)iRowAscent - pBig->GetAscent(), pasGlyphs->Get(0)->sPosition.y);
		AssertInt((int32)0, pasGlyphs->Get(1)->sPosition.y);
		AssertInt((int32)iRowAscent - pBig->GetAscent(), pasGlyphs->Get(2)->sPosition.y);
		AssertInt((int32)iRowAscent - (pSmall->GetAscent() - 4), pasGlyphs->Get(3)->sPosition.y);
		sSize = pLayout->GetSize();
		AssertInt(iRowAscent + iRowDescent, sSize.y);
		AssertTrue(sSize.y > pBig->GetLineHeight());

		pDraw = NULL;
		pLayout = NULL;
		pText = NULL;
		pSmall = NULL;
		pBig = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out szText starting in pFont with the changes, then kills them.
//
//////////////////////////////////////////////////////////////////////////
void LayoutWithChanges(Ptr<CTextLayout> pLayout, Ptr<CGlyphListFontDraw> pDraw, Ptr<CFont> pFont, char* szText, CArrayTextFontChange* pasChanges)
{
	CSimpleTextReader	cReader;
	CUTF8				cUTF8;
	Ptr<CText>			pText;

	cUTF8.Init(szText);
	cReader.Init(&pFont);
	pText = cReader.Read(&cUTF8, true, pasChanges);
	cReader.Kill();
	cUTF8.Kill();
	pasChanges->ReInit();

	pDraw->Begin();
	pLayout->Layout(pDraw, pText);
	pDraw->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextRectangle(int32 x, int32 y, int32 iWidth, int32 iHeight, ARGB32 sColour, STextDrawRectangle* psRectangle)
{
	AssertInt(x, psRectangle->x);
	AssertInt(y, psRectangle->y);
	AssertInt(iWidth, psRectangle->iWidth);
	AssertInt(iHeight, psRectangle->iHeight);
	AssertInt(sColour, psRectangle->sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// Underlines, overlines and strikethroughs are drawn along each decorated
// stretch of a line, placed by the largest font in the stretch.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFontDecorationLayout(void)
{
	ObjectsInit();
	{
		Ptr<CFont>					pSmall;
		Ptr<CFont>					pBig;
		Ptr<CFontDescription>		pSmallDescription;
		Ptr<CFontDescription>		pBigDescription;
		CArrayTextFontChange		asChanges;
		Ptr<CTextLayout>			pLayout;
		Ptr<CGlyphListFontDraw>		pDraw;
		CArrayGlyphPosition*		pasGlyphs;
		CArrayTextDrawRectangle*	pasRectangles;
		ARGB32						sRed;
		ARGB32						sBlue;
		int32						iA;
		int32						iB;
		int32						iSpace;
		int32						iBaseline;
		int32						iBigA;

		pSmall = CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", "ab ", 16.0f);
		pBig = CreateWhiteTrueTypeFontFrom("LiberationSans-Regular.ttf", "ab ", 32.0f);
		pSmallDescription = pSmall->GetDescription();
		pBigDescription = pBig->GetDescription();
		iA = pSmall->GetGlyph((uint32)'a', 1)->GetFullWidth();
		iB = pSmall->GetGlyph((uint32)'b', 1)->GetFullWidth();
		iSpace = pSmall->GetGlyph((uint32)' ', 1)->GetFullWidth();
		iBigA = pBig->GetGlyph((uint32)'a', 1)->GetFullWidth();
		sRed = Set32BitColour((uint8)0xFF, (uint8)0x00, (uint8)0x00);
		sBlue = Set32BitColour((uint8)0x00, (uint8)0x00, (uint8)0xFF);
		asChanges.Init();
		pLayout = OMalloc<CTextLayout>();
		pDraw = OMalloc<CGlyphListFontDraw>();

		//All of "ab ab" underlined in red: one line, the underline's thickness, below the baseline.
		asChanges.Add()->Init(0, &pSmall, 0, TD_Underline, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab ab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(1, pasRectangles->NumElements());
		iBaseline = pSmall->GetAscent();
		AssertTextRectangle(0, iBaseline + pSmallDescription->GetUnderlineTop(), iA + iB + iSpace + iA + iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));

		//Wrapped after the space, the space at the end of the first line isn't underlined.
		pLayout->SetWidth(iA + iB + iSpace + iA);
		asChanges.Add()->Init(0, &pSmall, 0, TD_Underline, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab ab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(2, pasRectangles->NumElements());
		AssertTextRectangle(0, iBaseline + pSmallDescription->GetUnderlineTop(), iA + iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));
		AssertTextRectangle(0, pSmall->GetLineHeight() + iBaseline + pSmallDescription->GetUnderlineTop(), iA + iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(1));
		pLayout->SetWidth(-1);

		//"aab" with the middle a big: one line placed and as thick as the big font says.
		asChanges.Add()->Init(0, &pSmall, 0, TD_Underline, sRed);
		asChanges.Add()->Init(1, &pBig, 0, TD_Underline, sRed);
		asChanges.Add()->Init(2, &pSmall, 0, TD_Underline, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "aab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(1, pasRectangles->NumElements());
		iBaseline = pBig->GetAscent();
		AssertTrue(pBigDescription->GetUnderlineTop() != pSmallDescription->GetUnderlineTop());
		AssertTextRectangle(0, iBaseline + pBigDescription->GetUnderlineTop(), iA + iBigA + iB, pBigDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));

		//The underline changes colour with the text but stays straight.
		asChanges.Add()->Init(0, &pSmall, 0, TD_Underline, sRed);
		asChanges.Add()->Init(2, &pSmall, 0, TD_Underline, sBlue);
		LayoutWithChanges(pLayout, pDraw, pSmall, "abab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(2, pasRectangles->NumElements());
		iBaseline = pSmall->GetAscent();
		AssertTextRectangle(0, iBaseline + pSmallDescription->GetUnderlineTop(), iA + iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));
		AssertTextRectangle(iA + iB, iBaseline + pSmallDescription->GetUnderlineTop(), iA + iB, pSmallDescription->GetUnderlineThickness(), sBlue, pasRectangles->Get(1));

		//An overline along the top of the ascent and a strikethrough through the middle.
		asChanges.Add()->Init(0, &pSmall, 0, TD_Overline | TD_Strikethrough, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(2, pasRectangles->NumElements());
		AssertTextRectangle(0, 0, iA + iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));
		AssertTextRectangle(0, iBaseline + pSmallDescription->GetStrikeoutTop(), iA + iB, pSmallDescription->GetStrikeoutThickness(), sRed, pasRectangles->Get(1));
		AssertTrue(pSmallDescription->GetStrikeoutTop() < 0);

		//A raised b underlined on its own has its line raised with it.
		asChanges.Add()->Init(1, &pSmall, 8, TD_Underline, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		AssertSize(1, pasRectangles->NumElements());
		iBaseline = pSmall->GetAscent() + 8;
		AssertTextRectangle(iA, iBaseline - 8 + pSmallDescription->GetUnderlineTop(), iB, pSmallDescription->GetUnderlineThickness(), sRed, pasRectangles->Get(0));

		//A space that isn't underlined splits the underline.
		asChanges.Add()->Init(0, &pSmall, 0, TD_Underline, sRed);
		asChanges.Add()->Init(2, &pSmall);
		asChanges.Add()->Init(3, &pSmall, 0, TD_Underline, sRed);
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab ab", &asChanges);
		pasRectangles = pDraw->GetRectangles();
		pasGlyphs = pDraw->GetGlyphs();
		AssertSize(5, pasGlyphs->NumElements());
		AssertSize(2, pasRectangles->NumElements());
		AssertInt((int32)0, pasRectangles->Get(0)->x);
		AssertInt(iA + iB, pasRectangles->Get(0)->iWidth);
		AssertInt((int32)iA + iB + iSpace, pasRectangles->Get(1)->x);
		AssertInt(iA + iB, pasRectangles->Get(1)->iWidth);

		//Nothing decorated, nothing drawn.
		LayoutWithChanges(pLayout, pDraw, pSmall, "ab ab", &asChanges);
		AssertSize(0, pDraw->GetRectangles()->NumElements());

		asChanges.Kill();
		pDraw = NULL;
		pLayout = NULL;
		pBigDescription = NULL;
		pSmallDescription = NULL;
		pBig = NULL;
		pSmall = NULL;
	}
	ObjectsKill(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueTypeFont(void)
{
	BeginTests();

	DataIOInit();

	TestTrueTypeFontLoad();
	TestTrueTypeFontGlyphIndex();
	TestTrueTypeFontMetrics();
	TestTrueTypeFontOutline();
	TestTrueTypeFontAllGlyphs();
	TestTrueTypeRasteriseGlyph();
	TestTrueTypeRasteriseText();
	TestTrueTypeDrawText();
	TestTrueTypeFontFactoryLazyGlyphs();
	TestTrueTypeFontFactoryColours();
	TestTrueTypeFontDescriptionWidths();
	TestTrueTypeFontColours();
	TestTrueTypeFontColoursTextRuns();
	TestTrueTypeFontMixedSizeLayout();
	TestTrueTypeFontMixedTypefaceLayout();
	TestTrueTypeFontScriptAndLineMetrics();
	TestTrueTypeFontScriptLayout();
	TestTrueTypeFontDecorationLayout();

	DataIOKill();

	TestStatistics();
}
