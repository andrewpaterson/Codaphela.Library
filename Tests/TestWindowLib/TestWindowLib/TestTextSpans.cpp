#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextDocument.h"
#include "WindowLib/TextSpans.h"
#include "WindowLib/TextBackgrounds.h"
#include "WindowLib/TextLineGaps.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The spans as "start+length" each, for comparing.
//
//////////////////////////////////////////////////////////////////////////
static char* TestTextSpansPrint(CTextSpans* pcSpans, CChars* pszDest)
{
	size			i;
	STextAreaSpan*	psSpan;

	pszDest->Clear();
	for (i = 0; i < pcSpans->NumSpans(); i++)
	{
		psSpan = pcSpans->Get(i);
		if (i > 0)
		{
			pszDest->Append(' ');
		}
		pszDest->Append((uint32)psSpan->uiStart);
		pszDest->Append('+');
		pszDest->Append((uint32)psSpan->uiLength);
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Spans follow the edits of the document they listen to: text put in a
// span, or at its end, stretches it; text put before it, even at its
// start, moves it; text taken out shrinks it and a span left empty goes.
//
//////////////////////////////////////////////////////////////////////////
void TestTextSpansEdits(void)
{
	CTextDocument		cDocument;
	CTextSpans			cSpans;
	STextStyleOverride	sRed;
	STextStyleOverride	sBold;
	CChars				sz;

	sz.Init();
	sRed.Init();
	sRed.SetColour(Set32BitColour(1.0f, 0, 0));
	sBold.Init();
	sBold.SetBold(true);

	cDocument.Init("hello world");
	cSpans.Init();
	cDocument.AddListener(&cSpans);
	cSpans.Add(0, 5, &sRed);
	cSpans.Add(6, 5, &sBold);
	AssertString("0+5 6+5", TestTextSpansPrint(&cSpans, &sz));

	cDocument.Insert(5, "XX");
	AssertString("0+7 8+5", TestTextSpansPrint(&cSpans, &sz));
	cDocument.Insert(0, "Y");
	AssertString("1+7 9+5", TestTextSpansPrint(&cSpans, &sz));
	cDocument.Insert(3, "Z");
	AssertString("1+8 10+5", TestTextSpansPrint(&cSpans, &sz));

	cDocument.Remove(2, 6);
	AssertString("1+4 6+5", TestTextSpansPrint(&cSpans, &sz));
	cDocument.Remove(4, 7);
	AssertString("1+3 4+4", TestTextSpansPrint(&cSpans, &sz));
	cDocument.Remove(4, 8);
	AssertString("1+3", TestTextSpansPrint(&cSpans, &sz));
	AssertString("YhoX", cDocument.Text());

	cSpans.Clear();
	AssertSize(0, cSpans.NumSpans());

	cDocument.Kill();
	cSpans.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Spans are applied in the order they were added, so where they overlap
// the last added wins.  Whether any changes the font or decorates.
//
//////////////////////////////////////////////////////////////////////////
void TestTextSpansApply(void)
{
	CTextSpans			cSpans;
	STextStyleOverride	sRed;
	STextStyleOverride	sGreen;
	STextStyleOverride	sUnderline;
	STextStyleOverride	sBold;
	STextStyle			sStyle;
	STextStyle			asStyles[6];
	ARGB32				sWhite;
	ARGB32				sRedColour;
	ARGB32				sGreenColour;
	size				i;

	sWhite = Set32BitColour(1.0f, 1.0f, 1.0f);
	sRedColour = Set32BitColour(1.0f, 0, 0);
	sGreenColour = Set32BitColour(0, 1.0f, 0);
	sRed.Init();
	sRed.SetColour(sRedColour);
	sGreen.Init();
	sGreen.SetColour(sGreenColour);
	sUnderline.Init();
	sUnderline.SetUnderline(true);
	sBold.Init();
	sBold.SetBold(true);

	cSpans.Init();
	cSpans.Add(0, 4, &sRed);
	cSpans.Add(2, 4, &sGreen);
	AssertFalse(cSpans.ChangesFont());

	sStyle.Init();
	sStyle.sColour = sWhite;
	AssertFalse(cSpans.HasDecorations(&sStyle));
	cSpans.ApplyAt(1, &sStyle);
	AssertInt(sRedColour, sStyle.sColour);
	sStyle.sColour = sWhite;
	cSpans.ApplyAt(3, &sStyle);
	AssertInt(sGreenColour, sStyle.sColour);
	sStyle.sColour = sWhite;
	cSpans.ApplyAt(6, &sStyle);
	AssertInt(sWhite, sStyle.sColour);

	//The styles of bytes 1 to 7, from byte 1.
	for (i = 0; i < 6; i++)
	{
		asStyles[i] = sStyle;
	}
	cSpans.Apply(asStyles, 1, 7);
	AssertInt(sRedColour, asStyles[0].sColour);
	AssertInt(sGreenColour, asStyles[1].sColour);
	AssertInt(sGreenColour, asStyles[4].sColour);
	AssertInt(sWhite, asStyles[5].sColour);

	cSpans.Add(0, 1, &sUnderline);
	AssertTrue(cSpans.HasDecorations(&sStyle));
	cSpans.Add(0, 1, &sBold);
	AssertTrue(cSpans.ChangesFont());

	cSpans.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A text area's spans, backgrounds and gaps all listen to its document,
// so each follows the same edits.
//
//////////////////////////////////////////////////////////////////////////
void TestTextSpansWithOtherListeners(void)
{
	CTextDocument		cDocument;
	CTextSpans			cSpans;
	CTextBackgrounds	cBackgrounds;
	CTextLineGaps		cGaps;
	STextStyleOverride	sRed;
	CChars				sz;

	sz.Init();
	sRed.Init();
	sRed.SetColour(Set32BitColour(1.0f, 0, 0));
	cDocument.Init("ab\ncd\nef");
	cSpans.Init();
	cBackgrounds.Init();
	cGaps.Init();
	cDocument.AddListener(&cSpans);
	cDocument.AddListener(&cBackgrounds);
	cDocument.AddListener(&cGaps);

	cSpans.Add(3, 2, &sRed);
	cBackgrounds.Add(TAB_Range, 3, 5, 0, sRed.sStyle.sColour);
	cBackgrounds.Add(TAB_Lines, 1, 2, 0, sRed.sStyle.sColour);
	cGaps.Set(1, 10);

	//Line 1 pushed down from its start.
	cDocument.Insert(3, "xy\n");
	AssertString("ab\nxy\ncd\nef", cDocument.Text());
	AssertString("6+2", TestTextSpansPrint(&cSpans, &sz));
	AssertSize(6, cBackgrounds.Get(0)->uiStart);
	AssertSize(8, cBackgrounds.Get(0)->uiEnd);
	AssertSize(2, cBackgrounds.Get(1)->uiStart);
	AssertSize(3, cBackgrounds.Get(1)->uiEnd);
	AssertInt(0, cGaps.GetHeight(1));
	AssertInt(10, cGaps.GetHeight(2));

	//The lines put in taken out again.
	cDocument.Remove(3, 6);
	AssertString("ab\ncd\nef", cDocument.Text());
	AssertString("3+2", TestTextSpansPrint(&cSpans, &sz));
	AssertSize(3, cBackgrounds.Get(0)->uiStart);
	AssertSize(1, cBackgrounds.Get(1)->uiStart);
	AssertInt(10, cGaps.GetHeight(1));

	cDocument.Kill();
	cGaps.Kill();
	cBackgrounds.Kill();
	cSpans.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextSpans(void)
{
	BeginTests();

	TestTextSpansEdits();
	TestTextSpansApply();
	TestTextSpansWithOtherListeners();

	TestStatistics();
}

