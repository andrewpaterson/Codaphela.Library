#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontReader.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Label.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
// Inter Regular at 14 points, white, written by TestSupportLib.  The
// font's hundreds of glyphs make validating every object on each allocation
// too slow, so the tests turn validation off while they use it and
// validate once at the end.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> ReadLabelFont(void)
{
	gcObjects.DisableValidation();
	return FontReader("Input" _FS_ "TrueTypeFont" _FS_ "Inter-Regular-14pt.font");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ValidateLabelObjects(void)
{
	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLabelObjectsRemoved(void)
{
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLabelRequiredSize(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFont>				pFont;
		Ptr<CLabel>				pLabel;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sSize;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Label Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFont = ReadLabelFont();
		AssertTrue(pFont.IsNotNull());
		AssertInt(19, pFont->GetAscent());
		AssertInt(5, pFont->GetDescent());

		//The width is the glyphs laid out end to end, the height is the font's line.
		pLabel = OMalloc<CLabel>(pTestWindow, pFont, "Hello World");
		AssertString("Hello World", pLabel->GetText());
		sSize = pLabel->GetRequiredSize();
		AssertInt((int)pFont->CalculateWidth("Hello World"), sSize.x);
		AssertInt(24, sSize.y);
		sSize = pLabel->GetDesiredSize();
		AssertInt32Vec2((int)pFont->CalculateWidth("Hello World"), 24, &sSize);

		//A desired size overrides the required size only on the axes it is set on.
		pLabel->SetDesiredSize(100, -1);
		sSize = pLabel->GetDesiredSize();
		AssertInt32Vec2(100, 24, &sSize);
		pLabel->SetDesiredSize(-1, 30);
		sSize = pLabel->GetDesiredSize();
		AssertInt32Vec2((int)pFont->CalculateWidth("Hello World"), 30, &sSize);
		pLabel->SetDesiredSize(-1, -1);

		//A label is one line, line feeds are left out.
		pLabel->SetText("Hello\nWorld");
		sSize = pLabel->GetRequiredSize();
		AssertInt32Vec2((int)pFont->CalculateWidth("HelloWorld"), 24, &sSize);

		//Even with no text a label is a line high.
		pLabel->SetText("");
		sSize = pLabel->GetRequiredSize();
		AssertInt32Vec2(0, 24, &sSize);

		//UTF8 is laid out a code point at a time.
		pLabel->SetText("\xC3\x81\xC3\xA9");
		sSize = pLabel->GetRequiredSize();
		AssertInt32Vec2((int)pFont->CalculateWidth("\xC3\x81\xC3\xA9"), 24, &sSize);
		AssertTrue(sSize.x > 0);

		ValidateLabelObjects();

		pLabel = NULL;
		pFont = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
struct SLabelTestComponent
{
	char*						szText;
	EContainerStyleHorizontal	eHorizontal;
	EContainerStyleVertical		eVertical;
};


//////////////////////////////////////////////////////////////////////////
//
// Labels are only placed by their alignment in a container.  Each label is
// in the same fill container so each sits where its own alignment puts it.
// The labels are transparent so the stretched one doesn't hide the centred
// one drawn under it.
//
//////////////////////////////////////////////////////////////////////////
void TestLabelFillContainerAlignments(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "LabelFillContainerAlignments";
	SLabelTestComponent		asLabels[6] = {
		{ "Top Left",		CSH_Left,		CSV_Top },
		{ "Top Right",		CSH_Right,		CSV_Top },
		{ "Centre",			CSH_Center,		CSV_Center },
		{ "Stretched",		CSH_Stretch,	CSV_Center },
		{ "Bottom Left",	CSH_Left,		CSV_Bottom },
		{ "Bottom Right",	CSH_Right,		CSV_Bottom } };
	int32					iWidth;
	int32					iHeight;

	iWidth = 240;
	iHeight = 100;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFont>				pFont;
		Ptr<CFillContainer>		pFill;
		Ptr<CClearCanvasDraw>	pBackground;
		Ptr<CLabel>				apLabels[6];
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sRequired;
		SInt32Vec2				sVec;
		CChars					szExpectedDirectory;
		CPointer				pNull;
		size					i;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		//The labels are transparent, they are drawn over the window's background.
		pBackground = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)40, 60, 110));
		pTestWindow = OMalloc<CWindow>("Label Window", &cNativeFactory, pTick, pBackground, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pFont = ReadLabelFont();
		AssertTrue(pFont.IsNotNull());

		for (i = 0; i < 6; i++)
		{
			apLabels[i] = OMalloc<CLabel>(pTestWindow, pFont, asLabels[i].szText);
			apLabels[i]->SetAlignment(asLabels[i].eHorizontal, asLabels[i].eVertical);
			pFill->AddComponent(apLabels[i]);
		}

		pTestWindow->Show();

		for (i = 0; i < 6; i++)
		{
			sRequired = apLabels[i]->GetRequiredSize();
			sVec = apLabels[i]->GetActualSize();
			if (asLabels[i].eHorizontal == CSH_Stretch)
			{
				AssertInt32Vec2(iWidth, sRequired.y, &sVec);
			}
			else
			{
				AssertInt32Vec2(sRequired.x, sRequired.y, &sVec);
			}
		}

		sVec = apLabels[0]->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = apLabels[1]->GetPosition();
		AssertInt32Vec2(iWidth - apLabels[1]->GetRequiredSize().x, 0, &sVec);
		sVec = apLabels[2]->GetPosition();
		AssertInt32Vec2((iWidth - apLabels[2]->GetRequiredSize().x) / 2, (iHeight - 24) / 2, &sVec);
		sVec = apLabels[3]->GetPosition();
		AssertInt32Vec2(0, (iHeight - 24) / 2, &sVec);
		sVec = apLabels[4]->GetPosition();
		AssertInt32Vec2(0, iHeight - 24, &sVec);
		sVec = apLabels[5]->GetPosition();
		AssertInt32Vec2(iWidth - apLabels[5]->GetRequiredSize().x, iHeight - 24, &sVec);

		ValidateLabelObjects();

		for (i = 0; i < 6; i++)
		{
			apLabels[i] = NULL;
		}
		pFont = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pBackground = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// The glyphs are translucent so a label drawn again must start empty or
// the edges of the glyphs get brighter each frame.
//
//////////////////////////////////////////////////////////////////////////
void TestLabelRedraw(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "LabelRedraw";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFont>				pFont;
		Ptr<CFillContainer>		pFill;
		Ptr<CClearCanvasDraw>	pBackground;
		Ptr<CLabel>				pLabel;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CChars					szFrame0;
		CChars					szFrame;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 80, 24, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 3);
		//The labels are transparent, they are drawn over the window's background.
		pBackground = OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)40, 60, 110));
		pTestWindow = OMalloc<CWindow>("Label Window", &cNativeFactory, pTick, pBackground, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pFont = ReadLabelFont();
		pLabel = OMalloc<CLabel>(pTestWindow, pFont, "Redrawn");
		pFill->AddComponent(pLabel);

		pTestWindow->Show();

		ValidateLabelObjects();

		pLabel = NULL;
		pFont = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pBackground = NULL;

		szFrame0.Init(szDirectory);
		szFrame0.Append(_FS_ "Frame0.png");
		szFrame.Init(szDirectory);
		szFrame.Append(_FS_ "Frame2.png");
		AssertFile(szFrame0.Text(), szFrame.Text());
		szFrame.Kill();
		szFrame0.Kill();
	}

	AssertLabelObjectsRemoved();
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLabel(void)
{
	BeginTests();
	ObjectsInit();

	TestLabelRequiredSize();
	TestLabelFillContainerAlignments();
	TestLabelRedraw();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
