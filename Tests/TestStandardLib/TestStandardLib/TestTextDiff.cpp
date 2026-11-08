#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StdRandom.h"
#include "StandardLib/MyersDiff.h"
#include "StandardLib/TextDiffProducer.h"
#include "StandardLib/TextDiffLineMapper.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertTextDiff(const char* szExpected, const char* szText1, const char* szText2, ETextDiffWhitespace eWhitespace, ETextDiffHighlight eHighlight, size iLine)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CChars				sz;

	cDiff.Init("Diff");
	cProducer.Init(eWhitespace, eHighlight);
	cProducer.ProduceFromText(&cDiff, "A", szText1, "B", szText2);

	sz.Init();
	cDiff.Print(&sz);
	Validate(PrivateAssertString(szExpected, sz.Text(), true, NULL, iLine, __FILE__));
	sz.Kill();

	cProducer.Kill();
	cDiff.Kill();
}


#define AssertDiff(e, t1, t2, w, h)	AssertTextDiff(e, t1, t2, w, h, __LINE__)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextContentLines(void)
{
	CTextContent	cContent;

	cContent.Init();
	AssertString("", cContent.GetTitle());
	AssertLSize(0, cContent.Length());
	AssertLSize(0, cContent.NumLines());
	AssertLSize(0, cContent.FindLine(0));
	AssertLSize(0, cContent.FindLine(5));
	cContent.Kill();

	cContent.Init("T", "a\r\nbb\n\nccc\rd");
	AssertString("T", cContent.GetTitle());
	AssertLSize(12, cContent.Length());
	AssertLSize(5, cContent.NumLines());
	AssertLSize(0, cContent.GetLineStart(0));
	AssertLSize(1, cContent.GetLineLength(0));
	AssertTrue(TLS_CRLF == cContent.GetLineSeparator(0));
	AssertLSize(3, cContent.GetLineStart(1));
	AssertLSize(2, cContent.GetLineLength(1));
	AssertTrue(TLS_LF == cContent.GetLineSeparator(1));
	AssertLSize(6, cContent.GetLineStart(2));
	AssertLSize(0, cContent.GetLineLength(2));
	AssertLSize(7, cContent.GetLineStart(3));
	AssertLSize(10, cContent.GetLineEnd(3));
	AssertTrue(TLS_CR == cContent.GetLineSeparator(3));
	AssertLSize(11, cContent.GetLineStart(4));
	AssertLSize(1, cContent.GetLineLength(4));
	AssertTrue(TLS_None == cContent.GetLineSeparator(4));
	AssertTrue('d' == *cContent.GetLineText(4));
	AssertLSize(11, cContent.GetLine(4)->uiStart);

	//The line an offset is in; a separator is its line's, and past the end is the last line.
	AssertLSize(0, cContent.FindLine(0));
	AssertLSize(0, cContent.FindLine(2));
	AssertLSize(1, cContent.FindLine(3));
	AssertLSize(1, cContent.FindLine(5));
	AssertLSize(2, cContent.FindLine(6));
	AssertLSize(3, cContent.FindLine(7));
	AssertLSize(3, cContent.FindLine(10));
	AssertLSize(4, cContent.FindLine(11));
	AssertLSize(4, cContent.FindLine(12));
	AssertLSize(4, cContent.FindLine(100));

	//A final separator doesn't start another line.
	cContent.Set("U", "a\n");
	AssertString("U", cContent.GetTitle());
	AssertLSize(1, cContent.NumLines());
	AssertTrue(TLS_LF == cContent.GetLineSeparator(0));

	//The text is copied with its length, zeros and all.
	cContent.Set("Z", "a\0b\nc", 5);
	AssertLSize(5, cContent.Length());
	AssertLSize(2, cContent.NumLines());
	AssertLSize(3, cContent.GetLineLength(0));
	AssertLSize(4, cContent.GetLineStart(1));
	cContent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextContentFile(void)
{
	CTextContent	cContent;

	AssertTrue(cContent.InitFromFile("Old", "Input/TextDiff/Old.txt"));
	AssertString("Old", cContent.GetTitle());
	AssertLSize(4, cContent.NumLines());
	AssertLSize(1, cContent.GetLineLength(1));

	AssertFalse(cContent.SetFromFile("Missing", "Input/TextDiff/Missing.txt"));
	AssertString("Missing", cContent.GetTitle());
	AssertLSize(0, cContent.NumLines());
	cContent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffLines(void)
{
	AssertDiff("--- A\n+++ B\n", "a\nb\nc\n", "a\nb\nc\n", TDW_Default, TDH_Lines);
	AssertDiff("--- A\n+++ B\n", "", "", TDW_Default, TDH_Lines);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -2,0 +2,1 @@\n"
			   "+x\n",
			   "a\nb\nc\n", "a\nb\nx\nc\n", TDW_Default, TDH_Lines);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,0 @@\n"
			   "-a\n"
			   "@@ -3,1 +2,0 @@\n"
			   "-d\n",
			   "a\nb\nc\nd\n", "b\nc\n", TDW_Default, TDH_Lines);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,0 +0,2 @@\n"
			   "+a\n"
			   "+b\n",
			   "", "a\nb\n", TDW_Default, TDH_Lines);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,2 +0,0 @@\n"
			   "-a\n"
			   "-b\n",
			   "a\nb\n", "", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -1,2 +1,1 @@\n"
			   "-b\n"
			   "-c\n"
			   "+x\n",
			   "a\nb\nc\nd\n", "a\nx\nd\n", TDW_Default, TDH_Lines);
}


