#include <string.h>
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntegerHelper.h"
#include "Assembler65Lib/Asm65Object.h"
#include "CppParserLib/CPP65Builder.h"
#include "CppParserLib/CPP65Generator.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


int32 TestCPP816Run(const char* szSource, bool bLockstep);


//What compiling some units for the W65C816 defined and called.
struct STestCPP816Labels
{
	CArrayChars		aszDefined;		//Global and weak labels, with their register suffixes (_memcpy.ax).
	CArrayChars		aszCalled;		//Labels the assembly units (or with bAllCalls every unit) use and don't define, without suffixes.
	CArrayChars		aszUsed;		//The same with their suffixes.
	CChars			szErrors;
};


//////////////////////////////////////////////////////////////////////////
//
// szName without its register suffix (_memcpy.ax is _memcpy): C++ labels
// have no dot.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816AssemblyBaseName(CChars* psz, const char* szName)
{
	const char*	szDot;

	szDot = strchr(szName, '.');
	psz->Init();
	psz->Append(szName, szDot ? (size)(szDot - szName) : strlen(szName));
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles aszUnits (TestCPP65AddUnits' paths) for the W65C816, the units'
// X816.asm functions left out of their C++ when bAssembly, and gets the
// labels their objects define and use.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816AssemblyLabels(STestCPP816Labels* psLabels, const char** aszUnits, bool bAssembly, bool bAllCalls)
{
	CCPP65Builder		cBuilder;
	bool				bWasAssembly;
	size				i;
	size				j;
	CAsm65Object*		pcObject;
	SAsm65ObjectSymbol*	psSymbol;
	bool				bAssemblyObject;
	CChars				szName;
	CChars				szBase;

	psLabels->aszDefined.Init();
	psLabels->aszCalled.Init();
	psLabels->aszUsed.Init();
	psLabels->szErrors.Init();

	bWasAssembly = CCPP65Generator::IsW65C816Assembly();
	CCPP65Generator::SetW65C816Assembly(bAssembly);
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Compile();
	psLabels->szErrors.Append(cBuilder.GetErrors());

	for (i = 0; i < cBuilder.NumUnitObjects(); i++)
	{
		pcObject = cBuilder.GetUnitObject(i);
		szName.Init(pcObject->GetName());
		bAssemblyObject = szName.EndsWithIgnoreCase(".asm");
		szName.Kill();
		for (j = 0; j < pcObject->NumSymbols(); j++)
		{
			psSymbol = pcObject->GetSymbol(j);
			if (psSymbol->iSection != ASM65_SECTION_UNDEFINED)
			{
				if (psSymbol->eBinding != AB65_Local)
				{
					psLabels->aszDefined.AddIfUnique(psSymbol->szName.Text());
				}
			}
			else if (bAllCalls || bAssemblyObject)
			{
				TestCPP816AssemblyBaseName(&szBase, psSymbol->szName.Text());
				psLabels->aszCalled.AddIfUnique(szBase.Text());
				psLabels->aszUsed.AddIfUnique(psSymbol->szName.Text());
				szBase.Kill();
			}
		}
	}
	psLabels->aszDefined.QuickSort();
	psLabels->aszCalled.QuickSort();

	cBuilder.Kill();
	CCPP65Generator::SetW65C816Assembly(bWasAssembly);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816AssemblyKill(STestCPP816Labels* psLabels)
{
	psLabels->szErrors.Kill();
	psLabels->aszUsed.Kill();
	psLabels->aszCalled.Kill();
	psLabels->aszDefined.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether some label in pasz has szBase's name (any suffix).
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP816AssemblyHasBase(CArrayChars* pasz, const char* szBase)
{
	size	i;
	CChars	sz;
	bool	bFound;

	bFound = false;
	for (i = 0; (i < pasz->NumElements()) && !bFound; i++)
	{
		TestCPP816AssemblyBaseName(&sz, pasz->GetText(i));
		bFound = sz.Equals(szBase);
		sz.Kill();
	}
	return bFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Each label X816.asm (with what's left of X.cpp) defines is one X.cpp
// compiled from C++ defines, register suffix and all: an entry whose suffix
// went stale when the rule or a declaration changed, or whose function is
// gone, is listed (a caller would fail to link, but nothing may call it
// yet).  The assembly needn't define all the C++ does: it keeps helpers to
// itself, and a function it lacks that is called fails to link.  The soft
// float's frame operand entries (.dp) are the assembly's own, and the
// runtime routines (R816_) go with whatever code uses them.  And the C++
// functions the assembly calls are ones the C++ calls too, or defines: a
// name that went stale (a member's, when a second overload of it gives it
// its parameters, _CChars__Init__FPctt) is listed.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816AssemblyReplaces(const char** aszCPlusPlus, const char** aszAssembly)
{
	STestCPP816Labels	sCPlusPlus;
	STestCPP816Labels	sAssembly;
	CChars				szExtra;
	CChars				szStale;
	size				i;
	char*				szLabel;
	CChars				sz;

	TestCPP816AssemblyLabels(&sCPlusPlus, aszCPlusPlus, false, true);
	TestCPP816AssemblyLabels(&sAssembly, aszAssembly, true, false);
	AssertString("", sCPlusPlus.szErrors.Text());
	AssertString("", sAssembly.szErrors.Text());

	szExtra.Init();
	for (i = 0; i < sAssembly.aszDefined.NumElements(); i++)
	{
		szLabel = sAssembly.aszDefined.GetText(i);
		sz.Init(szLabel);
		if (!sCPlusPlus.aszDefined.Contains(szLabel) && !sz.EndsWith(".dp") && !sz.StartsWith("R816_"))
		{
			szExtra.Append(szLabel)->Append(" ");
		}
		sz.Kill();
	}

	//C++ labels (mangled: a __ after the name) the assembly calls.
	szStale.Init();
	for (i = 0; i < sAssembly.aszCalled.NumElements(); i++)
	{
		szLabel = sAssembly.aszCalled.GetText(i);
		if ((strstr(szLabel + 1, "__") != NULL) && !sCPlusPlus.aszCalled.Contains(szLabel) && !TestCPP816AssemblyHasBase(&sCPlusPlus.aszDefined, szLabel))
		{
			szStale.Append(szLabel)->Append(" ");
		}
	}

	AssertString("", szExtra.Text());
	AssertString("", szStale.Text());

	szStale.Kill();
	szExtra.Kill();
	TestCPP816AssemblyKill(&sAssembly);
	TestCPP816AssemblyKill(&sCPlusPlus);
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's FAT (Fat*816.asm) and Libc65's soft float (softfloat816.asm,
// for softfloat.cpp and math.cpp) against the C++ they replace.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AssemblyReplacesCPlusPlus(void)
{
	static const char*	aszNames[] = { "FatClusterCache", "FatFile", "FatFilenameHelper", "FatInfoSectorCache", "FatVolume", NULL };
	const char*			aszCPlusPlus[2];
	const char*			aszAssembly[3];
	CChars				szCPlusPlus;
	CChars				szAssembly;
	size				i;

	for (i = 0; aszNames[i]; i++)
	{
		szCPlusPlus.Init("BaseLib/");
		szCPlusPlus.Append(aszNames[i])->Append(".cpp");
		szAssembly.Init("BaseLib/");
		szAssembly.Append(aszNames[i])->Append("816.asm");
		aszCPlusPlus[0] = szCPlusPlus.Text();
		aszCPlusPlus[1] = NULL;
		aszAssembly[0] = szCPlusPlus.Text();
		aszAssembly[1] = szAssembly.Text();
		aszAssembly[2] = NULL;
		TestCPP816AssemblyReplaces(aszCPlusPlus, aszAssembly);
		szAssembly.Kill();
		szCPlusPlus.Kill();
	}

	static const char*	aszFloatCPlusPlus[] = { "Libc65/softfloat.cpp", "Libc65/math.cpp", NULL };
	static const char*	aszFloatAssembly[] = { "Libc65/softfloat.cpp", "Libc65/math.cpp", "Libc65/softfloat816.asm", NULL };
	TestCPP816AssemblyReplaces(aszFloatCPlusPlus, aszFloatAssembly);
}


//////////////////////////////////////////////////////////////////////////
//
// Libc65's string816.asm and string816_16.asm have no C++ on the W65C816:
// a unit calling each of their functions as declared uses each register
// entry the assembly defines, by the same name (the suffix the fixed rule
// gives the declaration).  An entry gone stale, or one this unit doesn't
// call yet, is listed.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AssemblyStringEntries(void)
{
	static const char*	aszCalls[] = { "Input/CPP65/AssemblyEntries/StringCalls.cpp", NULL };
	static const char*	aszAssembly[] = { "Libc65/string816.asm", "Libc65/string816_16.asm", NULL };
	STestCPP816Labels	sCalls;
	STestCPP816Labels	sAssembly;
	CChars				szStale;
	size				i;
	char*				szLabel;

	TestCPP816AssemblyLabels(&sCalls, aszCalls, true, true);
	TestCPP816AssemblyLabels(&sAssembly, aszAssembly, true, false);
	AssertString("", sCalls.szErrors.Text());
	AssertString("", sAssembly.szErrors.Text());

	//Each register entry (a suffixed label) the assembly defines, called by that name.
	szStale.Init();
	for (i = 0; i < sAssembly.aszDefined.NumElements(); i++)
	{
		szLabel = sAssembly.aszDefined.GetText(i);
		if ((strchr(szLabel, '.') != NULL) && !sCalls.aszUsed.Contains(szLabel))
		{
			szStale.Append(szLabel)->Append(" ");
		}
	}
	AssertString("", szStale.Text());

	szStale.Kill();
	TestCPP816AssemblyKill(&sAssembly);
	TestCPP816AssemblyKill(&sCalls);
}


//////////////////////////////////////////////////////////////////////////
//
// string816.asm's large bit array finders, run on the W65C816 directly
// (their register entries) and through function pointers (their stack
// entries), against BaseLib's natively: an entry's argument offsets gone
// stale give the wrong bit.  (The others are run by TestCPP816's string
// tests.)  Each bit of the result is one that went wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AssemblyLargeBitFinders(void)
{
	uint8	aui[8] = { 0xFF, 0xEF, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 };
	CChars	szSource;

	szSource.Init(
		"unsigned long FindFirstSetBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned long FindFirstClearBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned long FindLastSetBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned long FindLastClearBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned long (*volatile gpFirstSet)(void*, unsigned long) = FindFirstSetBitLarge;\n"
		"unsigned long (*volatile gpFirstClear)(void*, unsigned long) = FindFirstClearBitLarge;\n"
		"unsigned long (*volatile gpLastSet)(void*, unsigned long) = FindLastSetBitLarge;\n"
		"unsigned long (*volatile gpLastClear)(void*, unsigned long) = FindLastClearBitLarge;\n"
		"unsigned char gauc[8] = { 0xFF, 0xEF, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00 };\n"
		"volatile unsigned long guiBits = 64;\n"
		"int main(void)\n"
		"{\n"
		"\tint w = 0;\n");
	szSource.Append("\tif (FindFirstSetBitLarge(gauc, guiBits) != ")->Append((int)FindFirstSetBitLarge(aui, 64))->Append("UL) { w |= 1; }\n");
	szSource.Append("\tif (gpFirstSet(gauc, guiBits) != ")->Append((int)FindFirstSetBitLarge(aui, 64))->Append("UL) { w |= 2; }\n");
	szSource.Append("\tif (FindFirstClearBitLarge(gauc, guiBits) != ")->Append((int)FindFirstClearBitLarge(aui, 64))->Append("UL) { w |= 4; }\n");
	szSource.Append("\tif (gpFirstClear(gauc, guiBits) != ")->Append((int)FindFirstClearBitLarge(aui, 64))->Append("UL) { w |= 8; }\n");
	szSource.Append("\tif (FindLastSetBitLarge(gauc, guiBits) != ")->Append((int)FindLastSetBitLarge(aui, 64))->Append("UL) { w |= 16; }\n");
	szSource.Append("\tif (gpLastSet(gauc, guiBits) != ")->Append((int)FindLastSetBitLarge(aui, 64))->Append("UL) { w |= 32; }\n");
	szSource.Append("\tif (FindLastClearBitLarge(gauc, guiBits) != ")->Append((int)FindLastClearBitLarge(aui, 64))->Append("UL) { w |= 64; }\n");
	szSource.Append("\tif (gpLastClear(gauc, guiBits) != ")->Append((int)FindLastClearBitLarge(aui, 64))->Append("UL) { w |= 128; }\n");
	szSource.Append("\treturn w;\n}\n");

	AssertInt(0, TestCPP816Run(szSource.Text(), false));
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// string816.asm's memcpy copies up to 32 bytes itself and more with the
// block move: every size either side of that, odd and even, from odd and
// even addresses, with the bytes either side of the destination left; one
// through its stack entry (a pointer), one across a bank (the small copy
// indexes long pointers) and one long.  Each bit of the result is one
// that went wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AssemblyMemcpySizes(void)
{
	AssertInt(0, TestCPP816Run(
		"#include <string.h>\n"
		"volatile unsigned int guiMost = 40;\n"
		"unsigned char gaucSource[48];\n"
		"unsigned char gaucDest[48];\n"
		"void* (*volatile gpMemcpy)(void*, const void*, size_t) = memcpy;\n"
		"int Check(unsigned int uiAt, unsigned int uiFrom, unsigned int n)\n"
		"{\n"
		"\tunsigned int i;\n"
		"\tfor (i = 0; i < 48; i++)\n"
		"\t{\n"
		"\t\tif (gaucDest[i] != (((i >= uiAt) && (i < uiAt + n)) ? gaucSource[i - uiAt + uiFrom] : 0xEE)) { return 1; }\n"
		"\t}\n"
		"\treturn 0;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tint w = 0;\n"
		"\tunsigned int i, n, o;\n"
		"\tunsigned char* pucSource = (unsigned char*)0x13FFF0;\n"
		"\tunsigned char* pucDest = (unsigned char*)0x14FFF5;\n"
		"\tfor (i = 0; i < 48; i++) { gaucSource[i] = (unsigned char)(i * 7 + 1); }\n"
		"\tfor (o = 0; o < 2; o++)\n"
		"\t{\n"
		"\t\tfor (n = 0; n <= guiMost; n++)\n"
		"\t\t{\n"
		"\t\t\tmemset(gaucDest, 0xEE, 48);\n"
		"\t\t\tif (memcpy(gaucDest + o + 1, gaucSource + 1 - o, n) != gaucDest + o + 1) { w |= 1; }\n"
		"\t\t\tw |= Check(o + 1, 1 - o, n) << 1;\n"
		"\t\t}\n"
		"\t}\n"
		"\tmemset(gaucDest, 0xEE, 48);\n"
		"\tif (gpMemcpy(gaucDest + 2, gaucSource + 5, 3) != gaucDest + 2) { w |= 4; }\n"
		"\tw |= Check(2, 5, 3) << 3;\n"
		"\tfor (i = 0; i < 32; i++) { pucSource[i] = (unsigned char)(i + 3); pucDest[i] = 0; }\n"
		"\tmemcpy(pucDest, pucSource, 31);\n"
		"\tfor (i = 0; i < 32; i++) { if (pucDest[i] != ((i < 31) ? i + 3 : 0)) { w |= 16; } }\n"
		"\tmemset(gaucDest, 0xEE, 48);\n"
		"\tmemcpy(gaucDest + 1, gaucSource, 46);\n"
		"\tw |= Check(1, 0, 46) << 5;\n"
		"\treturn w;\n"
		"}\n", false));
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816 assembly that stands in for C++ against the compiler: its
// names compared without running anything, and the entries no other test
// runs run once, so it's quick, where TestSupportLib65 and TestBaseLib65
// find such things only by running their programs.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Assembly(void)
{
	BeginTests();

	TestCPP816AssemblyStringEntries();
	TestCPP816AssemblyReplacesCPlusPlus();
	TestCPP816AssemblyLargeBitFinders();
	TestCPP816AssemblyMemcpySizes();

	TestStatistics();
}
