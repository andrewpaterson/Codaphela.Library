#include "BaseLib/GlobalMemory.h"
#include "BaseLib/MemoryFile.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontColours.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTheme.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);


//Counts a theme's change events.
class CTestWindowThemeListener
{
public:
	int		miChanges;

	void Changed(CWindowTheme* pcTheme) { miChanges++; }
};


//////////////////////////////////////////////////////////////////////////
//
// What fCall logs while the logger is captured, in szDest.
//
//////////////////////////////////////////////////////////////////////////
static void CaptureThemeLog(Ptr<CWindowTheme> pTheme, void (*fCall)(Ptr<CWindowTheme>), char* szDest, size uiSize)
{
	CMemoryFile		cMemory;
	CFileBasic		cBasicMemory;
	SLogConfig		sLogConfig;

	memset(szDest, 0, uiSize);
	sLogConfig = gcLogger.SetSilent();
	cMemory.Init();
	gcLogger.AddOutput(&cMemory);
	fCall(pTheme);
	gcLogger.RemoveOutput(&cMemory);
	gcLogger.SetConfig(&sLogConfig);

	cMemory.Open(EFM_Read);
	cBasicMemory.Init(&cMemory);
	if (cBasicMemory.Size() > 0)
	{
		cBasicMemory.ReadStringChars(szDest, (int)((cBasicMemory.Size() < (filePos)uiSize - 1) ? cBasicMemory.Size() : (filePos)uiSize - 1));
	}
	cMemory.Close();
	cMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReadUnsetThemeValues(Ptr<CWindowTheme> pTheme)
{
	AssertInt(0, pTheme->GetColour(TC_Text));
	AssertInt(0, pTheme->GetMetric(TM_ButtonPaddingX));
	AssertTrue(pTheme->GetImage(TI_Chevron).IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReadSetThemeValues(Ptr<CWindowTheme> pTheme)
{
	AssertInt(0x12345678, pTheme->GetColour(TC_Text));
	AssertInt(7, pTheme->GetMetric(TM_ButtonPaddingX));
	AssertTrue(pTheme->GetImage(TI_Chevron).IsNotNull());
}


//////////////////////////////////////////////////////////////////////////
//
// A theme has no looks of its own: reading a colour, size or icon that
// wasn't set returns nothing, logs an error naming it and is counted.
// Once set, it reads back quietly.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowThemeUnset(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CWindowTheme>			pJewel;
		size						uiUnsetReads;
		char						szLog[1024];

		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Theme Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
		AssertFalse(pTheme->IsColourSet(TC_Text));
		AssertFalse(pTheme->IsMetricSet(TM_ButtonPaddingX));
		AssertFalse(pTheme->IsImageSet(TI_Chevron));

		uiUnsetReads = CWindowTheme::GetTotalUnsetReads();
		CaptureThemeLog(pTheme, ReadUnsetThemeValues, szLog, 1024);
		AssertSize(uiUnsetReads + 3, CWindowTheme::GetTotalUnsetReads());
		AssertTrue(strstr(szLog, "Theme colour [Text] isn't set.") != NULL);
		AssertTrue(strstr(szLog, "Theme metric [ButtonPaddingX] isn't set.") != NULL);
		AssertTrue(strstr(szLog, "Theme image [Chevron] isn't set.") != NULL);

		pJewel = CJewelTheme::CreateDarkTheme(pTestWindow, Ptr<CFont>());
		pTheme->SetColour(TC_Text, 0x12345678);
		pTheme->SetMetric(TM_ButtonPaddingX, 7);
		pTheme->SetImage(TI_Chevron, pJewel->GetImage(TI_Chevron));
		AssertTrue(pTheme->IsColourSet(TC_Text));
		AssertTrue(pTheme->IsMetricSet(TM_ButtonPaddingX));
		AssertTrue(pTheme->IsImageSet(TI_Chevron));
		AssertFalse(pTheme->IsColourSet(TC_Caret));

		uiUnsetReads = CWindowTheme::GetTotalUnsetReads();
		CaptureThemeLog(pTheme, ReadSetThemeValues, szLog, 1024);
		AssertSize(uiUnsetReads, CWindowTheme::GetTotalUnsetReads());
		AssertString("", szLog);

		pJewel = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Changed tells the theme's users; setting values alone doesn't.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowThemeChanged(void)
{
	Ptr<CWindowTheme>			pTheme;
	CTestWindowThemeListener	cListener;

	cListener.miChanges = 0;
	pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
	pTheme->GetChangeEvent()->Add(&cListener, &CTestWindowThemeListener::Changed);
	pTheme->SetColour(TC_Text, 0x12345678);
	pTheme->SetMetric(TM_TabGap, 3);
	AssertInt(0, cListener.miChanges);
	pTheme->Changed();
	AssertInt(1, cListener.miChanges);
	pTheme->Changed();
	AssertInt(2, cListener.miChanges);
	AssertTrue(pTheme->GetChangeEvent()->Remove(&cListener));
	pTheme->Changed();
	AssertInt(2, cListener.miChanges);
	pTheme = NULL;

	AssertSize(0, gcObjects.NumFreeObjects());
}


//////////////////////////////////////////////////////////////////////////
//
// Jewel's theme sets every colour, size and icon there is, each with a
// name.  A table row left out when one is added would be zero: no colour
// is transparent black and no size is zero.  Made from white font
// colours, its font is in its own text colour.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowThemeJewelComplete(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFontColours>			pColours;
		size						uiUnsetReads;
		size						i;

		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Jewel Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		uiUnsetReads = CWindowTheme::GetTotalUnsetReads();
		pColours = OMalloc<CFontColours>(ReadLabelFont());
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, pColours);
		for (i = 0; i < TC_NumColours; i++)
		{
			AssertTrue(pTheme->IsColourSet((EThemeColour)i));
			AssertTrue(pTheme->GetColour((EThemeColour)i) != 0);
			AssertTrue(!StrEmpty(CWindowTheme::GetColourName((EThemeColour)i)));
		}
		for (i = 0; i < TM_NumMetrics; i++)
		{
			AssertTrue(pTheme->IsMetricSet((EThemeMetric)i));
			AssertTrue(pTheme->GetMetric((EThemeMetric)i) > 0);
			AssertTrue(!StrEmpty(CWindowTheme::GetMetricName((EThemeMetric)i)));
		}
		for (i = 0; i < TI_NumImages; i++)
		{
			AssertTrue(pTheme->IsImageSet((EThemeImage)i));
			AssertTrue(!StrEmpty(CWindowTheme::GetImageName((EThemeImage)i)));
		}
		AssertTrue(pTheme->GetMenuStyle().IsNotNull());
		AssertTrue(pTheme->GetComboMenuStyle().IsNotNull());
		AssertTrue(pTheme->GetMenuBarButtonImages().IsNotNull());
		AssertTrue(pTheme->GetFont() == pColours->GetFont(pTheme->GetColour(TC_Text)));
		AssertTrue(pTheme->GetMenuStyle()->GetFont() == pTheme->GetFont());
		AssertSize(uiUnsetReads, CWindowTheme::GetTotalUnsetReads());

		pColours = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	gcObjects.ValidateObjectsConsistency();
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWindowTheme(void)
{
	BeginTests();
	ObjectsInit();

	TestWindowThemeUnset();
	TestWindowThemeChanged();
	TestWindowThemeJewelComplete();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
// No widget anywhere in the tests read a colour, size or icon its theme
// wasn't given (see CWindowTheme::GetTotalUnsetReads).
//
//////////////////////////////////////////////////////////////////////////
void TestNoUnsetThemeReads(size uiUnsetReadsBefore)
{
	BeginTests();

	AssertSize(uiUnsetReadsBefore, CWindowTheme::GetTotalUnsetReads());

	TestStatistics();
}