//////////////////////////////////////////////////////////////////////////
//
// Separators compare equal whatever their kind.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffSeparators(void)
{
	AssertDiff("--- A\n+++ B\n", "a\r\nb\r\n", "a\nb\n", TDW_Default, TDH_Words);
	AssertDiff("--- A\n+++ B\n", "a\rb\r", "a\nb\n", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,2 +0,2 @@\n"
			   "-x\n"
			   "-y\n"
			   "+x1\n"
			   "+y1\n"
			   "~[x]>[x1]\n"
			   "~[y]>[y1]\n",
			   "x\r\ny\r\n", "x1\ny1\n", TDW_Default, TDH_Words);
}


//////////////////////////////////////////////////////////////////////////
//
// A missing separator at the end of the text is a difference unless
// whitespace is trimmed or ignored.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffNoNewline(void)
{
	AssertDiff("--- A\n+++ B\n", "x\ny", "x\ny", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -1,1 +1,1 @@\n"
			   "-b\n"
			   "\\ No newline at end of file\n"
			   "+b\n",
			   "a\nb", "a\nb\n", TDW_Default, TDH_Lines);
	AssertDiff("--- A\n+++ B\n"
			   "@@ -1,1 +1,1 @@\n"
			   "-b\n"
			   "\\ No newline at end of file\n"
			   "+b\n"
			   "~[]>[\\n]\n",
			   "a\nb", "a\nb\n", TDW_Default, TDH_Words);
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-a\n"
			   "+a\n"
			   "\\ No newline at end of file\n"
			   "~[\\n]>[]\n",
			   "a\n", "a", TDW_Default, TDH_Chars);
	AssertDiff("--- A\n+++ B\n", "a\nb", "a\nb\n", TDW_Trim, TDH_Words);
	AssertDiff("--- A\n+++ B\n", "a\nb", "a\nb\n", TDW_Ignore, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -1,1 +1,1 @@\n"
			   "-b\n"
			   "\\ No newline at end of file\n"
			   "+c\n"
			   "\\ No newline at end of file\n"
			   "~[b]>[c]\n",
			   "a\nb", "a\nc", TDW_Default, TDH_Words);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffWhitespace(void)
{
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-  a b\n"
			   "+a b  \n",
			   "  a b\n", "a b  \n", TDW_Default, TDH_Lines);
	AssertDiff("--- A\n+++ B\n", "  a b\n", "a b  \n", TDW_Trim, TDH_Lines);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-a b\n"
			   "+a  b\n",
			   "a b\n", "a  b\n", TDW_Trim, TDH_Lines);
	AssertDiff("--- A\n+++ B\n", "a b\n", "a\tb\n", TDW_Ignore, TDH_Lines);
	AssertDiff("--- A\n+++ B\n", "a b\n", "ab\n", TDW_Ignore, TDH_Lines);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffWords(void)
{
	AssertDiff("--- A\n+++ B\n"
			   "@@ -1,1 +1,1 @@\n"
			   "-int x = 1;\n"
			   "+int count = 12;\n"
			   "~[x]>[count]\n"
			   "~[1]>[12]\n",
			   "{\nint x = 1;\n}\n", "{\nint count = 12;\n}\n", TDW_Default, TDH_Words);

	//Words are compared whole.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-count++;\n"
			   "+counter++;\n"
			   "~[count]>[counter]\n",
			   "count++;\n", "counter++;\n", TDW_Default, TDH_Words);

	//An inserted word is an empty range on the left, after the token before it.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-return a;\n"
			   "+return a + b;\n"
			   "~[]>[ + b]\n",
			   "return a;\n", "return a + b;\n", TDW_Default, TDH_Words);

	//Whitespace between words is a token unless whitespace is ignored.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-f(a, b)\n"
			   "+f(a,b, c)\n"
			   "~[ ]>[]\n"
			   "~[]>[, c]\n",
			   "f(a, b)\n", "f(a,b, c)\n", TDW_Default, TDH_Words);
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-f(a, b)\n"
			   "+f(a,b, c)\n"
			   "~[]>[, c]\n",
			   "f(a, b)\n", "f(a,b, c)\n", TDW_Ignore, TDH_Words);

	//Line breaks inside a block are compared unless whitespace is trimmed or ignored.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,2 +0,1 @@\n"
			   "-a\n"
			   "-b\n"
			   "+a b\n"
			   "~[\\n]>[ ]\n",
			   "a\nb\n", "a b\n", TDW_Default, TDH_Words);
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,2 +0,1 @@\n"
			   "-a\n"
			   "-b\n"
			   "+a b\n"
			   "~[]>[ ]\n",
			   "a\nb\n", "a b\n", TDW_Trim, TDH_Words);
}


