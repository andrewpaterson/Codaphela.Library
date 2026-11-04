#include <string.h>
#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "CppParserLib/CPP65Builder.h"
#include "CppParserLib/CPP65ListingIndex.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//A function whose line 3 is in two pieces around line 4 (a for loop), with a header's line in it, then data.
static const char* gszTestListingA =
	"\t\t.section text._f\n"									//0
	"\t\t.global _f\n"											//1
	"_f:\n"														//2
	"\t\t.width (A, X), 16\n"									//3
	"\t\tphd\n"													//4
	"\t\t.line \"C:\\\\Src\\\\A.cpp\", 3\n"						//5
	"\t\t; A.cpp(3): for (i = 0; i < n; i++)\n"					//6
	"\t\tlda 0\n"												//7
	"L1:\n"														//8
	"\t\t.line \"C:\\\\Src\\\\A.cpp\", 4\n"						//9
	"\t\t; A.cpp(4): s += i;\n"									//10
	"\t\tclc\n"													//11
	"\n"														//12
	"\t\t.line \"C:\\\\Src\\\\A.cpp\", 3\n"						//13
	"\t\t; A.cpp(3): for (i = 0; i < n; i++)\n"					//14
	"\t\tinc A\n"												//15
	"\t\t.line \"C:\\\\Src\\\\Head.h\", 10\n"					//16
	"\t\t; Head.h(10): return x;\n"								//17
	"\t\trtl\n"													//18
	"\n"														//19
	"\t\t.fingerprint \"_f\", 1, 2\n"							//20
	"\t\t.section data._g\n"									//21
	"_g:\n"														//22
	"\t\t.word 0\n";											//23

//Another unit's listing with the header's line in it, its name in forward slashes.
static const char* gszTestListingB =
	"\t\t.section text._h\n"									//0
	"_h:\n"														//1
	"\t\t.line \"C:/Src/Head.h\", 10\n"							//2
	"\t\t; Head.h(10): return x;\n"								//3
	"\t\trtl\n";												//4

//Carriage returns and line feeds.
static const char* gszTestListingC =
	"\t\t.section text._k\r\n"									//0
	"_k:\r\n"													//1
	"\t\t.line \"C:\\\\Src\\\\K.cpp\", 1\r\n"					//2
	"\t\tnop\r\n";												//3


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertListingRange(size uiFirstLine, size uiEndLine, CCPP65ListingIndex* pcIndex, SCPP65ListingPlace* psPlace, size uiRange)
{
	SCPP65ListingRange*	psRange;

	psRange = pcIndex->GetRange(psPlace->uiFirstRange + uiRange);
	AssertSize(uiFirstLine, psRange->uiFirstLine);
	AssertSize(uiEndLine, psRange->uiEndLine);
}


