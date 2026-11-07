#include "BaseLib/GlobalMemory.h"
#include "WindowLib/TextLineGaps.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The gaps as "line:height:id" each, for comparing.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextLineGapsPrint(CTextLineGaps* pcGaps, CChars* pszDest)
{
	size			i;
	STextLayoutGap*	psGap;

	pszDest->Clear();
	for (i = 0; i < pcGaps->NumGaps(); i++)
	{
		psGap = pcGaps->GetGaps()->Get(i);
		if (i > 0)
		{
			pszDest->Append(' ');
		}
		pszDest->Append((uint32)psGap->uiLine);
		pszDest->Append(':');
		pszDest->Append(psGap->iHeight);
		pszDest->Append(':');
		pszDest->Append(psGap->uiId);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gaps are kept sorted by line, each with a new id that it keeps through
// changes of height.  Setting what is there, or removing what isn't,
// changes nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestTextLineGapsSet(void)
{
	CTextLineGaps	cGaps;
	CChars			sz;

	sz.Init();
	cGaps.Init();
	AssertTrue(cGaps.Set(5, 10));
	AssertTrue(cGaps.Set(2, 20));
	AssertTrue(cGaps.Set(8, 30));
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("2:20:2 5:10:1 8:30:3", sz.Text());

	AssertFalse(cGaps.Set(5, 10));
	AssertTrue(cGaps.Set(5, 15));
	AssertInt(15, cGaps.GetHeight(5));
	AssertInt(1, cGaps.GetId(5));
	AssertFalse(cGaps.Set(4, 0));
	AssertTrue(cGaps.Set(2, 0));
	AssertInt(0, cGaps.GetHeight(2));
	AssertInt(0, cGaps.GetId(2));
	AssertSize(2, cGaps.NumGaps());

	AssertSize(8, cGaps.FindLine(3));
	AssertSize(MAX_SIZE, cGaps.FindLine(2));

	AssertTrue(cGaps.Clear());
	AssertFalse(cGaps.Clear());
	AssertTrue(cGaps.Set(1, 5));
	AssertInt(4, cGaps.GetId(1));
	cGaps.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy leaves out gaps past the last line or of no height, says nothing
// changed when the lines and heights are the same, and new gaps' ids come
// after the copied ones.  The gaps from a line on are counted from it.
//
//////////////////////////////////////////////////////////////////////////
void TestTextLineGapsCopy(void)
{
	CTextLineGaps		cGaps;
	CTextLineGaps		cOther;
	CArrayTextLayoutGap	asFrom;
	CChars				sz;

	sz.Init();
	cOther.Init();
	cOther.Set(1, 10);
	cOther.Set(3, 20);
	cOther.Set(9, 30);
	cOther.Set(3, 25);

	cGaps.Init();
	AssertTrue(cGaps.Copy(cOther.GetGaps(), 5));
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("1:10:1 3:25:2", sz.Text());
	AssertFalse(cGaps.Copy(cOther.GetGaps(), 5));
	AssertTrue(cGaps.Copy(cOther.GetGaps(), 10));
	cGaps.Set(4, 40);
	AssertInt(4, cGaps.GetId(4));

	asFrom.Init();
	cGaps.GetGapsFrom(3, &asFrom);
	AssertSize(3, asFrom.NumElements());
	AssertSize(0, asFrom.Get(0)->uiLine);
	AssertSize(1, asFrom.Get(1)->uiLine);
	AssertSize(6, asFrom.Get(2)->uiLine);
	AssertInt(30, asFrom.Get(2)->iHeight);
	asFrom.Kill();

	cGaps.Kill();
	cOther.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Line feeds put in move the gaps of the lines after down, and the gap of
// the line they went in at the start of.  Lines removed take their gaps
// with them, but a line removed whole is replaced by the line after it,
// gap and all.
//
//////////////////////////////////////////////////////////////////////////
void TestTextLineGapsEdits(void)
{
	CTextLineGaps	cGaps;
	CChars			sz;

	sz.Init();
	cGaps.Init();
	cGaps.Set(1, 10);
	cGaps.Set(3, 30);
	cGaps.Set(5, 50);

	//Two line feeds inside line 3: its gap stays, line 5's moves down.
	cGaps.LinesInserted(3, false, 2);
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("1:10:1 3:30:2 7:50:3", sz.Text());

	//One at the start of line 3: it is pushed down with its gap.
	cGaps.LinesInserted(3, true, 1);
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("1:10:1 4:30:2 8:50:3", sz.Text());

	//From inside line 1 to inside line 4: lines 2 to 4 join line 1, which keeps its gap, and line 4's goes.
	cGaps.LinesRemoved(1, 4, false);
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("1:10:1 5:50:3", sz.Text());

	//Lines 1 to 4 removed whole: line 5 takes line 1's place with its own gap.
	cGaps.LinesRemoved(1, 5, true);
	TestTextLineGapsPrint(&cGaps, &sz);
	AssertString("1:50:3", sz.Text());
	cGaps.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextLineGaps(void)
{
	BeginTests();

	TestTextLineGapsSet();
	TestTextLineGapsCopy();
	TestTextLineGapsEdits();

	TestStatistics();
}

