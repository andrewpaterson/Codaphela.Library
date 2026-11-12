#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/LineZones.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);


#define TEST_LINE_ZONES_LINES	8


Ptr<CFont> ReadLabelFont(void);
Ptr<CFontFamily> CreateMonoTextAreaFamily(float32 fDefaultPixelSize);


//The pages each line has: "Line 2 page 1" over "text 2.1" (from 1).
class CTestLineZoneSource : public CLineZoneSource
{
public:
	size	mauiPages[TEST_LINE_ZONES_LINES];

	void Init(void)
	{
		memset(mauiPages, 0, sizeof(mauiPages));
	}

	void GetLines(CArrayLineZoneLines* pauiDest) override
	{
		size	i;

		pauiDest->ReInit();
		for (i = 0; i < TEST_LINE_ZONES_LINES; i++)
		{
			if (mauiPages[i] > 0)
			{
				*pauiDest->Add() = i;
			}
		}
	}

	size NumPages(size uiLine) override
	{
		return (uiLine < TEST_LINE_ZONES_LINES) ? mauiPages[uiLine] : 0;
	}

	void GetTitle(size uiLine, size uiPage, CChars* pszDest) override
	{
		pszDest->Append("Line ");
		pszDest->Append((uint32)(uiLine + 1));
		pszDest->Append(" page ");
		pszDest->Append((uint32)(uiPage + 1));
	}

