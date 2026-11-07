#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "WindowLib/TextAreaLayout.h"
#include "TestLib/Assert.h"


Ptr<CFont> ReadLabelFont(void);


//////////////////////////////////////////////////////////////////////////
//
// A layout of pcDocument in the label font.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CTextAreaLayout> CreateRowsLayout(CTextDocument* pcDocument)
{
	return OMalloc<CTextAreaLayout>(pcDocument, OMalloc<CFontFamily>(OMalloc<CFontColours>(ReadLabelFont())));
}


//////////////////////////////////////////////////////////////////////////
//
// Lines are rows (without word wrap); positions are found in them and
// placed along them.  Nothing is laid out until it is read.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaRowsLines(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	bool					bRowEnd;
	int32					x;

	ObjectsInit();
	cDocument.Init("ab\ncd\n");
	pLayout = CreateRowsLayout(&cDocument);
	AssertFalse(pLayout->HasRows());

	pLayout->ReadText();
	pLayout->LayoutText();
	AssertTrue(pLayout->HasRows());
	AssertSize(3, pLayout->NumLayoutLines());
	AssertSize(3, pLayout->GetRowStart(1));
	AssertSize(6, pLayout->GetRowStart(2));
	AssertSize(1, pLayout->FindRow(4, false));
	AssertSize(2, pLayout->GetRowEnd(0, &bRowEnd));
	AssertFalse(bRowEnd);
	AssertFalse(pLayout->IsRowWrapped(0));

	AssertInt(0, pLayout->GetPositionX(3, false));
	x = pLayout->GetPositionX(5, false);
	AssertTrue(x > pLayout->GetPositionX(4, false));
	AssertSize(5, pLayout->FindPositionInRow(1, x, &bRowEnd));
	AssertSize(2, pLayout->FindRowAt(pLayout->GetLayoutLine(2)->iTop));
	AssertSize(0, pLayout->FindRowAt(-10));
	AssertInt(pLayout->GetLayoutLine(2)->iTop + pLayout->GetLayoutLine(2)->iHeight, pLayout->GetTextSize().y);

	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Setting spans or gaps only notes the text changed; it is laid out
// again once, by UpdateText.  A gap goes below its line, and a y in it is
// in the row above.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaRowsDirty(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	STextStyleOverride		sRed;
	int32					iTop;

	ObjectsInit();
	cDocument.Init("ab\ncd");
	pLayout = CreateRowsLayout(&cDocument);
	pLayout->ReadText();
	pLayout->LayoutText();
	AssertFalse(pLayout->IsTextDirty());
	AssertFalse(pLayout->UpdateText());

	//A colour doesn't change the text's size, so asking for the size doesn't lay it out.
	sRed.Init();
	sRed.SetColour(Set32BitColour(1.0f, 0, 0));
	AssertTrue(pLayout->AddSpan(0, 1, &sRed));
	AssertFalse(pLayout->AddSpan(4, 2, &sRed));
	AssertSize(1, pLayout->NumSpans());
	AssertTrue(pLayout->IsTextDirty());
	AssertFalse(pLayout->UpdateSize());
	AssertTrue(pLayout->UpdateText());
	AssertFalse(pLayout->UpdateText());

	iTop = pLayout->GetLayoutLine(1)->iTop;
	AssertTrue(pLayout->GetGaps()->Set(0, 10));
	pLayout->GapsChanged();
	AssertTrue(pLayout->UpdateSize());
	AssertInt(iTop + 10, pLayout->GetLayoutLine(1)->iTop);
	AssertSize(0, pLayout->FindRowAt(iTop + 5));

	pLayout->ClearSpans();
	AssertTrue(pLayout->IsTextDirty());
	AssertSize(0, pLayout->NumSpans());

	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// With word wrap, lines are broken to the width set: a row that ends
// where a word didn't fit is wrapped, and its end is the start of the
// next row.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaRowsWordWrap(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	bool					bRowEnd;

	ObjectsInit();
	cDocument.Init("aaa bbb ccc");
	pLayout = CreateRowsLayout(&cDocument);
	pLayout->ReadText();
	pLayout->LayoutText();
	AssertSize(1, pLayout->NumLayoutLines());

	AssertTrue(pLayout->SetWordWrap(true));
	AssertFalse(pLayout->SetWordWrap(true));
	AssertTrue(pLayout->IsWordWrap());
	AssertFalse(pLayout->IsWrapWidthChanged());
	pLayout->SetWidth(pLayout->GetPositionX(7, false) + 1);
	AssertTrue(pLayout->IsWrapWidthChanged());
	pLayout->LayoutText();
	AssertFalse(pLayout->IsWrapWidthChanged());

	AssertSize(2, pLayout->NumLayoutLines());
	AssertTrue(pLayout->IsRowWrapped(0));
	AssertSize(8, pLayout->GetRowStart(1));
	AssertSize(8, pLayout->GetRowEnd(0, &bRowEnd));
	AssertTrue(bRowEnd);
	AssertSize(0, pLayout->FindRow(8, true));
	AssertSize(1, pLayout->FindRow(8, false));

	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appended text is in its colour and laid out with the rest.  A text set
// has no spans or gaps.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaRowsAppendAndSet(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	STextStyle				sStyle;

	ObjectsInit();
	cDocument.Init("ab");
	pLayout = CreateRowsLayout(&cDocument);
	pLayout->ReadText();
	pLayout->LayoutText();

	pLayout->Append("\ncd", Set32BitColour(1.0f, 0, 0));
	AssertString("ab\ncd", cDocument.Text());
	AssertSize(2, pLayout->NumLayoutLines());
	AssertSize(1, pLayout->NumSpans());
	pLayout->GetStyleAt(4, &sStyle);
	AssertInt(Set32BitColour(1.0f, 0, 0), sStyle.sColour);
	AssertTrue(pLayout->GetGaps()->Set(1, 5));

	cDocument.SetText("x");
	pLayout->TextSet();
	AssertSize(0, pLayout->NumSpans());
	AssertSize(0, pLayout->GetGaps()->NumGaps());
	AssertSize(1, pLayout->NumLayoutLines());

	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaRows(void)
{
	BeginTests();

	TestTextAreaRowsLines();
	TestTextAreaRowsDirty();
	TestTextAreaRowsWordWrap();
	TestTextAreaRowsAppendAndSet();

	TestStatistics();
}