//////////////////////////////////////////////////////////////////////////
//
// A word fragment of only whitespace and punctuation is compared again by
// character.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffGaps(void)
{
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-a   b\n"
			   "+a b\n"
			   "~[  ]>[]\n",
			   "a   b\n", "a b\n", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-p->x\n"
			   "+p.x\n"
			   "~[->]>[.]\n",
			   "p->x\n", "p.x\n", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-f( a)\n"
			   "+f(\ta)\n"
			   "~[ ]>[\t]\n",
			   "f( a)\n", "f(\ta)\n", TDW_Default, TDH_Words);
}


//////////////////////////////////////////////////////////////////////////
//
// Word fragments on a line separated only by whitespace are squashed.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffSquash(void)
{
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-x = a b;\n"
			   "+x = c d;\n"
			   "~[a b]>[c d]\n",
			   "x = a b;\n", "x = c d;\n", TDW_Default, TDH_Words);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-x = a b;\n"
			   "+x = c d;\n"
			   "~[a b]>[c d]\n",
			   "x = a b;\n", "x = c d;\n", TDW_Ignore, TDH_Words);

	//Not across lines.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,2 +0,2 @@\n"
			   "-a\n"
			   "-b\n"
			   "+c\n"
			   "+d\n"
			   "~[a]>[c]\n"
			   "~[b]>[d]\n",
			   "a\nb\n", "c\nd\n", TDW_Default, TDH_Words);

	//Not characters.
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-ab cd\n"
			   "+xb yd\n"
			   "~[a]>[x]\n"
			   "~[c]>[y]\n",
			   "ab cd\n", "xb yd\n", TDW_Default, TDH_Chars);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffChars(void)
{
	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-hello world\n"
			   "+help world\n"
			   "~[lo]>[p]\n",
			   "hello world\n", "help world\n", TDW_Default, TDH_Chars);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-abcdef\n"
			   "+abXdeYf\n"
			   "~[c]>[X]\n"
			   "~[]>[Y]\n",
			   "abcdef\n", "abXdeYf\n", TDW_Default, TDH_Chars);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-a b c\n"
			   "+abd\n"
			   "~[c]>[d]\n",
			   "a b c\n", "abd\n", TDW_Ignore, TDH_Chars);

	AssertDiff("--- A\n+++ B\n"
			   "@@ -0,1 +0,1 @@\n"
			   "-  ab\n"
			   "+ac  \n"
			   "~[b]>[c]\n",
			   "  ab\n", "ac  \n", TDW_Trim, TDH_Chars);
}


