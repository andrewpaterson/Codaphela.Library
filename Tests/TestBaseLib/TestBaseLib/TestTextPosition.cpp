#include "BaseLib/TextPosition.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextPositionSingleLine(void)
{
	CTextPositionPrinter	cPrinter;
	CChars					sz;
	CExternalString			cText;
	CChars					szDest;
	STextPosition			sPos;

	sz.Init("Backlog grooming");
	cText.Init(sz.Text(), sz.Length());

	szDest.Init();
	
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[3]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming\n   ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[0]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming\n^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[15]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming\n               ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[16]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming\n                ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[17]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming [^ OUT OF BOUNDS]", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[-1]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming [^ OUT OF BOUNDS]", szDest.Text());

	szDest.Kill();
	sz.Kill();

	sz.Init("B");
	cText.Init(sz.Text(), sz.Length());

	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[0]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("B\n^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[1]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("B\n ^\n", szDest.Text());

	szDest.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextPositionMultiLine(void)
{
	CTextPositionPrinter	cPrinter;
	CChars					sz;
	CExternalString			cText;
	CChars					szDest;
	STextPosition			sPos;

	sz.Init("Backlog grooming\nShoud be\nPart of the\nRelease Cycle.");
	cText.Init(sz.Text(), sz.Length());

	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[3]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]: Backlog grooming \n        ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[15]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]: Backlog grooming \n                    ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[16]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]: Backlog grooming \n                     ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[17]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[2]:  Shoud be \n      ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[24]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[2]:  Shoud be \n             ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[25]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[2]:  Shoud be \n              ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[26]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[3]:  Part of the \n      ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[36]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[3]:  Part of the \n                ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[37]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[3]:  Part of the \n                 ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[51]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[4]:  Release Cycle.\n                   ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[52]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[4]:  Release Cycle.\n                    ^\n", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[53]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming [^ OUT OF BOUNDS]", szDest.Text());

	szDest.Clear();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[-1]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("Backlog grooming [^ OUT OF BOUNDS]", szDest.Text());

	szDest.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextPositionEmptyPosition(void)
{
	CTextPositionPrinter	cPrinter;
	CChars					sz;
	CExternalString			cText;
	CChars					szDest;
	STextPosition			sPos;

	sz.Init("\n{\n");
	cText.Init(sz.Text(), sz.Length());
	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[0]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]:  \n     ^\n", szDest.Text());
	sz.Kill();
	szDest.Kill();

	sz.Init("XY\n{\n");
	cText.Init(sz.Text(), sz.Length());
	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[1]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]: XY \n      ^\n", szDest.Text());
	sz.Kill();
	szDest.Kill();

	sz.Init("XY\n{\n");
	cText.Init(sz.Text(), sz.Length());
	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[2]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[1]: XY \n       ^\n", szDest.Text());
	sz.Kill();
	szDest.Kill();


	sz.Init("XY\n{\n");
	cText.Init(sz.Text(), sz.Length());
	szDest.Init();
	sPos.Init(sz.Text(), sz.Length(), &(sz.Text()[3]));
	cPrinter.Init(&cText, &sPos);
	cPrinter.PrintPosition(&szDest);
	cPrinter.Kill();
	AssertString("[2]:  { \n      ^\n", szDest.Text());
	sz.Kill();
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextPosition(void)
{
	BeginTests();

	TestTextPositionSingleLine();
	TestTextPositionMultiLine();
	TestTextPositionEmptyPosition();

	TestStatistics();
}