	void GetText(size uiLine, size uiPage, CChars* pszDest) override
	{
		pszDest->Append("text ");
		pszDest->Append((uint32)(uiLine + 1));
		pszDest->Append('.');
		pszDest->Append((uint32)(uiPage + 1));
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A press and release at the middle of pComponent.
//
//////////////////////////////////////////////////////////////////////////
void TestLineZonesClick(Ptr<CWindow> pWindow, Ptr<CComponent> pComponent)
{
	pWindow->Layout();
	pWindow->PointerMoved(pComponent->GetPosition().x + pComponent->GetActualSize().x / 2, pComponent->GetPosition().y + pComponent->GetActualSize().y / 2);
	pWindow->InputCommand(CIC_ActivateComponent, true);
	pWindow->InputCommand(CIC_ExecuteComponent, true);
	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
// A press in the gutter next to line uiLine (from 0).
//
//////////////////////////////////////////////////////////////////////////
void TestLineZonesClickGutter(Ptr<CWindow> pWindow, Ptr<CCodeEditor> pEditor, size uiLine)
{
	CInt32Rect	cLine;

	pWindow->Layout();
	AssertTrue(pEditor->GetGutter()->GetLineBounds(uiLine, &cLine));
	pWindow->PointerMoved(pEditor->GetPosition().x + 2, pEditor->GetGutter()->GetPosition().y + cLine.GetTop() + cLine.GetHeight() / 2);
	pWindow->InputCommand(CIC_ActivateComponent, true);
	pWindow->InputCommand(CIC_ExecuteComponent, true);
	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
// Lines with pages are marked; clicking a mark shows its line's pages
// under it, stepped through with < and > (round), and clicking again hides
// them.  Lines without pages have nothing to show.
//
//////////////////////////////////////////////////////////////////////////
void TestLineZonesShow(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CLineZones>				pZones;
		Ptr<CLineZone>				pZone;
		CTestLineZoneSource			cSource;
		int32						iLine;
		CInt32Rect					cGap;
		SInt32Vec2					sCaption;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 400, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Line Zones Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateTestTheme(Ptr<CFont>()), CreateMonoTextAreaFamily(15.0f), "a\nb\nc\nd\ne");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pZones = OMalloc<CLineZones>(pEditor, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()), ReadLabelFont(), CreateMonoTextAreaFamily(15.0f), JEWEL_PANEL, JEWEL_TEXT);
		pTestWindow->Layout();
		iLine = pEditor->GetTextArea()->GetFont()->GetLineHeight();

		//No source, nothing marked or shown.
		AssertString(" 1 \n 2 \n 3 \n 4 \n 5 ", pEditor->GetGutter()->GetText());
		AssertFalse(pZones->Toggle(1));

		cSource.Init();
		cSource.mauiPages[1] = 2;
		cSource.mauiPages[3] = 1;
		pZones->SetSource(&cSource);
		pTestWindow->Layout();
		AssertString(" 1 \n+2 \n 3 \n+4 \n 5 ", pEditor->GetGutter()->GetText());
		AssertFalse(pZones->Toggle(0));
		AssertSize(0, pZones->NumZones());

		//Clicking line 2's mark shows its first page under it.
		TestLineZonesClickGutter(pTestWindow, pEditor, 1);
		AssertSize(1, pZones->NumZones());
		pZone = pZones->FindZone(1);
		AssertTrue(pZone.IsNotNull());
		AssertString(" 1 \n-2 \n 3 \n+4 \n 5 ", pEditor->GetGutter()->GetText());
		AssertString("Line 2 page 1 - 1 of 2", pZone->GetTitle());
		AssertString("text 2.1", pZone->GetText());
		AssertTrue(pEditor->GetTextArea()->GetLineGapBounds(1, &cGap));
		AssertInt(pEditor->GetTextArea()->GetPosition().y + cGap.GetTop(), pZone->GetPosition().y);
		AssertTrue(cGap.GetHeight() >= iLine);

		//The buttons are small: four pixel borders, and four pixels either side of the caption.
		sCaption = pZone->GetNextButton()->GetCaptionLabel()->GetActualSize();
		AssertInt(sCaption.x + 16, pZone->GetNextButton()->GetActualSize().x);
		AssertInt(sCaption.y + 8, pZone->GetNextButton()->GetActualSize().y);
		AssertTrue(pZone->HasButtons());

		//> and < step through the pages, round.
		TestLineZonesClick(pTestWindow, pZone->GetNextButton());
		AssertSize(1, pZone->GetPage());
		AssertString("Line 2 page 2 - 2 of 2", pZone->GetTitle());
		AssertString("text 2.2", pZone->GetText());
		TestLineZonesClick(pTestWindow, pZone->GetNextButton());
		AssertSize(0, pZone->GetPage());
		TestLineZonesClick(pTestWindow, pZone->GetPreviousButton());
		AssertSize(1, pZone->GetPage());

		//A line with one page has no < and >, and stepping stays on it.
		TestLineZonesClickGutter(pTestWindow, pEditor, 3);
		AssertSize(2, pZones->NumZones());
		AssertString("Line 4 page 1 - 1 of 1", pZones->FindZone(3)->GetTitle());
		AssertFalse(pZones->FindZone(3)->HasButtons());
		AssertTrue(pZones->FindZone(3)->GetNextButton()->GetParent().IsNull());
		pZones->FindZone(3)->Step(1);
		AssertSize(0, pZones->FindZone(3)->GetPage());

		//Clicking the open mark hides the pages, and the line is marked as having them again.
		TestLineZonesClickGutter(pTestWindow, pEditor, 3);
		AssertSize(1, pZones->NumZones());
		AssertTrue(pZones->FindZone(3).IsNull());
		AssertString(" 1 \n-2 \n 3 \n+4 \n 5 ", pEditor->GetGutter()->GetText());

		//No source: no marks, no zones.
		pZones->SetSource(NULL);
		pTestWindow->Layout();
		AssertSize(0, pZones->NumZones());
		AssertString(" 1 \n 2 \n 3 \n 4 \n 5 ", pEditor->GetGutter()->GetText());

		pZone = NULL;
		pZones = NULL;
		pEditor = NULL;
		pFill = NULL;
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
// Editing the code puts the source out of date: the marks go, the zones
// are greyed and keep what they show, follow their lines, and don't step
// or open.  Setting a source again brings each zone up to date with its
// line's pages (keeping its page if there are as many) or closes it, and
// a zone whose line was deleted goes.
//
//////////////////////////////////////////////////////////////////////////
void TestLineZonesOutOfDate(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		Ptr<CLineZones>				pZones;
		Ptr<CLineZone>				pZone;
		CTestLineZoneSource			cSource;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 400, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Line Zones Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateTestTheme(Ptr<CFont>()), CreateMonoTextAreaFamily(15.0f), "a\nb\nc\nd\ne");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pTextArea = pEditor->GetTextArea();
		pZones = OMalloc<CLineZones>(pEditor, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()), ReadLabelFont(), CreateMonoTextAreaFamily(15.0f), JEWEL_PANEL, JEWEL_TEXT);
		pTestWindow->Layout();

		cSource.Init();
		cSource.mauiPages[1] = 2;
		cSource.mauiPages[3] = 1;
		pZones->SetSource(&cSource);
		AssertTrue(pZones->Toggle(1));
		AssertTrue(pZones->Toggle(3));
		pZones->FindZone(1)->Step(1);
		pTestWindow->Layout();
		AssertFalse(pZones->IsOutOfDate());

		//A line put in at the top: everything moves down a line, out of date.
		pTextArea->SetCaretPosition(0);
		pTextArea->GetEditing()->Enter();
		pTestWindow->Layout();
		AssertTrue(pZones->IsOutOfDate());
		AssertString(" 1 \n 2 \n 3 \n 4 \n 5 \n 6 ", pEditor->GetGutter()->GetText());
		pZone = pZones->FindZone(2);
		AssertTrue(pZone.IsNotNull());
		AssertTrue(pZone->IsOutOfDate());
		AssertString("Line 2 page 2 - 2 of 2  (out of date)", pZone->GetTitle());
		AssertString("text 2.2", pZone->GetText());
		AssertTrue(pZones->FindZone(4).IsNotNull());
		pZone->Step(1);
		AssertSize(1, pZone->GetPage());
		AssertFalse(pZones->Toggle(5));

		//Closing a zone works out of date; the line isn't marked.
		AssertTrue(pZones->Toggle(4));
		AssertSize(1, pZones->NumZones());
		pTestWindow->Layout();
		AssertString(" 1 \n 2 \n 3 \n 4 \n 5 \n 6 ", pEditor->GetGutter()->GetText());

		//The source made again for the code as it is now: line 3 has three pages, line 6 one.
		cSource.Init();
		cSource.mauiPages[2] = 3;
		cSource.mauiPages[5] = 1;
		pZones->SetSource(&cSource);
		pTestWindow->Layout();
		AssertFalse(pZones->IsOutOfDate());
		AssertFalse(pZone->IsOutOfDate());
		AssertString("Line 3 page 2 - 2 of 3", pZone->GetTitle());
		AssertString("text 3.2", pZone->GetText());
		AssertString(" 1 \n 2 \n-3 \n 4 \n 5 \n+6 ", pEditor->GetGutter()->GetText());

		//A source with fewer pages for its line: the zone shows its first.
		cSource.mauiPages[2] = 1;
		pZones->SetSource(&cSource);
		AssertString("Line 3 page 1 - 1 of 1", pZone->GetTitle());

		//A line that has no pages now closes its zone.
		AssertTrue(pZones->Toggle(5));
		cSource.mauiPages[5] = 0;
		pZones->SetSource(&cSource);
		AssertSize(1, pZones->NumZones());
		AssertTrue(pZones->FindZone(5).IsNull());

		//Deleting the zone's line takes the zone with it.
		pTextArea->SetCaretPosition(pTextArea->GetLineStart(2));
		pTextArea->GetEditing()->DeleteLine();
		pTestWindow->Layout();
		AssertTrue(pZones->IsOutOfDate());
		AssertSize(0, pEditor->NumLineComponents());
		cSource.Init();
		cSource.mauiPages[1] = 1;
		pZones->SetSource(&cSource);
		pTestWindow->Layout();
		AssertSize(0, pZones->NumZones());
		AssertString(" 1 \n+2 \n 3 \n 4 \n 5 ", pEditor->GetGutter()->GetText());

		pZone = NULL;
		pZones = NULL;
		pTextArea = NULL;
		pEditor = NULL;
		pFill = NULL;
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
// An edit reaches the zones as it is made, not when the code is laid out
// again: they are out of date at once, and a zone whose line is deleted
// is gone from them at once.
//
//////////////////////////////////////////////////////////////////////////
void TestLineZonesFollowEditsAtOnce(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CFillContainer>			pFill;
		Ptr<CCodeEditor>			pEditor;
		Ptr<CTextArea>				pTextArea;
		Ptr<CLineZones>				pZones;
		CTestLineZoneSource			cSource;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 400, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Line Zones Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pEditor = OMalloc<CCodeEditor>(pTestWindow, CreateTestTheme(Ptr<CFont>()), CreateMonoTextAreaFamily(15.0f), "a\nb\nc\nd\ne");
		pEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
		pFill->AddComponent(pEditor);
		pTextArea = pEditor->GetTextArea();
		pZones = OMalloc<CLineZones>(pEditor, CJewelTheme::CreateDarkTheme(pTestWindow, ReadLabelFont()), ReadLabelFont(), CreateMonoTextAreaFamily(15.0f), JEWEL_PANEL, JEWEL_TEXT);
		pTestWindow->Layout();

		cSource.Init();
		cSource.mauiPages[1] = 1;
		cSource.mauiPages[3] = 1;
		pZones->SetSource(&cSource);
		AssertTrue(pZones->Toggle(1));
		AssertTrue(pZones->Toggle(3));
		AssertSize(2, pZones->NumZones());

		pTextArea->SetCaretPosition(0);
		pTextArea->GetEditing()->Enter();
		AssertTrue(pZones->IsOutOfDate());

		//The zone that was under line 2 is under line 3 now.
		pTextArea->SetCaretPosition(pTextArea->GetLineStart(2));
		pTextArea->GetEditing()->DeleteLine();
		AssertSize(1, pZones->NumZones());
		AssertTrue(pZones->FindZone(2).IsNull());
		AssertTrue(pZones->FindZone(3).IsNotNull());
		AssertTrue(pZones->GetZone(0) == pZones->FindZone(3));

		pZones = NULL;
		pTextArea = NULL;
		pEditor = NULL;
		pFill = NULL;
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
//
//////////////////////////////////////////////////////////////////////////
void TestLineZones(void)
{
	BeginTests();
	ObjectsInit();

	TestLineZonesShow();
	TestLineZonesOutOfDate();
	TestLineZonesFollowEditsAtOnce();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

