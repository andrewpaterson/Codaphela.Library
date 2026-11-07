#include "BaseLib/GlobalMemory.h"
#include "WindowLib/TextBackgrounds.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The backgrounds as "type:start-end" each, for comparing.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextBackgroundsPrint(CTextBackgrounds* pcBackgrounds, CChars* pszDest)
{
	size					i;
	STextAreaBackground*	psBackground;
	static const char*		aszTypes[] = { "R", "L", "B" };

	pszDest->Clear();
	for (i = 0; i < pcBackgrounds->NumBackgrounds(); i++)
	{
		psBackground = pcBackgrounds->Get(i);
		if (i > 0)
		{
			pszDest->Append(' ');
		}
		pszDest->Append(aszTypes[psBackground->eType]);
		pszDest->Append(':');
		pszDest->Append((uint32)psBackground->uiStart);
		pszDest->Append('-');
		pszDest->Append((uint32)psBackground->uiEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges move with text put before them, grow with text put strictly
// inside, shrink by what is removed from them and go when it is all of
// them.  Lines and boundaries aren't moved by text that has no line
// feeds, and changes say whether anything moved.
//
//////////////////////////////////////////////////////////////////////////
void TestTextBackgroundsText(void)
{
	CTextBackgrounds	cBackgrounds;
	CChars				sz;

	sz.Init();
	cBackgrounds.Init();
	AssertFalse(cBackgrounds.TextInserted(0, 5));
	cBackgrounds.Add(TAB_Range, 10, 20, 0, 0xFF0000FF);
	cBackgrounds.Add(TAB_Range, 30, 35, 0, 0xFF00FF00);
	cBackgrounds.Add(TAB_Lines, 1, 3, 0, 0xFFFF0000);
	AssertNull(cBackgrounds.Get(3));

	//Before the first, inside it, at its end (not inside), after both.
	AssertTrue(cBackgrounds.TextInserted(5, 2));
	AssertTrue(cBackgrounds.TextInserted(15, 3));
	AssertTrue(cBackgrounds.TextInserted(25, 1));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("R:12-25 R:36-41 L:1-3", sz.Text());
	AssertFalse(cBackgrounds.TextInserted(45, 1));

	//Part of the first, then all of the second.
	AssertTrue(cBackgrounds.TextRemoved(20, 30));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("R:12-20 R:26-31 L:1-3", sz.Text());
	AssertTrue(cBackgrounds.TextRemoved(22, 35));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("R:12-20 L:1-3", sz.Text());

	AssertTrue(cBackgrounds.Clear());
	AssertFalse(cBackgrounds.Clear());
	AssertFalse(cBackgrounds.TextRemoved(0, 1));
	cBackgrounds.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Line backgrounds and boundaries move with their lines; a line
// background grows with lines put in inside it and goes when its lines
// do.  A boundary stays one line.  Ranges aren't moved by lines.
//
//////////////////////////////////////////////////////////////////////////
void TestTextBackgroundsLines(void)
{
	CTextBackgrounds	cBackgrounds;
	CChars				sz;

	sz.Init();
	cBackgrounds.Init();
	AssertFalse(cBackgrounds.LinesInserted(0, true, 1));
	cBackgrounds.Add(TAB_Lines, 2, 4, 0, 0xFFFF0000);
	cBackgrounds.Add(TAB_Boundary, 5, 5, 2, 0xFF00FF00);
	cBackgrounds.Add(TAB_Range, 3, 6, 0, 0xFF0000FF);

	//Inside line 2: the background grows, the boundary moves down.
	AssertTrue(cBackgrounds.LinesInserted(2, false, 2));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("L:2-6 B:7-7 R:3-6", sz.Text());

	//At the start of line 2: the background moves down whole.
	AssertTrue(cBackgrounds.LinesInserted(2, true, 1));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("L:3-7 B:8-8 R:3-6", sz.Text());

	//Lines 4 to 8 joined onto line 3: the background keeps line 3, the boundary goes onto the line after it.
	AssertTrue(cBackgrounds.LinesRemoved(3, 8, false));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("L:3-4 B:4-4 R:3-6", sz.Text());

	//Lines 3 up to 4 removed whole: the background goes with them.
	AssertTrue(cBackgrounds.LinesRemoved(3, 4, true));
	TestTextBackgroundsPrint(&cBackgrounds, &sz);
	AssertString("B:3-3 R:3-6", sz.Text());
	cBackgrounds.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextBackgrounds(void)
{
	BeginTests();

	TestTextBackgroundsText();
	TestTextBackgroundsLines();

	TestStatistics();
}

