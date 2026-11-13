#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/Preprocessor.h"
#include "CppCompilerLib/HeaderFileMap.h"
#include "CppCompilerLib/HeaderFiles.h"
#include "CppCompilerLib/PPHeaderCache.h"
#include "TestLib/Assert.h"


#define TEST_PP_HEADER_CACHE_DIRECTORIES	2


//The headers the units include: their files, and the include directories (searched in order).
struct STestPPHeaderCacheHeaders
{
	CHeaderFileMap	cFiles;
	CHeaderFiles	acDirectories[TEST_PP_HEADER_CACHE_DIRECTORIES];
	size			uiDirectories;
	CChars			szBase;			//Input/HeaderCachePP, in full.
};


//////////////////////////////////////////////////////////////////////////
//
// The include directories: Input/HeaderCachePP's aszDirectories (NULL
// terminated; "" for itself, with its subdirectories).
//
//////////////////////////////////////////////////////////////////////////
static void TestPPHeaderCacheBegin(STestPPHeaderCacheHeaders* psHeaders, const char** aszDirectories)
{
	CFileUtil	cFileUtil;
	CChars		szDirectory;
	size		i;

	psHeaders->szBase.Init();
	cFileUtil.CurrentDirectory(&psHeaders->szBase);
	cFileUtil.AppendToPath(&psHeaders->szBase, "Input");
	cFileUtil.AppendToPath(&psHeaders->szBase, "HeaderCachePP");
	psHeaders->cFiles.Init();
	for (i = 0; aszDirectories[i]; i++)
	{
		szDirectory.Init(psHeaders->szBase);
		if (aszDirectories[i][0] != '\0')
		{
			cFileUtil.AppendToPath(&szDirectory, aszDirectories[i]);
		}
		AssertTrue(psHeaders->acDirectories[i].Init(szDirectory.Text(), &psHeaders->cFiles, aszDirectories[i][0] == '\0', false));
		szDirectory.Kill();
	}
	psHeaders->uiDirectories = i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestPPHeaderCacheEnd(STestPPHeaderCacheHeaders* psHeaders)
{
	size	i;

	for (i = 0; i < psHeaders->uiDirectories; i++)
	{
		psHeaders->acDirectories[i].Kill();
	}
	psHeaders->cFiles.Kill();
	psHeaders->szBase.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Everything the preprocessor made for pcFile, as text: its tokens (with
// their spacing, files, positions and expansions), the macro expansions,
// the lines skipped and the directives.
//
//////////////////////////////////////////////////////////////////////////
static void TestPPHeaderCacheDump(CTranslationUnit* pcFile, CChars* psz)
{
	CPreprocessedTokens*	pcTokens;
	SPreprocessedToken*		psToken;
	SMacroExpansion*		psExpansion;
	SInactiveRegion*		psRegion;
	SDirectiveLine*			psLine;
	size					i;

	pcTokens = pcFile->GetTokens();
	for (i = 0; i < pcTokens->NumTokens(); i++)
	{
		psToken = pcTokens->Get(i);
		psz->Append("T ");
		psz->Append(psToken->szText, psToken->uiLength);
		psz->Append(' ');
		psz->Append((int)psToken->uiKind);
		psz->Append(' ');
		psz->Append((int)(psToken->uiFlags & ~PTF_OUTPUT_TEXT));
		psz->Append(' ');
		psz->Append(psToken->pcFile ? psToken->pcFile->ShortName() : "-");
		psz->Append(' ');
		psz->Append(psToken->iLine);
		psz->Append(' ');
		psz->Append(psToken->iColumn);
		psz->Append(' ');
		psz->Append(psToken->iExpansion);
		psz->AppendNewLine();
	}
	for (i = 0; i < pcTokens->NumExpansions(); i++)
	{
		psExpansion = pcTokens->GetExpansion(i);
		psz->Append("E ");
		psz->Append(psExpansion->szName, psExpansion->uiNameLength);
		psz->Append(' ');
		psz->Append(psExpansion->pcFile ? psExpansion->pcFile->ShortName() : "-");
		psz->Append(' ');
		psz->Append(psExpansion->iLine);
		psz->Append(' ');
		psz->Append(psExpansion->iColumn);
		psz->Append(' ');
		psz->Append(psExpansion->iParent);
		psz->AppendNewLine();
	}
	for (i = 0; i < pcTokens->NumInactiveRegions(); i++)
	{
		psRegion = pcTokens->GetInactiveRegion(i);
		psz->Append("I ");
		psz->Append(psRegion->pcFile ? psRegion->pcFile->ShortName() : "-");
		psz->Append(' ');
		psz->Append(psRegion->iStartLine);
		psz->Append(' ');
		psz->Append(psRegion->iEndLine);
		psz->AppendNewLine();
	}
	for (i = 0; i < pcTokens->NumDirectiveLines(); i++)
	{
		psLine = pcTokens->GetDirectiveLine(i);
		psz->Append("D ");
		psz->Append(psLine->pcFile ? psLine->pcFile->ShortName() : "-");
		psz->Append(' ');
		psz->Append(psLine->iLine);
		psz->Append(' ');
		psz->Append(psLine->bActive ? 1 : 0);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses each unit in turn, its name aszNames[i] (in Input/
// HeaderCachePP; NULL terminated) and its text aszUnits[i], with the
// header cache pcCache (NULL for none), into paszDumps (TestPPHeaderCache
// Dump) and paszTexts (the preprocessed text).
//
//////////////////////////////////////////////////////////////////////////
static void TestPPHeaderCacheUnits(STestPPHeaderCacheHeaders* psHeaders, const char** aszNames, const char** aszUnits, CPPHeaderCache* pcCache, CArrayChars* paszDumps, CArrayChars* paszTexts)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;
	CChars				sz;
	CFileUtil			cFileUtil;
	size				i;
	size				j;
	bool				bResult;

	paszDumps->Init();
	paszTexts->Init();
	for (i = 0; aszNames[i]; i++)
	{
		szName.Init(psHeaders->szBase);
		cFileUtil.AppendToPath(&szName, aszNames[i]);
		cFile.Init(szName.Text(), NULL, false);
		cFile.SetContents((char*)aszUnits[i]);

		cConfig.Init("");
		cPreprocessor.Init(&cConfig);
		for (j = 0; j < psHeaders->uiDirectories; j++)
		{
			cPreprocessor.AddIncludeDirectory(&psHeaders->acDirectories[j]);
		}
		cPreprocessor.SetHeaderCache(pcCache);
		bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
		AssertString("", cPreprocessor.GetErrors()->Text());
		AssertTrue(bResult);

		sz.Init();
		TestPPHeaderCacheDump(&cFile, &sz);
		paszDumps->Add(sz.Text());
		sz.Kill();
		sz.Init();
		cFile.Print(&sz);
		paszTexts->Add(sz.Text());
		sz.Kill();

		cPreprocessor.Kill();
		cConfig.Kill();
		cFile.Kill();
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The units preprocessed with a header cache are the same as without.
// The cache's counts are in pcCache; the texts with it in paszTexts.
//
//////////////////////////////////////////////////////////////////////////
static void TestPPHeaderCacheSame(const char** aszDirectories, const char** aszNames, const char** aszUnits, CPPHeaderCache* pcCache, CArrayChars* paszTexts)
{
	STestPPHeaderCacheHeaders	sHeaders;
	CArrayChars					aszWithout;
	CArrayChars					aszTextsWithout;
	CArrayChars					aszWith;
	size						i;

	TestPPHeaderCacheBegin(&sHeaders, aszDirectories);
	TestPPHeaderCacheUnits(&sHeaders, aszNames, aszUnits, NULL, &aszWithout, &aszTextsWithout);
	TestPPHeaderCacheUnits(&sHeaders, aszNames, aszUnits, pcCache, &aszWith, paszTexts);
	AssertSize(aszWithout.NumElements(), aszWith.NumElements());
	for (i = 0; (i < aszWithout.NumElements()) && (i < aszWith.NumElements()); i++)
	{
		AssertString(aszWithout.GetText(i), aszWith.GetText(i));
		AssertString(aszTextsWithout.GetText(i), paszTexts->GetText(i));
	}
	aszWith.Kill();
	aszTextsWithout.Kill();
	aszWithout.Kill();
	TestPPHeaderCacheEnd(&sHeaders);
}


//////////////////////////////////////////////////////////////////////////
//
// Headers replayed: an include guard and the macro it defines (used after
// it), an #undef and #define of a macro the unit had, and an #if on a
// nested header's macro that depends on the unit's.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCacheReplayed(void)
{
	static const char*	aszDirectories[] = { "", NULL };
	static const char*	aszNames[] = { "Unit1.cpp", "Unit2.cpp", "Unit3.cpp", "Unit4.cpp", NULL };
	static const char*	aszUnits[] = {
		"#include \"Guard.h\"\n#include \"Undef.h\"\nint AfterUndef = TEMP_MACRO;\n#include \"NestedIf.h\"\n",
		"#include \"Guard.h\"\n#include \"Undef.h\"\nint AfterUndef = TEMP_MACRO;\n#include \"NestedIf.h\"\n",
		"#define FEATURE_OFF\n#include \"NestedIf.h\"\n#include \"Guard.h\"\nint AfterGuard = GUARD_VALUE;\n",
		"#define TEMP_MACRO 9\n#include \"Undef.h\"\nint AfterUndef = TEMP_MACRO;\n",
		NULL };
	CPPHeaderCache		cCache;
	CArrayChars			aszTexts;

	cCache.Init();
	TestPPHeaderCacheSame(aszDirectories, aszNames, aszUnits, &cCache, &aszTexts);

	//Unit2 replays Guard.h, Undef.h and NestedIf.h (with Config.h); Unit3 Guard.h (its FEATURE_OFF changes Config.h
	//and NestedIf.h); Unit4 Undef.h (which doesn't depend on TEMP_MACRO: it undefines it first).
	AssertSize(5, cCache.NumHits());
	AssertTrue(aszTexts.NumElements() == 4);
	if (aszTexts.NumElements() == 4)
	{
		AssertTrue(strstr(aszTexts.GetText(1), "AfterUndef = 2") != NULL);
		AssertTrue(strstr(aszTexts.GetText(1), "FeatureOn") != NULL);
		AssertTrue(strstr(aszTexts.GetText(2), "FeatureOff") != NULL);
		AssertTrue(strstr(aszTexts.GetText(2), "AfterGuard = 7") != NULL);
		AssertTrue(strstr(aszTexts.GetText(3), "AfterUndef = 2") != NULL);
	}
	aszTexts.Kill();
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A header whose text changes is forgotten with those that depend on it
// (NestedIf.h included Config.h); one that doesn't (Guard.h) is kept and
// replayed.  The unit preprocessed again sees the new text, as it does
// with no cache.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCacheForget(void)
{
	static const char*			aszDirectories[] = { "", NULL };
	static const char*			aszNames[] = { "Unit1.cpp", NULL };
	static const char*			aszUnits[] = { "#include \"Guard.h\"\n#include \"NestedIf.h\"\n", NULL };
	STestPPHeaderCacheHeaders	sHeaders;
	CPPHeaderCache				cCache;
	CHeaderFile*				pcConfig;
	CArrayChars					aszDumps;
	CArrayChars					aszTexts;
	CArrayChars					aszDumpsWithout;
	CArrayChars					aszTextsWithout;

	TestPPHeaderCacheBegin(&sHeaders, aszDirectories);
	cCache.Init();
	TestPPHeaderCacheUnits(&sHeaders, aszNames, aszUnits, &cCache, &aszDumps, &aszTexts);
	AssertSize(3, cCache.NumHeaders());
	AssertTrue(strstr(aszTexts.GetText(0), "FeatureOn") != NULL);
	aszDumps.Kill();
	aszTexts.Kill();

	pcConfig = sHeaders.acDirectories[0].GetFile((char*)"Config.h");
	AssertNotNull(pcConfig);
	cCache.Forget(pcConfig);
	AssertSize(1, cCache.NumHeaders());
	pcConfig->Unload();
	pcConfig->SetContents("#define FEATURE 0\n");

	TestPPHeaderCacheUnits(&sHeaders, aszNames, aszUnits, &cCache, &aszDumps, &aszTexts);
	AssertSize(1, cCache.NumHits());
	AssertSize(3, cCache.NumHeaders());
	AssertTrue(strstr(aszTexts.GetText(0), "FeatureOff") != NULL);
	AssertTrue(strstr(aszTexts.GetText(0), "Guarded = 7") != NULL);

	TestPPHeaderCacheUnits(&sHeaders, aszNames, aszUnits, NULL, &aszDumpsWithout, &aszTextsWithout);
	AssertString(aszDumpsWithout.GetText(0), aszDumps.GetText(0));
	AssertString(aszTextsWithout.GetText(0), aszTexts.GetText(0));

	aszTextsWithout.Kill();
	aszDumpsWithout.Kill();
	aszTexts.Kill();
	aszDumps.Kill();
	cCache.Kill();
	TestPPHeaderCacheEnd(&sHeaders);
}


//////////////////////////////////////////////////////////////////////////
//
// Headers never kept: one using __COUNTER__, and one ending in a function-
// like macro's name whose arguments are in the unit after it.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCacheNotKept(void)
{
	static const char*	aszDirectories[] = { "", NULL };
	static const char*	aszNames[] = { "Unit1.cpp", "Unit2.cpp", "Unit3.cpp", "Unit4.cpp", NULL };
	static const char*	aszUnits[] = {
		"int Before = __COUNTER__;\n#include \"Counter.h\"\n",
		"int Before = __COUNTER__;\n#include \"Counter.h\"\n",
		"#define CALL(x) ((x) + 1)\n#include \"EndsWithCall.h\"\n(5);\n",
		"#define CALL(x) ((x) + 1)\n#include \"EndsWithCall.h\"\n(5);\n",
		NULL };
	CPPHeaderCache		cCache;
	CArrayChars			aszTexts;

	cCache.Init();
	TestPPHeaderCacheSame(aszDirectories, aszNames, aszUnits, &cCache, &aszTexts);
	AssertSize(0, cCache.NumHeaders());
	AssertSize(0, cCache.NumHits());
	if (aszTexts.NumElements() == 4)
	{
		AssertTrue(strstr(aszTexts.GetText(1), "CounterValue = 1") != NULL);
		AssertTrue(strstr(aszTexts.GetText(3), "Called = ((5) + 1)") != NULL);
	}
	aszTexts.Kill();
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// #include_next goes on from the directory the header was found in: the
// header (and the one it includes next) is replayed.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCacheIncludeNext(void)
{
	static const char*	aszDirectories[] = { "First", "Second", NULL };
	static const char*	aszNames[] = { "Unit1.cpp", "Unit2.cpp", NULL };
	static const char*	aszUnits[] = {
		"#include <Next.h>\n",
		"#include <Next.h>\n",
		NULL };
	CPPHeaderCache		cCache;
	CArrayChars			aszTexts;

	cCache.Init();
	TestPPHeaderCacheSame(aszDirectories, aszNames, aszUnits, &cCache, &aszTexts);
	AssertSize(1, cCache.NumHits());
	if (aszTexts.NumElements() == 2)
	{
		AssertTrue(strstr(aszTexts.GetText(1), "SecondNext") != NULL);
		AssertTrue(strstr(aszTexts.GetText(1), "FirstNext") != NULL);
	}
	aszTexts.Kill();
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A quoted #include in a header looks in the directories of the files
// including it: A/Outer.h's "Only.h" is B's for units in B and C's for
// units in C, so a unit in C doesn't replay it as a unit in B kept it.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCacheIncludersDirectories(void)
{
	static const char*	aszDirectories[] = { "", NULL };
	static const char*	aszNames[] = { "B" _FS_ "Unit1.cpp", "C" _FS_ "Unit2.cpp", "B" _FS_ "Unit3.cpp", NULL };
	static const char*	aszUnits[] = {
		"#include \"A/Outer.h\"\n",
		"#include \"A/Outer.h\"\n",
		"#include \"A/Outer.h\"\n",
		NULL };
	CPPHeaderCache		cCache;
	CArrayChars			aszTexts;

	cCache.Init();
	TestPPHeaderCacheSame(aszDirectories, aszNames, aszUnits, &cCache, &aszTexts);
	AssertSize(1, cCache.NumHits());
	if (aszTexts.NumElements() == 3)
	{
		AssertTrue(strstr(aszTexts.GetText(0), "OnlyB") != NULL);
		AssertTrue(strstr(aszTexts.GetText(1), "OnlyC") != NULL);
		AssertTrue(strstr(aszTexts.GetText(2), "OnlyB") != NULL);
	}
	aszTexts.Kill();
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHeaderCache(void)
{
	BeginTests();

	TestPreprocessorHeaderCacheReplayed();
	TestPreprocessorHeaderCacheNotKept();
	TestPreprocessorHeaderCacheIncludeNext();
	TestPreprocessorHeaderCacheIncludersDirectories();
	TestPreprocessorHeaderCacheForget();

	TestStatistics();
}