//////////////////////////////////////////////////////////////////////////
//
// Fragment offsets are into each side's whole text.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffOffsets(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	STextDiffBlock*		psBlock;
	STextDiffFragment*	psFragment;

	cDiff.Init("Offsets");
	cProducer.Init();
	cProducer.ProduceFromText(&cDiff, "A", "one\ntwo\n", "B", "one\ntoo\n");

	AssertString("Offsets", cDiff.GetTitle());
	AssertTrue(TDW_Default == cDiff.GetWhitespace());
	AssertTrue(TDH_Words == cDiff.GetHighlight());
	AssertFalse(cDiff.IsIdentical());
	AssertLSize(1, cDiff.NumBlocks());
	AssertLSize(1, cDiff.NumFragments());
	psBlock = cDiff.GetBlock(0);
	AssertTrue(TDBT_Modified == psBlock->GetType());
	AssertLSize(1, psBlock->uiNumFragments);
	psFragment = cDiff.GetFragment(psBlock, 0);
	AssertLSize(4, psFragment->uiStart1);
	AssertLSize(7, psFragment->uiEnd1);
	AssertLSize(4, psFragment->uiStart2);
	AssertLSize(7, psFragment->uiEnd2);

	//Produce again with other policies and the same contents.
	cProducer.SetHighlight(TDH_Chars);
	cProducer.Produce(&cDiff);
	AssertTrue(TDH_Chars == cDiff.GetHighlight());
	AssertLSize(1, cDiff.NumBlocks());
	psBlock = cDiff.GetBlock(0);
	AssertLSize(1, psBlock->uiNumFragments);
	psFragment = cDiff.GetFragment(psBlock, 0);
	AssertLSize(5, psFragment->uiStart1);
	AssertLSize(6, psFragment->uiEnd1);
	AssertLSize(5, psFragment->uiStart2);
	AssertLSize(6, psFragment->uiEnd2);

	cProducer.SetHighlight(TDH_Lines);
	cProducer.SetWhitespace(TDW_Ignore);
	cProducer.Produce(&cDiff);
	AssertTrue(TDW_Ignore == cDiff.GetWhitespace());
	AssertLSize(1, cDiff.NumBlocks());
	AssertLSize(0, cDiff.GetBlock(0)->uiNumFragments);
	AssertLSize(0, cDiff.NumFragments());

	cProducer.Kill();
	cDiff.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffBlockTypes(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	STextDiffBlock*		psBlock;

	cDiff.Init("Types");
	cProducer.Init();
	cProducer.ProduceFromText(&cDiff, "A", "a\nb\nc\nd\n", "B", "x\na\nc\nD\nE\n");
	AssertLSize(3, cDiff.NumBlocks());

	psBlock = cDiff.GetBlock(0);
	AssertTrue(TDBT_Inserted == psBlock->GetType());
	AssertLSize(0, psBlock->NumLines1());
	AssertLSize(1, psBlock->NumLines2());
	AssertLSize(0, psBlock->uiNumFragments);

	psBlock = cDiff.GetBlock(1);
	AssertTrue(TDBT_Deleted == psBlock->GetType());
	AssertLSize(1, psBlock->uiStartLine1);
	AssertLSize(1, psBlock->NumLines1());
	AssertLSize(0, psBlock->NumLines2());

	psBlock = cDiff.GetBlock(2);
	AssertTrue(TDBT_Modified == psBlock->GetType());
	AssertLSize(3, psBlock->uiStartLine1);
	AssertLSize(1, psBlock->NumLines1());
	AssertLSize(3, psBlock->uiStartLine2);
	AssertLSize(2, psBlock->NumLines2());
	AssertLSize(1, psBlock->uiNumFragments);

	cProducer.Kill();
	cDiff.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks too big to compare by character have no fragments.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffBigBlock(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CLargeChars			sz1;
	CLargeChars			sz2;

	sz1.Init();
	sz1.Append('a', TEXT_DIFF_MAX_REFINE_LENGTH);
	sz1.Append("\n");
	sz2.Init();
	sz2.Append('a', TEXT_DIFF_MAX_REFINE_LENGTH);
	sz2.Append("b\n");

	cDiff.Init("Big");
	cProducer.Init(TDW_Default, TDH_Chars);
	cProducer.ProduceFromText(&cDiff, "A", sz1.Text(), "B", sz2.Text());
	AssertLSize(1, cDiff.NumBlocks());
	AssertLSize(0, cDiff.GetBlock(0)->uiNumFragments);

	//Exactly the limit is refined.
	sz2.Kill();
	sz2.Init();
	sz2.Append('a', TEXT_DIFF_MAX_REFINE_LENGTH - 1);
	sz2.Append("b\n");
	cProducer.ProduceFromText(&cDiff, "A", sz1.Text(), "B", sz2.Text());
	AssertLSize(1, cDiff.NumBlocks());
	AssertLSize(1, cDiff.GetBlock(0)->uiNumFragments);

	cProducer.Kill();
	cDiff.Kill();
	sz2.Kill();
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Enough distinct lines to grow the intern table well past its start.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffManyLines(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CChars				sz1;
	CChars				sz2;
	uint32				ui;
	lsize				uiBlock;
	bool				bLinesCorrect;

	sz1.Init();
	sz2.Init();
	for (ui = 0; ui < 500; ui++)
	{
		sz1.Append("line ");
		sz1.Append(ui);
		sz1.AppendNewLine();
		sz2.Append("line ");
		sz2.Append(ui);
		if (ui % 7 == 3)
		{
			sz2.Append(" changed");
		}
		sz2.AppendNewLine();
	}

	cDiff.Init("Many");
	cProducer.Init();
	cProducer.ProduceFromText(&cDiff, "A", sz1.Text(), "B", sz2.Text());
	AssertLSize(71, cDiff.NumBlocks());
	bLinesCorrect = true;
	for (uiBlock = 0; uiBlock < cDiff.NumBlocks(); uiBlock++)
	{
		if ((cDiff.GetBlock(uiBlock)->uiStartLine1 != uiBlock * 7 + 3) || (cDiff.GetBlock(uiBlock)->NumLines1() != 1) || (cDiff.GetBlock(uiBlock)->uiNumFragments != 1))
		{
			bLinesCorrect = false;
		}
	}
	AssertTrue(bLinesCorrect);

	cProducer.Kill();
	cDiff.Kill();
	sz2.Kill();
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffFiles(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CChars				sz;

	cDiff.Init("Files");
	cProducer.Init();
	AssertTrue(cProducer.ProduceFromFiles(&cDiff, "Input/TextDiff/Old.txt", "Input/TextDiff/New.txt"));

	sz.Init();
	cDiff.Print(&sz);
	AssertString("--- Input/TextDiff/Old.txt\n"
				 "+++ Input/TextDiff/New.txt\n"
				 "@@ -0,1 +0,1 @@\n"
				 "-int Add(int a, int b)\n"
				 "+int Add(int a, int b, int c)\n"
				 "~[]>[, int c]\n"
				 "@@ -2,1 +2,1 @@\n"
				 "-\treturn a + b;\n"
				 "+\treturn a + b + c;\n"
				 "~[]>[ + c]\n",
				 sz.Text());
	sz.Kill();

	AssertFalse(cProducer.ProduceFromFiles(&cDiff, "Input/TextDiff/Missing1.txt", "Input/TextDiff/Missing2.txt"));
	AssertTrue(cDiff.IsIdentical());

	cProducer.Kill();
	cDiff.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMyersDiffEdges(void)
{
	CMyersDiff		cMyers;
	CArrayDiffRange	acChanges;
	lsize			auiAbc[3] = { 1, 2, 3 };
	lsize			auiXyz[2] = { 7, 8 };
	SDiffRange*		psChange;

	cMyers.Init();
	acChanges.Init();

	cMyers.Diff(auiAbc, 0, auiXyz, 0, &acChanges);
	AssertLSize(0, acChanges.NumElements());

	cMyers.Diff(auiAbc, 3, auiAbc, 3, &acChanges);
	AssertLSize(0, acChanges.NumElements());

	cMyers.Diff(auiAbc, 3, auiXyz, 0, &acChanges);
	AssertLSize(1, acChanges.NumElements());
	psChange = acChanges.Get(0);
	AssertLSize(0, psChange->uiStart1);
	AssertLSize(3, psChange->Length1());
	AssertLSize(0, psChange->Length2());

	//Nothing in common is one change.  It isn't merged with the change already in the array.
	cMyers.Diff(auiAbc, 3, auiXyz, 2, &acChanges);
	AssertLSize(2, acChanges.NumElements());
	psChange = acChanges.Get(1);
	AssertLSize(0, psChange->uiStart1);
	AssertLSize(3, psChange->uiEnd1);
	AssertLSize(0, psChange->uiStart2);
	AssertLSize(2, psChange->uiEnd2);

	acChanges.Kill();
	cMyers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize LongestCommonSubsequence(lsize* paui1, lsize uiLength1, lsize* paui2, lsize uiLength2)
{
	CArrayLargeTemplate<lsize>	auiTable;
	lsize*						pauiTable;
	lsize						ui1;
	lsize						ui2;
	lsize						uiWidth;
	lsize						uiResult;
	lsize						uiUp;
	lsize						uiLeft;

	uiWidth = uiLength2 + 1;
	auiTable.Init();
	auiTable.Resize((uiLength1 + 1) * uiWidth);
	pauiTable = auiTable.GetData();
	for (ui1 = 0; ui1 <= uiLength1; ui1++)
	{
		for (ui2 = 0; ui2 <= uiLength2; ui2++)
		{
			if ((ui1 == 0) || (ui2 == 0))
			{
				pauiTable[ui1 * uiWidth + ui2] = 0;
			}
			else if (paui1[ui1 - 1] == paui2[ui2 - 1])
			{
				pauiTable[ui1 * uiWidth + ui2] = pauiTable[(ui1 - 1) * uiWidth + ui2 - 1] + 1;
			}
			else
			{
				uiUp = pauiTable[(ui1 - 1) * uiWidth + ui2];
				uiLeft = pauiTable[ui1 * uiWidth + ui2 - 1];
				pauiTable[ui1 * uiWidth + ui2] = uiUp > uiLeft ? uiUp : uiLeft;
			}
		}
	}
	uiResult = pauiTable[uiLength1 * uiWidth + uiLength2];
	auiTable.Kill();
	return uiResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Checks the changes are in order, don't touch, and that the elements
// between them match.  Returns the number of elements changed.
//
//////////////////////////////////////////////////////////////////////////
bool CheckMyersChanges(CArrayDiffRange* pacChanges, lsize* paui1, lsize uiLength1, lsize* paui2, lsize uiLength2, lsize* puiEdits)
{
	lsize		uiChange;
	lsize		uiPosition1;
	lsize		uiPosition2;
	lsize		uiMatchEnd1;
	SDiffRange*	psChange;

	uiPosition1 = 0;
	uiPosition2 = 0;
	*puiEdits = 0;
	for (uiChange = 0; uiChange <= pacChanges->NumElements(); uiChange++)
	{
		psChange = (uiChange < pacChanges->NumElements()) ? pacChanges->Get(uiChange) : NULL;
		if (psChange != NULL)
		{
			if ((psChange->uiStart1 < uiPosition1) || (psChange->uiEnd1 < psChange->uiStart1) || (psChange->uiEnd2 < psChange->uiStart2) ||
				((psChange->Length1() == 0) && (psChange->Length2() == 0)) ||
				((uiChange > 0) && (psChange->uiStart1 == uiPosition1)))
			{
				return false;
			}
		}

		uiMatchEnd1 = (psChange != NULL) ? psChange->uiStart1 : uiLength1;
		while (uiPosition1 < uiMatchEnd1)
		{
			if ((uiPosition2 >= uiLength2) || (paui1[uiPosition1] != paui2[uiPosition2]))
			{
				return false;
			}
			uiPosition1++;
			uiPosition2++;
		}

		if (psChange != NULL)
		{
			if (uiPosition2 != psChange->uiStart2)
			{
				return false;
			}
			*puiEdits += psChange->Length1() + psChange->Length2();
			uiPosition1 = psChange->uiEnd1;
			uiPosition2 = psChange->uiEnd2;
		}
	}
	return uiPosition2 == uiLength2;
}


//////////////////////////////////////////////////////////////////////////
//
// Random sequences: the changes are well formed and a minimal edit
// (n + m - 2 * LCS elements).
//
//////////////////////////////////////////////////////////////////////////
void TestMyersDiffRandom(void)
{
	CRandom			cRandom;
	CMyersDiff		cMyers;
	CArrayDiffRange	acChanges;
	lsize			auiSequence1[60];
	lsize			auiSequence2[60];
	lsize			uiLength1;
	lsize			uiLength2;
	lsize			ui;
	lsize			uiEdits;
	int32			iTest;
	int32			iAlphabet;
	bool			bWellFormed;
	bool			bMinimal;

	cRandom.Init(1234);
	cMyers.Init();
	acChanges.Init();
	bWellFormed = true;
	bMinimal = true;
	for (iTest = 0; iTest < 2000; iTest++)
	{
		iAlphabet = cRandom.Next(1, 6);
		uiLength1 = (lsize)cRandom.Next(0, 60);
		uiLength2 = (lsize)cRandom.Next(0, 60);
		for (ui = 0; ui < uiLength1; ui++)
		{
			auiSequence1[ui] = (lsize)cRandom.Next(0, iAlphabet);
		}
		for (ui = 0; ui < uiLength2; ui++)
		{
			//Half the time the second sequence is an edit of the first.
			if (((iTest & 1) == 0) && (ui < uiLength1) && (cRandom.Next(0, 3) != 0))
			{
				auiSequence2[ui] = auiSequence1[ui];
			}
			else
			{
				auiSequence2[ui] = (lsize)cRandom.Next(0, iAlphabet);
			}
		}

		acChanges.ReInit();
		cMyers.Diff(auiSequence1, uiLength1, auiSequence2, uiLength2, &acChanges);
		if (!CheckMyersChanges(&acChanges, auiSequence1, uiLength1, auiSequence2, uiLength2, &uiEdits))
		{
			bWellFormed = false;
		}
		else if (uiEdits != uiLength1 + uiLength2 - 2 * LongestCommonSubsequence(auiSequence1, uiLength1, auiSequence2, uiLength2))
		{
			bMinimal = false;
		}
	}
	AssertTrue(bWellFormed);
	AssertTrue(bMinimal);

	acChanges.Kill();
	cMyers.Kill();
	cRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A diff's paths are kept for the next: a diff after a bigger one finds
// the same changes as a diff on its own.
//
//////////////////////////////////////////////////////////////////////////
void TestMyersDiffReused(void)
{
	CMyersDiff		cReused;
	CMyersDiff		cFresh;
	CArrayDiffRange	acReused;
	CArrayDiffRange	acFresh;
	lsize			auiBig1[200];
	lsize			auiBig2[200];
	lsize			auiSmall1[] = { 1, 2, 3, 4, 5, 6 };
	lsize			auiSmall2[] = { 1, 7, 3, 5, 8, 6, 9 };
	lsize			ui;
	SDiffRange*		psReused;
	SDiffRange*		psFresh;

	for (ui = 0; ui < 200; ui++)
	{
		auiBig1[ui] = ui % 7;
		auiBig2[ui] = (ui * 3) % 5;
	}

	cReused.Init();
	acReused.Init();
	cReused.Diff(auiBig1, 200, auiBig2, 200, &acReused);
	acReused.ReInit();
	cReused.Diff(auiSmall1, 6, auiSmall2, 7, &acReused);

	cFresh.Init();
	acFresh.Init();
	cFresh.Diff(auiSmall1, 6, auiSmall2, 7, &acFresh);

	AssertLSize(4, acFresh.NumElements());  //2 to 7, 4 removed, 8 and 9 added.
	AssertLSize(acFresh.NumElements(), acReused.NumElements());
	for (ui = 0; ui < acFresh.NumElements(); ui++)
	{
		psReused = acReused.Get(ui);
		psFresh = acFresh.Get(ui);
		AssertLSize(psFresh->uiStart1, psReused->uiStart1);
		AssertLSize(psFresh->uiEnd1, psReused->uiEnd1);
		AssertLSize(psFresh->uiStart2, psReused->uiStart2);
		AssertLSize(psFresh->uiEnd2, psReused->uiEnd2);
	}

	acFresh.Kill();
	cFresh.Kill();
	acReused.Kill();
	cReused.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Maps lines between the two sides of szText1 and szText2's diff: the
// first iLeftToRight of afLeft map to the same of afRight, then the next
// iRightToLeft of afRight map to the same of afLeft.  Mapping isn't always
// reversible: a block that is a point on one side maps all its lines on
// the other to that point.
//
//////////////////////////////////////////////////////////////////////////
void AssertTextDiffLineMap(const char* szText1, const char* szText2, float64* afLeft, float64* afRight, int iLeftToRight, int iRightToLeft, size iLine)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CTextDiffLineMapper	cMapper;
	int					i;

	cDiff.Init("Diff");
	cProducer.Init(TDW_Default, TDH_Lines);
	cProducer.ProduceFromText(&cDiff, "A", szText1, "B", szText2);
	cMapper.Init(&cDiff);
	for (i = 0; i < iLeftToRight; i++)
	{
		Validate(PrivateAssertDouble(afRight[i], cMapper.TransferLine(TDS_Left, afLeft[i]), 6, NULL, iLine, __FILE__));
	}
	for (i = 0; i < iRightToLeft; i++)
	{
		Validate(PrivateAssertDouble(afLeft[iLeftToRight + i], cMapper.TransferLine(TDS_Right, afRight[iLeftToRight + i]), 6, NULL, iLine, __FILE__));
	}
	cMapper.Kill();
	cProducer.Kill();
	cDiff.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Unchanged lines map one to one, shifted by the blocks before them.
// Inside a block a line maps proportionally; a block empty on one side is
// a point there and its lines on the other side are skipped over.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffLineMapper(void)
{
	//The first iLeftToRight pairs map left to right, the rest right to left.
	float64	afSameLeft[] =		{ 0, 1.5, 7, 2.25 };
	float64	afSameRight[] =		{ 0, 1.5, 7, 2.25 };
	float64	afInsertLeft[] =	{ 0, 0.5, 1, 1.5, 0, 1, 1, 1, 1.5 };
	float64	afInsertRight[] =	{ 0, 0.5, 3, 3.5, 0, 1, 2, 3, 3.5 };
	float64	afModifyLeft[] =	{ 1, 2, 2.5, 3, 4, 2, 3, 5 };
	float64	afModifyRight[] =	{ 1, 3, 4, 5, 6, 3, 5, 7 };
	float64	afStartLeft[] =		{ 0, 0.5, 0, 0, 0.5 };
	float64	afStartRight[] =	{ 2, 2.5, 0, 1.5, 2.5 };
	float64	afEndLeft[] =		{ 0, 1, 2, 3, 0.5, 2, 3 };
	float64	afEndRight[] =		{ 0, 1, 1, 2, 0.5, 1, 2 };

	AssertTextDiffLineMap("a\nb\nc\n", "a\nb\nc\n", afSameLeft, afSameRight, 2, 2, __LINE__);
	AssertTextDiffLineMap("a\nb\n", "a\nX\nY\nb\n", afInsertLeft, afInsertRight, 4, 5, __LINE__);
	AssertTextDiffLineMap("a\nb\nc\nd\n", "a\nX\nY\nZ\nW\nd\n", afModifyLeft, afModifyRight, 5, 3, __LINE__);
	AssertTextDiffLineMap("", "a\nb\n", afStartLeft, afStartRight, 2, 3, __LINE__);
	AssertTextDiffLineMap("a\nb\n", "a\n", afEndLeft, afEndRight, 4, 3, __LINE__);
}


//////////////////////////////////////////////////////////////////////////
//
// The mapping as it is defined: the first block the line is before or in,
// found by looking at each block in turn.
//
//////////////////////////////////////////////////////////////////////////
float64 TextDiffTransferLineByScan(CSimpleTextDiff* pcDiff, ETextDiffSide eFrom, float64 fLine)
{
	lsize			uiBlock;
	STextDiffBlock*	psBlock;
	float64			fFromStart;
	float64			fFromEnd;
	float64			fToStart;
	float64			fToEnd;

	fFromEnd = 0;
	fToEnd = 0;
	for (uiBlock = 0; uiBlock < pcDiff->NumBlocks(); uiBlock++)
	{
		psBlock = pcDiff->GetBlock(uiBlock);
		fFromStart = (float64)((eFrom == TDS_Left) ? psBlock->uiStartLine1 : psBlock->uiStartLine2);
		fFromEnd = (float64)((eFrom == TDS_Left) ? psBlock->uiEndLine1 : psBlock->uiEndLine2);
		fToStart = (float64)((eFrom == TDS_Left) ? psBlock->uiStartLine2 : psBlock->uiStartLine1);
		fToEnd = (float64)((eFrom == TDS_Left) ? psBlock->uiEndLine2 : psBlock->uiEndLine1);
		if (fLine < fFromStart)
		{
			return fLine + (fToStart - fFromStart);
		}
		if (fLine < fFromEnd)
		{
			return fToStart + (fLine - fFromStart) * (fToEnd - fToStart) / (fFromEnd - fFromStart);
		}
	}
	return fLine + (fToEnd - fFromEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// With many blocks of every kind (lines added, removed and changed, at the
// start and the end too) the mapper finds the same line as looking at
// each block in turn, from before the first line to past the last.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiffLineMapperManyBlocks(void)
{
	CSimpleTextDiff		cDiff;
	CTextDiffProducer	cProducer;
	CTextDiffLineMapper	cMapper;
	CChars				szText1;
	CChars				szText2;
	int					i;
	float64				fLine;
	float64				fExpected;
	ETextDiffSide		eFrom;

	szText1.Init();
	szText2.Init();
	szText2.Append("start\n");
	for (i = 0; i < 60; i++)
	{
		switch (i % 4)
		{
		case 0:
			szText1.Append("same ")->Append(i)->Append("\n");
			szText2.Append("same ")->Append(i)->Append("\n");
			break;
		case 1:
			szText1.Append("removed ")->Append(i)->Append("\n");
			break;
		case 2:
			szText1.Append("same ")->Append(i)->Append("\n");
			szText2.Append("same ")->Append(i)->Append("\n");
			szText2.Append("added ")->Append(i)->Append("\n");
			szText2.Append("added ")->Append(i)->Append(" again\n");
			break;
		case 3:
			szText1.Append("old ")->Append(i)->Append("\n");
			szText1.Append("old ")->Append(i)->Append(" again\n");
			szText2.Append("new ")->Append(i)->Append("\n");
			break;
		}
	}
	szText1.Append("end\n");

	cDiff.Init("Diff");
	cProducer.Init(TDW_Default, TDH_Lines);
	cProducer.ProduceFromText(&cDiff, "A", szText1.Text(), "B", szText2.Text());
	AssertTrue(cDiff.NumBlocks() > 25);  //An insertion and a change touch, and are one block.
	cMapper.Init(&cDiff);

	for (eFrom = TDS_Left; eFrom <= TDS_Right; eFrom = (ETextDiffSide)(eFrom + 1))
	{
		for (fLine = -2; fLine < 140; fLine += 0.25)
		{
			fExpected = TextDiffTransferLineByScan(&cDiff, eFrom, fLine);
			AssertDouble(fExpected, cMapper.TransferLine(eFrom, fLine), 6);
		}
	}

	cMapper.Kill();
	cProducer.Kill();
	cDiff.Kill();
	szText2.Kill();
	szText1.Kill();

	cDiff.Init("Diff");
	cMapper.Init(&cDiff);
	AssertDouble(3.5, cMapper.TransferLine(TDS_Left, 3.5), 6);
	cMapper.Kill();
	cDiff.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDiff(void)
{
	BeginTests();
	MemoryInit();

	TestTextContentLines();
	TestTextContentFile();
	TestTextDiffLines();
	TestTextDiffSeparators();
	TestTextDiffNoNewline();
	TestTextDiffWhitespace();
	TestTextDiffWords();
	TestTextDiffGaps();
	TestTextDiffSquash();
	TestTextDiffChars();
	TestTextDiffOffsets();
	TestTextDiffBlockTypes();
	TestTextDiffBigBlock();
	TestTextDiffManyLines();
	TestTextDiffFiles();
	TestTextDiffLineMapper();
	TestTextDiffLineMapperManyBlocks();
	TestMyersDiffEdges();
	TestMyersDiffRandom();
	TestMyersDiffReused();

	MemoryKill();
	TestStatistics();
}
