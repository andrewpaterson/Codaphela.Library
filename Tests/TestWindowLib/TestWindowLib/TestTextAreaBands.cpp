#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "WindowLib/TextAreaLayout.h"
#include "WindowLib/TextAreaBands.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Bands at aiTops (the last is the bottom), each with its character of
// szContent mixed in.
//
//////////////////////////////////////////////////////////////////////////
static void InitTestBands(CTextAreaBands* pcBands, int32* aiTops, size uiNumBands, const char* szContent)
{
	size	i;

	pcBands->Init();
	for (i = 0; i < uiNumBands; i++)
	{
		pcBands->Add(aiTops[i], aiTops[i + 1]);
		pcBands->Mix(i, &szContent[i], 1);
	}
	pcBands->Complete();
}


//////////////////////////////////////////////////////////////////////////
//
// The same bands with the same things in them haven't changed; a band
// with something else in it has; bands that moved have, where they were
// and where they are, joined where they touch.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBandsCompare(void)
{
	CTextAreaBands	cBefore;
	CTextAreaBands	cNow;
	CArrayInt32Vec2	asChanged;
	int32			aiTops[] = { 0, 10, 20, 30 };
	int32			aiMoved[] = { 0, 10, 25, 35 };
	int32			aiShorter[] = { 0, 10, 20 };

	asChanged.Init();
	InitTestBands(&cBefore, aiTops, 3, "abc");

	InitTestBands(&cNow, aiTops, 3, "abc");
	cNow.Compare(&cBefore, &asChanged);
	AssertSize(0, asChanged.NumElements());
	cNow.Kill();

	InitTestBands(&cNow, aiTops, 3, "aXc");
	cNow.Compare(&cBefore, &asChanged);
	AssertSize(1, asChanged.NumElements());
	AssertInt(10, asChanged.Get(0)->x);
	AssertInt(20, asChanged.Get(0)->y);
	cNow.Kill();
	asChanged.ReInit();

	//The second band grew (a gap below its row), so the third moved: all from 10 down.
	InitTestBands(&cNow, aiMoved, 3, "abc");
	cNow.Compare(&cBefore, &asChanged);
	AssertSize(1, asChanged.NumElements());
	AssertInt(10, asChanged.Get(0)->x);
	AssertInt(35, asChanged.Get(0)->y);
	cNow.Kill();
	asChanged.ReInit();

	//A band gone, and changes apart from each other.
	InitTestBands(&cNow, aiShorter, 2, "Xb");
	cNow.Compare(&cBefore, &asChanged);
	AssertSize(2, asChanged.NumElements());
	AssertInt(0, asChanged.Get(0)->x);
	AssertInt(10, asChanged.Get(0)->y);
	AssertInt(20, asChanged.Get(1)->x);
	AssertInt(30, asChanged.Get(1)->y);
	cNow.Kill();

	cBefore.Kill();
	asChanged.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Something drawn across bands goes into each band it crosses; the band
// a y is in is found, past the last is none.  Taking bands clears those
// taken.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBandsMixAll(void)
{
	CTextAreaBands	cBands;
	CTextAreaBands	cTaken;
	int32			aiTops[] = { 0, 10, 20, 30 };
	uint32			auiHashes[3];
	char			c;
	size			i;

	InitTestBands(&cBands, aiTops, 3, "aaa");
	for (i = 0; i < 3; i++)
	{
		auiHashes[i] = cBands.Get(i)->uiHash;
	}
	AssertTrue(auiHashes[0] == auiHashes[1]);

	c = 'z';
	cBands.MixAll(15, 21, &c, 1);
	AssertTrue(auiHashes[0] == cBands.Get(0)->uiHash);
	AssertTrue(auiHashes[1] != cBands.Get(1)->uiHash);
	AssertTrue(auiHashes[2] != cBands.Get(2)->uiHash);

	AssertSize(0, cBands.FindBand(-5));
	AssertSize(0, cBands.FindBand(9));
	AssertSize(1, cBands.FindBand(10));
	AssertSize(3, cBands.FindBand(30));

	cTaken.Init();
	AssertFalse(cTaken.IsComplete());
	cTaken.Take(&cBands);
	AssertTrue(cTaken.IsComplete());
	AssertSize(3, cTaken.NumBands());
	AssertFalse(cBands.IsComplete());
	AssertSize(0, cBands.NumBands());

	cTaken.Kill();
	cBands.Kill();
}


Ptr<CFont> ReadLabelFont(void);


//////////////////////////////////////////////////////////////////////////
//
// A band for each row that shows, the last to the end given; a row whose
// text changed is the only band that did.
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBandsAddRows(void)
{
	CTextDocument			cDocument;
	Ptr<CTextAreaLayout>	pLayout;
	CTextAreaBands			cBefore;
	CTextAreaBands			cNow;
	CArrayInt32Vec2			asChanged;
	STextLayoutLine*		psSecond;

	ObjectsInit();
	cDocument.Init("ab\ncd");
	pLayout = OMalloc<CTextAreaLayout>(&cDocument, OMalloc<CFontFamily>(OMalloc<CFontColours>(ReadLabelFont())));
	pLayout->ReadText();
	pLayout->LayoutText();
	psSecond = pLayout->GetLayoutLine(1);

	cBefore.Init();
	cBefore.AddRows(&pLayout, &cDocument, 0, 1000, 500);
	cBefore.Complete();
	AssertSize(2, cBefore.NumBands());
	AssertInt(pLayout->GetLayoutLine(0)->iTop, cBefore.Get(0)->iTop);
	AssertInt(psSecond->iTop, cBefore.Get(0)->iBottom);
	AssertInt(500, cBefore.Get(1)->iBottom);

	//Only the rows from iTop to iBottom.
	cNow.Init();
	cNow.AddRows(&pLayout, &cDocument, psSecond->iTop, 1000, 500);
	AssertSize(1, cNow.NumBands());
	cNow.Kill();

	cDocument.SetText("ab\ncX");
	pLayout->TextSet();
	cNow.Init();
	cNow.AddRows(&pLayout, &cDocument, 0, 1000, 500);
	cNow.Complete();
	asChanged.Init();
	cNow.Compare(&cBefore, &asChanged);
	AssertSize(1, asChanged.NumElements());
	AssertInt(psSecond->iTop, asChanged.Get(0)->x);
	AssertInt(500, asChanged.Get(0)->y);

	asChanged.Kill();
	cNow.Kill();
	cBefore.Kill();
	pLayout = NULL;
	cDocument.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextAreaBands(void)
{
	BeginTests();

	TestTextAreaBandsCompare();
	TestTextAreaBandsMixAll();
	TestTextAreaBandsAddRows();

	TestStatistics();
}