//////////////////////////////////////////////////////////////////////////
//
// A source line's code is from the line after its .line to the next
// directive that isn't an instruction, less blank lines; its places are
// a function in a listing each, the pieces of a function together.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65ListingIndexText(void)
{
	CCPP65ListingIndex			cIndex;
	CArrayCPP65ListingPlaces	asPlaces;
	SCPP65ListingPlace*			psPlace;
	CChars						szLines;

	cIndex.Init();
	asPlaces.Init();
	cIndex.AddListing("A.lst", gszTestListingA);
	cIndex.AddListing("B.lst", gszTestListingB);
	cIndex.AddListing("C.lst", gszTestListingC);
	AssertSize(3, cIndex.NumListings());
	AssertString("B.lst", cIndex.GetListingName(1));
	AssertSize(24, cIndex.NumListingLines(0));

	//Head.h is one file whichever way its slashes go.
	AssertSize(3, cIndex.NumFiles());
	AssertString("C:\\Src\\A.cpp", cIndex.GetFile(0));
	AssertSize(6, cIndex.NumRanges());

	//A.cpp's line 3 (2 from 0) is two pieces of _f, the label between them kept.
	cIndex.GetPlaces("C:\\Src\\A.cpp", 2, &asPlaces);
	AssertSize(1, asPlaces.NumElements());
	psPlace = asPlaces.Get(0);
	AssertSize(0, psPlace->uiListing);
	AssertString("_f", cIndex.GetFunction(psPlace->uiFunction));
	AssertSize(2, psPlace->uiNumRanges);
	AssertListingRange(6, 9, &cIndex, psPlace, 0);
	AssertListingRange(14, 16, &cIndex, psPlace, 1);
	szLines.Init();
	cIndex.AppendLines(0, 6, 9, &szLines);
	AssertString("\t\t; A.cpp(3): for (i = 0; i < n; i++)\n\t\tlda 0\nL1:\n", szLines.Text());
	szLines.Kill();

	//Line 4 without the blank line after it; the file found whatever its case and slashes.
	cIndex.GetPlaces("c:/src/a.CPP", 3, &asPlaces);
	AssertSize(1, asPlaces.NumElements());
	AssertListingRange(10, 12, &cIndex, asPlaces.Get(0), 0);

	//The header's line is in both units, in the order they were added.
	cIndex.GetPlaces("C:\\Src\\Head.h", 9, &asPlaces);
	AssertSize(2, asPlaces.NumElements());
	psPlace = asPlaces.Get(0);
	AssertSize(0, psPlace->uiListing);
	AssertString("_f", cIndex.GetFunction(psPlace->uiFunction));
	AssertListingRange(17, 19, &cIndex, psPlace, 0);
	psPlace = asPlaces.Get(1);
	AssertSize(1, psPlace->uiListing);
	AssertString("_h", cIndex.GetFunction(psPlace->uiFunction));
	AssertListingRange(3, 5, &cIndex, psPlace, 0);

	//Carriage returns aren't in the function's name, and the last line ends its range.
	cIndex.GetPlaces("C:\\Src\\K.cpp", 0, &asPlaces);
	AssertSize(1, asPlaces.NumElements());
	psPlace = asPlaces.Get(0);
	AssertString("_k", cIndex.GetFunction(psPlace->uiFunction));
	AssertListingRange(3, 4, &cIndex, psPlace, 0);

	//Lines with no code, and files that aren't in any listing, have no places.
	cIndex.GetPlaces("C:\\Src\\A.cpp", 0, &asPlaces);
	AssertSize(0, asPlaces.NumElements());
	cIndex.GetPlaces("C:\\Src\\A.cpp", 4, &asPlaces);
	AssertSize(0, asPlaces.NumElements());
	cIndex.GetPlaces("C:\\Src\\None.cpp", 2, &asPlaces);
	AssertSize(0, asPlaces.NumElements());

	asPlaces.Kill();
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The listings the compiler writes with the line table on are indexed:
// Maths.cpp's Triple and Main.cpp's main (Triple inlined into it is under
// the call, see TestCPP816ListingInlinedFromUnit).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65ListingIndexBuild(void)
{
	static const char*			aszUnits[] = { "Input/CPP65/ListingInlineProject/Maths.cpp", "Input/CPP65/ListingInlineProject/Main.cpp", NULL };
	CCPP65Builder				cBuilder;
	CFileUtil					cFileUtil;
	CCPP65ListingIndex			cIndex;
	CArrayCPP65ListingPlaces	asPlaces;
	SCPP65ListingPlace*			psPlace;
	SCPP65ListingRange*			psRange;
	CChars						szFile;
	CChars						szLines;

	cFileUtil.RemoveDir("Output" _FS_ "CPP65ListingIndex");
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP65ListingIndex");
	cBuilder.SetLineTable(true);
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	cIndex.Init();
	asPlaces.Init();
	AssertTrue(cIndex.ReadListing("Output" _FS_ "CPP65ListingIndex" _FS_ "Maths.lst"));
	AssertTrue(cIndex.ReadListing("Output" _FS_ "CPP65ListingIndex" _FS_ "Main.lst"));
	AssertFalse(cIndex.ReadListing("Output" _FS_ "CPP65ListingIndex" _FS_ "None.lst"));

	//main's line 27 (26 from 0) is in Main's listing, the comment first.
	szFile.Init("Input" _FS_ "CPP65" _FS_ "ListingInlineProject" _FS_ "Main.cpp");
	cFileUtil.FullPath(&szFile);
	cIndex.GetPlaces(szFile.Text(), 26, &asPlaces);
	szFile.Kill();
	AssertSize(1, asPlaces.NumElements());
	psPlace = asPlaces.Get(0);
	AssertSize(1, psPlace->uiListing);
	AssertString("_main", cIndex.GetFunction(psPlace->uiFunction));
	psRange = cIndex.GetRange(psPlace->uiFirstRange);
	szLines.Init();
	cIndex.AppendLines(psPlace->uiListing, psRange->uiFirstLine, psRange->uiEndLine, &szLines);
	AssertTrue(szLines.StartsWith("\t\t; Main.cpp(27): int r = Triple(gi, 5);\n"));
	AssertTrue(szLines.Contains("\t\trtl\n"));
	szLines.Kill();

	//Triple's line 13 is in Maths' listing.
	szFile.Init("Input" _FS_ "CPP65" _FS_ "ListingInlineProject" _FS_ "Maths.cpp");
	cFileUtil.FullPath(&szFile);
	cIndex.GetPlaces(szFile.Text(), 12, &asPlaces);
	szFile.Kill();
	AssertSize(1, asPlaces.NumElements());
	psPlace = asPlaces.Get(0);
	AssertSize(0, psPlace->uiListing);
	AssertTrue(strncmp(cIndex.GetFunction(psPlace->uiFunction), "_Triple", 7) == 0);

	asPlaces.Kill();
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65ListingIndex(void)
{
	BeginTests();

	TestCPP65ListingIndexText();
	TestCPP65ListingIndexBuild();

	TestStatistics();
}

