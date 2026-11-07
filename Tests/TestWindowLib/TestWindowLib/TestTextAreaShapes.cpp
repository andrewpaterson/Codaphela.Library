#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "WindowLib/TextAreaLayout.h"
#include "WindowLib/TextAreaShapes.h"
#include "TestLib/Assert.h"


Ptr<CFont> ReadLabelFont(void);


//////////////////////////////////////////////////////////////////////////
//
// A layout of pcDocument in the label font, read and laid out.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CTextAreaLayout> CreateShapesLayout(CTextDocument* pcDocument)
{
	Ptr<CTextAreaLayout>	pLayout;

	pLayout = OMalloc<CTextAreaLayout>(pcDocument, OMalloc<CFontFamily>(OMalloc<CFontColours>(ReadLabelFont())));
	pLayout->ReadText();
	pLayout->LayoutText();
	return pLayout;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertShape(int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, SGeometryCommand* psCommand)
{
	AssertInt(iLeft, psCommand->cBounds.GetLeft());
	AssertInt(iTop, psCommand->cBounds.GetTop());
	AssertInt(iWidth, psCommand->cBounds.GetWidth());
	AssertInt(iHeight, psCommand->cBounds.GetHeight());
}


//////////////////////////////////////////////////////////////////////////
//
// A range is a block on each row it is on: to the end of a row it goes
// past and a new line's width further, from the start of a row it comes
// from.  A range of nothing is no block.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaShapesRange(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	Ptr<CGeometryMap>		pMap;
	CTextAreaShapes			cShapes;
	STextLayoutLine*		psFirst;
	STextLayoutLine*		psSecond;
	int32					iLeft;

	ObjectsInit();
	cDocument.Init("ab\ncd");
	pLayout = CreateShapesLayout(&cDocument);
	pMap = OMalloc<CGeometryMap>();
	cShapes.Init(&pLayout, &cDocument);

	cShapes.AddRange(pMap, 1, 4, Set32BitColour(1.0f, 0, 0));
	AssertSize(2, pMap->NumCommands());
	psFirst = pLayout->GetLayoutLine(0);
	psSecond = pLayout->GetLayoutLine(1);
	iLeft = pLayout->GetPositionX(1, false);
	AssertShape(iLeft, psFirst->iTop, pLayout->GetRowRight(0) + pLayout->GetNewLineWidth() - iLeft, psFirst->iHeight, pMap->GetCommand(0));
	AssertShape(0, psSecond->iTop, pLayout->GetPositionX(4, true), psSecond->iHeight, pMap->GetCommand(1));

	cShapes.AddRange(pMap, 2, 2, Set32BitColour(1.0f, 0, 0));
	AssertSize(2, pMap->NumCommands());

	cShapes.Kill();
	pMap = NULL;
	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// Lines are a block for each run of them without a gap between, across
// the width given; a boundary is centred on its line's top, kept inside
// the text, past the last line at the bottom; a range is the range's
// blocks.  Each is over the ones before it, in the order added.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaShapesBackgrounds(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	Ptr<CGeometryMap>		pMap;
	CTextAreaShapes			cShapes;
	CTextBackgrounds		cBackgrounds;
	int32					aiTops[3];
	int32					aiBottoms[3];
	size					i;

	ObjectsInit();
	cDocument.Init("ab\ncd\nef");
	pLayout = OMalloc<CTextAreaLayout>(&cDocument, OMalloc<CFontFamily>(OMalloc<CFontColours>(ReadLabelFont())));
	AssertTrue(pLayout->GetGaps()->Set(0, 10));
	pLayout->ReadText();
	pLayout->LayoutText();
	for (i = 0; i < 3; i++)
	{
		AssertTrue(pLayout->GetLineRows(i, &aiTops[i], &aiBottoms[i]));
	}
	AssertTrue(aiTops[1] >= aiBottoms[0] + 10);

	pMap = OMalloc<CGeometryMap>();
	cShapes.Init(&pLayout, &cDocument);
	cBackgrounds.Init();
	cBackgrounds.Add(TAB_Lines, 0, 3, 0, Set32BitColour(1.0f, 0, 0));
	cBackgrounds.Add(TAB_Boundary, 1, 1, 4, Set32BitColour(0, 1.0f, 0));
	cBackgrounds.Add(TAB_Boundary, 3, 3, 4, Set32BitColour(0, 1.0f, 0));
	cBackgrounds.Add(TAB_Range, 0, 1, 0, Set32BitColour(0, 0, 1.0f));
	cShapes.AddBackgrounds(pMap, &cBackgrounds, 100);

	AssertSize(5, pMap->NumCommands());
	AssertShape(0, aiTops[0], 100, aiBottoms[0] - aiTops[0], pMap->GetCommand(0));
	AssertShape(0, aiTops[1], 100, aiBottoms[2] - aiTops[1], pMap->GetCommand(1));
	AssertShape(0, aiTops[1] - 2, 100, 4, pMap->GetCommand(2));
	AssertShape(0, pLayout->GetTextSize().y - 4, 100, 4, pMap->GetCommand(3));
	AssertShape(0, aiTops[0], pLayout->GetPositionX(1, true), aiBottoms[0] - aiTops[0], pMap->GetCommand(4));

	cBackgrounds.Kill();
	cShapes.Kill();
	pMap = NULL;
	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaShapes(void)
{
	BeginTests();

	TestTextAreaShapesRange();
	TestTextAreaShapesBackgrounds();

	TestStatistics();
}

