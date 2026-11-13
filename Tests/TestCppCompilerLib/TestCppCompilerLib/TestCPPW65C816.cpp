#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Assembler65Lib/AsmW65C816Profile.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/TextFile.h"
#include "MicroprocessorLib/W65C816InstructionFactory.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/IRW65C816Backend.h"
#include "CppCompilerLib/CPP65IRFrontEnd.h"
#include "CppCompilerLib/CPP65Generator.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//////////////////////////////////////////////////////////////////////////
//
// C++ compiled for the W65C816 (CCPP65Builder with CPPT_W65C816): the generator's C++, the IR front end and
// CIRW65C816Backend, linked with the W65C816's start up and runtime routines and run on CFastW65C816.
//
//////////////////////////////////////////////////////////////////////////


//How the programs the helpers below build are compiled: the defaults, but for what a test sets for the builds it makes
//(and sets back).
static CCPPCompileOptions	gcTestCPP816Options = *GetDefaultCPPCompileOptions();


//The native results TestCPP65Generator.cpp's FloatHelper and Quaternion tests check against.
extern CArrayInt	gaiFloatHelperResults;
extern CArrayInt	gaiQuaternionResults;
extern CArrayInt	gaiQuaternionIsFloat;
void	FloatHelperExpected(void);
void	QuaternionExpected(void);
uint64	TestCPP65FloatUlps(uint32 uiActual, uint32 uiExpected);


#define TEST_CPP816_STEPS		50000000
#define TEST_CPP816_SLOW_STEPS	500000000
#define TEST_CPP816_RESULTS		0x100000
#define TEST_CPP816_BASELIB_MEMORY	0x1000000
#define TEST_CPP816_BASELIB_LIMIT	0x1C0000


//Libc65's units for the W65C816 (no 65C64 assembly).
static const char* gaszTestCPP816Libc65[] = { "string.cpp", "stringW65C816.asm", "stringW65C816_16.asm", "stdlib.cpp", "ctype.cpp", "time.cpp", "math.cpp", "softfloat.cpp", "softfloatW65C816.asm", "new.cpp", "signal.cpp", "stdio.cpp", "host.cpp", NULL };


//////////////////////////////////////////////////////////////////////////
//
// Runs the program a few instructions at a time and counts where it is after each run by the function (the symbol
// in the linker's map at or before the address), then prints the functions it was most often in (W65C816.txt step
// 7).  Returns how many instructions ran.
//
//////////////////////////////////////////////////////////////////////////
size TestCPP816Profile(CCPP65Builder* pcBuilder, CFastW65C816* pcFast, size uiMaxSteps)
{
	CArrayTemplate<uint32>	auiAddresses;
	CArrayTemplate<int32>	aiCounts;
	CArrayChars				aszNames;
	const char*				szLine;
	const char*				szName;
	uint32					uiAddress;
	size					uiSteps;
	size					uiRun;
	size					ui;
	size					uiBest;
	int32					iZero;
	int32					iTotal;
	int						iShown;

	//The map's functions in address order: "  0x00012345 G Name (object)".
	auiAddresses.Init();
	aszNames.Init();
	aiCounts.Init();
	iZero = 0;
	for (szLine = strstr(pcBuilder->GetMap(), "Symbols\n"); szLine; szLine = strchr(szLine + 1, '\n'))
	{
		if ((szLine[1] == ' ') && (szLine[3] == '0') && (szLine[4] == 'x') && ((szLine[14] == 'G') || ((szLine[14] == 'L') && !((szLine[16] == 'L') && (szLine[17] >= '0') && (szLine[17] <= '9')))))
		{
			uiAddress = (uint32)strtoul(&szLine[3], NULL, 16);
			szName = &szLine[16];
			auiAddresses.Add(&uiAddress);
			aszNames.Add(szName, 0, (size)(strchr(szName, ' ') - szName));
			aiCounts.Add(&iZero);
		}
	}

	uiSteps = 0;
	iTotal = 0;
	while ((uiSteps < uiMaxSteps) && !pcFast->IsStopped())
	{
		uiRun = pcFast->Run(97);
		uiSteps += uiRun;
		if (uiRun == 0)
		{
			break;
		}
		uiAddress = pcFast->GetProgramAddress();
		uiBest = (size)-1;
		for (ui = 0; ui < auiAddresses.NumElements(); ui++)
		{
			if ((*auiAddresses.Get(ui) <= uiAddress) && ((uiBest == (size)-1) || (*auiAddresses.Get(ui) >= *auiAddresses.Get(uiBest))))
			{
				uiBest = ui;
			}
		}
		if (uiBest != (size)-1)
		{
			(*aiCounts.Get(uiBest))++;
			iTotal++;
		}
	}

	for (iShown = 0; iShown < 12; iShown++)
	{
		uiBest = 0;
		for (ui = 1; ui < aiCounts.NumElements(); ui++)
		{
			if (*aiCounts.Get(ui) > *aiCounts.Get(uiBest))
			{
				uiBest = ui;
			}
		}
		if ((aiCounts.NumElements() == 0) || (*aiCounts.Get(uiBest) == 0))
		{
			break;
		}
		fprintf(stderr, "CPP816_PROFILE %5.1f%% %s\n", 100.0 * *aiCounts.Get(uiBest) / (iTotal ? iTotal : 1), aszNames.GetText(uiBest));
		*aiCounts.Get(uiBest) = 0;
	}
	aiCounts.Kill();
	aszNames.Kill();
	auiAddresses.Kill();
	return uiSteps;
}


//Memory a test program reads and writes: puiLoad's uiLoadBytes are put at uiLoadAddress before it runs, and the
//uiSaveBytes at uiSaveAddress copied to puiSave after.
struct STestCPP816Memory
{
	const uint8*	puiLoad;
	size			uiLoadBytes;
	uint32			uiLoadAddress;
	uint8*			puiSave;
	size			uiSaveBytes;
	uint32			uiSaveAddress;
};


//////////////////////////////////////////////////////////////////////////
//
// Compiles and links szSource (if given) and the units (TestCPP65AddUnits, without the 65C64 assembly ones) for
// the W65C816 and runs it in CAsm65Memory's 2MB (with psMemory's bytes put in and taken out, if given).  The labels
// in szStubs must be exactly the ones used but defined by no unit.  The first uiResults longs at 0010:0000 are copied
// to puiResults.  It may run uiMaxSteps instructions.  Returns main's int (16 bits, sign extended), or -1000000 if it
// didn't build or stop.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP816RunProject(const char** aszUnits, const char* szStubs, const char* szSource, bool bLockstep = false, uint32* puiResults = NULL, size uiResults = 0, size uiMaxSteps = TEST_CPP816_STEPS, bool bBaseLib = false, STestCPP816Memory* psMemory = NULL)
{
	CCPP65Builder	cBuilder;
	CAsm65Memory	cMemory;
	CFastW65C816		cFast;
	CChars			szMissing;
	CChars			szProject;
	CFileUtil		cFileUtil;
	size			uiSteps;
	size			i;
	int32			iResult;
	bool			bResult;

	iResult = -1000000;
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	bResult = cBuilder.AddStartUp();
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	if (szSource)
	{
		bResult &= cBuilder.AddSource("Test.cpp", szSource);
	}
	if (aszUnits)
	{
		TestCPP65AddUnits(&cBuilder, aszUnits, false);
	}
	if (bBaseLib)
	{
		//All of BaseLib and Libc65, each unit linked only when it is used.
		szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ "Libraries" _FS_ "BaseLib" _FS_ "BaseLib.vcxproj");
		cFileUtil.FullPath(&szProject);
		bResult &= cBuilder.AddVisualStudioProject("BaseLib", szProject.Text(), true);
		szProject.Kill();
		szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szProject);
		for (i = 0; gaszTestCPP816Libc65[i]; i++)
		{
			cBuilder.AddUnit("Libc65", szProject.Text(), gaszTestCPP816Libc65[i], true);
		}
		szProject.Kill();

		//A program with BaseLib in it is bigger: it ends below the stack, with the heap above 2MB in 16MB.
		cBuilder.SetLimit(TEST_CPP816_BASELIB_LIMIT);
	}
	bResult &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bResult);
	szMissing.Init();
	cBuilder.GetMissingNames(&szMissing);
	AssertString(szStubs, szMissing.Text());
	szMissing.Kill();
	if (bResult)
	{
		cMemory.Init(bBaseLib ? TEST_CPP816_BASELIB_MEMORY : ASM65_MEMORY_LOW_SIZE);
		cMemory.Load(cBuilder.GetOutput());
		for (i = 0; psMemory && (i < psMemory->uiLoadBytes); i++)
		{
			cMemory.SetByte(psMemory->uiLoadAddress + (uint32)i, psMemory->puiLoad[i]);
		}
		cMemory.InitFast65816(&cFast);
		cFast.Reset();
		if (bLockstep)
		{
			uiSteps = TestCPP816RunLockstepChecked(&cMemory, &cFast, uiMaxSteps);
		}
		else if (getenv("CPP816_PROFILE"))
		{
			uiSteps = TestCPP816Profile(&cBuilder, &cFast, uiMaxSteps);
		}
		else
		{
			uiSteps = TestCPP816RunChecked(&cMemory, &cFast, uiMaxSteps);
		}
		AssertTrue(uiSteps < uiMaxSteps);
		if (getenv("CPP816_STEPS"))
		{
			//How fast the code is (W65C816.txt step 7).
			fprintf(stderr, "CPP816_STEPS %s: %d instructions %d cycles\n", (aszUnits && aszUnits[0]) ? aszUnits[0] : "source", (int)uiSteps, (int)cFast.NumCycles());
		}
		AssertTrue(cFast.IsStopped());
		AssertInt(0, (int)cMemory.NumBadAccesses());
		if (cFast.IsStopped())
		{
			iResult = (int16)cFast.GetC();
			AssertIntHex(GetCPPTarget(CPPT_W65C816)->GetStackTop(), (uint32)cFast.GetS());
		}
		for (i = 0; i < uiResults; i++)
		{
			puiResults[i] = cMemory.GetLong(TEST_CPP816_RESULTS + 4 * (uint32)i);
		}
		for (i = 0; psMemory && (i < psMemory->uiSaveBytes); i++)
		{
			psMemory->puiSave[i] = cMemory.GetByte(psMemory->uiSaveAddress + (uint32)i);
		}
		cFast.Kill();
		cMemory.Kill();
	}
	cBuilder.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles, links and runs szSource, with Libc65's string routines (the optimiser makes loops calls of them).
// Returns main's int (16 bits, sign extended), or -1000000 if it didn't build or stop.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP816Run(const char* szSource, bool bLockstep = false)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };

	return TestCPP816RunProject(aszUnits, "", szSource, bLockstep);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Basics(void)
{
	AssertInt(42, TestCPP816Run("int main(void) { return 42; }\n", true));
	AssertInt(-5, TestCPP816Run("int main(void) { int a = 7; int b = 12; return a - b; }\n"));

	//int is 16 bits: it wraps, and sizeof says so.
	AssertInt(2, TestCPP816Run("int main(void) { return sizeof(int); }\n"));
	AssertInt(4, TestCPP816Run("int main(void) { return sizeof(long) + sizeof(short) - 2; }\n"));
	AssertInt(-32768, TestCPP816Run("int main(void) { int a = 32767; a = a + 1; return a; }\n"));
	AssertInt(1 + 2, TestCPP816Run("unsigned guWrapped = 65535u + 1u;\nunsigned guAll = 0u - 1;\nint main(void) { return (guWrapped == 0) + (guAll == 65535u) * 2; }\n"));

	//long is 32 bits.
	AssertInt(1234, TestCPP816Run("int main(void) { long a = 100000; long b = a * 3 + 1234; return (int)(b - 300000); }\n", true));
	AssertInt(3, TestCPP816Run("int main(void) { long a = 1000000; long b = 300000; return (int)(a / b); }\n"));

	//Libc65's headers: the 32 bit types are long, size_t is 32 bits and INT_MAX is 16 bits'.
	AssertInt(4 * 1000 + 4 * 100 + 2 * 10 + 1, TestCPP816Run(
		"#include <stdint.h>\n"
		"#include <stddef.h>\n"
		"#include <limits.h>\n"
		"int main(void)\n"
		"{\n"
		"\tint32_t i = INT32_MAX;\n"
		"\treturn sizeof(i) * 1000 + sizeof(size_t) * 100 + sizeof(int16_t) * 10 + (INT_MAX == 32767);\n"
		"}\n"));

	//Signed / and % by powers of 2 are shifts and adds (negative values round towards 0); the sum was worked out natively.
	AssertInt(7, TestCPP816Run(
		"long Mix(long s, int x) { return s * 3 + x / 2 + x % 2 + x / 4 * 5 + x % 4 * 7 + x / 8 + x % 16 * 11; }\n"
		"long MixLong(long s, long l) { return s * 5 + l / 16 + l % 16 + l / 1024 + l % 2; }\n"
		"int main()\n"
		"{\n"
		"	long s = 0;\n"
		"	for (int i = -1000; i <= 1000; i++) { s = Mix(s, i) & 0xFFFFFFF; }\n"
		"	for (int i = -1000; i <= 1000; i += 7) { s = MixLong(s, (long)i * 99991) & 0xFFFFFFF; }\n"
		"	int x = -32767 - 1; s += x / 2 + x % 4 + x / 16384; x = 32767; s += x / 2 + x % 4 + x / 16384;\n"
		"	long l = -2147483647L - 1; s += l / 2 + l % 8;\n"
		"	return ((s & 0xFFFFFFF) == 259971084L) ? 7 : 1;\n"
		"}\n"));

	//An expression's type is the one its code has.  int + unsigned int is an unsigned int, also when the unsigned
	//int is a constant variable (they are the same width); pointer - pointer is as wide as a pointer, a long.  The
	//overload called and sizeof both show it.
	AssertInt(2 * 1000 + 3 * 100 + 2 * 10 + 4, TestCPP816Run(
		"int F(int i) { return 1; }\n"
		"int F(unsigned int u) { return 2; }\n"
		"int F(long l) { return 3; }\n"
		"int main(void)\n"
		"{\n"
		"\tconst unsigned int u = 5;\n"
		"\tint x = 3;\n"
		"\tchar a[4];\n"
		"\treturn F(x + u) * 1000 + F(&a[3] - &a[0]) * 100 + sizeof(x + u) * 10 + sizeof(&a[3] - &a[0]);\n"
		"}\n"));

	//Pointer arithmetic steps by the size of the class pointed to (and pointer - pointer divides by it), also when
	//it is the first thing in the program that needs the class laid out.
	AssertInt(2 * 100 + 3 * 10 + 4, TestCPP816Run(
		"struct SEntry { int a; int b; };\n"
		"int Count(SEntry* psStart, SEntry* psEnd) { return (int)(psEnd - psStart); }\n"
		"int Last(SEntry* p, int n) { int i; for (i = 0; i < n; i++) { if (i > 0) { p++; } } return p->b; }\n"
		"int Skip(SEntry* p) { p += 2; return p->a; }\n"
		"int main() { SEntry a[3]; a[0].b = 1; a[1].b = 2; a[2].b = 3; a[2].a = 4; return Count(&a[0], &a[2]) * 100 + Last(a, 3) * 10 + Skip(a); }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Loops, arrays, pointers, functions and recursion.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Functions(void)
{
	AssertInt(55, TestCPP816Run(
		"int Fibonacci(int n)\n"
		"{\n"
		"\tif (n < 2) { return n; }\n"
		"\treturn Fibonacci(n - 1) + Fibonacci(n - 2);\n"
		"}\n"
		"int main(void) { return Fibonacci(10); }\n", true));

	AssertInt(150, TestCPP816Run(
		"int gaiTable[5] = { 10, 20, 30, 40, 50 };\n"
		"int Sum(int* pi, int iCount)\n"
		"{\n"
		"\tint iSum = 0;\n"
		"\tfor (int i = 0; i < iCount; i++) { iSum += pi[i]; }\n"
		"\treturn iSum;\n"
		"}\n"
		"int main(void) { return Sum(gaiTable, 5); }\n"));

	AssertInt(11, TestCPP816Run(
		"int Length(const char* sz)\n"
		"{\n"
		"\tint i = 0;\n"
		"\twhile (sz[i] != 0) { i++; }\n"
		"\treturn i;\n"
		"}\n"
		"int main(void) { return Length(\"Hello world\"); }\n"));

	AssertInt(321, TestCPP816Run(
		"void Reverse(char* sz, int iLength)\n"
		"{\n"
		"\tfor (int i = 0; i < iLength / 2; i++)\n"
		"\t{\n"
		"\t\tchar c = sz[i];\n"
		"\t\tsz[i] = sz[iLength - 1 - i];\n"
		"\t\tsz[iLength - 1 - i] = c;\n"
		"\t}\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tchar sz[4] = { '1', '2', '3', 0 };\n"
		"\tReverse(sz, 3);\n"
		"\treturn (sz[0] - '0') * 100 + (sz[1] - '0') * 10 + (sz[2] - '0');\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Classes: fields, member functions, constructors and a virtual call.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Classes(void)
{
	AssertInt(7 * 3 + 2, TestCPP816Run(
		"class CPoint\n"
		"{\n"
		"public:\n"
		"\tint\tmiX;\n"
		"\tint\tmiY;\n"
		"\tCPoint(int iX, int iY) { miX = iX; miY = iY; }\n"
		"\tint Dot(CPoint* psOther) { return miX * psOther->miX + miY * psOther->miY; }\n"
		"};\n"
		"int main(void)\n"
		"{\n"
		"\tCPoint a(7, 1);\n"
		"\tCPoint b(3, 2);\n"
		"\treturn a.Dot(&b);\n"
		"}\n", true));

	AssertInt(10 + 200, TestCPP816Run(
		"class CShape\n"
		"{\n"
		"public:\n"
		"\tvirtual int Area(void) { return 0; }\n"
		"};\n"
		"class CSquare : public CShape\n"
		"{\n"
		"public:\n"
		"\tint\tmiSide;\n"
		"\tCSquare(int iSide) { miSide = iSide; }\n"
		"\tint Area(void) override { return miSide * miSide; }\n"
		"};\n"
		"class CBar : public CShape\n"
		"{\n"
		"public:\n"
		"\tint\tmiLength;\n"
		"\tCBar(int iLength) { miLength = iLength; }\n"
		"\tint Area(void) override { return miLength; }\n"
		"};\n"
		"int Total(CShape* pcA, CShape* pcB) { return pcA->Area() + pcB->Area(); }\n"
		"int main(void)\n"
		"{\n"
		"\tCBar cBar(10);\n"
		"\tCSquare cSquare(14);\n"
		"\treturn Total(&cBar, &cSquare) + 4;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A delegating constructor (C() : C(7)) has the constructor it names make
// the object, base, vtable pointer and fields included, then runs its own
// body; on both processors.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DelegatingConstructors(void)
{
	const char*	szSource;

	szSource =
		"class CBase\n"
		"{\n"
		"public:\n"
		"\tint\tmiBase;\n"
		"\tCBase(int i) : miBase(i) {}\n"
		"\tvirtual int Get(void) { return miBase; }\n"
		"};\n"
		"class CDerived : public CBase\n"
		"{\n"
		"public:\n"
		"\tint\tmiA;\n"
		"\tint\tmiB = 5;\n"
		"\tCDerived(int a) : CBase(a * 10), miA(a) {}\n"
		"\tCDerived() : CDerived(7) { miA += 1; }\n"
		"\tCDerived(int a, int b) : CDerived(a + b) {}\n"
		"\tint Get(void) override { return miBase + miA + miB; }\n"
		"};\n"
		"int main(void)\n"
		"{\n"
		"\tCDerived\tc;\n"
		"\tCDerived\td(1, 2);\n"
		"\tCBase*\t\tp;\n"
		"\tp = &c;\n"
		"\treturn p->Get() * 100 + d.Get();\n"
		"}\n";
	AssertInt((70 + 8 + 5) * 100 + (30 + 3 + 5), TestCPP816Run(szSource, true));
	AssertInt((70 + 8 + 5) * 100 + (30 + 3 + 5), TestCPP65Main(szSource));
}


//////////////////////////////////////////////////////////////////////////
//
// Bit-fields share a unit of their type's size, from its lowest bit, as
// MSVC lays them out: a new unit for another size or when the bits run out
// or after a zero width one.  Reads extend a signed one's top bit; writes
// keep the unit's other bits; an assignment's value is what the bit-field
// keeps.  An aggregate initialiser leaves out unnamed ones, and a const
// reference binds to a copy.  A bit-field has no address.  On both
// processors (S is 6 bytes on the W65C816, 12 on the 65C64).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BitFields(void)
{
	const char*	szSource;
	CChars		szAssembly;
	CChars		szErrors;

	szSource =
		"struct S\n"
		"{\n"
		"\tunsigned\ta : 3;\n"
		"\tunsigned\tb : 5;\n"
		"\tint\t\t\tc : 4;\n"
		"\tunsigned\t: 0;\n"
		"\tunsigned\td : 2;\n"
		"\tbool\t\te : 1;\n"
		"};\n"
		"struct T\n"
		"{\n"
		"\tunsigned char\tx : 4;\n"
		"\tunsigned char\ty : 4;\n"
		"};\n"
		"int main(void)\n"
		"{\n"
		"\tS\t\t\t\ts = { 9, 31, 7, 3, true };\n"
		"\tT\t\t\t\tt;\n"
		"\tint\t\t\t\tr;\n"
		"\tint\t\t\t\tx;\n"
		"\tunsigned char*\tpc;\n"
		"\tr = 0;\n"
		"\ts.c += 2;\n"
		"\tr += s.a;\n"
		"\tr += s.b * 10;\n"
		"\tr += s.c * 100;\n"
		"\ts.d++;\n"
		"\tr += s.d * 1000;\n"
		"\tr += s.e ? 2000 : 0;\n"
		"\tx = (s.b = 33);\n"
		"\tr += x * 4000;\n"
		"\tt.x = 5;\n"
		"\tt.y = 12;\n"
		"\tpc = (unsigned char*)&t;\n"
		"\tr += (*pc == 0xC5) ? 8000 : 0;\n"
		"\tconst int& rc = s.c;\n"
		"\tr += rc;\n"
		"\treturn r + (int)sizeof(S);\n"
		"}\n";
	AssertInt(1 + 310 - 700 + 0 + 2000 + 4000 + 8000 - 7 + 6, TestCPP816Run(szSource, true));
	AssertInt(1 + 310 - 700 + 0 + 2000 + 4000 + 8000 - 7 + 12, TestCPP65Main(szSource));

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "struct S { unsigned a : 3; };\nunsigned* F(S* p) { return &p->a; }\n"));
	AssertTrue(szErrors.Contains("A bit-field has no address"));
	szErrors.Kill();
	szAssembly.Kill();

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "struct S { unsigned a : 40; unsigned b : 0; };\n int F(S* p) { return p->a; }\n"));
	AssertTrue(szErrors.Contains("A bit-field can't be wider than its type"));
	AssertTrue(szErrors.Contains("A named bit-field can't have a width of 0"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// array[index] of an array (not a pointer) under 64KB has its offset worked
// out in 16 bits (CCPP65IRFrontEnd::NarrowSubscript): 2D member arrays, local
// and global arrays, 6 byte elements and char, int and 32 bit indices; a
// pointer into an array still takes negative indices.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NarrowSubscripts(void)
{
	AssertInt(5 + 60 + 700 + 8000 + 3 + 40, TestCPP816Run(
		"struct SSix { short a; short b; short c; };\n"
		"class CTable\n"
		"{\n"
		"public:\n"
		"\tvoid*\tmapv[27][16];\n"
		"\tlong\tmal[30];\n"
		"\tSSix\tmas[200];\n"
		"};\n"
		"char gac[300];\n"
		"int main(void)\n"
		"{\n"
		"\tCTable\t\tcTable;\n"
		"\tint\t\t\tai[10];\n"
		"\tint\t\t\ti;\n"
		"\tchar\t\tc;\n"
		"\tunsigned long\tul;\n"
		"\tint*\t\tpi;\n"
		"\tfor (i = 0; i < 10; i++) { ai[i] = i; }\n"
		"\tfor (i = 0; i < 200; i++) { cTable.mas[i].b = (short)(i * 3); }\n"
		"\tfor (i = 0; i < 300; i++) { gac[i] = (char)(i & 0x7F); }\n"
		"\tcTable.mapv[26][15] = &ai[5];\n"
		"\tc = 20;\n"
		"\tul = 29;\n"
		"\tcTable.mal[ul] = 700;\n"
		"\tpi = &ai[6];\n"
		"\treturn *(int*)cTable.mapv[26][15] + cTable.mas[c].b + (int)cTable.mal[ul] + (gac[i - 1] == 43) * 8000 + pi[-3] + ai[(int)(ul - 25)] * 10;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes that only byte arithmetic, logic and byte stores read aren't masked
// when they are worked out (CIRW65C816Backend::FindLowByteUses): the bytes
// stored are right, with the destination one of the sources too, and the
// sums of them (which read whole words) are.  Signed 32 bit comparisons of
// two values are a subtraction's sign (SignedLessFlags), right at both ends
// of the range, where it overflows; if (a < b) ... else if (a > b) keeps
// the compare whose Z the second reads.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ByteMasksAndSignedCompares(void)
{
	AssertInt(255, TestCPP816Run(
		"typedef unsigned char uint8;\n"
		"typedef unsigned int uint16;\n"
		"typedef long int32;\n"
		"uint8 gauiSource[64];\n"
		"uint8 gauiDest[64];\n"
		"void Mix(uint8* puiDest, const uint8* puiSource, uint16 uiCount, uint8 uiKey)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tfor (i = 0; i < uiCount; i++)\n"
		"\t{\n"
		"\t\tpuiDest[i] = (uint8)((puiSource[i] * 7 + (i / 3)) & 0xFF) ^ uiKey;\n"
		"\t}\n"
		"}\n"
		"void Blend(uint8* puiDest, const uint8* puiA, const uint8* puiB, uint16 uiCount)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tfor (i = 0; i < uiCount; i++)\n"
		"\t{\n"
		"\t\tpuiDest[i] = (uint8)(puiA[i] + puiB[i] - 3);\n"
		"\t\tpuiDest[i] = (uint8)(~puiDest[i] | (puiA[i] & puiB[i]));\n"
		"\t}\n"
		"}\n"
		"uint16 Sum(const uint8* pui, uint16 uiCount)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tuint16\tuiSum;\n"
		"\tuiSum = 0;\n"
		"\tfor (i = 0; i < uiCount; i++) { uiSum += pui[i]; }\n"
		"\treturn uiSum;\n"
		"}\n"
		"int Less(int32 a, int32 b) { if (a < b) { return 1; } return 0; }\n"
		"int NotMore(int32 a, int32 b) { if (a <= b) { return 1; } return 0; }\n"
		"int More(int32 a, int32 b) { return (a > b) ? 5 : 2; }\n"
		"int Order(int32 a, int32 b) { if (a < b) { return -1; } if (a > b) { return 1; } return 0; }\n"
		"int32 Largest(const int32* pl, uint16 uiCount)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tint32\tlMost;\n"
		"\tlMost = pl[0];\n"
		"\tfor (i = 1; i < uiCount; i++) { if (pl[i] > lMost) { lMost = pl[i]; } }\n"
		"\treturn lMost;\n"
		"}\n"
		"int32 gal[8] = { 5, -7, 0x7FFFFFFF, -0x7FFFFFFF - 1, 70000, -70000, 65536, -1 };\n"
		"int main(void)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tuint16\tj;\n"
		"\tint\t\tiResult;\n"
		"\tint\t\tiLess;\n"
		"\tint\t\tiNotMore;\n"
		"\tint\t\tiMore;\n"
		"\tint\t\tiOrder;\n"
		"\n"
		"\tfor (i = 0; i < 64; i++) { gauiSource[i] = (uint8)(i * 37 + 11); }\n"
		"\tMix(gauiDest, gauiSource, 64, 0x5A);\n"
		"\tiResult = 0;\n"
		"\tif (Sum(gauiDest, 64) == 8619) { iResult += 1; }\n"
		"\tBlend(gauiDest, gauiSource, gauiDest, 64);\n"
		"\tif (Sum(gauiDest, 64) == 12513) { iResult += 2; }\n"
		"\tiLess = 0; iNotMore = 0; iMore = 0; iOrder = 0;\n"
		"\tfor (i = 0; i < 8; i++)\n"
		"\t{\n"
		"\t\tfor (j = 0; j < 8; j++)\n"
		"\t\t{\n"
		"\t\t\tiLess += Less(gal[i], gal[j]);\n"
		"\t\t\tiNotMore += NotMore(gal[i], gal[j]);\n"
		"\t\t\tiMore += More(gal[i], gal[j]);\n"
		"\t\t\tiOrder += Order(gal[i], gal[j]) * (int)(i + 1);\n"
		"\t\t}\n"
		"\t}\n"
		"\tif (iLess == 28) { iResult += 4; }\n"
		"\tif (iNotMore == 36) { iResult += 8; }\n"
		"\tif (iMore == 28 * 5 + 36 * 2) { iResult += 16; }\n"
		"\tif (iOrder == -4) { iResult += 32; }\n"
		"\tif (Largest(gal, 8) == 0x7FFFFFFF) { iResult += 64; }\n"
		"\tif (Largest(gal + 3, 5) == 70000) { iResult += 128; }\n"
		"\treturn iResult;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Loops that divide their counter by constants (CIRCounterPasses::
// ReduceDivisions keeps the quotients and remainders as counters): a 32 bit
// counter from 0, a 16 bit one from a start that isn't known (divided once
// before the loop) and up to the top of its range, a loop that doesn't go
// round, and a step of 5 with a divide that only some ways round do.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816CounterDivisions(void)
{
	AssertInt(127, TestCPP816Run(
		"typedef unsigned long uint32;\n"
		"typedef unsigned int uint16;\n"
		"unsigned char gauc[4000];\n"
		"uint32 Fill(uint32 ulSize)\n"
		"{\n"
		"\tuint32\ti;\n"
		"\tuint32\tulSum;\n"
		"\tulSum = 0;\n"
		"\tfor (i = 0; i < ulSize; i++)\n"
		"\t{\n"
		"\t\tgauc[i] = (unsigned char)((i * 7 + i / 251) & 0xff);\n"
		"\t\tif (i % 3 == 2) { ulSum += i / 251; }\n"
		"\t}\n"
		"\treturn ulSum;\n"
		"}\n"
		"uint16 Count(uint16 uiFrom, uint16 uiTo)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tuint16\tuiCount;\n"
		"\tuiCount = 0;\n"
		"\tfor (i = uiFrom; i < uiTo; i++)\n"
		"\t{\n"
		"\t\tif (i % 7 == 3) { uiCount++; }\n"
		"\t\tuiCount += i / 1000;\n"
		"\t}\n"
		"\treturn uiCount;\n"
		"}\n"
		"uint16 Step(uint16 uiFrom)\n"
		"{\n"
		"\tuint16\ti;\n"
		"\tuint16\tuiCount;\n"
		"\tuiCount = 0;\n"
		"\tfor (i = uiFrom; i < 60000; i += 5)\n"
		"\t{\n"
		"\t\tuiCount += i % 13;\n"
		"\t\tif (i & 64) { uiCount += i / 13; }\n"
		"\t}\n"
		"\treturn uiCount;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tuint32\tulSum;\n"
		"\tuint32\tulBytes;\n"
		"\tuint16\ti;\n"
		"\tint\t\tiResult;\n"
		"\n"
		"\tulSum = Fill(4000);\n"
		"\tulBytes = 0;\n"
		"\tfor (i = 0; i < 4000; i++) { ulBytes += gauc[i]; }\n"
		"\tiResult = 0;\n"
		"\tif (ulSum == 9960) { iResult += 1; }\n"
		"\tif (ulBytes == 508936) { iResult += 2; }\n"
		"\tif (Count(3, 3000) == 3429) { iResult += 4; }\n"
		"\tif (Count(64000, 65535) == 33458) { iResult += 8; }\n"
		"\tif (Count(9, 9) == 0) { iResult += 16; }\n"
		"\tif (Step(7) == 21519) { iResult += 32; }\n"
		"\tif (Step(59999) == 4619) { iResult += 64; }\n"
		"\treturn iResult;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Calls through pointers (the target's bank and high byte pushed as a word,
// the target - 1 written over a byte's room and the high byte: no sep m),
// with 32 bit, 16 bit and no results and arguments in registers and on the
// stack; and signed comparisons of two values whose C and Z a second test
// reads (if (a < b) ... else if (a > b), and a < b as a value): the top
// words compared as they are, C put right where the signs differ.  16 and
// 32 bits, at both ends of their ranges; a sort through a pointer's order.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816PointerCallsAndOrder(void)
{
	AssertInt(255, TestCPP816Run(
		"typedef long int32;\n"
		"typedef unsigned long uint32;\n"
		"int Order(int32 a, int32 b) { if (a < b) { return -1; } if (a > b) { return 1; } return 0; }\n"
		"int Order16(int a, int b) { if (a < b) { return -1; } if (a > b) { return 1; } return 0; }\n"
		"bool Less(int a, int b) { return a < b; }\n"
		"bool NotLess(int32 a, int32 b) { return a >= b; }\n"
		"int32 Add3(int32 a, int32 b, int32 c) { return a + b + c; }\n"
		"int Twice(int i) { return i * 2; }\n"
		"void Bump(int* pi) { (*pi) += 5; }\n"
		"int32 gal[8] = { 5, -7, 0x7FFFFFFF, -0x7FFFFFFF - 1, 70000, -70000, 65536, -1 };\n"
		"int gai[6] = { 3, -3, 32767, -32767 - 1, 0, 256 };\n"
		"typedef int (*FOrder)(int32, int32);\n"
		"typedef int32 (*FAdd3)(int32, int32, int32);\n"
		"typedef int (*FTwice)(int);\n"
		"typedef void (*FBump)(int*);\n"
		"struct SCalls { FOrder fOrder; FAdd3 fAdd3; FTwice fTwice; FBump fBump; };\n"
		"SCalls gsCalls = { Order, Add3, Twice, Bump };\n"
		"int Sorted(int32* pl, int iCount, FOrder fOrder)\n"
		"{\n"
		"\tint\t\ti;\n"
		"\tint\t\tj;\n"
		"\tint32\tlSwap;\n"
		"\tfor (i = 1; i < iCount; i++)\n"
		"\t{\n"
		"\t\tfor (j = i; (j > 0) && (fOrder(pl[j - 1], pl[j]) > 0); j--)\n"
		"\t\t{\n"
		"\t\t\tlSwap = pl[j]; pl[j] = pl[j - 1]; pl[j - 1] = lSwap;\n"
		"\t\t}\n"
		"\t}\n"
		"\tfor (i = 1; i < iCount; i++) { if (pl[i - 1] > pl[i]) { return 0; } }\n"
		"\treturn 1;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tvolatile SCalls*\tpsCalls;\n"
		"\tint\t\ti;\n"
		"\tint\t\tj;\n"
		"\tint\t\tiResult;\n"
		"\tint\t\tiOrder;\n"
		"\tint\t\tiOrder16;\n"
		"\tint\t\tiLess;\n"
		"\tint\t\tiNotLess;\n"
		"\tint\t\tiBumped;\n"
		"\n"
		"\tpsCalls = &gsCalls;\n"
		"\tiOrder = 0; iOrder16 = 0; iLess = 0; iNotLess = 0;\n"
		"\tfor (i = 0; i < 8; i++)\n"
		"\t{\n"
		"\t\tfor (j = 0; j < 8; j++)\n"
		"\t\t{\n"
		"\t\t\tiOrder += psCalls->fOrder(gal[i], gal[j]) * (i + 1);\n"
		"\t\t\tiNotLess += NotLess(gal[i], gal[j]) ? 1 : 0;\n"
		"\t\t}\n"
		"\t}\n"
		"\tfor (i = 0; i < 6; i++)\n"
		"\t{\n"
		"\t\tfor (j = 0; j < 6; j++)\n"
		"\t\t{\n"
		"\t\t\tiOrder16 += Order16(gai[i], gai[j]) * (i + 1);\n"
		"\t\t\tiLess += Less(gai[i], gai[j]) ? 1 : 0;\n"
		"\t\t}\n"
		"\t}\n"
		"\tiResult = 0;\n"
		"\tif (iOrder == -4) { iResult += 1; }\n"
		"\tif (iNotLess == 36) { iResult += 2; }\n"
		"\tif (iOrder16 == 3) { iResult += 4; }\n"
		"\tif (iLess == 15) { iResult += 8; }\n"
		"\tif (psCalls->fAdd3(100000, 200000, -50000) == 250000) { iResult += 16; }\n"
		"\tif (psCalls->fTwice(21) == 42) { iResult += 32; }\n"
		"\tiBumped = 7;\n"
		"\tpsCalls->fBump(&iBumped);\n"
		"\tif (iBumped == 12) { iResult += 64; }\n"
		"\tif (Sorted(gal, 8, psCalls->fOrder) && (gal[0] == -0x7FFFFFFF - 1) && (gal[7] == 0x7FFFFFFF) && (gal[3] == -1)) { iResult += 128; }\n"
		"\treturn iResult;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// The words of a value that are always 0 (a zext's high word, an and's with
// a constant, a shift right's: CIRW65C816Backend::FindZeroWords) are read
// as the constant, not from the frame: 32 bit adds, subs either way round,
// logic, compares, a pushed argument and shifts of such values give what
// they should.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ZeroWords(void)
{
	AssertInt(8191, TestCPP816Run(
		"typedef unsigned long uint32;\n"
		"typedef unsigned int uint16;\n"
		"uint32 Pass(uint32 ul) { return ul + 1; }\n"
		"uint32 Add(uint32 ul, uint16 ui) { return ul + ui; }\n"
		"uint32 Sub(uint32 ul, uint16 ui) { return ul - ui; }\n"
		"uint32 SubFrom(uint32 ul, uint16 ui) { return (uint32)ui - ul; }\n"
		"uint32 Or(uint32 ul, uint16 ui) { return ul | ui; }\n"
		"uint32 Xor(uint32 ul, uint16 ui) { return ul ^ ui; }\n"
		"uint32 And(uint32 ul, uint16 ui) { return ul & ui; }\n"
		"int Above(uint32 ul, uint16 ui) { return ul > (uint32)ui; }\n"
		"int Same(uint32 ul, uint16 ui) { return ul == (uint32)ui; }\n"
		"uint32 Masked(uint32 ul, uint32 ul2) { return (ul & 0xFFFF) + ul2; }\n"
		"uint32 High(uint32 ul, uint32 ul2) { return (ul >> 16) + ul2; }\n"
		"uint32 Pushed(uint16 ui) { return Pass((uint32)ui); }\n"
		"uint32 Shifted(uint16 ui) { return ((uint32)ui << 3) + ((uint32)ui << 1); }\n"
		"uint32 gaul[16];\n"
		"int main(void)\n"
		"{\n"
		"\tvolatile uint32\tul;\n"
		"\tvolatile uint16\tui;\n"
		"\tint\t\t\t\tiResult;\n"
		"\n"
		"\tul = 0x1234FFF0;\n"
		"\tui = 0x0011;\n"
		"\tiResult = 0;\n"
		"\tif (Add(ul, ui) == 0x12350001) { iResult += 1; }\n"
		"\tif (Sub(0x12340005, ui) == 0x1233FFF4) { iResult += 2; }\n"
		"\tif (SubFrom(5, ui) == 12) { iResult += 4; }\n"
		"\tif (SubFrom(ul, ui) == 0xEDCB0021) { iResult += 8; }\n"
		"\tif (Or(ul, 0x000F) == 0x1234FFFF) { iResult += 16; }\n"
		"\tif (Xor(ul, 0xFFFF) == 0x1234000F) { iResult += 32; }\n"
		"\tif (And(ul, 0xFF0F) == 0x0000FF00) { iResult += 64; }\n"
		"\tif (Above(ul, ui) && !Above(0x10, ui) && Above(0x10000, 0xFFFF)) { iResult += 128; }\n"
		"\tif (Same(0x11, ui) && !Same(0x10011, ui)) { iResult += 256; }\n"
		"\tif (Masked(ul, 0x10000010) == 0x10010000) { iResult += 512; }\n"
		"\tif (High(ul, 0xFFFFF000) == 0x00000234) { iResult += 1024; }\n"
		"\tif (Pushed(0xFFFF) == 0x10000) { iResult += 2048; }\n"
		"\tif (Shifted(0xFFFF) == 0x9FFF6) { iResult += 4096; }\n"
		"\treturn iResult;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// An index the branches keep between two bytes (first <= index <= last):
// (index - first) * 12 is worked out in 16 bits (NarrowMultiplies with the
// branches' ranges), at both ends of the range and outside it; and a loop
// whose count is a byte indexes 4 byte elements through Y.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816GuardedIndices(void)
{
	AssertInt(1 + 20 + 300 + 4000, TestCPP816Run(
		"typedef unsigned int size;\n"
		"typedef unsigned char uint8;\n"
		"struct SChild { long lType; long lA; long lB; };\n"
		"class CNode\n"
		"{\n"
		"public:\n"
		"\tuint8\tmuiFirstIndex;\n"
		"\tuint8\tmuiLastIndex;\n"
		"\tuint8\tmuiFlags;\n"
		"\tuint8\tmuiPad;\n"
		"\tSChild*\tGetChildren(void) { if (muiFlags & 1) { return (SChild*)((char*)this + sizeof(CNode)); } return 0; }\n"
		"\tlong**\tGetNodes(void) { return (long**)((char*)this + sizeof(CNode)); }\n"
		"\tbool\tContainsIndex(size uiIndex) { return (uiIndex >= muiFirstIndex) && (uiIndex <= muiLastIndex); }\n"
		"\tSChild*\tGet(size uiIndex);\n"
		"\tsize\tFindIndex(long* pl);\n"
		"};\n"
		"SChild* CNode::Get(size uiIndex)\n"
		"{\n"
		"\tif (ContainsIndex(uiIndex))\n"
		"\t{\n"
		"\t\treturn &(GetChildren()[uiIndex - muiFirstIndex]);\n"
		"\t}\n"
		"\treturn 0;\n"
		"}\n"
		"size CNode::FindIndex(long* pl)\n"
		"{\n"
		"\tsize\ti;\n"
		"\tsize\tuiNumIndices;\n"
		"\tlong**\tapl;\n"
		"\tuiNumIndices = (uint8)(muiLastIndex - muiFirstIndex);\n"
		"\tapl = GetNodes();\n"
		"\tfor (i = 0; i <= uiNumIndices; i++)\n"
		"\t{\n"
		"\t\tif (apl[i] == pl)\n"
		"\t\t{\n"
		"\t\t\treturn i + muiFirstIndex;\n"
		"\t\t}\n"
		"\t}\n"
		"\treturn 0xFFFF;\n"
		"}\n"
		"char gacMemory[3200];\n"
		"long gal[256];\n"
		"int main(void)\n"
		"{\n"
		"\tCNode*\tpcNode;\n"
		"\tlong**\tapl;\n"
		"\tint\t\ti;\n"
		"\tint\t\tiResult;\n"
		"\tpcNode = (CNode*)gacMemory;\n"
		"\tpcNode->muiFirstIndex = 0;\n"
		"\tpcNode->muiLastIndex = 255;\n"
		"\tpcNode->muiFlags = 1;\n"
		"\tiResult = 0;\n"
		"\tif (((char*)pcNode->Get(0) == gacMemory + 4) && ((char*)pcNode->Get(255) == gacMemory + 4 + 255 * 12) && (pcNode->Get(256) == 0)) { iResult += 1; }\n"
		"\tpcNode->muiFirstIndex = 200;\n"
		"\tif (((char*)pcNode->Get(200) == gacMemory + 4) && ((char*)pcNode->Get(255) == gacMemory + 4 + 55 * 12) && (pcNode->Get(199) == 0) && (pcNode->Get(0xFFFF) == 0)) { iResult += 20; }\n"
		"\tpcNode->muiFirstIndex = 3;\n"
		"\tpcNode->muiLastIndex = 3;\n"
		"\tif (((char*)pcNode->Get(3) == gacMemory + 4) && (pcNode->Get(2) == 0) && (pcNode->Get(4) == 0)) { iResult += 300; }\n"
		"\tpcNode->muiFirstIndex = 0;\n"
		"\tpcNode->muiLastIndex = 255;\n"
		"\tapl = pcNode->GetNodes();\n"
		"\tfor (i = 0; i < 256; i++) { apl[i] = &gal[i]; }\n"
		"\tif ((pcNode->FindIndex(&gal[0]) == 0) && (pcNode->FindIndex(&gal[255]) == 255) && (pcNode->FindIndex(&gal[77]) == 77) && (pcNode->FindIndex(0) == 0xFFFF)) { iResult += 4000; }\n"
		"\treturn iResult;\n"
		"}\n"));
}


int32 TestCPP816RunListed(const char* szSource, const char** aszUnits, CChars* pszListing, uint32* puiResults, size uiResults, bool bReturns);


//////////////////////////////////////////////////////////////////////////
//
// The thunks' frames (CPP65_816_VCALL_THIS and the rest): a virtual function
// called through a member pointer, this adjusted for a secondary base's
// vtable, with arguments that must arrive as they were pushed.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Thunks(void)
{
	static const char*	aszNone[] = { NULL };
	CChars				szListing;

	AssertInt(0, TestCPP816RunListed(
		"struct A { int a; virtual int F(int x) { return x + a; } virtual int G(int x, int y) { return x * a + y; } };\n"
		"struct B { int b; virtual int H(int x) { return x - b; } };\n"
		"struct C : A, B { int c; int H(int x) { return x - c; } int G(int x, int y) { return x * c + y; } };\n"
		"volatile int gi = 1;\n"
		"int main(void)\n"
		"{\n"
		"\tC o; A* pa = &o; B* pb = &o; int w = 0;\n"
		"\to.a = 2; o.b = 3; o.c = 5;\n"
		"\tint (A::*pg)(int, int) = &A::G;\n"
		"\tint (A::*pf)(int) = &A::F;\n"
		"\tif (gi == 0) { pg = 0; }\n"
		"\tif ((pa->*pg)(7, 1) != 36) { w |= 1; }\n"
		"\tif ((pa->*pf)(7) != 9) { w |= 2; }\n"
		"\tif (pb->H(7) != 2) { w |= 4; }\n"
		"\treturn w;\n"
		"}\n", aszNone, &szListing, NULL, 0, true));
	AssertTrue(szListing.Contains("__vcall_0_1:"));
	AssertTrue(szListing.Contains("lda [[D + 10]:24 + Y]"));
	AssertTrue(szListing.Contains("lda [S + 4]"));
	AssertTrue(szListing.Contains("sta [S + 6]"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit counts read the runtime's tables, however many there are, not a table
// of their own each.  A big array's rest is zeroed all at once.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816SharedTables(void)
{
	static const char*	aszNone[] = { NULL };
	CChars				szListing;

	AssertInt(0, TestCPP816RunListed(
		"volatile unsigned long long gull = 0x100000ULL;\n"
		"int A(void) { return __builtin_clzll(gull); }\n"
		"int B(void) { return __builtin_ctzll(gull); }\n"
		"int C(void) { return __builtin_clzll(gull << 3); }\n"
		"int D(void) { return __builtin_ctzll(gull << 3); }\n"
		"int main(void)\n"
		"{\n"
		"\tchar sz[256] = \"x\"; int i; int w = 0;\n"
		"\tif ((A() != 43) || (B() != 20)) { w |= 1; }\n"
		"\tif ((C() != 40) || (D() != 23)) { w |= 2; }\n"
		"\tif (sz[0] != 'x') { w |= 4; }\n"
		"\tfor (i = 1; i < 256; i++) { if (sz[i]) { w |= 8; } }\n"
		"\treturn w;\n"
		"}\n", aszNone, &szListing, NULL, 0, true));
	AssertTrue(szListing.Contains("R816_Clz8"));
	AssertTrue(szListing.Contains("R816_Ctz8"));
	AssertFalse(szListing.Contains("Str0:"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A constant is worked out in its own type even while the type of another is
// being found: A<65535u + 1> is A<0> with a 16 bit int, whose T is long long,
// so the sum around it doesn't wrap.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NestedConstants(void)
{
	AssertInt(1, TestCPP816Run(
		"template<long N> struct A { typedef char T; };\n"
		"template<> struct A<0> { typedef long long T; };\n"
		"enum { V = ((A<65535u + 1>::T)1 + 65535u) >> 8 };\n"
		"int main(void) { return V == 256; }\n"));
}


extern const char*	gszTestCPP65WideStrings;


//////////////////////////////////////////////////////////////////////////
//
// The wide strings of TestCPP65IRWideStrings: on the W65C816 their elements
// are unsigned int and unsigned long.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816WideStrings(void)
{
	AssertInt(0, TestCPP816Run(gszTestCPP65WideStrings));
}


extern const char*	gszTestCPP65OverloadedComma;


//////////////////////////////////////////////////////////////////////////
//
// The overloaded commas of TestCPP65IROverloadedComma.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816OverloadedComma(void)
{
	AssertInt(0, TestCPP816Run(gszTestCPP65OverloadedComma));
}


//////////////////////////////////////////////////////////////////////////
//
// A signed int converted to float (extended to 32 bits in the frame) has no
// hand counted branch for its sign word (bcc C + 1 kept the flow passes off
// the function).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816SignedToFloat(void)
{
	static const char*	aszFloats[] = { "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", NULL };
	CChars				szListing;

	szListing.Init();
	AssertInt(-500, TestCPP816RunListed(
		"int Convert(int x) { float f = (float)x; return (int)(f / 2.0f); }\n"
		"int main(void) { return Convert(-1000); }\n", aszFloats, &szListing, NULL, 0, true));
	AssertTrue(szListing.Contains("_Convert__Fs:"));
	AssertFalse(szListing.Contains("C +"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// x / d and x % d of a d that isn't a constant share one divide (the
// remainder is x - q * d, CIRDivisionPasses::ShareDivisions), either way round,
// signed and unsigned, 16 and 32 bits.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816SharedDivisions(void)
{
	static const char*	aszNoUnits[] = { NULL };
	CChars				szListing;

	AssertInt(1, TestCPP816Run(
		"int Check(long x, long d) { long q = x / d; long r = x % d; return (q * d + r == x) && ((r < 0 ? -r : r) < (d < 0 ? -d : d)) && ((r == 0) || ((r < 0) == (x < 0))) ? 1 : 0; }\n"
		"int CheckU(unsigned long x, unsigned long d) { unsigned long r = x % d; unsigned long q = x / d; return (q * d + r == x) && (r < d) ? 1 : 0; }\n"
		"int Check16(int x, int d) { int r = x % d; int q = x / d; return (q * d + r == x) && ((r < 0 ? -r : r) < (d < 0 ? -d : d)) && ((r == 0) || ((r < 0) == (x < 0))) ? 1 : 0; }\n"
		"int main(void)\n"
		"{\n"
		"\tint\tiOk;\n"
		"\tiOk = Check(1000000L, 7L) & Check(-1000000L, 7L) & Check(1000000L, -7L) & Check(-123456789L, 100000L) & Check(5L, 9L);\n"
		"\tiOk &= CheckU(4000000000UL, 3UL) & CheckU(4000000000UL, 70000UL) & CheckU(65535UL, 65536UL) & CheckU(0xFFFFFFFFUL, 0x10001UL);\n"
		"\tiOk &= Check16(30000, 7) & Check16(-30000, 7) & Check16(-30000, -7) & Check16(3, 30000);\n"
		"\tiOk &= ((-7L / 2L) == -3L) & ((-7L % 2L) == -1L) & ((7UL / 2UL) == 3UL);\n"
		"\treturn iOk;\n"
		"}\n"));

	//16, 32 and 64 bits, unsigned and signed, either way round: the urem or srem takes the remainder the divide left at
	//D + 5 (CIROptimiser::SetDivideRemainders, CIRW65C816Backend::RemainderLeft), with no call and no multiply.
	szListing.Init();
	AssertInt(1, TestCPP816RunListed(
		"unsigned int gauiX[] = { 50000, 7, 65535, 3, 1234 };\n"
		"unsigned int gauiD[] = { 37, 9, 1, 65535, 300 };\n"
		"unsigned long gaulX[] = { 4000000000UL, 70000UL, 0xFFFFFFFFUL, 5UL, 123456789UL };\n"
		"unsigned long gaulD[] = { 3UL, 70000UL, 0x10001UL, 9UL, 1000UL };\n"
		"int Check16(unsigned int x, unsigned int d) { unsigned int q = x / d; unsigned int r = x % d; return (q * d + r == x) && (r < d); }\n"
		"int Check16R(unsigned int x, unsigned int d) { unsigned int r = x % d; unsigned int q = x / d; return (q * d + r == x) && (r < d); }\n"
		"int Check32(unsigned long x, unsigned long d) { unsigned long q = x / d; unsigned long r = x % d; return (q * d + r == x) && (r < d); }\n"
		"int Check32R(unsigned long x, unsigned long d) { unsigned long r = x % d; unsigned long q = x / d; return (q * d + r == x) && (r < d); }\n"
		"int gaiX[] = { -30000, 7, 32767, -3, 1234 };\n"
		"int gaiD[] = { 37, -9, -1, 30000, -300 };\n"
		"long galX[] = { -2000000000L, 70000L, 2147483647L, -5L, 123456789L };\n"
		"long galD[] = { 3L, -70000L, 65537L, -9L, -1000L };\n"
		"int CheckS(int x, int d, int q, int r) { return (q * d + r == x) && ((r == 0) || ((r < 0) == (x < 0))) && ((r < 0 ? -r : r) < (d < 0 ? -d : d)); }\n"
		"int CheckSL(long x, long d, long q, long r) { return (q * d + r == x) && ((r == 0) || ((r < 0) == (x < 0))) && ((r < 0 ? -r : r) < (d < 0 ? -d : d)); }\n"
		"int Check16S(int x, int d) { int q = x / d; int r = x % d; return CheckS(x, d, q, r); }\n"
		"int Check16SR(int x, int d) { int r = x % d; int q = x / d; return CheckS(x, d, q, r); }\n"
		"int Check32S(long x, long d) { long q = x / d; long r = x % d; return CheckSL(x, d, q, r); }\n"
		"int Check32SR(long x, long d) { long r = x % d; long q = x / d; return CheckSL(x, d, q, r); }\n"
		"unsigned long long gaullX[] = { 0xFEDCBA9876543210ULL, 70000ULL, 0xFFFFFFFFFFFFFFFFULL, 5ULL, 123456789012345ULL };\n"
		"unsigned long long gaullD[] = { 0x123456789ULL, 7ULL, 0x100000001ULL, 9ULL, 1000ULL };\n"
		"long long gallX[] = { -0x7EDCBA9876543210LL, -70000LL, 0x7FFFFFFFFFFFFFFFLL, -5LL, 123456789012345LL };\n"
		"long long gallD[] = { 0x123456789LL, -7LL, -0x100000001LL, 9LL, -1000LL };\n"
		"int Check64(unsigned long long x, unsigned long long d) { unsigned long long q = x / d; unsigned long long r = x % d; return (q * d + r == x) && (r < d); }\n"
		"int Check64R(unsigned long long x, unsigned long long d) { unsigned long long r = x % d; unsigned long long q = x / d; return (q * d + r == x) && (r < d); }\n"
		"int CheckSLL(long long x, long long d, long long q, long long r) { return (q * d + r == x) && ((r == 0) || ((r < 0) == (x < 0))) && ((r < 0 ? -r : r) < (d < 0 ? -d : d)); }\n"
		"int Check64S(long long x, long long d) { long long q = x / d; long long r = x % d; return CheckSLL(x, d, q, r); }\n"
		"int Check64SR(long long x, long long d) { long long r = x % d; long long q = x / d; return CheckSLL(x, d, q, r); }\n"
		"int main(void)\n"
		"{\n"
		"\tint\tiOk;\n"
		"\tint\ti;\n"
		"\tiOk = 1;\n"
		"\tfor (i = 0; i < 5; i++)\n"
		"\t{\n"
		"\t\tiOk &= Check16(gauiX[i], gauiD[i]) & Check16R(gauiX[i], gauiD[i]) & Check32(gaulX[i], gaulD[i]) & Check32R(gaulX[i], gaulD[i]);\n"
		"\t\tiOk &= Check16S(gaiX[i], gaiD[i]) & Check16SR(gaiX[i], gaiD[i]) & Check32S(galX[i], galD[i]) & Check32SR(galX[i], galD[i]);\n"
		"\t\tiOk &= Check64(gaullX[i], gaullD[i]) & Check64R(gaullX[i], gaullD[i]) & Check64S(gallX[i], gallD[i]) & Check64SR(gallX[i], gallD[i]);\n"
		"\t}\n"
		"\treturn iOk;\n"
		"}\n", aszNoUnits, &szListing, NULL, 0, true));
	AssertFalse(szListing.Contains("R816_URem"));
	AssertFalse(szListing.Contains("R816_SRem"));
	AssertTrue(szListing.Contains("jsl R816_SDiv16"));
	AssertTrue(szListing.Contains("jsl R816_SDiv32"));
	AssertTrue(szListing.Contains("jsl R816_UDiv64"));
	AssertTrue(szListing.Contains("jsl R816_SDiv64"));
	AssertTrue(szListing.Contains("jsl R816_UDiv16"));
	AssertTrue(szListing.Contains("jsl R816_UDiv32"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Signed 64 bit divides and remainders whose magnitudes fit in 32 bits use
// the 32 bit routine (gszIR816Signed64); wider ones the 64 bit loop.  Every
// sign, at the boundary and either side of it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Divide64(void)
{
	AssertInt(1, TestCPP816Run(
		"int Check(long long x, long long d, long long q, long long r) { return ((x / d) == q) && ((x % d) == r) ? 1 : 0; }\n"
		"int main(void)\n"
		"{\n"
		"\tint\tiOk;\n"
		"\tiOk = Check(1000000LL, 7LL, 142857LL, 1LL) & Check(-1000000LL, 7LL, -142857LL, -1LL) & Check(1000000LL, -7LL, -142857LL, 1LL) & Check(-1000000LL, -7LL, 142857LL, -1LL);\n"
		"\tiOk &= Check(4294967295LL, 65537LL, 65535LL, 0LL) & Check(-4294967295LL, 2LL, -2147483647LL, -1LL);\n"
		"\tiOk &= Check(4294967296LL, 3LL, 1431655765LL, 1LL) & Check(-4294967296LL, 4294967296LL, -1LL, 0LL);\n"
		"\tiOk &= Check(1000000000000LL, 1000LL, 1000000000LL, 0LL) & Check(-1000000000007LL, 1000LL, -1000000000LL, -7LL);\n"
		"\tiOk &= Check(123456789012345LL, 4294967297LL, 28744LL, 2249027377LL);\n"
		"\treturn iOk;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// 1 << n looked up in R816_Bits16 and R816_Bits32 (CIRW65C816Backend::
// BitShift), and __builtin_clz and ctz (the runtime's R816_Clz16 ...),
// against loops, for every count and bit.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BitTables(void)
{
	AssertInt(1, TestCPP816Run(
		"volatile int giCount;\n"
		"int Clz32(unsigned long x) { int n = 0; if (x == 0) { return 32; } while (!(x & 0x80000000UL)) { x <<= 1; n++; } return n; }\n"
		"int Ctz32(unsigned long x) { int n = 0; if (x == 0) { return 32; } while (!(x & 1)) { x >>= 1; n++; } return n; }\n"
		"int Clz16(unsigned int x) { int n = 0; if (x == 0) { return 16; } while (!(x & 0x8000)) { x <<= 1; n++; } return n; }\n"
		"int Ctz16(unsigned int x) { int n = 0; if (x == 0) { return 16; } while (!(x & 1)) { x >>= 1; n++; } return n; }\n"
		"int main(void)\n"
		"{\n"
		"\tint\t\t\ti;\n"
		"\tint\t\t\tj;\n"
		"\tint\t\t\tn;\n"
		"\tunsigned long\tul;\n"
		"\tunsigned long\tulBit;\n"
		"\tunsigned int\tuBit;\n"
		"\tunsigned long\tulX;\n"
		"\tfor (i = 0; i < 32; i++)\n"
		"\t{\n"
		"\t\tgiCount = i;\n"
		"\t\tn = giCount;\n"
		"\t\tul = 1;\n"
		"\t\tfor (j = 0; j < i; j++) { ul *= 2; }\n"
		"\t\tulBit = (unsigned long)1 << n;\n"
		"\t\tif (ulBit != ul) { return 100 + i; }\n"
		"\t\tif (i < 16) { uBit = 1u << n; if (uBit != (unsigned int)ul) { return 200 + i; } }\n"
		"\t\tfor (j = 0; j < 4; j++)\n"
		"\t\t{\n"
		"\t\t\tulX = (ul | (ul >> (j + 1))) & ~((j == 3) ? ul : 0);\n"
		"\t\t\tif (__builtin_clzl(ulX) != Clz32(ulX)) { return 300 + i; }\n"
		"\t\t\tif (__builtin_ctzl(ulX) != Ctz32(ulX)) { return 400 + i; }\n"
		"\t\t\tif (__builtin_clz((unsigned int)ulX) != Clz16((unsigned int)ulX)) { return 500 + i; }\n"
		"\t\t\tif (__builtin_ctz((unsigned int)ulX) != Ctz16((unsigned int)ulX)) { return 600 + i; }\n"
		"\t\t}\n"
		"\t}\n"
		"\tif ((__builtin_clzl(0) != 32) || (__builtin_ctzl(0) != 32) || (__builtin_clz(0) != 16) || (__builtin_ctz(0) != 16)) { return 2; }\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// __builtin_clz and ctz (R816_Clz16, R816_Ctz16) of every 16 bit value,
// and of 4000 pseudo random 32 bit (R816_Clz32, R816_Ctz32) and 64 bit
// (CountTable) values of every width, against loops.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816CountZeros(void)
{
	AssertInt(1, TestCPP816Run(
		"int Clz64(unsigned long long x) { int n = 0; if (x == 0) { return 64; } while (!(x & 0x8000000000000000ULL)) { x <<= 1; n++; } return n; }\n"
		"int Ctz64(unsigned long long x) { int n = 0; if (x == 0) { return 64; } while (!(x & 1)) { x >>= 1; n++; } return n; }\n"
		"int Clz16(unsigned int x) { int n = 0; if (x == 0) { return 16; } while (!(x & 0x8000)) { x <<= 1; n++; } return n; }\n"
		"int Ctz16(unsigned int x) { int n = 0; if (x == 0) { return 16; } while (!(x & 1)) { x >>= 1; n++; } return n; }\n"
		"int main(void)\n"
		"{\n"
		"\tunsigned int\t\tu;\n"
		"\tunsigned long\t\tul;\n"
		"\tunsigned long long\tull;\n"
		"\tunsigned long\t\tulRandom;\n"
		"\tint\t\t\t\ti;\n"
		"\tu = 0;\n"
		"\tdo\n"
		"\t{\n"
		"\t\tif (__builtin_clz(u) != Clz16(u)) { return 100; }\n"
		"\t\tif (__builtin_ctz(u) != Ctz16(u)) { return 200; }\n"
		"\t\tu++;\n"
		"\t} while (u != 0);\n"
		"\tulRandom = 12345;\n"
		"\tfor (i = 0; i < 4000; i++)\n"
		"\t{\n"
		"\t\tulRandom = ulRandom * 1103515245UL + 12345UL;\n"
		"\t\tul = ulRandom >> ((ulRandom >> 27) & 31);\n"
		"\t\tif ((ulRandom & 0x300) == 0) { ul <<= (ulRandom >> 10) & 31; }\n"
		"\t\tif (__builtin_clzl(ul) != ((ul == 0) ? 32 : Clz64(ul) - 32)) { return 300; }\n"
		"\t\tif (__builtin_ctzl(ul) != ((ul == 0) ? 32 : Ctz64(ul))) { return 400; }\n"
		"\t\tull = ((unsigned long long)ul << ((ulRandom >> 4) & 31)) ^ ((unsigned long long)ulRandom << 32);\n"
		"\t\tif (__builtin_clzll(ull) != Clz64(ull)) { return 500; }\n"
		"\t\tif (__builtin_ctzll(ull) != Ctz64(ull)) { return 600; }\n"
		"\t}\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// 64 bit 1 << n looked up in R816_Bits64 (CIRW65C816Backend::BitShift), and the
// masks soft float rounds with ((1 << n) - 1, 1 << (n - 1)), for every
// count, against doubling; nothing calls R816_Shl64.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BitTable64(void)
{
	static const char*	aszNoUnits[] = { NULL };
	CChars				szListing;

	szListing.Init();
	AssertInt(1, TestCPP816RunListed(
		"volatile int giCount;\n"
		"int main(void)\n"
		"{\n"
		"\tint\t\t\t\t\ti;\n"
		"\tint\t\t\t\t\tj;\n"
		"\tint\t\t\t\t\tn;\n"
		"\tunsigned long long\tull;\n"
		"\tfor (i = 0; i < 64; i++)\n"
		"\t{\n"
		"\t\tgiCount = i;\n"
		"\t\tn = giCount;\n"
		"\t\tull = 1;\n"
		"\t\tfor (j = 0; j < i; j++) { ull += ull; }\n"
		"\t\tif (((unsigned long long)1 << n) != ull) { return 100 + i; }\n"
		"\t\tif ((((unsigned long long)1 << n) - 1) != (ull - 1)) { return 200 + i; }\n"
		"\t\tif ((i > 0) && (((unsigned long long)1 << (n - 1)) != (ull >> 1))) { return 300 + i; }\n"
		"\t}\n"
		"\treturn 1;\n"
		"}\n", aszNoUnits, &szListing, NULL, 0, true));
	AssertTrue(szListing.Contains("R816_Bits64"));
	AssertFalse(szListing.Contains("jsl R816_Shl64"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An int count of 64 bit shifts, extended to 64 bits for the IR, has only
// its low word worked out (CIRW65C816Backend::Extend, OnlyCounts): the shift
// routines read only its 6 bits.  Its sign words aren't, here or in the
// shifts by a signed count, and the results are as natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ShiftCounts(void)
{
	static const char*	aszNoUnits[] = { NULL };
	static const char*	aszLines[] = { "ullSum += gullValue >> n;", "ullSum ^= gullValue << n;", "ullSum += (unsigned long long)((long long)gullValue >> n);", NULL };
	CChars				szListing;
	char*				szShift;
	char*				szCall;
	char*				szEnd;
	unsigned long long	ullValue;
	unsigned long long	ullSum;
	int					i;

	ullValue = 0x923456789ABCDEF1ULL;
	ullSum = 0;
	for (i = 0; i < 64; i++)
	{
		ullSum += ullValue >> i;
		ullSum ^= ullValue << i;
		ullSum += (unsigned long long)((long long)ullValue >> i);
	}

	szListing.Init();
	AssertInt((int)((ullSum ^ (ullSum >> 16) ^ (ullSum >> 32) ^ (ullSum >> 48)) & 0x7FFF), TestCPP816RunListed(
		"volatile int giCount;\n"
		"unsigned long long gullValue = 0x923456789ABCDEF1ULL;\n"
		"int main(void)\n"
		"{\n"
		"\tunsigned long long ullSum = 0;\n"
		"\tint i;\n"
		"\tfor (i = 0; i < 64; i++)\n"
		"\t{\n"
		"\t\tgiCount = i;\n"
		"\t\tint n = giCount;\n"
		"\t\tullSum += gullValue >> n;\n"
		"\t\tullSum ^= gullValue << n;\n"
		"\t\tullSum += (unsigned long long)((long long)gullValue >> n);\n"
		"\t}\n"
		"\treturn (int)((ullSum ^ (ullSum >> 16) ^ (ullSum >> 32) ^ (ullSum >> 48)) & 0x7FFF);\n"
		"}\n", aszNoUnits, &szListing, NULL, 0, true));

	//From each shift's line to its call, nothing works out a sign.
	for (i = 0; aszLines[i]; i++)
	{
		szShift = strstr(szListing.Text(), aszLines[i]);
		AssertNotNull(szShift);
		szCall = szShift ? strstr(szShift, "jsl R816_") : NULL;
		AssertNotNull(szCall);
		if (szShift && szCall)
		{
			for (szEnd = szShift; szEnd < szCall; szEnd++)
			{
				AssertFalse((strncmp(szEnd, "eor 0xFFFF", 10) == 0) || (strncmp(szEnd, "adc 0xFFFF", 10) == 0));
			}
		}
	}
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by constants of 16 bits or more (whole words moved, then bits):
// 32 and 64 bit shl, lshr and ashr, against one bit shifts repeated.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816WordShifts(void)
{
	AssertInt(1, TestCPP816Run(
		"volatile int giOne = 1;\n"
		"unsigned long ShlL(unsigned long x, int n) { while (n-- > 0) { x <<= giOne; } return x; }\n"
		"unsigned long ShrL(unsigned long x, int n) { while (n-- > 0) { x >>= giOne; } return x; }\n"
		"long SarL(long x, int n) { while (n-- > 0) { x >>= giOne; } return x; }\n"
		"unsigned long long ShlQ(unsigned long long x, int n) { while (n-- > 0) { x <<= giOne; } return x; }\n"
		"unsigned long long ShrQ(unsigned long long x, int n) { while (n-- > 0) { x >>= giOne; } return x; }\n"
		"long long SarQ(long long x, int n) { while (n-- > 0) { x >>= giOne; } return x; }\n"
		"int Check(unsigned long ul, unsigned long long ull)\n"
		"{\n"
		"\tlong\t\tl = (long)ul;\n"
		"\tlong long\tll = (long long)ull;\n"
		"\tif (((ul << 16) != ShlL(ul, 16)) || ((ul >> 17) != ShrL(ul, 17)) || ((l >> 24) != SarL(l, 24)) || ((ul << 31) != ShlL(ul, 31)) || ((l >> 31) != SarL(l, 31))) { return 0; }\n"
		"\tif (((ull << 16) != ShlQ(ull, 16)) || ((ull >> 33) != ShrQ(ull, 33)) || ((ll >> 40) != SarQ(ll, 40)) || ((ull << 48) != ShlQ(ull, 48)) || ((ll >> 63) != SarQ(ll, 63)) || ((ull >> 32) != ShrQ(ull, 32))) { return 0; }\n"
		"\treturn 1;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\treturn Check(0x89ABCDEFUL, 0xFEDCBA9876543210ULL) && Check(0x12345678UL, 0x0123456789ABCDEFULL) && Check(0xFFFFFFFFUL, 0x8000000000000001ULL) ? 1 : 0;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A 2 word load or store through an array index kept in Y (FoldIndices,
// IRF_InArray) reads the index for each word, so the index can't be kept in
// X: returning a 32 bit load loads its high word first and puts it in X
// (tax) before the low word's txy (TrueMallocator's FindFree returned
// mauiSecondBitmaps[iFirst] with iFirst counted in X).  Counted in loops
// here so the index is X's, and called through pointers so not inlined.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816IndexedWords(void)
{
	AssertInt(1, TestCPP816Run(
		"volatile unsigned long gulValue;\n"
		"class CMaps\n"
		"{\n"
		"public:\n"
		"\tunsigned long\tmaul[30];\n"
		"\tvoid*\t\t\tmapv[30];\n"
		"};\n"
		"CMaps\tgcMaps;\n"
		"unsigned long Get(unsigned long ulMask) { int i = 0; while (!(ulMask & 1)) { ulMask >>= 1; i++; } return gcMaps.maul[i]; }\n"
		"void* GetPointer(unsigned long ulMask) { int i = 1; while (!(ulMask & 1)) { ulMask >>= 1; i++; } return gcMaps.mapv[i]; }\n"
		"void Put(unsigned long ulMask, unsigned long ul) { int i = 0; while (!(ulMask & 1)) { ulMask >>= 1; i++; } gcMaps.maul[i] = ul; }\n"
		"unsigned long (*volatile gpGet)(unsigned long) = Get;\n"
		"void* (*volatile gpGetPointer)(unsigned long) = GetPointer;\n"
		"void (*volatile gpPut)(unsigned long, unsigned long) = Put;\n"
		"int main(void)\n"
		"{\n"
		"\tint\ti;\n"
		"\tfor (i = 0; i < 29; i++)\n"
		"\t{\n"
		"\t\tgcMaps.maul[i] = 0x10001UL * (unsigned long)(i + 1);\n"
		"\t\tgcMaps.mapv[i] = &gcMaps.maul[i];\n"
		"\t}\n"
		"\tfor (i = 0; i < 28; i++)\n"
		"\t{\n"
		"\t\tgulValue = 1UL << i;\n"
		"\t\tif (gpGet(gulValue) != 0x10001UL * (unsigned long)(i + 1)) { return 100 + i; }\n"
		"\t\tif (gpGetPointer(gulValue) != &gcMaps.maul[i + 1]) { return 200 + i; }\n"
		"\t\tgpPut(gulValue, 0x20003UL * (unsigned long)i);\n"
		"\t\tif (gcMaps.maul[i] != 0x20003UL * (unsigned long)i) { return 300 + i; }\n"
		"\t}\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit shl and lshr by 1 to 7 bits written straight to the result
// (ShiftTwoWords): to a place apart from the value, which is used after,
// and in the value's own place, against one bit shifts repeated.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816SmallShifts(void)
{
	AssertInt(1, TestCPP816Run(
		"volatile int giOne = 1;\n"
		"volatile unsigned long gul;\n"
		"unsigned long ShlL(unsigned long x, int n) { while (n-- > 0) { x <<= giOne; } return x; }\n"
		"unsigned long ShrL(unsigned long x, int n) { while (n-- > 0) { x >>= giOne; } return x; }\n"
		"int Check(unsigned long ul)\n"
		"{\n"
		"\tunsigned long\tx;\n"
		"\tgul = ul;\n"
		"\tul = gul;\n"
		"\tif (((ul << 1) != ShlL(ul, 1)) || ((ul >> 1) != ShrL(ul, 1)) || ((ul << 3) != ShlL(ul, 3)) || ((ul >> 5) != ShrL(ul, 5))) { return 0; }\n"
		"\tif (((ul << 7) != ShlL(ul, 7)) || ((ul >> 7) != ShrL(ul, 7)) || ((ul << 2) + (ul >> 6) != ShlL(ul, 2) + ShrL(ul, 6))) { return 0; }\n"
		"\tx = gul;\n"
		"\tx >>= 3;\n"
		"\tx <<= 2;\n"
		"\tx >>= 1;\n"
		"\tx <<= 1;\n"
		"\treturn x == ShlL(ShrL(ShlL(ShrL(ul, 3), 2), 1), 1);\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\treturn Check(0x89ABCDEFUL) && Check(0x12345678UL) && Check(0xFFFFFFFFUL) && Check(0x80000001UL) && Check(0x00018000UL) ? 1 : 0;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A 32 bit value whose one use is the next instruction, a multiply, divide
// or remainder by a scratch runtime routine (R816_Mul32, R816_UDiv32 ...),
// is worked out straight into the routine's operand bytes (CanFuse): as the
// first operand and the second, with an add's carry between its words.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ScratchOperands(void)
{
	AssertInt(1, TestCPP816Run(
		"unsigned long MulXor(unsigned long a, unsigned long b, unsigned long c) { return (a ^ b) * c; }\n"
		"unsigned long DivSum(unsigned long a, unsigned long b, unsigned long c) { return a / (b + c); }\n"
		"unsigned long RemDifference(unsigned long a, unsigned long b, unsigned long c) { return (a - b) % c; }\n"
		"long DivOr(long a, long b, long c) { return (a | b) / c; }\n"
		"unsigned long (*volatile gpMulXor)(unsigned long, unsigned long, unsigned long) = MulXor;\n"
		"unsigned long (*volatile gpDivSum)(unsigned long, unsigned long, unsigned long) = DivSum;\n"
		"unsigned long (*volatile gpRemDifference)(unsigned long, unsigned long, unsigned long) = RemDifference;\n"
		"long (*volatile gpDivOr)(long, long, long) = DivOr;\n"
		"int main(void)\n"
		"{\n"
		"\tif (gpMulXor(0x89ABCDEFUL, 0x13572468UL, 0x00012345UL) != 0x439F6663UL) { return 2; }\n"
		"\tif (gpDivSum(0xFEDCBA98UL, 0x01234567UL, 0x00071C3BUL) != 0xDAUL) { return 3; }\n"
		"\tif (gpRemDifference(0xFEDCBA98UL, 0x01234567UL, 0x00071C3BUL) != 0x117DCUL) { return 4; }\n"
		"\tif (gpDivOr((long)0x87654321UL, 0x00F00F00L, -1234L) != 1632063L) { return 5; }\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A load may have its pointer's place (PackTemporaries): a 1 word load, and
// a 2 word one when a phi joins them (p = p->psNext), whose second word is
// kept on the stack while the first, which overwrites the pointer, is read
// (Load).  Walked through lists, against the nodes' array.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816PointerPlaceLoads(void)
{
	AssertInt(1, TestCPP816Run(
		"struct SNode\n"
		"{\n"
		"\tSNode*\tpsNext;\n"
		"\tlong\tl;\n"
		"\tint\t\ti;\n"
		"};\n"
		"SNode\tgasNodes[20];\n"
		"long Sum(SNode* ps) { long l = 0; for (; ps; ps = ps->psNext) { l += ps->l; } return l; }\n"
		"int Last(SNode* ps) { while (ps->psNext) { ps = ps->psNext; } return ps->i; }\n"
		"long Second(SNode* ps) { SNode* psNext = ps->psNext; return psNext->l; }\n"
		"int Count(SNode* ps) { int i = 0; while (ps) { i += ps->i; ps = ps->psNext; } return i; }\n"
		"long (*volatile gpSum)(SNode*) = Sum;\n"
		"int (*volatile gpLast)(SNode*) = Last;\n"
		"long (*volatile gpSecond)(SNode*) = Second;\n"
		"int (*volatile gpCount)(SNode*) = Count;\n"
		"int main(void)\n"
		"{\n"
		"\tint\t\ti;\n"
		"\tlong\tl;\n"
		"\tint\t\tiCount;\n"
		"\tl = 0;\n"
		"\tiCount = 0;\n"
		"\tfor (i = 0; i < 20; i++)\n"
		"\t{\n"
		"\t\tgasNodes[i].psNext = (i < 19) ? &gasNodes[i + 1] : 0;\n"
		"\t\tgasNodes[i].l = 0x10003L * (long)(i + 1);\n"
		"\t\tgasNodes[i].i = i * 3 + 1;\n"
		"\t\tl += gasNodes[i].l;\n"
		"\t\tiCount += gasNodes[i].i;\n"
		"\t}\n"
		"\tif (gpSum(&gasNodes[0]) != l) { return 2; }\n"
		"\tif (gpLast(&gasNodes[3]) != 58) { return 3; }\n"
		"\tif (gpSecond(&gasNodes[4]) != 0x10003L * 6) { return 4; }\n"
		"\tif (gpCount(&gasNodes[0]) != iCount) { return 5; }\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit unsigned divides and remainders by divisors under 65536 (R816_UDiv32
// and R816_URem32, with the dividend's high word below the divisor and not),
// checked as q * d + r == x with r < d for pseudo random x and d of every
// size.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NarrowDivisors(void)
{
	AssertInt(1, TestCPP816Run(
		"volatile unsigned long gulX;\n"
		"volatile unsigned long gulD;\n"
		"int Check(void)\n"
		"{\n"
		"\tunsigned long\tx = gulX;\n"
		"\tunsigned long\td = gulD;\n"
		"\tunsigned long\tq = x / d;\n"
		"\tunsigned long\tr = x % gulD;\n"
		"\treturn (q * d + r == x) && (r < d);\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tunsigned long\tul;\n"
		"\tint\t\t\ti;\n"
		"\tul = 0x2468ACEUL;\n"
		"\tfor (i = 0; i < 400; i++)\n"
		"\t{\n"
		"\t\tul = ul * 1664525UL + 1013904223UL;\n"
		"\t\tgulX = ul;\n"
		"\t\tgulD = (ul >> (i & 15)) & 0xFFFF;\n"
		"\t\tif (gulD == 0) { gulD = 1; }\n"
		"\t\tif (!Check()) { return 100 + i; }\n"
		"\t\tgulX = ul | 0xFFFF0000UL;\n"
		"\t\tif (!Check()) { return 1000 + i; }\n"
		"\t}\n"
		"\tgulX = 0xFFFFFFFFUL; gulD = 1; if (!Check()) { return 2; }\n"
		"\tgulD = 0xFFFF; if (!Check()) { return 3; }\n"
		"\tgulD = 0x8000; if (!Check()) { return 4; }\n"
		"\tgulX = 0x00010000UL; gulD = 1; if (!Check()) { return 5; }\n"
		"\treturn 1;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Statements and operators: switch, do, 16 bit multiply, divide and shifts, unsigned long, function pointers,
// globals and a destructor.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Statements(void)
{
	AssertInt(1 + 20 + 300 + 4000, TestCPP816Run(
		"int Pick(int i)\n"
		"{\n"
		"\tswitch (i)\n"
		"\t{\n"
		"\tcase 0: return 1;\n"
		"\tcase 1: return 20;\n"
		"\tcase 5: return 300;\n"
		"\tdefault: return 4000;\n"
		"\t}\n"
		"}\n"
		"int main(void) { return Pick(0) + Pick(1) + Pick(5) + Pick(9); }\n"));

	//alignas on a class and on members lays them out as MSVC does (BaseLib reads and writes them as raw bytes), also
	//inside #pragma pack(1).
	AssertInt(8 + 8 * 10 + 16 * 100 + 8 * 1000, TestCPP816Run(
		"struct alignas(8) A { char c; };\n"
		"struct B { char c; alignas(4) char d; };\n"
		"struct C { char c; A a; };\n"
		"int main(void) { C c; return sizeof(A) + sizeof(B) * 10 + sizeof(C) * 100 + (int)((char*)&c.a - (char*)&c) * 1000; }\n"));
	AssertInt(4 + 8 * 10 + 4 * 100, TestCPP816Run(
		"struct alignas(long) D { char c; };\n"
		"#pragma pack(push, 1)\n"
		"struct P { char c; alignas(4) int i; };\n"
		"#pragma pack(pop)\n"
		"int main(void) { P p; return sizeof(D) + sizeof(P) * 10 + (int)((char*)&p.i - (char*)&p) * 100; }\n"));

	//Attributed statements ([[fallthrough]];, [[likely]] if) compile to the statements.
	AssertInt(3 + 2 + 0 + 1, TestCPP816Run(
		"int Fall(int i)\n"
		"{\n"
		"\tint r = 0;\n"
		"\tswitch (i)\n"
		"\t{\n"
		"\tcase 1:\n"
		"\t\tr += 1;\n"
		"\t\t[[fallthrough]];\n"
		"\tcase 2:\n"
		"\t\tr += 2;\n"
		"\t\tbreak;\n"
		"\t}\n"
		"\t[[likely]] if (i > 2) r++;\n"
		"\treturn r;\n"
		"}\n"
		"int main(void) { return Fall(1) + Fall(2) + Fall(0) + Fall(3); }\n"));

	AssertInt(720 / 7 + 720 % 7 + (720 >> 3) + (-720 >> 2) + (3 << 4), TestCPP816Run(
		"int Factorial(int n) { int r = 1; do { r *= n; n--; } while (n > 1); return r; }\n"
		"int main(void) { int f = Factorial(6); int m = -f; return f / 7 + f % 7 + (f >> 3) + (m >> 2) + (3 << 4); }\n", true));

	AssertInt(10, TestCPP816Run(
		"unsigned long Hash(const char* sz)\n"
		"{\n"
		"\tunsigned long h = 5381;\n"
		"\twhile (*sz) { h = h * 33 + (unsigned char)*sz; sz++; }\n"
		"\treturn h;\n"
		"}\n"
		"int main(void) { return (int)(Hash(\"816\") % 11); }\n"));

	AssertInt(3 * 4 + 3 + 4, TestCPP816Run(
		"int Add(int a, int b) { return a + b; }\n"
		"int Multiply(int a, int b) { return a * b; }\n"
		"int Apply(int (*fOperation)(int, int), int a, int b) { return fOperation(a, b); }\n"
		"int main(void) { return Apply(Multiply, 3, 4) + Apply(Add, 3, 4); }\n"));

	AssertInt(6 + 100, TestCPP816Run(
		"int giCount = 0;\n"
		"class CCounter\n"
		"{\n"
		"public:\n"
		"\tint\tmiAdd;\n"
		"\tCCounter(int iAdd) { miAdd = iAdd; giCount += iAdd; }\n"
		"\t~CCounter() { giCount += 100; }\n"
		"};\n"
		"int Make(void) { CCounter a(1); CCounter b(5); return giCount; }\n"
		"int main(void) { int i = Make(); return i + giCount - 206 + 100; }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Globals made at start up, in source order: constructed objects and a scalar with a non constant initialiser.
// Destructors run after main (and so can't change its result).  A global no one uses is dropped by the linker,
// with its start up routines, as on the 65C64.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Globals(void)
{
	AssertInt(3 + 40 + 7, TestCPP816Run(
		"int Seven(void) { return 7; }\n"
		"class CValue\n"
		"{\n"
		"public:\n"
		"\tint\tmiValue;\n"
		"\tCValue(int iValue) { miValue = iValue; }\n"
		"};\n"
		"CValue gcA(3);\n"
		"CValue gcB(40);\n"
		"int giSeven = Seven();\n"
		"int main(void) { return gcA.miValue + gcB.miValue + giSeven; }\n", true));

	AssertInt(12, TestCPP816Run(
		"int giMade = 0;\n"
		"class CMade\n"
		"{\n"
		"public:\n"
		"\tCMade() { giMade = giMade * 10 + 1; }\n"
		"\t~CMade() { giMade = 0; }\n"
		"};\n"
		"class CSecond\n"
		"{\n"
		"public:\n"
		"\tCSecond() { giMade = giMade * 10 + 2; }\n"
		"};\n"
		"CMade gcMade;\n"
		"CSecond gcSecond;\n"
		"int main(void) { return (&gcMade != 0) && (&gcSecond != 0) ? giMade : -1; }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// The builtins softfloat.cpp and the libraries use: __builtin_clz, ctz and popcount at each width (and L literals
// are long).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Builtins(void)
{
	AssertInt(-32768, TestCPP816Run("int main(void) { return (int)((1UL << 31) >> 16); }\n"));
	AssertInt(-32768, TestCPP816Run("int main(void) { unsigned long u = 1UL << 31; return (int)(u >> 16); }\n"));
	AssertInt(31, TestCPP816Run("int main(void) { unsigned long u = 1UL << 31; return __builtin_ctzl(u); }\n"));
	AssertInt(31, TestCPP816Run("int main(void) { return __builtin_ctzl(0x80000000UL); }\n"));
	AssertInt(4, TestCPP816Run("int main(void) { return sizeof(1UL); }\n"));
	AssertInt(1 + 14 * 10 + 3 * 100, TestCPP816Run(
		"int main(void) { unsigned int u = 0x4000; return __builtin_clz(u) + __builtin_ctz(u) * 10 + __builtin_popcount(0x7) * 100; }\n"));
	AssertInt(14 + 31 * 100, TestCPP816Run(
		"int main(void) { unsigned long u = 0x00024000; return __builtin_clzl(u) + __builtin_ctzl(1UL << 31) * 100; }\n"));
	AssertInt(10 + 63 * 100, TestCPP816Run(
		"int main(void) { unsigned long long u = 0x0020000000000000ULL; return __builtin_clzll(u) + __builtin_ctzll(1ULL << 63) * 100; }\n"));
	AssertInt(40, TestCPP816Run(
		"int main(void) { unsigned long long u = 0xFF00FF00FF00FF00ULL; return __builtin_popcountll(u) + __builtin_popcountll(0xFFULL); }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Loops that fill, copy, measure or compare memory, which the optimiser
// makes memset, memmove, strlen, memcmp, memchr and strchr calls
// (ReplaceLoopIdioms), give
// what the loops would: a fill of bytes and of 16 bit zeros, a copy of 16 bit
// elements, copies that overlap both ways (a copy up into itself repeats the
// first bytes, as the loop does, so it stays a loop), lengths counted by an
// index and by a pointer, and compares that differ first, in the middle,
// last or not at all, and searches by pointer, by index and to a terminator
// that find the first, middle, last, a zero or nothing.  Returns the answers
// wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816LoopIdioms(void)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };

	AssertInt(0, TestCPP816RunProject(aszUnits, "",
		"unsigned char	gauc[300];\n"
		"short			gas[100];\n"
		"short			gas2[100];\n"
		"void Fill(unsigned char* puc, unsigned char uc, long iCount)\n"
		"{\n"
		"	long	i;\n"
		"	for (i = 0; i < iCount; i++)\n"
		"	{\n"
		"		puc[i] = uc;\n"
		"	}\n"
		"}\n"
		"void Zero(short* ps, unsigned long uiCount)\n"
		"{\n"
		"	unsigned long	i;\n"
		"	for (i = 0; i < uiCount; i++)\n"
		"	{\n"
		"		ps[i] = 0;\n"
		"	}\n"
		"}\n"
		"void Copy(short* psTo, short* psFrom, unsigned long uiCount)\n"
		"{\n"
		"	unsigned long	i;\n"
		"	for (i = 0; i < uiCount; i++)\n"
		"	{\n"
		"		psTo[i] = psFrom[i];\n"
		"	}\n"
		"}\n"
		"int Length(const char* sz)\n"
		"{\n"
		"	int	i;\n"
		"	i = 0;\n"
		"	while (sz[i] != 0)\n"
		"	{\n"
		"		i++;\n"
		"	}\n"
		"	return i;\n"
		"}\n"
		"unsigned long PointerLength(const char* sz)\n"
		"{\n"
		"	const char*	pc;\n"
		"	pc = sz;\n"
		"	while (*pc)\n"
		"	{\n"
		"		pc++;\n"
		"	}\n"
		"	return (unsigned long)(pc - sz);\n"
		"}\n"
		"int Same(unsigned char* puc1, unsigned char* puc2, unsigned long uiCount)\n"
		"{\n"
		"	unsigned long	i;\n"
		"	for (i = 0; i < uiCount; i++)\n"
		"	{\n"
		"		if (puc1[i] != puc2[i])\n"
		"		{\n"
		"			return 0;\n"
		"		}\n"
		"	}\n"
		"	return 1;\n"
		"}\n"
		"const unsigned char* Find(const unsigned char* puc, unsigned char uc, unsigned long uiCount)\n"
		"{\n"
		"	unsigned long	i;\n"
		"	for (i = 0; i < uiCount; i++)\n"
		"	{\n"
		"		if (puc[i] == uc)\n"
		"		{\n"
		"			return &puc[i];\n"
		"		}\n"
		"	}\n"
		"	return 0;\n"
		"}\n"
		"long FindIndex(const unsigned char* puc, unsigned char uc, long iCount)\n"
		"{\n"
		"	long	i;\n"
		"	for (i = 0; i < iCount; i++)\n"
		"	{\n"
		"		if (puc[i] == uc)\n"
		"		{\n"
		"			return i;\n"
		"		}\n"
		"	}\n"
		"	return -1;\n"
		"}\n"
		"const char* Chr(const char* sz, char c)\n"
		"{\n"
		"	while (*sz != c)\n"
		"	{\n"
		"		if (*sz == 0)\n"
		"		{\n"
		"			return 0;\n"
		"		}\n"
		"		sz++;\n"
		"	}\n"
		"	return sz;\n"
		"}\n"
		"void CopyBytes(unsigned char* pucTo, unsigned char* pucFrom, long iCount)\n"
		"{\n"
		"	long	i;\n"
		"	for (i = 0; i < iCount; i++)\n"
		"	{\n"
		"		pucTo[i] = pucFrom[i];\n"
		"	}\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	long	i;\n"
		"	int		iWrong;\n"
		"	iWrong = 0;\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		gauc[i] = 1;\n"
		"	}\n"
		"	Fill(gauc + 3, 9, 200);\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		iWrong += gauc[i] != (((i >= 3) && (i < 203)) ? 9 : 1);\n"
		"	}\n"
		"	for (i = 0; i < 100; i++)\n"
		"	{\n"
		"		gas[i] = (short)(i * 3 - 50);\n"
		"		gas2[i] = 5;\n"
		"	}\n"
		"	Zero(gas2 + 10, 50);\n"
		"	Copy(gas2 + 70, gas + 20, 25);\n"
		"	for (i = 0; i < 100; i++)\n"
		"	{\n"
		"		iWrong += gas2[i] != (((i >= 10) && (i < 60)) ? 0 : (((i >= 70) && (i < 95)) ? (short)((i - 50) * 3 - 50) : 5));\n"
		"	}\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		gauc[i] = (unsigned char)i;\n"
		"	}\n"
		"	CopyBytes(gauc + 1, gauc, 100);\n"
		"	CopyBytes(gauc + 150, gauc + 160, 100);\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		iWrong += gauc[i] != ((i <= 100) ? 0 : (((i >= 150) && (i < 250)) ? (unsigned char)(i + 10) : (unsigned char)i));\n"
		"	}\n"
		"	iWrong += (Length(\"\") != 0) + (Length(\"Hello world\") != 11) + (PointerLength(\"\") != 0) + (PointerLength(\"Hello\") != 5);\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		gauc[i] = (i == 250) ? 0 : 'a';\n"
		"	}\n"
		"	iWrong += (Length((char*)gauc) != 250) + (PointerLength((char*)gauc + 10) != 240);\n"
		"	iWrong += (Same(gauc, gauc + 1, 200) != 1) + (Same(gauc, gauc + 1, 0) != 1);\n"
		"	gauc[0] = 'b';\n"
		"	iWrong += (Same(gauc, gauc + 1, 200) != 0);\n"
		"	gauc[0] = 'a';\n"
		"	gauc[100] = 'c';\n"
		"	iWrong += (Same(gauc, gauc + 1, 200) != 0) + (Same(gauc + 1, gauc + 2, 98) != 1);\n"
		"	gauc[100] = 'a';\n"
		"	gauc[200] = 'c';\n"
		"	iWrong += (Same(gauc, gauc + 1, 200) != 0) + (Same(gauc, gauc + 1, 199) != 1);\n"
		"	gauc[0] = 'x';\n"
		"	gauc[120] = 'y';\n"
		"	gauc[249] = 'z';\n"
		"	iWrong += (Find(gauc, 'x', 250) != gauc) + (Find(gauc, 'y', 250) != gauc + 120) + (Find(gauc, 'z', 250) != gauc + 249);\n"
		"	iWrong += (Find(gauc, 'z', 249) != 0) + (Find(gauc, 'q', 300) != 0) + (Find(gauc, 'x', 0) != 0) + (Find(gauc + 1, 0, 299) != gauc + 250);\n"
		"	iWrong += (FindIndex(gauc, 'x', 250) != 0) + (FindIndex(gauc, 'y', 250) != 120) + (FindIndex(gauc, 'z', 250) != 249);\n"
		"	iWrong += (FindIndex(gauc, 'z', 249) != -1) + (FindIndex(gauc, 'q', 300) != -1) + (FindIndex(gauc, 'x', 0) != -1);\n"
		"	iWrong += (Chr((char*)gauc, 'x') != (char*)gauc) + (Chr((char*)gauc, 'y') != (char*)gauc + 120) + (Chr((char*)gauc, 'z') != (char*)gauc + 249);\n"
		"	iWrong += (Chr((char*)gauc, 'q') != 0) + (Chr((char*)gauc, 0) != (char*)gauc + 250) + (Chr(\"\", 'a') != 0) + (Chr(\"\", 0) == 0);\n"
		"	return iWrong;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// MemSwp in stringW65C816.asm (BaseLib's is left out for the W65C816): every
// length from 0 to 40 and 255 to 257, both buffers swapped and the bytes
// after them unchanged.  Returns the bytes wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MemSwp(void)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };

	AssertInt(0, TestCPP816RunProject(aszUnits, "",
		"void MemSwp(const void* pv1, const void* pv2, unsigned long uiLength);\n"
		"unsigned char gauc1[300];\n"
		"unsigned char gauc2[300];\n"
		"int Check(unsigned long uiLength)\n"
		"{\n"
		"	unsigned long	i;\n"
		"	int				iWrong;\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		gauc1[i] = (unsigned char)(i + 1);\n"
		"		gauc2[i] = (unsigned char)(i * 3 + 7);\n"
		"	}\n"
		"	MemSwp(gauc1, gauc2, uiLength);\n"
		"	iWrong = 0;\n"
		"	for (i = 0; i < 300; i++)\n"
		"	{\n"
		"		if (gauc1[i] != ((i < uiLength) ? (unsigned char)(i * 3 + 7) : (unsigned char)(i + 1)))\n"
		"		{\n"
		"			iWrong++;\n"
		"		}\n"
		"		if (gauc2[i] != ((i < uiLength) ? (unsigned char)(i + 1) : (unsigned char)(i * 3 + 7)))\n"
		"		{\n"
		"			iWrong++;\n"
		"		}\n"
		"	}\n"
		"	return iWrong;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"	unsigned long	ul;\n"
		"	int				iWrong;\n"
		"	iWrong = 0;\n"
		"	for (ul = 0; ul <= 40; ul++)\n"
		"	{\n"
		"		iWrong += Check(ul);\n"
		"	}\n"
		"	iWrong += Check(255) + Check(256) + Check(257);\n"
		"	return iWrong;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib code with the Libc65 code it needs, as TestCPP65GeneratorBaseLib runs it on the 65C64.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BaseLib(void)
{
	static const char*	aszMD5[] = { "BaseLib/MD5.cpp", "Libc65/string.cpp", "Libc65/stringW65C816.asm", "Input/CPP65/MD5Project/Main.cpp", NULL };
	static const char*	aszMersenneTwister[] = { "BaseLib/MersenneTwister.cpp", "Libc65/time.cpp", "Input/CPP65/MersenneTwisterProject/Main.cpp", NULL };
	static const char*	aszSort[] = { "BaseLib/QuickSort.cpp", "BaseLib/BubbleSort.cpp", "BaseLib/BinarySearch.cpp", "Input/CPP65/SortProject/MemSwpStub.cpp", "Input/CPP65/SortProject/Main.cpp", NULL };
	static const char*	aszRedBlackTree[] = { "BaseLib/RedBlackTree.cpp", "Libc65/string.cpp", "Libc65/stringW65C816.asm", "Libc65/stdlib.cpp", "BaseLib/TrueMallocator.cpp", "Libc65/ctype.cpp", "Input/CPP65/RedBlackTreeProject/FreeListStub.cpp", "Input/CPP65/RedBlackTreeProject/Main.cpp", NULL };
	static const char*	aszStatic[] = { "Input/CPP65/StaticProject/Other.cpp", NULL };

	AssertInt(0x3F, TestCPP816RunProject(aszMD5, "", NULL));
	AssertInt(0xF, TestCPP816RunProject(aszRedBlackTree, "", NULL));
	AssertInt(11 + 22 * 100, TestCPP816RunProject(aszStatic, "", "static int giValue = 10;\nstatic int Value(void) { return 1; }\nint Other(void);\nint main() { return Value() + giValue + Other() * 100; }\n"));
	//Every 32 bit multiply and remainder is a runtime call (Init and the twists' % 624): 57M instructions
	//(2026-10-25), before step 7's optimisation.
	AssertInt(0xFF, TestCPP816RunProject(aszMersenneTwister, "", NULL, false, NULL, 0, TEST_CPP816_SLOW_STEPS));
	AssertInt(0x7, TestCPP816RunProject(aszSort, "", NULL));
}


//////////////////////////////////////////////////////////////////////////
//
// A unit compiled again keeps what it defines (Input/CPP65/CompiledAgainProject).  Counter.cpp is nothing but its
// includes: Counter.inl defines CCounter's members and Thrice, none of them inline.  It is compiled before
// Other.cpp, whose Twice they call, so it is compiled again once Twice is exported; by then the header index has
// seen Counter.inl, and the definitions were left unparsed with the class nothing in the unit names, so the
// unit's object was empty and the program didn't link.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816CompiledAgain(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/CompiledAgainProject/Counter.cpp", "Input/CPP65/CompiledAgainProject/Other.cpp", "Input/CPP65/CompiledAgainProject/Main.cpp", NULL };

	AssertInt(21, TestCPP816RunProject(aszUnits, "", NULL));
}


//////////////////////////////////////////////////////////////////////////
//
// Sprite drawing in C style (Input/CPP65/SpriteProject: Sprite.h and Sprite.cpp as given, Image.cpp's DrawPixel on a
// 320 x 200 byte screen at 0011:0000): a container of an opaque and a transparent sprite, iterated and drawn.  The
// expected values are from the same sources built natively (with DrawPixel on an array): 20 pixels set, 2 images and
// the screen's hash.  DrawPixel is inlined across the units.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Sprites(void)
{
	static const char*	aszSprites[] = { "Input/CPP65/SpriteProject/Sprite.cpp", "Input/CPP65/SpriteProject/Image.cpp", "Libc65/stringW65C816.asm", "Input/CPP65/SpriteProject/Main.cpp", NULL };
	uint32				auiResults[2];

	CCPP65Builder		cBuilder;
	CChars				szProject;
	CFileUtil			cFileUtil;
	int64				iAddress;

	AssertInt(20, TestCPP816RunProject(aszSprites, "", NULL, true, auiResults, 2));
	AssertIntHex(974038828, auiResults[0]);
	AssertInt(2, auiResults[1]);

	//DrawPixel (Image.cpp) is inlined into DrawSprite (Sprite.cpp, compiled before it, so again at the end) and into
	//nothing else: it is left out of the program.
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	TestCPP65AddUnits(&cBuilder, aszSprites, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());
	AssertTrue(cBuilder.GetSymbol("_DrawSprite__FssPC13SSpriteHeader", &iAddress));
	AssertFalse(cBuilder.GetSymbol("_DrawPixel__Fssh", &iAddress));
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Functions inlined across units that name their unit's own symbols (a
// string, a static global, a static local) aren't exported: the names mean
// other things in the unit calling them.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816LocalSymbols(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/LocalSymbolsProject/Name.cpp", "Libc65/stringW65C816.asm", "Input/CPP65/LocalSymbolsProject/Main.cpp", NULL };

	AssertInt(7, TestCPP816RunProject(aszUnits, "", NULL));
}


//////////////////////////////////////////////////////////////////////////
//
// A loop that only steps q and x until x is x0 + d (when d isn't 0) goes:
// q is p + d after it.  x wraps past 65535 in the second call.  And a loop
// counter times a value the loop doesn't change, wrapping.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ExitValues(void)
{
	//y * w (16 bit, wrapping past 65535) as a phi stepped by w (ReduceMultiplies).
	AssertInt(-19760,
		TestCPP816Run(
			"unsigned int F(unsigned int y0, unsigned int w, unsigned int n)\n"
			"{\n\tunsigned int s = 0;\n\tfor (unsigned int y = y0; y < n; y++)\n\t{\n\t\ts ^= y * w;\n\t}\n\treturn s;\n}\n"
			"int main(void) { return (int)F(3, 1000, 90); }\n", true));
	AssertInt(-23988,
		TestCPP816Run(
			"unsigned int G(unsigned int y, unsigned int w)\n"
			"{\n\tunsigned int s = 0;\n\tfor (unsigned int i = 0; i < 200; i++)\n\t{\n\t\tif ((i % 3) == 0)\n\t\t{\n\t\t\ty++;\n\t\t}\n\t\ts ^= y * w;\n\t}\n\treturn s;\n}\n"
			"int main(void) { return (int)G(5, 700); }\n", true));
	AssertInt(300 + 2 + 0,
		TestCPP816Run(
			"int Skip(const char* p, unsigned int x0, unsigned int d)\n"
			"{\n\tconst char* q = p;\n\tunsigned int x = x0;\n"
			"\tif (d > 0)\n\t{\n\t\tdo\n\t\t{\n\t\t\tq++;\n\t\t\tx++;\n\t\t} while (x != x0 + d);\n\t}\n"
			"\treturn (int)(q - p);\n}\n"
			"int main(void) { char a[1]; return Skip(a, 65531, 300) + Skip(a, 65535, 2) + Skip(a, 7, 0); }\n", true));
}


//////////////////////////////////////////////////////////////////////////
//
// Optimisations whose IR tests are in TestIR, run: a load isn't moved past
// a store that may write it (SinkLoads), an instruction both ways of an if
// work out is worked out once (HoistCommon), and a loop's counter is
// replaced by the other value it steps (ReplaceLoopCounters), n = 0 too.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MovedCode(void)
{
	AssertInt(10,
		TestCPP816Run(
			"int H(int* p, int* q, int f) { int v = *p; if (f) { *q = 5; return v; } return 0; }\n"
			"int main(void) { int a = 1; int r = H(&a, &a, 1) * 10; return r + H(&a, &a, 0); }\n", true));
	AssertInt((3 * 4 + 1 + 100) + (5 * 2 - 1),
		TestCPP816Run(
			"int K(int a, int b, int f) { int c = a < b; int r; if (f) r = a * b + 1; else r = a * b - 1; if (c) r += 100; return r; }\n"
			"int main(void) { return K(3, 4, 1) + K(5, 2, 0); }\n", true));
	AssertInt(3 + 4 + 5 + 6 + 7,
		TestCPP816Run(
			"int L(const char* p, int x0, int n) { int s = 0; int x = x0; for (int i = 0; i < n; i++) { s += p[x]; x++; } return s; }\n"
			"int main(void) { char a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }; return L(a, 2, 5) + L(a, 0, 0) * 100; }\n", true));
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib units compiled for the W65C816 without errors: the IR of each
// function verifies after the optimiser (Float32Vec2TransformCoords's two
// muls of its counter once read a moved instruction's result, and the
// first parameter was replaced).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816CompileBaseLibUnits(void)
{
	static const char*	aszUnits[] = { "BaseLib/Float32Vec2.cpp", "BaseLib/Float32Vec3.cpp", "BaseLib/Float32Vec4.cpp", NULL };
	CCPP65Builder		cBuilder;

	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A restrict global pointer promises that what it reaches isn't reached
// another way.  Here it breaks the promise, pointing at the int the loop
// reads: with __restrict the read leaves the loop and sees the int before
// the stores (0x101 three times); without it each read sees them (0x100,
// then 0, then 0).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Restrict(void)
{
	static const char*	szSource =
		"int giWidth;\n"
		"unsigned char* RESTRICT gpuMemory;\n"
		"int Sum(int n) { int s = 0; for (int i = 0; i < n; i++) { gpuMemory[i] = 0; s += giWidth; } return s; }\n"
		"int main(void) { gpuMemory = (unsigned char*)&giWidth; giWidth = 0x101; return Sum(3); }\n";
	CChars	szRestrict;
	CChars	szPlain;

	szRestrict.Init("#define RESTRICT __restrict\n");
	szRestrict.Append(szSource);
	szPlain.Init("#define RESTRICT\n");
	szPlain.Append(szSource);
	AssertInt(0x303, TestCPP816Run(szRestrict.Text()));
	AssertInt(0x100, TestCPP816Run(szPlain.Text()));
	szPlain.Kill();
	szRestrict.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A restrict parameter breaking its promise in the same way: p is giWidth.
// Sum is called through a volatile pointer so it isn't inlined (restrict is
// dropped then).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816RestrictParameter(void)
{
	static const char*	szSource =
		"int giWidth;\n"
		"int Sum(unsigned char* RESTRICT p, int n) { int s = 0; for (int i = 0; i < n; i++) { p[i] = 0; s += giWidth; } return s; }\n"
		"int (* volatile gpfSum)(unsigned char*, int) = Sum;\n"
		"int main(void) { giWidth = 0x101; return gpfSum((unsigned char*)&giWidth, 3); }\n";
	CChars	szRestrict;
	CChars	szPlain;

	szRestrict.Init("#define RESTRICT __restrict\n");
	szRestrict.Append(szSource);
	szPlain.Init("#define RESTRICT\n");
	szPlain.Append(szSource);
	AssertInt(0x303, TestCPP816Run(szRestrict.Text()));
	AssertInt(0x100, TestCPP816Run(szPlain.Text()));
	szPlain.Kill();
	szRestrict.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Tide/Video's sprites (Input/CPP65/TideVideoProject: Image.c, Image.h, Sprite.c and Sprite.h as they are but for
// guiImageMemory declared __restrict, Main.c as MainSprites.c without its font and heap) drawn from Input/SFT/Graphics.sft (put at 0008:0000) into an 8 bit
// 320 x 200 screen at 0011:0000: the background, Maki standing and walking (clipped at both sides).  The expected
// hash is the same sources built natively; the screen is saved to Output/Graphics.raw.  DrawPixel, which reads
// globals, is inlined across the units.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816TideVideo(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/TideVideoProject/Image.c", "Input/CPP65/TideVideoProject/Sprite.c", "Libc65/stringW65C816.asm", "Input/CPP65/TideVideoProject/Main.c", NULL };
	STestCPP816Memory	sMemory;
	CFileBasic			cFile;
	CFileUtil			cFileUtil;
	CCPP65Builder		cBuilder;
	CChars				szProject;
	int64				iAddress;
	uint8*				puiSft;
	uint8*				puiScreen;
	size				uiSftBytes;
	uint32				auiResults[2];

	cFile.Init(DiskFile("Input" _FS_ "SFT" _FS_ "Graphics.sft"));
	AssertTrue(cFile.Open(EFM_Read));
	uiSftBytes = (size)cFile.Size();
	AssertInt(77719, (int)uiSftBytes);
	puiSft = (uint8*)malloc(uiSftBytes);
	AssertInt(1, (int)cFile.Read(puiSft, uiSftBytes, 1));
	cFile.Close();
	cFile.Kill();

	puiScreen = (uint8*)malloc(320 * 200);
	sMemory.puiLoad = puiSft;
	sMemory.uiLoadBytes = uiSftBytes;
	sMemory.uiLoadAddress = 0x080000;
	sMemory.puiSave = puiScreen;
	sMemory.uiSaveBytes = 320 * 200;
	sMemory.uiSaveAddress = 0x110000;
	AssertInt(8, TestCPP816RunProject(aszUnits, "", NULL, false, auiResults, 2, TEST_CPP816_STEPS, false, &sMemory));
	AssertIntHex(0xD06A69C6, auiResults[0]);
	AssertInt(8, auiResults[1]);

	cFileUtil.MakeDir("Output");
	cFile.Init(DiskFile("Output" _FS_ "Graphics.raw"));
	AssertTrue(cFile.Open(EFM_Write_Create));
	cFile.Write(puiScreen, 320 * 200, 1);
	cFile.Close();
	cFile.Kill();
	free(puiScreen);
	free(puiSft);

	//DrawPixel (Image.c, which reads Image.c's globals) is inlined into DrawSprite (Sprite.c) across the units, the
	//globals named the same: it is left out of the program.
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());
	AssertTrue(cBuilder.GetSymbol("_DrawSprite__FssPC13SSpriteHeader", &iAddress));
	AssertTrue(cBuilder.GetSymbol("_guiImageWidth", &iAddress));
	AssertFalse(cBuilder.GetSymbol("_DrawPixel__Fssh", &iAddress));
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's FloatHelper and SQuaternion with softfloat.cpp (and math.cpp) compiled for the W65C816, against the
// same calls natively, as on the 65C64.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Floats(void)
{
	static const char*	aszFloatHelper[] = { "BaseLib/FloatHelper.cpp", "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", "Libc65/math.cpp", "Libc65/stringW65C816.asm", "Input/CPP65/FloatHelperProject/Main.cpp", NULL };
	static const char*	aszQuaternion[] = { "BaseLib/Quaternion.cpp", "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", "Libc65/stringW65C816.asm", "Input/CPP65/QuaternionProject/Main.cpp", NULL };
	uint32				auiResults[256];
	size				i;
	uint32				uiExpected;

	gaiFloatHelperResults.Init();
	FloatHelperExpected();
	AssertTrue(gaiFloatHelperResults.NumElements() <= 256);
	AssertInt((int)gaiFloatHelperResults.NumElements(), TestCPP816RunProject(aszFloatHelper, "", NULL, false, auiResults, gaiFloatHelperResults.NumElements(), TEST_CPP816_SLOW_STEPS));
	for (i = 0; i < gaiFloatHelperResults.NumElements(); i++)
	{
		AssertIntHex((uint32)gaiFloatHelperResults.GetValue(i), auiResults[i]);
	}
	gaiFloatHelperResults.Kill();

	gaiQuaternionResults.Init();
	gaiQuaternionIsFloat.Init();
	QuaternionExpected();
	AssertTrue(gaiQuaternionResults.NumElements() <= 256);
	AssertInt((int)gaiQuaternionResults.NumElements(), TestCPP816RunProject(aszQuaternion, "", NULL, false, auiResults, gaiQuaternionResults.NumElements(), TEST_CPP816_SLOW_STEPS));
	for (i = 0; i < gaiQuaternionResults.NumElements(); i++)
	{
		uiExpected = (uint32)gaiQuaternionResults.GetValue(i);
#ifndef _DEBUG
		//Release is /fp:fast, which can reorder or fuse the native float maths and move its result by an ulp.
		if (gaiQuaternionIsFloat.GetValue(i) && (TestCPP65FloatUlps(auiResults[i], uiExpected) <= 1))
		{
			uiExpected = auiResults[i];
		}
#endif // _DEBUG
		AssertIntHex(uiExpected, auiResults[i]);
	}
	gaiQuaternionIsFloat.Kill();
	gaiQuaternionResults.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A builder for TestCPP816NoFloats: the W65C816, the start up and the math
// tables, and no floats if bNoFloats.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816NoFloatsBuilder(CCPP65Builder* pcBuilder, bool bNoFloats)
{
	CFileUtil	cFileUtil;
	CChars		szLibc65;

	pcBuilder->Init();
	pcBuilder->SetTarget(CPPT_W65C816);
	pcBuilder->SetNoFloats(bNoFloats);
	TestCPP65AddIncludeDirectories(pcBuilder);
	AssertTrue(pcBuilder->AddStartUp());
	szLibc65.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szLibc65);
	pcBuilder->AddMathTables(szLibc65.Text());
	szLibc65.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A build with no floats (CCPP65Builder::SetNoFloats, the HCP65's kernal):
// floating point in the program's own code is an error at its statement;
// a header's float declarations that aren't used are not; and a library
// function that uses the soft float inside (Half.cpp, no float in its
// signature) is a link error naming the function.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NoFloats(void)
{
	CCPP65Builder	cBuilder;
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	CChars			szLibc65;
	bool			bLinked;

	TestCPP816NoFloatsBuilder(&cBuilder, true);
	AssertFalse(cBuilder.AddSource("Test.cpp", "int main()\n{\n\tint i = 3;\n\tfloat f = (float)i;\n\treturn (int)(f * 2.0f);\n}\n"));
	AssertTrue(strstr(cBuilder.GetErrors(), "Floating point is used here") != NULL);
	AssertTrue(strstr(cBuilder.GetErrors(), "Test.cpp") != NULL);
	AssertTrue(strstr(cBuilder.GetErrors(), "(4") != NULL);
	cBuilder.Kill();

	TestCPP816NoFloatsBuilder(&cBuilder, true);
	AssertTrue(cBuilder.AddSource("Test.cpp", "#include <math.h>\nfloat Scale(float f);\nint main() { return 1; }\n"));
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());
	cBuilder.Kill();

	szDirectory.Init("Input" _FS_ "CPP65" _FS_ "NoFloats");
	cFileUtil.FullPath(&szDirectory);
	szLibc65.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szLibc65);

	TestCPP816NoFloatsBuilder(&cBuilder, false);
	AssertTrue(cBuilder.AddSource("Test.cpp", "int Half(int i);\nint main() { return Half(8); }\n"));
	cBuilder.AddUnit("NoFloats", szDirectory.Text(), "Half.cpp", true);
	cBuilder.AddUnit("Libc65", szLibc65.Text(), "softfloatW65C816.asm", true);
	AssertTrue(cBuilder.Link());
	cBuilder.Kill();

	TestCPP816NoFloatsBuilder(&cBuilder, true);
	AssertTrue(cBuilder.AddSource("Test.cpp", "int Half(int i);\nint main() { return Half(8); }\n"));
	cBuilder.AddUnit("NoFloats", szDirectory.Text(), "Half.cpp", true);
	cBuilder.AddUnit("Libc65", szLibc65.Text(), "softfloatW65C816.asm", true);
	bLinked = cBuilder.Link();
	AssertFalse(bLinked);
	AssertTrue(strstr(cBuilder.GetErrors(), "Floating point is used, and this build has none") != NULL);
	AssertTrue(strstr(cBuilder.GetErrors(), "Half") != NULL);
	AssertTrue(strstr(cBuilder.GetErrors(), "_SoftFloat") != NULL);
	cBuilder.Kill();

	szLibc65.Kill();
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// new and delete over Libc65's operator new, operator delete and malloc, as TestCPP65GeneratorNewDelete.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NewDelete(void)
{
	static const char*	aszUnits[] = { "Libc65/new.cpp", "Libc65/stdlib.cpp", "BaseLib/TrueMallocator.cpp", "Libc65/string.cpp", "Libc65/stringW65C816.asm", "Libc65/ctype.cpp", NULL };

	AssertInt(7, TestCPP816RunProject(aszUnits, "", "int main() { int* p = new int(7); int r = *p; delete p; return r; }\n"));

	AssertInt(2600 + 50 + 5, TestCPP816RunProject(aszUnits, "",
		"int giMade;\n"
		"int giGone;\n"
		"struct C { int m; C() : m(3) { giMade++; } C(int v) : m(v) { giMade++; } ~C() { giGone++; } };\n"
		"int main()\n"
		"{\n"
		"	C* p = new C(5);\n"
		"	int r = p->m;\n"
		"	delete p;\n"
		"	C* a = new C[4];\n"
		"	for (int i = 0; i < 4; i++) r += a[i].m;\n"
		"	delete[] a;\n"
		"	int* q = new int[10];\n"
		"	for (int i = 0; i < 10; i++) q[i] = i;\n"
		"	r += q[9];\n"
		"	delete[] q;\n"
		"	delete (C*)0;\n"
		"	return r * 100 + giMade * 10 + giGone;\n"
		"}\n"));

	AssertInt(321, TestCPP816RunProject(aszUnits, "",
		"int giLog;\n"
		"struct B { virtual ~B() { giLog = giLog * 10 + 1; } };\n"
		"struct D : B { ~D() { giLog = giLog * 10 + 2; } };\n"
		"struct E { int m; E() : m(0) {} ~E() { giLog = giLog * 10 + 3; } };\n"
		"int main()\n"
		"{\n"
		"	E* e = new E[1];\n"
		"	delete[] e;\n"
		"	B* p = new D;\n"
		"	delete p;\n"
		"	return giLog;\n"
		"}\n"));

	AssertInt(1000, TestCPP816RunProject(aszUnits, "",
		"int main()\n"
		"{\n"
		"	int n = 0;\n"
		"	for (int i = 0; i < 1000; i++) { int* p = new int[1000]; if (p) n++; delete[] p; }\n"
		"	return n;\n"
		"}\n"));

	//new T() and new T{} value initialise: a class without constructors of its own is zeroed (arrays in it too), in
	//place or (big ones) with memset, before a member's constructor runs.  The heap block is dirtied first.
	AssertInt(7, TestCPP816RunProject(aszUnits, "",
		"#include <stdlib.h>\n"
		"#include <string.h>\n"
		"struct SPlain { int a; long b[4]; };\n"
		"struct SBig { long c[40]; };\n"
		"class CMember { public: int m; CMember() { m = 7; } };\n"
		"struct SMixed { CMember cm; long d[3]; };\n"
		"void Dirty(int n) { void* p = malloc(n); memset(p, 0x55, n); free(p); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Dirty(sizeof(SPlain));\n"
		"	SPlain* a = new SPlain();\n"
		"	if ((a->a != 0) || (a->b[0] != 0) || (a->b[3] != 0)) r |= 1;\n"
		"	Dirty(sizeof(SBig));\n"
		"	SBig* b = new SBig{};\n"
		"	if ((b->c[0] != 0) || (b->c[39] != 0)) r |= 2;\n"
		"	Dirty(sizeof(SMixed));\n"
		"	SMixed* c = new SMixed();\n"
		"	if ((c->d[0] != 0) || (c->d[2] != 0)) r |= 4;\n"
		"	if (c->cm.m != 7) r |= 8;\n"
		"	delete a;\n"
		"	delete b;\n"
		"	delete c;\n"
		"	return r + 7;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Bugs found running TestBaseLib on the W65C816.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816TestBaseLibBugs(void)
{
	//Pointers compare as 24 bit addresses (across banks).
	AssertInt(1 + 2 + 4 + 8 + 16, TestCPP816Run(
		"int Compare(char* pa, char* pb, char* pc)\n"
		"{\n"
		"\treturn (pa < pb) + (pb > pa) * 2 + (pc >= pa) * 4 + (pa <= pb) * 8 + ((pc >= pa) && (pc < pb)) * 16;\n"
		"}\n"
		"int main(void) { return Compare((char*)0x1FFF0, (char*)0x30010, (char*)0x20005); }\n"));

	static const char*	aszMersenneTwister[] = { "BaseLib/MersenneTwister.cpp", "Libc65/time.cpp", NULL };

	//CMersenneTwister::Next(int, int) gives the same numbers as natively (18248 is the native checksum).
	AssertInt(18248, TestCPP816RunProject(aszMersenneTwister, "",
		"#include \"BaseLib/MersenneTwister.h\"\n"
		"int main()\n"
		"{\n"
		"\tCMersenneTwister c;\n"
		"\tint s = 0;\n"
		"\tc.Init(3456897);\n"
		"\tfor (int i = 0; i < 100; i++) { int a = c.Next(0, 580 - i); int b = c.Next(0, 2); s = (s * 31 + a * 3 + b) & 0x7FFF; }\n"
		"\treturn s;\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS));

	//An object whose copy isn't its bytes is passed by the address of a copy the caller makes and destroys.
	AssertInt(7 + 2 * 10 + 2 * 100 + 9 * 1000, TestCPP816Run(
		"int giCopies;\n"
		"int giGone;\n"
		"class CHeld\n"
		"{\n"
		"public:\n"
		"\tint\tmi;\n"
		"\tCHeld(int i) { mi = i; }\n"
		"\tCHeld(const CHeld& c) { mi = c.mi; giCopies++; }\n"
		"\t~CHeld() { giGone++; }\n"
		"};\n"
		"int Value(CHeld c, int iAdd) { int i = c.mi + iAdd; c.mi = 1000; return i; }\n"
		"int main(void)\n"
		"{\n"
		"\tint r;\n"
		"\tint s;\n"
		"\t{\n"
		"\t\tCHeld h(5);\n"
		"\t\tr = Value(h, 2);\n"
		"\t\ts = Value(h, 4) + h.mi - 5;\n"
		"\t}\n"
		"\treturn r + giCopies * 10 + (giGone - 1) * 100 + s * 1000;\n"
		"}\n"));

	AssertInt(1 + 20, TestCPP816RunProject(NULL, "",
		"#include <string.h>\n"
		"#include \"BaseLib/TypeConverter.h\"\n"
		"#include \"BaseLib/NumberControl.h\"\n"
		"#include \"BaseLib/Calculator.h\"\n"
		"#include \"BaseLib/CalculatorParser.h\"\n"
		"int Eval(const char* sz, const char* szExpected)\n"
		"{\n"
		"\tCCalculator\t\t\tcCalculator;\n"
		"\tCCalculatorParser\tcParser;\n"
		"\tCNumber\t\t\t\tcResult;\n"
		"\tCCalcExpression*\tpcExpression;\n"
		"\tCCalculatorSymbols\tcSymbols;\n"
		"\tCChars\t\t\t\tszResult;\n"
		"\tint\t\t\t\t\tiSame;\n"
		"\tcSymbols.Init(false);\n"
		"\tcCalculator.Init(&cSymbols);\n"
		"\tcParser.Init(&cCalculator);\n"
		"\tpcExpression = (CCalcExpression*)cParser.Parse(sz);\n"
		"\tcParser.Kill();\n"
		"\tcResult = cCalculator.Eval(pcExpression);\n"
		"\tszResult.Init();\n"
		"\tcResult.Print(&szResult);\n"
		"\tiSame = strcmp(szResult.Text(), szExpected) == 0;\n"
		"\tszResult.Kill();\n"
		"\tcCalculator.Kill();\n"
		"\tdelete pcExpression;\n"
		"\tcSymbols.Kill();\n"
		"\treturn iSame;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tTypeConverterInit();\n"
		"\tNumberInit();\n"
		"\treturn Eval(\"-1\", \"-1\") + Eval(\"1 == -1\", \"0\") * 20;\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));

	AssertInt(3 + 40 + 500, TestCPP816RunProject(NULL, "",
		"long double Half(long double ld) { return ld / 2.0L; }\n"
		"int main(void)\n"
		"{\n"
		"\tlong double ld = 1.5;\n"
		"\tdouble d = (double)(ld * 2);\n"
		"\tlong double l2 = 0.0L;\n"
		"\tl2 = l2 * 10 + 4;\n"
		"\treturn (int)d + (int)(l2 * 10) + (int)(Half(1000.0L));\n"
		"}\n", false, NULL, 0, TEST_CPP816_STEPS, true));

	AssertInt(14, TestCPP816RunProject(NULL, "",
		"#include \"BaseLib/Number.h\"\n"
		"#include \"BaseLib/NumberControl.h\"\n"
		"int main(void)\n"
		"{\n"
		"\tCNumber\ta;\n"
		"\tCNumber\tb;\n"
		"\tNumberInit();\n"
		"\ta.Init(100);\n"
		"\tb.Init(7);\n"
		"\ta.Divide(&b);\n"
		"\treturn a.IntValue();\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));
	AssertInt(1, TestCPP816RunProject(NULL, "",
		"#include <string.h>\n"
		"#include \"BaseLib/FloatPrinter.h\"\n"
		"#include \"BaseLib/NumberControl.h\"\n"
		"int main(void)\n"
		"{\n"
		"\tchar\tsz[64];\n"
		"\tNumberInit();\n"
		"\tDoubleToString(sz, 64, 345.34, 2);\n"
		"\treturn strcmp(sz, \"345.34\") == 0;\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));
	AssertInt(1 + 20 + 300, TestCPP816RunProject(NULL, "",
		"#include \"BaseLib/Number.h\"\n"
		"#include \"BaseLib/NumberControl.h\"\n"
		"int main(void)\n"
		"{\n"
		"\tCNumber\ta;\n"
		"\tCNumber\tb;\n"
		"\tNumberInit();\n"
		"\ta.Init(1);\n"
		"\tb.Init(-1);\n"
		"\treturn (a.Equals(&b) ? 0 : 1) + (b.IsNegative() ? 20 : 0) + ((b.IntValue() == -1) ? 300 : 0);\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));

	static const char*	aszFloat[] = { "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", NULL };

	AssertInt(0x407, TestCPP816RunProject(aszFloat, "",
		"int Exponent(double f, int iShift)\n"
		"{\n"
		"\tunsigned char* pui = (unsigned char*)&f;\n"
		"\treturn (int)((*((long long*)pui) & 0x7ff0000000000000) >> iShift);\n"
		"}\n"
		"int main(void) { return Exponent(345.34, 52); }\n"));
	AssertInt(0x407, TestCPP816RunProject(aszFloat, "",
		"template<class FLOAT, class INTEGER>\n"
		"int Exponent(FLOAT f, INTEGER iMask, int iShift)\n"
		"{\n"
		"\tunsigned char* pui = (unsigned char*)&f;\n"
		"\treturn (int)((*((INTEGER*)pui) & iMask) >> iShift);\n"
		"}\n"
		"int main(void) { return Exponent<double, long long>(345.34, 0x7ff0000000000000, 52); }\n"));

	AssertInt(8 + 7 * 10, TestCPP816Run(
		"int main(void)\n"
		"{\n"
		"\tunion { long long i64; signed char ia8[8]; };\n"
		"\ti64 = 0x0102030405060708LL;\n"
		"\treturn ia8[0] + ia8[1] * 10;\n"
		"}\n"));
	AssertInt(8 + 7 * 10, TestCPP816Run(
		"int main(void)\n"
		"{\n"
		"\tlong long i64;\n"
		"\ti64 = 0x0102030405060708LL;\n"
		"\tsigned char* p = (signed char*)&i64;\n"
		"\treturn p[0] + p[1] * 10;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by constants (whole words moved, then a byte, then the bits) of
// 16, 32 and 64 bit values, signed and unsigned, positive and negative (and
// the edges), by every amount, against the same shifts natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ConstantShifts(void)
{
	static const uint16	aui16[] = { 0x9AB1, 0x5A3C, 0x8000, 0x7FFF, 0xFFFF };
	static const uint32	aui32[] = { 0x9ABCDEF1, 0x5ABCDEF1, 0x80000000, 0xFFFFFFFF };
	static const uint64	aui64[] = { 0x9ABCDEF123456789ULL, 0x1ABCDEF123456789ULL };
	CChars				szSource;
	int					i;
	size				uiValue;
	uint16				ui16;
	int16				i16;
	uint32				ui32;
	int32				i32;
	uint64				ui64;
	int64				i64;
	uint32				uiSum;

	uiSum = 0;
	szSource.Init("volatile unsigned short gui16;\nvolatile short gi16;\nvolatile unsigned long gui32;\nvolatile long gi32;\nvolatile unsigned long long gui64;\nvolatile long long gi64;\n");
	szSource.Append("unsigned long s = 0;\n");
	for (uiValue = 0; uiValue < sizeof(aui16) / sizeof(uint16); uiValue++)
	{
		ui16 = aui16[uiValue];
		i16 = (int16)aui16[uiValue];
		szSource.Append("void Shift16_")->Append((int)uiValue)->Append("(void)\n{\n\tgui16 = ")->Append((int)ui16)->Append("U;\n\tgi16 = (short)")->Append((int)ui16)->Append("U;\n");
		for (i = 1; i < 16; i++)
		{
			uiSum = uiSum * 3 + (uint16)(ui16 << i) + (uint16)(ui16 >> i) + (uint16)(i16 >> i);
			szSource.Append("\ts = s * 3 + (unsigned short)(gui16 << ")->Append(i)->Append(") + (unsigned short)(gui16 >> ")->Append(i)->Append(") + (unsigned short)(gi16 >> ")->Append(i)->Append(");\n");
		}
		szSource.Append("}\n");
	}
	for (uiValue = 0; uiValue < sizeof(aui32) / sizeof(uint32); uiValue++)
	{
		ui32 = aui32[uiValue];
		i32 = (int32)aui32[uiValue];
		szSource.Append("void Shift32_")->Append((int)uiValue)->Append("(void)\n{\n\tgui32 = ")->Append((int64)ui32)->Append("UL;\n\tgi32 = (long)")->Append((int64)ui32)->Append("UL;\n");
		for (i = 1; i < 32; i++)
		{
			uiSum = uiSum * 3 + (ui32 << i) + (ui32 >> i) + (uint32)(i32 >> i);
			szSource.Append("\ts = s * 3 + (gui32 << ")->Append(i)->Append(") + (gui32 >> ")->Append(i)->Append(") + (unsigned long)(gi32 >> ")->Append(i)->Append(");\n");
		}
		szSource.Append("}\n");
	}
	for (uiValue = 0; uiValue < sizeof(aui64) / sizeof(uint64); uiValue++)
	{
		ui64 = aui64[uiValue];
		i64 = (int64)aui64[uiValue];
		szSource.Append("void Shift64_")->Append((int)uiValue)->Append("(void)\n{\n\tgui64 = ")->Append((int64)(ui64 >> 32))->Append("ULL << 32 | ")->Append((int64)(ui64 & 0xFFFFFFFF))->Append("ULL;\n\tgi64 = (long long)gui64;\n");
		for (i = 1; i < 64; i++)
		{
			uiSum = uiSum * 5 + (uint32)(ui64 << i) + (uint32)((ui64 << i) >> 32) + (uint32)(ui64 >> i) + (uint32)((ui64 >> i) >> 32) + (uint32)(i64 >> i) + (uint32)((i64 >> i) >> 32);
			szSource.Append("\ts = s * 5 + (unsigned long)(gui64 << ")->Append(i)->Append(") + (unsigned long)((gui64 << ")->Append(i)->Append(") >> 32) + (unsigned long)(gui64 >> ")->Append(i)->Append(") + (unsigned long)((gui64 >> ")->Append(i)->Append(") >> 32) + (unsigned long)(gi64 >> ")->Append(i)->Append(") + (unsigned long)((gi64 >> ")->Append(i)->Append(") >> 32);\n");
		}
		szSource.Append("}\n");
	}
	szSource.Append("int main(void)\n{\n");
	for (uiValue = 0; uiValue < sizeof(aui16) / sizeof(uint16); uiValue++)
	{
		szSource.Append("\tShift16_")->Append((int)uiValue)->Append("();\n");
	}
	for (uiValue = 0; uiValue < sizeof(aui32) / sizeof(uint32); uiValue++)
	{
		szSource.Append("\tShift32_")->Append((int)uiValue)->Append("();\n");
	}
	for (uiValue = 0; uiValue < sizeof(aui64) / sizeof(uint64); uiValue++)
	{
		szSource.Append("\tShift64_")->Append((int)uiValue)->Append("();\n");
	}
	szSource.Append("\treturn (int)(s ^ (s >> 16)) & 0x7FFF;\n}\n");
	AssertInt((int)((uiSum ^ (uiSum >> 16)) & 0x7FFF), TestCPP816Run(szSource.Text()));
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by counts known only when they run (word, byte and the jump into
// the unrolled bit steps, or the loop in memory for 64 bits) of 8, 16, 32
// and 64 bit values, signed and unsigned, by every count from 0 to the
// width less one, against the same shifts natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816VariableShifts(void)
{
	static const uint16	aui16[] = { 0x9AB1, 0x5A3C, 0x8000, 0x7FFF, 0xFFFF };
	static const uint32	aui32[] = { 0x9ABCDEF1, 0x5ABCDEF1, 0x80000000, 0xFFFFFFFF };
	static const uint64	aui64[] = { 0x9ABCDEF123456789ULL, 0x1ABCDEF123456789ULL };
	static const uint8	aui8[] = { 0x9A, 0x5C, 0x80, 0xFF };
	CChars				szSource;
	int					i;
	size				uiValue;
	uint32				uiSum;

	uiSum = 0;
	szSource.Init("unsigned short aui16[] = { 0x9AB1, 0x5A3C, 0x8000, 0x7FFF, 0xFFFF };\n");
	szSource.Append("unsigned long aui32[] = { 0x9ABCDEF1, 0x5ABCDEF1, 0x80000000, 0xFFFFFFFF };\n");
	szSource.Append("unsigned long long aui64[] = { 0x9ABCDEF123456789ULL, 0x1ABCDEF123456789ULL };\n");
	szSource.Append("unsigned char aui8[] = { 0x9A, 0x5C, 0x80, 0xFF };\n");
	szSource.Append("volatile int giCount;\n");
	szSource.Append("int main(void)\n{\n\tunsigned long s;\n\tint v;\n\tint i;\n\ts = 0;\n");
	szSource.Append("\tfor (v = 0; v < 5; v++) { for (i = 0; i < 16; i++) { giCount = i; s = s * 3 + (unsigned short)(aui16[v] << giCount) + (unsigned short)(aui16[v] >> giCount) + (unsigned short)((short)aui16[v] >> giCount); } }\n");
	szSource.Append("\tfor (v = 0; v < 4; v++) { for (i = 0; i < 32; i++) { giCount = i; s = s * 3 + (aui32[v] << giCount) + (aui32[v] >> giCount) + (unsigned long)((long)aui32[v] >> giCount); } }\n");
	szSource.Append("\tfor (v = 0; v < 2; v++) { for (i = 0; i < 64; i++) { giCount = i; s = s * 5 + (unsigned long)(aui64[v] << giCount) + (unsigned long)((aui64[v] << giCount) >> 32) + (unsigned long)(aui64[v] >> giCount) + (unsigned long)((long long)aui64[v] >> giCount); } }\n");
	szSource.Append("\tfor (v = 0; v < 4; v++) { for (i = 0; i < 8; i++) { giCount = i; s = s * 3 + (unsigned char)(aui8[v] << giCount) + (unsigned char)(aui8[v] >> giCount) + (unsigned char)((signed char)aui8[v] >> giCount); } }\n");
	szSource.Append("\treturn (int)(s ^ (s >> 16)) & 0x7FFF;\n}\n");

	for (uiValue = 0; uiValue < sizeof(aui16) / sizeof(uint16); uiValue++)
	{
		for (i = 0; i < 16; i++)
		{
			uiSum = uiSum * 3 + (uint16)(aui16[uiValue] << i) + (uint16)(aui16[uiValue] >> i) + (uint16)((int16)aui16[uiValue] >> i);
		}
	}
	for (uiValue = 0; uiValue < sizeof(aui32) / sizeof(uint32); uiValue++)
	{
		for (i = 0; i < 32; i++)
		{
			uiSum = uiSum * 3 + (aui32[uiValue] << i) + (aui32[uiValue] >> i) + (uint32)((int32)aui32[uiValue] >> i);
		}
	}
	for (uiValue = 0; uiValue < sizeof(aui64) / sizeof(uint64); uiValue++)
	{
		for (i = 0; i < 64; i++)
		{
			uiSum = uiSum * 5 + (uint32)(aui64[uiValue] << i) + (uint32)((aui64[uiValue] << i) >> 32) + (uint32)(aui64[uiValue] >> i) + (uint32)((int64)aui64[uiValue] >> i);
		}
	}
	for (uiValue = 0; uiValue < sizeof(aui8) / sizeof(uint8); uiValue++)
	{
		for (i = 0; i < 8; i++)
		{
			uiSum = uiSum * 3 + (uint8)(aui8[uiValue] << i) + (uint8)(aui8[uiValue] >> i) + (uint8)((int8)aui8[uiValue] >> i);
		}
	}
	AssertInt((int)((uiSum ^ (uiSum >> 16)) & 0x7FFF), TestCPP816Run(szSource.Text(), true));
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Explicit destructor calls (with placement new, as an object is made and
// destroyed in memory the caller owns): through a pointer, through a base
// pointer to a virtual destructor, on a named object, and of a class without
// a destructor (nothing happens).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DestructorCalls(void)
{
	AssertInt(1 + 20 + 300 + 4000, TestCPP816RunProject(NULL, "",
		"#include <new>\n"
		"int giDestroyed = 0;\n"
		"class CPlain { public: int mi; ~CPlain() { giDestroyed += mi; } };\n"
		"class CBase { public: virtual ~CBase() { giDestroyed += 1000; } };\n"
		"class CDerived : public CBase { public: ~CDerived() { giDestroyed += 3000; } };\n"
		"class CTrivial { public: int mi; };\n"
		"int main(void)\n"
		"{\n"
		"\tlong\t\talBuffer[8];\n"
		"\tCPlain*\t\tpcPlain;\n"
		"\tCBase*\t\tpcBase;\n"
		"\tCTrivial*\tpcTrivial;\n"
		"\tpcPlain = new(alBuffer) CPlain;\n"
		"\tpcPlain->mi = 1;\n"
		"\tpcPlain->~CPlain();\n"
		"\tpcPlain = new(alBuffer) CPlain;\n"
		"\tpcPlain->mi = 20;\n"
		"\t(*pcPlain).~CPlain();\n"
		"\tpcPlain = new(alBuffer) CPlain;\n"
		"\tpcPlain->mi = 300;\n"
		"\tpcPlain->CPlain::~CPlain();\n"
		"\tpcBase = new(alBuffer) CDerived;\n"
		"\tpcBase->~CBase();\n"
		"\tpcTrivial = new(alBuffer) CTrivial;\n"
		"\tpcTrivial->~CTrivial();\n"
		"\treturn giDestroyed;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast: down from a first and a second base, across, to void*, a NULL
// pointer, up (a conversion), to a class the object isn't, and an ambiguous
// base (a diamond) which gives NULL.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DynamicCast(void)
{
	AssertInt(8191, TestCPP816Run(
		"struct CBase { int mi; virtual int Who() { return 1; } };\n"
		"struct CListener { int ml; virtual int Heard() { return 3; } };\n"
		"struct CDerived : CBase, CListener { int mj; virtual int Who() { return 2; } };\n"
		"struct COther : CBase { int mo; };\n"
		"struct CDeeper : CDerived { int mk; };\n"
		"struct CLeft : CBase { int mx; };\n"
		"struct CRight : CBase { int my; };\n"
		"struct CDiamond : CLeft, CRight, CListener { int mz; };\n"
		"int main()\n"
		"{\n"
		"	CDerived d; COther o; CDeeper e; CDiamond a; int r = 0;\n"
		"	CBase* pb = &d; CListener* pl = &d; CBase* pe = &e; CDerived* pde = &e; CBase* pn = 0; CListener* pal = &a;\n"
		"	if (dynamic_cast<CDerived*>(pb) == &d) r += 1;\n"
		"	if (dynamic_cast<COther*>(pb) == 0) r += 2;\n"
		"	if (dynamic_cast<CDerived*>(pl) == &d) r += 4;\n"
		"	if (dynamic_cast<CListener*>(pb) == pl) r += 8;\n"
		"	if (dynamic_cast<void*>(pl) == (void*)&d) r += 16;\n"
		"	if (dynamic_cast<CDerived*>(pn) == 0) r += 32;\n"
		"	if (dynamic_cast<CDerived*>(pe) == pde) r += 64;\n"
		"	if (dynamic_cast<CBase*>(&d) == pb) r += 128;\n"
		"	if (dynamic_cast<CDeeper*>(pb) == 0) r += 256;\n"
		"	if (dynamic_cast<CBase*>(pal) == 0) r += 512;\n"
		"	if (dynamic_cast<CLeft*>(pal) == (CLeft*)&a) r += 1024;\n"
		"	if (dynamic_cast<CDerived*>(pl)->Who() == 2) r += 2048;\n"
		"	if (dynamic_cast<CDerived*>((CBase*)&o) == 0) r += 4096;\n"
		"	return r;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast across units (see TestCPP65IRDynamicCastUnits).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816DynamicCastUnits(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/DynamicCastProject/Main.cpp", "Input/CPP65/DynamicCastProject/Square.cpp", NULL };

	AssertInt(1 + 400 + 1000, TestCPP816RunProject(aszUnits, "", NULL));
}


//////////////////////////////////////////////////////////////////////////
//
// A member template overloaded with a plain member of the same name and
// parameters (CObjects::Get<M>(const char*) and Get(const char*)) calls the
// plain member from inside the template: a template's arguments that can't be
// deduced take it out of the overloads.  Get<CString>("Door") returned null in
// TestLayeredObjects on both targets.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MemberTemplateCallsPlainOverload(void)
{
	AssertInt(12, TestCPP816Run(
		"struct CPointer { int m; };\n"
		"template<class M> struct Ptr\n"
		"{\n"
		"\tint m;\n"
		"\tPtr() { m = 0; }\n"
		"\tPtr(const CPointer& c) { m = c.m; }\n"
		"};\n"
		"typedef long long OIndex;\n"
		"class CObjects\n"
		"{\n"
		"public:\n"
		"\t\t\t\t\t\tCPointer\tGet(OIndex oi) { CPointer c; c.m = 1; return c; }\n"
		"\t\t\t\t\t\tCPointer\tGet(const char* szName) { CPointer c; c.m = (szName[0] == 'D') ? 2 : 3; return c; }\n"
		"\ttemplate<class M>\tPtr<M>\t\tGet(OIndex oi);\n"
		"\ttemplate<class M>\tPtr<M>\t\tGet(const char* szName);\n"
		"};\n"
		"template<class M> Ptr<M> CObjects::Get(OIndex oi) { return Get(oi); }\n"
		"template<class M> Ptr<M> CObjects::Get(const char* szName) { return Get(szName); }\n"
		"struct CString { int m; };\n"
		"int main()\n"
		"{\n"
		"\tCObjects\t\tcObjects;\n"
		"\tPtr<CString>\tpByIndex;\n"
		"\tPtr<CString>\tpByName;\n"
		"\tpByIndex = cObjects.Get<CString>(5);\n"
		"\tpByName = cObjects.Get<CString>(\"Door\");\n"
		"\treturn pByIndex.m * 10 + pByName.m;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// An overloaded function's name for a pointer to a function is the overload
// of the pointer's type: in global and local initialisers, an assignment, an
// argument, a return and casts, whichever overload is declared first.  The
// overload of the first declared used to be taken (a volatile global pointer
// in TestCPP816StringRegisters called FindFirstClearBit(void*, size) for
// FindFirstClearBit(uint32)).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816OverloadedFunctionPointers(void)
{
	AssertInt(511, TestCPP816Run(
		"unsigned short F(void* p, unsigned short n) { return 1; }\n"
		"unsigned short F(unsigned long u) { return 2; }\n"
		"typedef unsigned short (*FLong)(unsigned long);\n"
		"typedef unsigned short (*FArray)(void*, unsigned short);\n"
		"FLong gpLong = F;\n"
		"unsigned short (*volatile gpVolatile)(unsigned long) = &F;\n"
		"FArray gpArray = F;\n"
		"unsigned short Call(FLong p, unsigned long u) { return p(u); }\n"
		"FLong Get(void) { return F; }\n"
		"int main()\n"
		"{\n"
		"	FLong pLocal = F;\n"
		"	FLong pAssigned;\n"
		"	pAssigned = &F;\n"
		"	int r = 0;\n"
		"	if (gpLong(1) == 2) r |= 1;\n"
		"	if (gpVolatile(1) == 2) r |= 2;\n"
		"	if (gpArray(0, 0) == 1) r |= 4;\n"
		"	if (pLocal(1) == 2) r |= 8;\n"
		"	if (pAssigned(1) == 2) r |= 16;\n"
		"	if (Call(F, 1) == 2) r |= 32;\n"
		"	if (Get()(1) == 2) r |= 64;\n"
		"	if (((FLong)&F)(1) == 2) r |= 128;\n"
		"	if (((FArray)F)(0, 0) == 1) r |= 256;\n"
		"	return r;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Builds and runs szSource on the W65C816 with pcProfile guiding it (NULL
// for none), profiling the run into pcMeasure (NULL to not) with the
// program's line table.  Returns main's result and its cycles, and the
// program's map in pszMap (when not NULL).
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP816RunProfiled(const char* szSource, CAsmW65C816Profile* pcProfile, CAsmW65C816Profile* pcMeasure, uint64* puiCycles, CChars* pszMap = NULL)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	CCPP65Builder		cBuilder;
	CAsm65Memory		cMemory;
	CFastW65C816			cFast;
	CChars				szProject;
	CFileUtil			cFileUtil;
	int32				iResult;

	iResult = -1000000;
	cBuilder.Init();
	*cBuilder.GetOptions() = gcTestCPP816Options;
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetFrameProfile(pcProfile);
	cBuilder.SetLineTable(pcMeasure != NULL);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());
	if (pszMap)
	{
		pszMap->Append(cBuilder.GetMap());
	}

	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(cBuilder.GetOutput());
	cMemory.InitFast65816(&cFast);
	cFast.Reset();
	if (pcMeasure)
	{
		pcMeasure->Run(&cMemory, &cFast, cBuilder.GetMap(), TEST_CPP816_STEPS, cBuilder.GetLineTable());
	}
	else
	{
		TestCPP816RunChecked(&cMemory, &cFast, TEST_CPP816_STEPS);
	}
	AssertTrue(cFast.IsStopped());
	if (cFast.IsStopped())
	{
		iResult = (int16)cFast.GetC();
	}
	*puiCycles = cFast.NumCycles();
	cFast.Kill();
	cMemory.Kill();
	cBuilder.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A profile by line: a call's line counts how often it ran, in the
// function it is in, and its call sites how often each line called Mix and
// the cycles inside the calls (Mix's own and the runtime routines' it
// called, as nothing else calls it).  With the profile the hot call (in the loop) of a
// function too big to inline without one is inlined; the cold one isn't.
// A function compiled from other code since (another fingerprint) starts
// its profile again.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ProfileGuidedInlining(void)
{
	static const char*	szSource =
		"int Mix(int a, int b)\n"
		"{\n"
		"\tint x = a * 3 + b;\n"
		"\tx ^= (x >> 2); x += a & 7; x -= b | 5; x ^= x << 1;\n"
		"\tx += a * b; x ^= (b >> 3); x += 11; x -= a; x ^= 0x55;\n"
		"\tx += b * 5; x ^= (a >> 1); x += (b & 3) * 9; x ^= (x >> 4);\n"
		"\tx += a ^ b; x -= (a | b) >> 2; x ^= (b << 2); x += (a & b) * 7;\n"
		"\tx ^= (x >> 3); x += a & 5; x -= b | 9; x ^= x << 2; x += a * 5;\n"
		"\tx ^= (b >> 2); x += 13; x -= b; x ^= 0x33; x += a * 9; x ^= (b >> 1);\n"
		"\tx += (a & 7) * 3; x ^= (x >> 5); x += a | b; x -= (a ^ b) >> 3; x ^= (a << 3);\n"
		"\tx ^= (x >> 1); x += a * 3; x -= b | 5; x ^= x << 1; x += (a & b) * 7; x ^= (b >> 1); x += a ^ 11;\n"
		"\tx ^= (x >> 2); x += a * 4; x -= b | 6; x ^= x << 2; x += (a & b) * 8; x ^= (b >> 2); x += a ^ 12;\n"
		"\tx ^= (x >> 3); x += a * 5; x -= b | 7; x ^= x << 3; x += (a & b) * 9; x ^= (b >> 3); x += a ^ 13;\n"
		"\tx ^= (x >> 4); x += a * 6; x -= b | 8; x ^= x << 1; x += (a & b) * 10; x ^= (b >> 4); x += a ^ 14;\n"
		"\tx ^= (x >> 5); x += a * 7; x -= b | 9; x ^= x << 2; x += (a & b) * 11; x ^= (b >> 1); x += a ^ 15;\n"
		"\tx ^= (x >> 1); x += a * 8; x -= b | 10; x ^= x << 3; x += (a & b) * 12; x ^= (b >> 2); x += a ^ 16;\n"
		"\tx ^= (x >> 2); x += a * 9; x -= b | 11; x ^= x << 1; x += (a & b) * 13; x ^= (b >> 3); x += a ^ 17;\n"
		"\tx ^= (x >> 3); x += a * 10; x -= b | 12; x ^= x << 2; x += (a & b) * 14; x ^= (b >> 4); x += a ^ 18;\n"
		"\treturn x;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"\tint r = 0;\n"
		"\tfor (int i = 0; i < 2000; i++)\n"
		"\t{\n"
		"\t\tr += Mix(i, r & 255);\n"
		"\t}\n"
		"\tr += Mix(r, 3);\n"
		"\treturn r;\n"
		"}\n";
	CAsmW65C816Profile			cProfile;
	CAsmW65C816Profile			cGuided;
	CAsmW65C816Profile			cAgain;
	CAsmW65C816Profile			cRead;
	CChars					szChanged;
	SAsmW65C816CallSiteProfile*	psCallSite;
	SAsmW65C816CallSiteProfile*	psOnce;
	SAsmW65C816CallSiteProfile*	psRead;
	SAsmW65C816FunctionProfile*	psMix;
	SAsmW65C816FunctionProfile*	psMain;
	int32					iResult;
	uint64					uiPlain;
	uint64					uiGuided;

	cProfile.Init();
	iResult = TestCPP816RunProfiled(szSource, NULL, &cProfile, &uiPlain);
	psMix = cProfile.Get("_Mix__Fss");
	AssertNotNull((void*)psMix);
	AssertLong(2001, psMix ? (int64)psMix->ulliCalls : 0);
	AssertTrue(cProfile.GetLine("_main", "Test.cpp", 26) != NULL);
	AssertLong(2000, cProfile.GetLine("_main", "Test.cpp", 26) ? (int64)cProfile.GetLine("_main", "Test.cpp", 26)->ulliEntries : 0);
	AssertLong(2000, (int64)cProfile.LineEntries("Test.cpp", 26));
	psMain = cProfile.Get("_main");
	AssertTrue(psMain && (psMain->uiHash != 0) && (psMain->iTime != 0));

	//Its call sites: each line's calls of Mix, with the cycles inside them; written and read back.
	psCallSite = cProfile.GetCallSite("_Mix__Fss", "Test.cpp", 26);
	AssertNotNull((void*)psCallSite);
	AssertLong(2000, psCallSite ? (int64)psCallSite->ulliCalls : 0);
	psOnce = cProfile.GetCallSite("_Mix__Fss", "Test.cpp", 28);
	AssertNotNull((void*)psOnce);
	AssertLong(1, psOnce ? (int64)psOnce->ulliCalls : 0);
	AssertTrue(psCallSite && psOnce && psMix && (psCallSite->ulliCycles + psOnce->ulliCycles >= psMix->ulliCycles) && (psOnce->ulliCycles > 0));
	cRead.Init();
	AssertTrue(cProfile.Write("Output" _FS_ "CallSites.profile"));
	AssertTrue(cRead.Read("Output" _FS_ "CallSites.profile"));
	psRead = cRead.GetCallSite("_Mix__Fss", "Test.cpp", 26);
	AssertTrue(psCallSite && psRead && (psRead->ulliCalls == psCallSite->ulliCalls) && (psRead->ulliCycles == psCallSite->ulliCycles));
	cRead.Kill();

	//Guided: only the call in main's loop is inlined.
	cGuided.Init();
	AssertInt(iResult, TestCPP816RunProfiled(szSource, &cProfile, &cGuided, &uiGuided));
	psMix = cGuided.Get("_Mix__Fss");
	AssertLong(1, psMix ? (int64)psMix->ulliCalls : 0);
	AssertTrue(uiGuided < uiPlain);

	//A profile of the guided build (the call's line has no code left: the callee's lines ran in main instead) still
	//inlines it (CCPP65IRFrontEnd::InlinedEntries).
	cAgain.Init();
	AssertInt(iResult, TestCPP816RunProfiled(szSource, &cGuided, &cAgain, &uiGuided));
	psMix = cAgain.Get("_Mix__Fss");
	AssertLong(1, psMix ? (int64)psMix->ulliCalls : 0);
	cAgain.Kill();

	//main compiled from other code since (another fingerprint): its old profile doesn't guide it.
	szChanged.Init(szSource);
	szChanged.Replace("\tr += Mix(r, 3);\n", "\tr += Mix(r, 3);\n\tr ^= 1;\n");
	cAgain.Init();
	TestCPP816RunProfiled(szChanged.Text(), &cProfile, &cAgain, &uiGuided);
	psMix = cAgain.Get("_Mix__Fss");
	AssertLong(2001, psMix ? (int64)psMix->ulliCalls : 0);
	cAgain.Kill();
	szChanged.Kill();

	cGuided.Kill();
	cProfile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Hot divides and remainders of a byte's values by constants (char / 10,
// unsigned char % 7) are looked up in tables (R816_STable10, R816_UTable7)
// when a profile says they are hot, and give what the reciprocals give for
// every byte; char multiplies look the product up in MathMult.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ProfileGuidedDivisionTables(void)
{
	static const char*	szSource =
		"int main()\n"
		"{\n"
		"\tunsigned short h = 0;\n"
		"\tfor (int k = 0; k < 3; k++)\n"
		"\t{\n"
		"\t\tfor (int i = 0; i < 256; i++)\n"
		"\t\t{\n"
		"\t\t\tsigned char c = (signed char)i;\n"
		"\t\t\tunsigned char u = (unsigned char)(i + k);\n"
		"\t\t\tsigned char q = c / 10;\n"
		"\t\t\tsigned char r = c % 10;\n"
		"\t\t\tunsigned char uq = u / 7;\n"
		"\t\t\tunsigned char ur = u % 7;\n"
		"\t\t\tsigned char m = (signed char)(c * (signed char)(i >> 3));\n"
		"\t\t\th = h * 31 + (unsigned char)q;\n"
		"\t\t\th = h * 31 + (unsigned char)r;\n"
		"\t\t\th = h * 31 + uq;\n"
		"\t\t\th = h * 31 + ur;\n"
		"\t\t\th = h * 31 + (unsigned char)m;\n"
		"\t\t}\n"
		"\t}\n"
		"\treturn (int)(short)h;\n"
		"}\n";
	CAsmW65C816Profile	cProfile;
	CChars			szPlainMap;
	CChars			szGuidedMap;
	uint64			uiPlain;
	uint64			uiGuided;
	uint16			uiHash;
	int				k;
	int				i;
	signed char		c;
	unsigned char	u;

	uiHash = 0;
	for (k = 0; k < 3; k++)
	{
		for (i = 0; i < 256; i++)
		{
			c = (signed char)i;
			u = (unsigned char)(i + k);
			uiHash = (uint16)(uiHash * 31 + (unsigned char)(signed char)(c / 10));
			uiHash = (uint16)(uiHash * 31 + (unsigned char)(signed char)(c % 10));
			uiHash = (uint16)(uiHash * 31 + (unsigned char)(u / 7));
			uiHash = (uint16)(uiHash * 31 + (unsigned char)(u % 7));
			uiHash = (uint16)(uiHash * 31 + (unsigned char)(signed char)(c * (signed char)(i >> 3)));
		}
	}

	cProfile.Init();
	szPlainMap.Init();
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, &cProfile, &uiPlain, &szPlainMap));
	AssertFalse(szPlainMap.Contains("R816_STable10"));
	AssertFalse(szPlainMap.Contains("R816_UTable7"));
	AssertFalse(szPlainMap.Contains("R816_Mul16"));

	szGuidedMap.Init();
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, &cProfile, NULL, &uiGuided, &szGuidedMap));
	AssertTrue(szGuidedMap.Contains("R816_STable10"));
	AssertTrue(szGuidedMap.Contains("R816_UTable7"));
	AssertTrue(uiGuided < uiPlain);
	szGuidedMap.Kill();
	szPlainMap.Kill();
	cProfile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The type traits: __is_enum of an enum, __is_abstract of a class with a
// pure virtual function (not of one overriding it), __is_pod of a plain
// struct (not of one with a copy constructor or a vtable).  And the l
// builtins take an unsigned long (32 bits) on the W65C816.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816TypeTraits(void)
{
	static const char*	szSource =
		"enum E { eA };\n"
		"struct A { virtual int F(void) = 0; };\n"
		"struct B : A { int F(void) { return 1; } };\n"
		"struct P { int i; };\n"
		"struct Q { Q(const Q& q) { i = q.i; } int i; };\n"
		"struct V { virtual int G(void) { return 2; } };\n"
		"int main()\n"
		"{\n"
		"\treturn __is_enum(E) + __is_enum(P) * 2 + __is_abstract(A) * 4 + __is_abstract(B) * 8 + __is_pod(P) * 16 + __is_pod(Q) * 32 + __is_pod(V) * 64 + __is_polymorphic(B) * 128;\n"
		"}\n";
	uint64	uiCycles;

	//1 (enum E) + 4 (abstract A) + 16 (pod P) + 128 (polymorphic B).
	AssertInt(149, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
	AssertInt(17 + 31 * 100, TestCPP816RunProfiled("int main() { return __builtin_ctzl(0x20000UL) + __builtin_clzl(1UL) * 100; }", NULL, NULL, &uiCycles));

	//Of 0: the width in bits.
	AssertInt(32 + 64 * 100, TestCPP816RunProfiled("int main() { return __builtin_ctzl(0UL) + __builtin_clzll(0ULL) * 100; }", NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// A uint16 array indexed by i >> 2 (an arithmetic shift of any int16 is
// within -8192 to 8191, so the scaled index fits Y) and by i >> 5, from
// below the pointer as well as above it, loaded and stored.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ShiftedWordIndices(void)
{
	static const char*	szSource =
		"unsigned short gaui[700];\n"
		"unsigned short Get(unsigned short* p, short i) { return p[i >> 2]; }\n"
		"unsigned short (*volatile gpGet)(unsigned short*, short) = Get;\n"
		"int main()\n"
		"{\n"
		"\tunsigned short* p = &gaui[350];\n"
		"\tunsigned short h = 0;\n"
		"\tfor (short i = -1400; i < 1400; i++) { p[i >> 2] = (unsigned short)(i * 7); }\n"
		"\tfor (short i = 1399; i >= -1400; i -= 3) { h = h * 31 + p[i >> 2]; }\n"
		"\tfor (short i = -11200; i < 11200; i += 13) { h = h * 31 + p[i >> 5]; }\n"
		"\th = h * 31 + gpGet(p, -1400);\n"
		"\th = h * 31 + gpGet(p, 1399);\n"
		"\treturn (int)(short)h;\n"
		"}\n";
	uint16	aui[700];
	uint16*	pui;
	uint16	uiHash;
	int		i;
	uint64	uiCycles;

	memset(aui, 0, sizeof(aui));
	pui = &aui[350];
	for (i = -1400; i < 1400; i++)
	{
		pui[i >> 2] = (uint16)(i * 7);
	}
	uiHash = 0;
	for (i = 1399; i >= -1400; i -= 3)
	{
		uiHash = (uint16)(uiHash * 31 + pui[i >> 2]);
	}
	for (i = -11200; i < 11200; i += 13)
	{
		uiHash = (uint16)(uiHash * 31 + pui[i >> 5]);
	}
	uiHash = (uint16)(uiHash * 31 + pui[-1400 >> 2]);
	uiHash = (uint16)(uiHash * 31 + pui[1399 >> 2]);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// __builtin_bcd_add and __builtin_bcd_subtract (R816_BcdAdd and
// R816_BcdSubtract, the W65C816's decimal mode) give what BaseLib's
// BCDAdd and BCDSubtract give: carries and borrows through the words, a
// carry or borrow in, and none at all with no words.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BCDBuiltins(void)
{
	static const char*	szSource =
		"unsigned short gauiA[3] = { 0x9999, 0x1234, 0x0000 };\n"
		"unsigned short gauiB[3] = { 0x0001, 0x8765, 0x0000 };\n"
		"unsigned short guiHash;\n"
		"void Mix(unsigned short ui) { guiHash = guiHash * 31 + ui; }\n"
		"void MixAll(unsigned short uiCarry) { Mix(uiCarry); Mix(gauiA[0]); Mix(gauiA[1]); Mix(gauiA[2]); }\n"
		"int main()\n"
		"{\n"
		"\tMixAll(__builtin_bcd_add(gauiA, gauiB, 3, 0));\n"
		"\tMixAll(__builtin_bcd_subtract(gauiA, gauiB, 3, 1));\n"
		"\tMixAll(__builtin_bcd_subtract(gauiA, gauiB, 2, 0));\n"
		"\tMixAll(__builtin_bcd_add(gauiA, gauiB, 1, 1));\n"
		"\tMixAll(__builtin_bcd_add(gauiA, gauiB, 0, 1));\n"
		"\tMixAll(__builtin_bcd_subtract(gauiA, gauiB, 0, 1));\n"
		"\treturn (int)(short)guiHash;\n"
		"}\n";
	uint16	auiA[3] = { 0x9999, 0x1234, 0x0000 };
	uint16	auiB[3] = { 0x0001, 0x8765, 0x0000 };
	uint16	auiCarries[6];
	uint16	uiHash;
	int		i;
	int		j;
	uint64	uiCycles;

	uiHash = 0;
	for (i = 0; i < 6; i++)
	{
		switch (i)
		{
		case 0:		auiCarries[i] = BCDAdd(auiA, auiB, 3, 0);		break;
		case 1:		auiCarries[i] = BCDSubtract(auiA, auiB, 3, 1);	break;
		case 2:		auiCarries[i] = BCDSubtract(auiA, auiB, 2, 0);	break;
		case 3:		auiCarries[i] = BCDAdd(auiA, auiB, 1, 1);		break;
		case 4:		auiCarries[i] = BCDAdd(auiA, auiB, 0, 1);		break;
		default:	auiCarries[i] = BCDSubtract(auiA, auiB, 0, 1);	break;
		}
		uiHash = (uint16)(uiHash * 31 + auiCarries[i]);
		for (j = 0; j < 3; j++)
		{
			uiHash = (uint16)(uiHash * 31 + auiA[j]);
		}
	}
	AssertShort((uint16)0, auiCarries[0]);
	AssertShort((uint16)0, auiCarries[1]);
	AssertShort((uint16)1, auiCarries[4]);
	AssertShort((uint16)1, auiCarries[5]);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// Loads and stores through a pointer plus a signed 16 bit index (biased:
// (p - 32768) + zext(i ^ 32768) with the index in Y), negative and
// positive, bytes and words, in a loop and not.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BiasedIndices(void)
{
	static const char*	szSource =
		"char gac[600];\n"
		"short gas[600];\n"
		"char Get(char* p, short i) { return p[i]; }\n"
		"char (*volatile gpGet)(char*, short) = Get;\n"
		"int main()\n"
		"{\n"
		"\tchar* pc = &gac[300];\n"
		"\tshort* ps = &gas[300];\n"
		"\tunsigned short h = 0;\n"
		"\tfor (short i = -300; i < 300; i++) { pc[i] = (char)(i * 7); ps[i] = (short)(i * 1001); }\n"
		"\tfor (short i = 299; i >= -300; i--) { h = h * 31 + (unsigned char)pc[i]; h = h * 31 + (unsigned short)ps[i]; }\n"
		"\th = h * 31 + (unsigned char)gpGet(pc, -300);\n"
		"\th = h * 31 + (unsigned char)gpGet(pc, 299);\n"
		"\th = h * 31 + (unsigned char)gpGet(pc, 0);\n"
		"\treturn (int)(short)h;\n"
		"}\n";
	signed char	ac[600];
	short		as[600];
	uint16		uiHash;
	int			i;
	uint64		uiCycles;

	for (i = -300; i < 300; i++)
	{
		ac[i + 300] = (signed char)(i * 7);
		as[i + 300] = (short)(i * 1001);
	}
	uiHash = 0;
	for (i = 299; i >= -300; i--)
	{
		uiHash = (uint16)(uiHash * 31 + (unsigned char)ac[i + 300]);
		uiHash = (uint16)(uiHash * 31 + (unsigned short)as[i + 300]);
	}
	uiHash = (uint16)(uiHash * 31 + (unsigned char)ac[0]);
	uiHash = (uint16)(uiHash * 31 + (unsigned char)ac[599]);
	uiHash = (uint16)(uiHash * 31 + (unsigned char)ac[300]);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer chosen between one base plus different i16 indices (and null)
// and only loaded and stored through is the base plus the chosen index
// (MergeAddressPhis); a choice between two bases is left as it is.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MergedAddressPhis(void)
{
	static const char*	szSource =
		"char gac[400];\n"
		"char gbc[400];\n"
		"struct SDigits { short iWhole; char ac[400]; };\n"
		"SDigits gs;\n"
		"char* At(SDigits* p, short i) { if (i > 0) { return &p->ac[p->iWhole - i]; } else if (i < 0) { return &p->ac[p->iWhole - i - 1]; } return 0; }\n"
		"void Set(SDigits* p, short i, char c) { *At(p, i) = c; }\n"
		"char Get(SDigits* p, short i) { return *At(p, i); }\n"
		"char* Pick(short b, short i) { return b ? &gac[i + 100] : &gbc[i + 200]; }\n"
		"int main()\n"
		"{\n"
		"\tunsigned short h = 0;\n"
		"\tgs.iWhole = 200;\n"
		"\tfor (short i = -199; i <= 199; i++) { if (i != 0) { Set(&gs, i, (char)(i * 3)); } }\n"
		"\tfor (short i = 199; i >= -199; i--) { if (i != 0) { h = h * 31 + (unsigned char)Get(&gs, i); } }\n"
		"\tfor (short i = -100; i < 100; i++) { *Pick(i & 1, i) = (char)(i + 5); }\n"
		"\tfor (short i = 0; i < 400; i++) { h = h * 31 + (unsigned char)gac[i]; h = h * 31 + (unsigned char)gbc[i]; }\n"
		"\treturn (int)(short)h;\n"
		"}\n";
	signed char	ac[400];
	signed char	abc[400];
	signed char	ad[400];
	uint16		uiHash;
	int			i;
	uint64		uiCycles;

	memset(ac, 0, sizeof(ac));
	memset(abc, 0, sizeof(abc));
	memset(ad, 0, sizeof(ad));
	for (i = -199; i <= 199; i++)
	{
		if (i != 0)
		{
			ad[(i > 0) ? (200 - i) : (200 - i - 1)] = (signed char)(i * 3);
		}
	}
	uiHash = 0;
	for (i = 199; i >= -199; i--)
	{
		if (i != 0)
		{
			uiHash = (uint16)(uiHash * 31 + (unsigned char)ad[(i > 0) ? (200 - i) : (200 - i - 1)]);
		}
	}
	for (i = -100; i < 100; i++)
	{
		if (i & 1)
		{
			ac[i + 100] = (signed char)(i + 5);
		}
		else
		{
			abc[i + 200] = (signed char)(i + 5);
		}
	}
	for (i = 0; i < 400; i++)
	{
		uiHash = (uint16)(uiHash * 31 + (unsigned char)ac[i]);
		uiHash = (uint16)(uiHash * 31 + (unsigned char)abc[i]);
	}
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// 32 bit loop counters whose start and bound fit 16 bits count in 16 bits:
// up and down, < <= > >=, starting past the bound, near the ends of 16
// bits, with the counter's value used in the loop and after it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816NarrowCounters(void)
{
	static const char*	szSource =
		"short gas[7] = { -32767, -5, 0, 7, 32766, 100, -1 };\n"
		"long Sum(short a, short b) { long t = 0; for (long i = a; i <= b; i++) { t += i; } return t; }\n"
		"long Down(short a, short b) { long t = 0; long i; for (i = a; i >= b; i--) { t += i * 3; } return t + i; }\n"
		"long Lt(short a, short b) { long i; long n = 0; for (i = a; i < b; i++) { n++; } return n * 7 + i; }\n"
		"long Gt(short a, short b) { long i; long n = 0; for (i = a; i > b; i--) { n++; } return n * 5 - i; }\n"
		"long (*volatile gpSum)(short, short) = Sum;\n"
		"long (*volatile gpDown)(short, short) = Down;\n"
		"long (*volatile gpLt)(short, short) = Lt;\n"
		"long (*volatile gpGt)(short, short) = Gt;\n"
		"int main()\n"
		"{\n"
		"\tunsigned long r = 0;\n"
		"\tfor (int a = 0; a < 7; a++)\n"
		"\t{\n"
		"\t\tfor (int b = 0; b < 7; b++)\n"
		"\t\t{\n"
		"\t\t\tr = r * 31 + (unsigned long)gpSum(gas[a], gas[b]);\n"
		"\t\t\tr = r * 31 + (unsigned long)gpDown(gas[a], gas[b]);\n"
		"\t\t\tr = r * 31 + (unsigned long)gpLt(gas[a], gas[b]);\n"
		"\t\t\tr = r * 31 + (unsigned long)gpGt(gas[a], gas[b]);\n"
		"\t\t}\n"
		"\t}\n"
		"\treturn (int)r;\n"
		"}\n";
	uint64	uiCycles;

	AssertInt(-24560, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP816IntIndexPointers' functions natively, with 16 bit ints.
//
//////////////////////////////////////////////////////////////////////////
uint16 TestCPP816IntIndexSubtract(char* pc, const char* pd, int16 iThis, int16 iOther, int16 iFirst)
{
	char	cCarry;
	char	cResult;

	cCarry = 0;
	for (; iOther >= iFirst; iOther--, iThis--)
	{
		cResult = (char)(pc[iThis] + 10 - (pd[iOther] + cCarry));
		if (cResult >= 10)
		{
			cCarry = 0;
			cResult -= 10;
		}
		else
		{
			cCarry = 1;
		}
		pc[iThis] = cResult;
	}
	return (uint16)cCarry;
}


uint16 TestCPP816IntIndexSum(int16* as, int16 iStart, int16 iEnd)
{
	uint16	t;
	int16	i;

	t = 0;
	for (i = iStart; i < iEnd; i++)
	{
		t = (uint16)(t + (uint16)(as[i] * 3));
	}
	return t;
}


//////////////////////////////////////////////////////////////////////////
//
// int (16 bit) indices stepped through arrays (two going down together, as
// CNumber's PrivateSubtract does, and one going up through shorts) become
// pointers stepped round the loop, not the index sign extended and added
// each time: the same results, in under 66000 cycles (72255 without).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816IntIndexPointers(void)
{
	static const char*	szSource =
		"char gac[40];\n"
		"char gad[40];\n"
		"short gas[40];\n"
		"int Subtract(char* pc, const char* pd, int iThis, int iOther, int iFirst)\n"
		"{\n"
		"\tchar cCarry = 0;\n"
		"\tfor (; iOther >= iFirst; iOther--, iThis--)\n"
		"\t{\n"
		"\t\tchar cResult = pc[iThis] + 10 - (pd[iOther] + cCarry);\n"
		"\t\tif (cResult >= 10) { cCarry = 0; cResult -= 10; } else { cCarry = 1; }\n"
		"\t\tpc[iThis] = cResult;\n"
		"\t}\n"
		"\treturn cCarry;\n"
		"}\n"
		"unsigned int Sum(int iStart, int iEnd) { unsigned int t = 0; for (int i = iStart; i < iEnd; i++) { t += (unsigned int)(gas[i] * 3); } return t; }\n"
		"int (*volatile gpSubtract)(char*, const char*, int, int, int) = Subtract;\n"
		"unsigned int (*volatile gpSum)(int, int) = Sum;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int r = 0;\n"
		"\tfor (int i = 0; i < 40; i++) { gac[i] = (char)((i * 7) % 10); gad[i] = (char)((i * 3 + 1) % 10); gas[i] = (short)(i * 101 - 2000); }\n"
		"\tr = r * 31 + (unsigned int)gpSubtract(gac, gad, 39, 30, 0);\n"
		"\tr = r * 31 + (unsigned int)gpSubtract(gac + 20, gad + 10, 19, 29, 12);\n"
		"\tfor (int i = 0; i < 40; i++) { r = r * 31 + (unsigned int)gac[i]; }\n"
		"\tr = r * 31 + (unsigned int)gpSum(0, 40) + (unsigned int)gpSum(5, 6) + (unsigned int)gpSum(7, 7);\n"
		"\treturn (int)r;\n"
		"}\n";
	uint64			uiCycles;
	char			ac[40];
	char			ad[40];
	int16			as[40];
	uint16			uiResult;
	int				i;

	//The same natively, with 16 bit ints.
	for (i = 0; i < 40; i++)
	{
		ac[i] = (char)((i * 7) % 10);
		ad[i] = (char)((i * 3 + 1) % 10);
		as[i] = (int16)(i * 101 - 2000);
	}
	uiResult = 0;
	uiResult = (uint16)(uiResult * 31 + TestCPP816IntIndexSubtract(ac, ad, 39, 30, 0));
	uiResult = (uint16)(uiResult * 31 + TestCPP816IntIndexSubtract(ac + 20, ad + 10, 19, 29, 12));
	for (i = 0; i < 40; i++)
	{
		uiResult = (uint16)(uiResult * 31 + (uint16)ac[i]);
	}
	uiResult = (uint16)(uiResult * 31 + TestCPP816IntIndexSum(as, 0, 40) + TestCPP816IntIndexSum(as, 5, 6) + TestCPP816IntIndexSum(as, 7, 7));

	AssertInt((int16)uiResult, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
	AssertTrue(uiCycles < 66000);
}


//////////////////////////////////////////////////////////////////////////
//
// Divides and remainders by constants of values in a known range (a char,
// an unsigned char, an and) are a multiply and shifts, not the runtime's
// divide: every value gives the same quotients and remainders.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ConstantDivisions(void)
{
	static const char*	szSource =
		"int main()\n"
		"{\n"
		"\tunsigned int r = 0;\n"
		"\tfor (int i = -128; i < 128; i++) { char c = (char)i; r = r * 31 + (unsigned int)(c / 10) * 7 + (unsigned int)(c % 10); }\n"
		"\tfor (int i = 0; i < 256; i++) { unsigned char u = (unsigned char)i; r = r * 31 + u / 10 + (u % 7) * 3 + u / 3; }\n"
		"\tfor (int i = 0; i < 1000; i++) { int x = i & 511; r = r * 31 + x / 9 + x % 9; }\n"
		"\treturn (int)r;\n"
		"}\n";
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;
	uint64			uiCycles;

	AssertInt(26921, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", szSource, false, NULL, CPPT_W65C816));
	AssertString("", szErrors.Text());
	AssertFalse(szAssembly.Contains("R816_SDiv") || szAssembly.Contains("R816_SRem") || szAssembly.Contains("R816_UDiv") || szAssembly.Contains("R816_URem"));
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times szText is in the 816 assembly of szSource's functions
// (compiled, not linked), not in the runtime's routines after them.
//
//////////////////////////////////////////////////////////////////////////
int TestCPP816CountInAssembly(const char* szSource, const char* szText)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;
	int				iCount;
	const char*		szAt;
	const char*		szEnd;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", szSource, false, NULL, CPPT_W65C816));
	AssertString("", szErrors.Text());
	szEnd = strstr(szAssembly.Text(), ".section text.R816_");
	iCount = 0;
	for (szAt = strstr(szAssembly.Text(), szText); szAt && (!szEnd || (szAt < szEnd)); szAt = strstr(szAt + 1, szText))
	{
		iCount++;
	}
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// gmtime's divides: x / 86400 and x % 86400 of any x share one runtime
// divide (by 675, after x / 128); the remainder made positive is narrowed
// by the branch around its fix-up, so its divides by 60 and 3600 are
// multiplies (60 = 4 * 15: (r >> 2) * m fits where r * m doesn't), as are
// those of the days (a quotient's range) by 7 and 365 on either side of a
// branch on its sign.
//
//////////////////////////////////////////////////////////////////////////
static uint16 TestCPP816DivisionRangesSplit(int32 x)
{
	int32	q;
	int32	r;
	uint16	h;

	q = x / 86400;
	r = x % 86400;
	if (r < 0)
	{
		r += 86400;
		q--;
	}
	h = (uint16)((r % 60) + ((r / 60) % 60) * 3 + (r / 3600) * 5);
	return (uint16)(h * 31 + (uint16)((q + 4) % 7) + (uint16)((q >= 0) ? q / 365 : -((364 - q) / 365)));
}


void TestCPP816DivisionRanges(void)
{
	static const char*	szSplit =
		"unsigned int Split(long x)\n"
		"{\n"
		"\tlong q = x / 86400;\n"
		"\tlong r = x % 86400;\n"
		"\tif (r < 0) { r += 86400; q--; }\n"
		"\tunsigned int h = (unsigned int)(r % 60) + (unsigned int)((r / 60) % 60) * 3 + (unsigned int)(r / 3600) * 5;\n"
		"\treturn h * 31 + (unsigned int)((q + 4) % 7) + (unsigned int)(q >= 0 ? q / 365 : -((364 - q) / 365));\n"
		"}\n";
	static const char*	szMain =
		"unsigned int (*volatile gpSplit)(long) = Split;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (long l = -40; l < 40; l++) { h = h * 31 + gpSplit(l * 53687091L + l * 7919L); }\n"
		"\th = h * 31 + gpSplit(2147483647L);\n"
		"\th = h * 31 + gpSplit(-2147483647L - 1);\n"
		"\treturn (int)h;\n"
		"}\n";
	CChars		szSource;
	uint16		uiHash;
	int32		l;
	uint64		uiCycles;

	uiHash = 0;
	for (l = -40; l < 40; l++)
	{
		uiHash = (uint16)(uiHash * 31 + TestCPP816DivisionRangesSplit((int32)((uint32)l * 53687091u + (uint32)l * 7919u)));
	}
	uiHash = (uint16)(uiHash * 31 + TestCPP816DivisionRangesSplit(2147483647));
	uiHash = (uint16)(uiHash * 31 + TestCPP816DivisionRangesSplit(-2147483647 - 1));

	szSource.Init(szSplit);
	szSource.Append(szMain);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource.Text(), NULL, NULL, &uiCycles));
	szSource.Kill();

	AssertInt(1, TestCPP816CountInAssembly(szSplit, "jsl R816_SDiv32"));
	AssertInt(0, TestCPP816CountInAssembly(szSplit, "jsl R816_SRem32"));
	AssertInt(0, TestCPP816CountInAssembly(szSplit, "jsl R816_UDiv32"));
	AssertInt(0, TestCPP816CountInAssembly(szSplit, "jsl R816_URem32"));
}


//////////////////////////////////////////////////////////////////////////
//
// A 32 bit divide by a wide constant 2^k * m (m under 65536) is x / 2^k
// (shifts) then the runtime's narrow divide by m: a quotient, a remainder
// alone ((x / 2^k) % m << k plus x % 2^k) and an unsigned remainder (by
// 3 * 65536: x >> 16 is in range for a multiply) are right for negative
// and positive x.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816WideDivisors(void)
{
	static const char*	szFunctions =
		"long Quotient(long x) { return x / 86400; }\n"
		"long Remainder(long x) { return x % 86400; }\n"
		"unsigned long URemainder(unsigned long x) { return x % 196608UL; }\n";
	static const char*	szMain =
		"long (*volatile gpQuotient)(long) = Quotient;\n"
		"long (*volatile gpRemainder)(long) = Remainder;\n"
		"unsigned long (*volatile gpURemainder)(unsigned long) = URemainder;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (long l = -40; l < 40; l++)\n"
		"\t{\n"
		"\t\tlong x = l * 53687091L + l * 7919L;\n"
		"\t\th = h * 31 + (unsigned int)gpQuotient(x) + (unsigned int)gpRemainder(x) * 3 + (unsigned int)(gpURemainder((unsigned long)x) >> 2) * 5;\n"
		"\t}\n"
		"\th = h * 31 + (unsigned int)gpQuotient(-2147483647L - 1) + (unsigned int)gpRemainder(-2147483647L - 1) + (unsigned int)gpURemainder(4294967295UL);\n"
		"\treturn (int)h;\n"
		"}\n";
	CChars		szSource;
	uint16		uiHash;
	int32		l;
	int32		x;
	int32		iMin;
	uint64		uiCycles;

	uiHash = 0;
	for (l = -40; l < 40; l++)
	{
		x = (int32)((uint32)l * 53687091u + (uint32)l * 7919u);
		uiHash = (uint16)(uiHash * 31 + (uint16)(x / 86400) + (uint16)(x % 86400) * 3 + (uint16)(((uint32)x % 196608u) >> 2) * 5);
	}
	iMin = -2147483647 - 1;
	uiHash = (uint16)(uiHash * 31 + (uint16)(iMin / 86400) + (uint16)(iMin % 86400) + (uint16)(4294967295u % 196608u));

	szSource.Init(szFunctions);
	szSource.Append(szMain);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource.Text(), NULL, NULL, &uiCycles));
	szSource.Kill();

	AssertInt(2, TestCPP816CountInAssembly(szFunctions, "lda 675"));
	AssertInt(1, TestCPP816CountInAssembly(szFunctions, "jsl R816_SDiv32"));
	AssertInt(1, TestCPP816CountInAssembly(szFunctions, "jsl R816_SRem32"));
	AssertInt(0, TestCPP816CountInAssembly(szFunctions, "jsl R816_URem32"));
	AssertInt(0, TestCPP816CountInAssembly(szFunctions, "jsl R816_UDiv32"));
}


//////////////////////////////////////////////////////////////////////////
//
// A uint16 array element's bytes through an index from a byte field (as
// FAT's long file name parts are put together): p + sext(x) * 2 + c with x
// from -141 to 3429 is (p + -141 * 2) + zext(x * 2 + 282 + c), the index
// worked out in 16 bits (no 32 bit shift) and the bases shared.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ScaledIndices(void)
{
	static const char*	szSetPart =
		"struct SQ { unsigned char auiPad[16]; unsigned short auiName[256]; unsigned char uiSequence; };\n"
		"void SetPart(SQ* ps, signed char iOffset, unsigned char* puiChars, signed char iRaw)\n"
		"{\n"
		"\t((unsigned char*)&ps->auiName[((ps->uiSequence - 1) * 13) + iOffset])[0] = puiChars[iRaw + 0];\n"
		"\t((unsigned char*)&ps->auiName[((ps->uiSequence - 1) * 13) + iOffset])[1] = puiChars[iRaw + 1];\n"
		"}\n";
	static const char*	szMain =
		"SQ gsQ;\n"
		"unsigned char gauiRaw[32];\n"
		"void (*volatile gpSetPart)(SQ*, signed char, unsigned char*, signed char) = SetPart;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (int i = 0; i < 32; i++) { gauiRaw[i] = (unsigned char)(i * 7 + 3); }\n"
		"\tfor (int j = 1; j < 20; j++)\n"
		"\t{\n"
		"\t\tgsQ.uiSequence = (unsigned char)j;\n"
		"\t\tfor (int i = -1; i < 13; i++) { gpSetPart(&gsQ, (signed char)i, gauiRaw, (signed char)(i + 1 + (j & 7))); }\n"
		"\t\tfor (int i = 0; i < 256; i++) { h = h * 3 + gsQ.auiName[i]; }\n"
		"\t}\n"
		"\treturn (int)h;\n"
		"}\n";
	CChars		szSource;
	uint16		auiName[256];
	uint8		auiRaw[32];
	uint16		uiHash;
	int			i;
	int			j;
	int			iIndex;
	uint64		uiCycles;

	memset(auiName, 0, sizeof(auiName));
	for (i = 0; i < 32; i++)
	{
		auiRaw[i] = (uint8)(i * 7 + 3);
	}
	uiHash = 0;
	for (j = 1; j < 20; j++)
	{
		for (i = -1; i < 13; i++)
		{
			iIndex = ((j - 1) * 13) + i;
			if (iIndex >= 0)
			{
				auiName[iIndex] = (uint16)(auiRaw[i + 1 + (j & 7)] | (auiRaw[i + 2 + (j & 7)] << 8));
			}
		}
		for (i = 0; i < 256; i++)
		{
			uiHash = (uint16)(uiHash * 3 + auiName[i]);
		}
	}

	//The element before auiName (-1 at sequence 1) is the pad's last two bytes: never read back.
	szSource.Init(szSetPart);
	szSource.Append(szMain);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource.Text(), NULL, NULL, &uiCycles));
	szSource.Kill();

	AssertInt(0, TestCPP816CountInAssembly(szSetPart, "rol "));
}


//////////////////////////////////////////////////////////////////////////
//
// Two signed indices in one block: biasing the first moves the block's
// instructions, and the second's index was looked up where its sext had
// been (another i16's sext three instructions before it): b[m] read b[j].
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BiasedIndicesInOneBlock(void)
{
	static const char*	szSource =
		"char gac[16];\n"
		"char gbc[16];\n"
		"long gl;\n"
		"short gs;\n"
		"char Get(char* a, char* b, short i, short j, short m, long* pl, short* ps)\n"
		"{\n"
		"\tchar c = a[i];\n"
		"\t*pl = j;\n"
		"\t*ps = 0;\n"
		"\treturn c + b[m];\n"
		"}\n"
		"char (*volatile gpGet)(char*, char*, short, short, short, long*, short*) = Get;\n"
		"int main()\n"
		"{\n"
		"\tfor (int i = 0; i < 16; i++) { gac[i] = (char)(i + 1); gbc[i] = (char)(i * 16); }\n"
		"\treturn gpGet(gac, gbc, 1, 2, 3, &gl, &gs) + gl * 100;\n"
		"}\n";
	uint64	uiCycles;

	AssertInt(250, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
}


//////////////////////////////////////////////////////////////////////////
//
// The Mersenne Twister's (i + 1) % 624 and (i + 397) % 624 with i a loop
// counter below 624 are a compare and a subtract (BoundDivisions), not a
// reciprocal multiply: ConstantDivisions' ranges once found them first
// (TestArrayTemplate's Twist 22% slower).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816BoundedRemainders(void)
{
	static const char*	szSource =
		"typedef unsigned long uint32;\n"
		"struct CMT { uint32 mauiState[624]; unsigned short muiIndex; void Twist(void); };\n"
		"void CMT::Twist(void)\n"
		"{\n"
		"\tuint32\ti;\n"
		"\tuint32\tuiCombined;\n"
		"\tuint32\tuiTwisted;\n"
		"\n"
		"\tfor (i = 0; i < 624; i++)\n"
		"\t{\n"
		"\t\tuiCombined = (mauiState[i] & 0x80000000UL) | (mauiState[(i + 1) % 624] & 0x7FFFFFFFUL);\n"
		"\t\tuiTwisted = uiCombined >> 1;\n"
		"\t\tif (uiCombined & 1)\n"
		"\t\t{\n"
		"\t\t\tuiTwisted ^= 0x9908B0DFUL;\n"
		"\t\t}\n"
		"\t\tmauiState[i] = mauiState[(i + 397) % 624] ^ uiTwisted;\n"
		"\t}\n"
		"\tmuiIndex = 0;\n"
		"}\n"
		"CMT gc;\n"
		"int main()\n"
		"{\n"
		"\tfor (int i = 0; i < 624; i++) gc.mauiState[i] = (uint32)i * 2654435761UL;\n"
		"\tgc.Twist();\n"
		"\tunsigned int h = 0;\n"
		"\tfor (int i = 0; i < 624; i++) h = h * 31 + (unsigned int)gc.mauiState[i] + (unsigned int)(gc.mauiState[i] >> 16);\n"
		"\treturn (int)h;\n"
		"}\n";
	uint32	auiState[624];
	uint32	uiCombined;
	uint32	uiTwisted;
	uint16	uiHash;
	int		i;
	uint64	uiCycles;

	for (i = 0; i < 624; i++)
	{
		auiState[i] = (uint32)i * 2654435761u;
	}
	for (i = 0; i < 624; i++)
	{
		uiCombined = (auiState[i] & 0x80000000u) | (auiState[(i + 1) % 624] & 0x7FFFFFFFu);
		uiTwisted = uiCombined >> 1;
		if (uiCombined & 1)
		{
			uiTwisted ^= 0x9908B0DFu;
		}
		auiState[i] = auiState[(i + 397) % 624] ^ uiTwisted;
	}
	uiHash = 0;
	for (i = 0; i < 624; i++)
	{
		uiHash = (uint16)(uiHash * 31 + (uint16)auiState[i] + (uint16)(auiState[i] >> 16));
	}
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
	AssertTrue(uiCycles < 780000);	//750160 with BoundDivisions, 830348 with a multiply.
}


//////////////////////////////////////////////////////////////////////////
//
// TimSort's merge loop (two arms in a do while(true) left by gotos and
// breaks, then a galloping loop): its blocks in reverse post order made it
// 14% slower (TestArrayTemplate's mergeLo and mergeHi), so the 816 keeps
// the order they were made in.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP816MergeLoopNative(int16* pi1, int16 iLen1, int16* pi2, int16 iLen2, int16* piDest)
{
	int16	iMinGallop;
	int16	iCount1;
	int16	iCount2;

	iMinGallop = 7;
	for (;;)
	{
		iCount1 = 0;
		iCount2 = 0;
		for (;;)
		{
			if (*pi2 - *pi1 < 0)
			{
				*piDest++ = *pi2++;
				iCount2++;
				iCount1 = 0;
				if (--iLen2 == 0) goto outer;
				if (iCount2 >= iMinGallop) break;
			}
			else
			{
				*piDest++ = *pi1++;
				iCount1++;
				iCount2 = 0;
				if (--iLen1 == 0) goto outer;
				if (iCount1 >= iMinGallop) break;
			}
		}
		do
		{
			*piDest++ = *pi1++;
			if (--iLen1 == 0) goto outer;
			*piDest++ = *pi2++;
			if (--iLen2 == 0) goto outer;
			if (iMinGallop > 0) iMinGallop--;
		}
		while ((*pi1 - *pi2 < 0) && (iMinGallop > 3));
		iMinGallop += 2;
	}
outer:
	while (iLen1-- > 0) *piDest++ = *pi1++;
	while (iLen2-- > 0) *piDest++ = *pi2++;
}


void TestCPP816MergeLoop(void)
{
	static const char*	szSource =
		"int Compare(const void* a, const void* b) { return *(const int*)a - *(const int*)b; }\n"
		"int (*volatile gpCompare)(const void*, const void*) = Compare;\n"
		"int gaiA[256];\n"
		"int gaiB[256];\n"
		"int gaiOut[512];\n"
		"void Merge(int* pi1, int iLen1, int* pi2, int iLen2, int* piDest, int (*Cmp)(const void*, const void*))\n"
		"{\n"
		"\tint\tiMinGallop = 7;\n"
		"\twhile (1)\n"
		"\t{\n"
		"\t\tint\tiCount1 = 0;\n"
		"\t\tint\tiCount2 = 0;\n"
		"\t\tdo\n"
		"\t\t{\n"
		"\t\t\tif (Cmp(pi2, pi1) < 0)\n"
		"\t\t\t{\n"
		"\t\t\t\t*piDest++ = *pi2++;\n"
		"\t\t\t\tiCount2++;\n"
		"\t\t\t\tiCount1 = 0;\n"
		"\t\t\t\tif (--iLen2 == 0) goto outer;\n"
		"\t\t\t\tif (iCount2 >= iMinGallop) break;\n"
		"\t\t\t}\n"
		"\t\t\telse\n"
		"\t\t\t{\n"
		"\t\t\t\t*piDest++ = *pi1++;\n"
		"\t\t\t\tiCount1++;\n"
		"\t\t\t\tiCount2 = 0;\n"
		"\t\t\t\tif (--iLen1 == 0) goto outer;\n"
		"\t\t\t\tif (iCount1 >= iMinGallop) break;\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t\twhile (true);\n"
		"\t\tdo\n"
		"\t\t{\n"
		"\t\t\t*piDest++ = *pi1++;\n"
		"\t\t\tif (--iLen1 == 0) goto outer;\n"
		"\t\t\t*piDest++ = *pi2++;\n"
		"\t\t\tif (--iLen2 == 0) goto outer;\n"
		"\t\t\tif (iMinGallop > 0) iMinGallop--;\n"
		"\t\t}\n"
		"\t\twhile (Cmp(pi1, pi2) < 0 && iMinGallop > 3);\n"
		"\t\tiMinGallop += 2;\n"
		"\t}\n"
		"outer:\n"
		"\twhile (iLen1-- > 0) *piDest++ = *pi1++;\n"
		"\twhile (iLen2-- > 0) *piDest++ = *pi2++;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (int k = 0; k < 4; k++)\n"
		"\t{\n"
		"\t\tfor (int i = 0; i < 256; i++) { gaiA[i] = i * 4 + (k & 1); gaiB[i] = i * 4 + 2 - ((i >> 3) & 1) * k; }\n"
		"\t\tMerge(gaiA, 256, gaiB, 256, gaiOut, gpCompare);\n"
		"\t\tfor (int i = 0; i < 512; i++) h = h * 31 + (unsigned int)gaiOut[i];\n"
		"\t}\n"
		"\treturn (int)h;\n"
		"}\n";
	int16	aiA[256];
	int16	aiB[256];
	int16	aiOut[512];
	uint16	uiHash;
	int		i;
	int		k;
	uint64	uiCycles;

	uiHash = 0;
	for (k = 0; k < 4; k++)
	{
		for (i = 0; i < 256; i++)
		{
			aiA[i] = (int16)(i * 4 + (k & 1));
			aiB[i] = (int16)(i * 4 + 2 - ((i >> 3) & 1) * k);
		}
		TestCPP816MergeLoopNative(aiA, 256, aiB, 256, aiOut);
		for (i = 0; i < 512; i++)
		{
			uiHash = (uint16)(uiHash * 31 + (uint16)aiOut[i]);
		}
	}
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
	//1032421 in the blocks' order, 1173183 in reverse post order; +15.4K clearing the arrays (bss) at start up: 1087842 with
	//every argument pushed.  Register arguments made it 1220702: Compare (two pointers, one in A and X, one pushed) is
	//only called through a pointer, so through its plain label, which pushed the pushed one again and called the body;
	//it moves it down instead now, 1177778.
	AssertTrue(uiCycles < 1200000);
}


//////////////////////////////////////////////////////////////////////////
//
// An object returned by value is built in a slot and copied to the hidden
// result a word at a time: its pointer field is stored as a pointer and
// loaded as an integer.  The slot's accesses are made one type
// (UnifySlotTypes), so it is split and promoted, and the fields go
// straight to the result (FAT's SFatCache CFatInfoSectorCache::ReadSector).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ReturnedObjectSlots(void)
{
	static const char*	szSource =
		"struct SCache { void* pv; unsigned long uiStamp; };\n"
		"struct CSector { unsigned long long uiStamp; unsigned long uiSector; char ac[16]; };\n"
		"CSector gas[4];\n"
		"CSector* Get(unsigned long ui) { return (ui < 4) ? &gas[ui] : 0; }\n"
		"SCache Read(unsigned long ui)\n"
		"{\n"
		"\tCSector*\tpc;\n"
		"\tSCache\t\ts;\n"
		"\n"
		"\tpc = Get(ui);\n"
		"\tif (pc != 0)\n"
		"\t{\n"
		"\t\ts.pv = pc->ac;\n"
		"\t\ts.uiStamp = (unsigned long)pc->uiStamp;\n"
		"\t}\n"
		"\telse\n"
		"\t{\n"
		"\t\ts.pv = 0;\n"
		"\t\ts.uiStamp = 0xffffffff;\n"
		"\t}\n"
		"\treturn s;\n"
		"}\n"
		"SCache (*volatile gpRead)(unsigned long) = Read;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (int i = 0; i < 4; i++) gas[i].uiStamp = (unsigned long long)(i * 1000 + 7);\n"
		"\tfor (unsigned long j = 0; j < 100; j++) { SCache s = gpRead(j & 7); h = h * 31 + (unsigned int)s.uiStamp + (s.pv ? 1 : 0); }\n"
		"\treturn (int)h;\n"
		"}\n";
	uint16	uiHash;
	uint32	j;
	uint32	uiIndex;
	uint32	uiStamp;
	uint64	uiCycles;

	uiHash = 0;
	for (j = 0; j < 100; j++)
	{
		uiIndex = j & 7;
		uiStamp = (uiIndex < 4) ? (uint32)(uiIndex * 1000 + 7) : 0xFFFFFFFF;
		uiHash = (uint16)(uiHash * 31 + (uint16)uiStamp + ((uiIndex < 4) ? 1 : 0));
	}
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource, NULL, NULL, &uiCycles));
	AssertTrue(uiCycles < 58000);	//54330 promoted, 65558 through the slot.
}


//////////////////////////////////////////////////////////////////////////
//
// x % c == 0 and x % c != 0 for 16 and 32 bit, signed (negative too, and
// the most negative) and unsigned x, odd and even c, are a multiply by the
// inverse of c's odd part and a rotate (DivisibilityTests): no runtime
// remainder.
//
//////////////////////////////////////////////////////////////////////////
static int TestCPP816DivisibilityLeap(int16 y)
{
	return ((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0));
}


static int TestCPP816DivisibilityTestsNative(int16 x, uint16 u, int32 l, uint32 ul)
{
	return ((x % 7) == 0) + ((x % 12) != 0) * 2 + ((u % 100) == 0) * 4 + ((u % 6) == 0) * 8 + ((l % 86400) == 0) * 16 + ((ul % 1000) != 0) * 32 + ((l % 3) == 0) * 64;
}


void TestCPP816DivisibilityTests(void)
{
	static const char*	szFunctions =
		"int Leap(int y) { return ((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0)); }\n"
		"int Tests(int x, unsigned int u, long l, unsigned long ul)\n"
		"{\n"
		"\treturn ((x % 7) == 0) + ((x % 12) != 0) * 2 + ((u % 100) == 0) * 4 + ((u % 6) == 0) * 8 + ((l % 86400) == 0) * 16 + ((ul % 1000) != 0) * 32 + ((l % 3) == 0) * 64;\n"
		"}\n";
	static const char*	szMain =
		"int (*volatile gpLeap)(int) = Leap;\n"
		"int (*volatile gpTests)(int, unsigned int, long, unsigned long) = Tests;\n"
		"int main()\n"
		"{\n"
		"\tunsigned int h = 0;\n"
		"\tfor (long i = -33000; i < 33000; i += 7)\n"
		"\t{\n"
		"\t\tint x = (int)i;\n"
		"\t\th = h * 31 + (unsigned int)gpLeap(x);\n"
		"\t\th = h * 31 + (unsigned int)gpTests(x, (unsigned int)x, i * 4321, (unsigned long)i * 1000UL);\n"
		"\t}\n"
		"\th = h * 31 + (unsigned int)gpTests(-32768, 65535, -2147483647L - 1, 4294967295UL);\n"
		"\th = h * 31 + (unsigned int)gpTests(32767, 0, 2147483647L, 0);\n"
		"\th = h * 31 + (unsigned int)gpLeap(-32768) + (unsigned int)gpLeap(-400) * 3;\n"
		"\treturn (int)h;\n"
		"}\n";
	CChars		szSource;
	uint16		uiHash;
	int32		i;
	int16		x;
	uint64		uiCycles;

	uiHash = 0;
	for (i = -33000; i < 33000; i += 7)
	{
		x = (int16)i;
		uiHash = (uint16)(uiHash * 31 + TestCPP816DivisibilityLeap(x));
		uiHash = (uint16)(uiHash * 31 + TestCPP816DivisibilityTestsNative(x, (uint16)x, (int32)((uint32)i * 4321u), (uint32)i * 1000u));
	}
	uiHash = (uint16)(uiHash * 31 + TestCPP816DivisibilityTestsNative(-32768, 65535, -2147483647 - 1, 4294967295u));
	uiHash = (uint16)(uiHash * 31 + TestCPP816DivisibilityTestsNative(32767, 0, 2147483647, 0));
	uiHash = (uint16)(uiHash * 31 + TestCPP816DivisibilityLeap(-32768) + TestCPP816DivisibilityLeap(-400) * 3);

	szSource.Init(szFunctions);
	szSource.Append(szMain);
	AssertInt((int16)uiHash, TestCPP816RunProfiled(szSource.Text(), NULL, NULL, &uiCycles));
	szSource.Kill();

	AssertInt(0, TestCPP816CountInAssembly(szFunctions, "jsl R816_SRem"));
	AssertInt(0, TestCPP816CountInAssembly(szFunctions, "jsl R816_URem"));
}


//////////////////////////////////////////////////////////////////////////
//
// A small function that takes an object by value (passed as its bytes: the
// callee takes the parameter's address) is inlined, with the object in a
// slot in the caller; the callee changing its copy changes the caller's
// object neither.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816InlineObjectParameters(void)
{
	static const char*	szSource =
		"struct SPair { int a; int b; char c; };\n"
		"int Weigh(SPair s, int k) { s.a += k; return s.a * 2 + s.b - s.c; }\n"
		"int main()\n"
		"{\n"
		"\tSPair\tp;\n"
		"\tint\t\tr = 0;\n"
		"\tp.a = 1; p.b = 10; p.c = 3;\n"
		"\tfor (int i = 0; i < 20; i++)\n"
		"\t{\n"
		"\t\tp.b = i;\n"
		"\t\tr += Weigh(p, i);\n"
		"\t}\n"
		"\treturn (p.a == 1) ? r : -1;\n"
		"}\n";
	CAsmW65C816Profile			cProfile;
	SAsmW65C816FunctionProfile*	psWeigh;
	uint64					uiCycles;

	cProfile.Init();
	AssertInt(20 * 2 + 2 * 190 + 190 - 60, TestCPP816RunProfiled(szSource, NULL, &cProfile, &uiCycles));
	psWeigh = cProfile.Get("_Weigh__FC5SPairs");
	AssertTrue((psWeigh == NULL) || (psWeigh->ulliCalls == 0));
	cProfile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A variadic function's named parameter of a class that isn't trivially
// copyable (SupportLib's CTileLayerCel::SetTiles takes a Ptr) is passed by
// the address of a copy the caller makes and destroys, as any call's; the
// last named parameter stays on the stack for va_start.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816VariadicObjectParameters(void)
{
	AssertInt(1 + 100 + 60 + 1000 + 10000, TestCPP816RunProject(NULL, "",
		"#include <stdarg.h>\n"
		"int giCopies = 0;\n"
		"int giDestroyed = 0;\n"
		"class CPtr { public: int* mp; CPtr(int* p) { mp = p; } CPtr(const CPtr& c) { mp = c.mp; giCopies++; } ~CPtr() { giDestroyed++; } };\n"
		"int Sum(int x, CPtr p, int n, ...)\n"
		"{\n"
		"\tva_list\tap;\n"
		"\tint\t\tt;\n"
		"\tint\t\ti;\n"
		"\tva_start(ap, n);\n"
		"\tt = x + *p.mp;\n"
		"\tfor (i = 0; i < n; i++) t += va_arg(ap, int);\n"
		"\tva_end(ap);\n"
		"\treturn t;\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tint\t\tv = 100;\n"
		"\tCPtr\tc(&v);\n"
		"\tint\t\tr = Sum(1, c, 3, 10, 20, 30);\n"
		"\treturn r + giCopies * 1000 + giDestroyed * 10000;\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource for eTarget (it must compile) and returns how many
// warnings mention szWarning.
//
//////////////////////////////////////////////////////////////////////////
int TestCPP816CountWarnings(const char* szSource, const char* szWarning, ECPPTarget eTarget = CPPT_W65C816)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;
	CChars			szWarnings;
	int				iCount;
	const char*		sz;

	cBuilder.Init();
	szAssembly.Init();
	szErrors.Init();
	szWarnings.Init();
	AssertTrue(CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", szSource, false, NULL, eTarget, &szWarnings, NULL, false, false, false, true, &gcTestCPP816Options));
	AssertString("", szErrors.Text());

	iCount = 0;
	for (sz = strstr(szWarnings.Text(), szWarning); sz; sz = strstr(sz + 1, szWarning))
	{
		iCount++;
	}
	szWarnings.Kill();
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function whose label starts with szPrefix (in the linked
// program's map) starts with an aligned frame's prologue: phd, tsc, tax (a
// frame that isn't aligned is phd, tsc, sec).
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// The address of the function whose label starts with szPrefix: its body's
// (a register function's label with its suffix, after the stub at the plain
// label) if it has one.  0 if there is none.
//
//////////////////////////////////////////////////////////////////////////
uint32 TestCPP816FunctionAddress(CCPP65Builder* pcBuilder, const char* szPrefix)
{
	const char*	szLine;
	const char*	szEnd;
	uint32		uiAddress;

	uiAddress = 0;
	for (szLine = strstr(pcBuilder->GetMap(), "Symbols\n"); szLine; szLine = strchr(szLine + 1, '\n'))
	{
		if ((szLine[1] == ' ') && (szLine[3] == '0') && (szLine[4] == 'x') && (strncmp(&szLine[16], szPrefix, strlen(szPrefix)) == 0))
		{
			szEnd = strchr(&szLine[16], ' ');
			if (szEnd && (memchr(&szLine[16], '.', (size)(szEnd - &szLine[16])) != NULL))
			{
				return (uint32)strtoul(&szLine[3], NULL, 16);
			}
			if (uiAddress == 0)
			{
				uiAddress = (uint32)strtoul(&szLine[3], NULL, 16);
			}
		}
	}
	return uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
// The register words a function's prologue pushes after phd (pha, phx,
// phy: a register function's), from the byte after phd at uiAddress.
//
//////////////////////////////////////////////////////////////////////////
int TestCPP816RegisterPushes(CAsm65Memory* pcMemory, uint32 uiAddress)
{
	int		i;
	uint8	uiOpcode;

	for (i = 0; i < 3; i++)
	{
		uiOpcode = pcMemory->GetByte(uiAddress + 1 + (uint32)i);
		if ((uiOpcode != 0x48) && (uiOpcode != 0xDA) && (uiOpcode != 0x5A))
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function whose label starts with szPrefix has an aligned
// frame: phd, its register pushes, then tsc and tax.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP816IsAligned(CCPP65Builder* pcBuilder, CAsm65Memory* pcMemory, const char* szPrefix)
{
	uint32	uiAddress;
	int		iPushes;

	uiAddress = TestCPP816FunctionAddress(pcBuilder, szPrefix);
	if (uiAddress == 0)
	{
		AssertString("A function", szPrefix);
		return false;
	}
	iPushes = TestCPP816RegisterPushes(pcMemory, uiAddress);
	return (pcMemory->GetByte(uiAddress) == 0x0B) && (pcMemory->GetByte(uiAddress + 1 + iPushes) == 0x3B) && (pcMemory->GetByte(uiAddress + 2 + iPushes) == 0xAA);
}


//////////////////////////////////////////////////////////////////////////
//
// Builds szSource for the W65C816 (aligned frames estimated, or measured by
// pcProfile if given) and runs it, measuring its functions into pcMeasure (if given).
// Returns main's int; *puiCycles gets the cycles, and *pbAligned (for each
// of the aszPrefixes' functions) whether it has an aligned frame.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP816RunFrames(const char* szSource, CAsmW65C816Profile* pcProfile, CAsmW65C816Profile* pcMeasure, uint64* puiCycles, const char** aszPrefixes, bool* abAligned)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	CCPP65Builder		cBuilder;
	CAsm65Memory		cMemory;
	CFastW65C816			cFast;
	CChars				szProject;
	CFileUtil			cFileUtil;
	size				i;
	int32				iResult;

	iResult = -1000000;
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetFrameProfile(pcProfile);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());

	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(cBuilder.GetOutput());
	for (i = 0; aszPrefixes[i]; i++)
	{
		abAligned[i] = TestCPP816IsAligned(&cBuilder, &cMemory, aszPrefixes[i]);
	}
	cMemory.InitFast65816(&cFast);
	cFast.Reset();
	if (pcMeasure)
	{
		pcMeasure->Run(&cMemory, &cFast, cBuilder.GetMap(), TEST_CPP816_STEPS);
	}
	else
	{
		TestCPP816RunChecked(&cMemory, &cFast, TEST_CPP816_STEPS);
	}
	AssertTrue(cFast.IsStopped());
	if (cFast.IsStopped())
	{
		iResult = (int16)cFast.GetC();
		AssertIntHex(GetCPPTarget(CPPT_W65C816)->GetStackTop(), (uint32)cFast.GetS());
	}
	*puiCycles = cFast.NumCycles();
	cFast.Kill();
	cMemory.Kill();
	cBuilder.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A function gets D on a page (a cycle less each direct page access) when
// it's worth it and it can't recurse: by default when its loops' accesses
// are estimated to be well past the cost; with a profile (CAsmW65C816Profile,
// CCPP65Builder::SetFrameProfile) when they were measured to be.  Its
// arguments are copied up to the aligned frame (stored, or pushed when the
// frame passes the page) and S as it was is kept for the epilogue.  (The
// functions are called through pointers, so they aren't inlined.)
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AlignedFrames(void)
{
	static const char*	szSource =
		"int Sum(int* p, int n, int k) { int s = 0; for (int i = 0; i < n; i++) { s += p[i] * k; } return s; }\n"
		"long Wide(int n, long add) { int a[200]; for (int i = 0; i < 200; i++) { a[i] = i + n; } long s = add; for (int i = 0; i < 200; i++) { s += a[i]; } return s; }\n"
		"int Cold(int* p, int n) { int s = 0; for (int i = 0; i < n; i++) { s += p[i] * p[i] - i; } return s; }\n"
		"int (*volatile gpSum)(int*, int, int) = Sum;\n"
		"long (*volatile gpWide)(int, long) = Wide;\n"
		"int (*volatile gpCold)(int*, int) = Cold;\n"
		"long Big(int n, long add) { int a[200]; for (int i = 0; i < 200; i++) { a[i] = i + n; } long s = add; for (int i = 0; i < 200; i++) { s += a[i]; } if (n > 0) { s += Big(n - 1, 1); } return s; }\n"
		"long (*volatile gpBig)(int, long) = Big;\n"
		"int main()\n"
		"{\n"
		"	int v[100];\n"
		"	long w = 0;\n"
		"	int r = 0;\n"
		"	for (int i = 0; i < 100; i++) { v[i] = i; }\n"
		"	for (int i = 0; i < 5; i++) { r += gpSum(v, 100, 1); }\n"
		"	for (int i = 0; i < 20; i++) { w += gpWide(i, 1); }\n"
		"	int c = gpCold(v, 3);\n"
		"	long b = gpBig(5, 7);\n"
		"	return ((r == 24750) && (w == 436020L) && (c == 2) && (b == 122412L)) ? 7 : 1;\n"
		"}\n";
	static const char*	aszPrefixes[] = { "_Sum", "_Wide", "_Cold", "_Big", NULL };
	CAsmW65C816Profile		cProfile;
	uint64				uiEstimated;
	uint64				uiMeasured;
	bool				abAligned[4];

	//Estimated: Sum, Wide (its frame past the page) and Cold have loops; Big calls itself.
	cProfile.Init();
	AssertInt(7, TestCPP816RunFrames(szSource, NULL, &cProfile, &uiEstimated, aszPrefixes, abAligned));
	AssertTrue(abAligned[0]);
	AssertTrue(abAligned[1]);
	AssertTrue(abAligned[2]);
	AssertFalse(abAligned[3]);
	AssertTrue((cProfile.Get("_Big__Fsi") != NULL) && cProfile.Get("_Big__Fsi")->bRecursive);
	AssertTrue((cProfile.Get("_Sum__FPsss") != NULL) && !cProfile.Get("_Sum__FPsss")->bRecursive);
	AssertTrue((cProfile.Get("_Wide__Fsi") != NULL) && !cProfile.Get("_Wide__Fsi")->bRecursive);

	//Measured: Cold's loop ran 3 times once, not worth a page of stack.
	AssertInt(7, TestCPP816RunFrames(szSource, &cProfile, NULL, &uiMeasured, aszPrefixes, abAligned));
	AssertTrue(abAligned[0]);
	AssertTrue(abAligned[1]);
	AssertFalse(abAligned[2]);
	AssertFalse(abAligned[3]);
	cProfile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The argument words the function whose label starts with szPrefix copies
// into its aligned frame: the lda long,X + sta dp pairs after its aligned
// prologue (phd, tsc, tax, sec, sbc, and, tcs, tcd).  -1 if it isn't aligned.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP816AlignedCopies(CCPP65Builder* pcBuilder, CAsm65Memory* pcMemory, const char* szPrefix)
{
	uint32		uiAddress;
	int32		iCopies;

	if (!TestCPP816IsAligned(pcBuilder, pcMemory, szPrefix))
	{
		return -1;
	}
	uiAddress = TestCPP816FunctionAddress(pcBuilder, szPrefix);
	uiAddress += 12 + (uint32)TestCPP816RegisterPushes(pcMemory, uiAddress);
	for (iCopies = 0; (pcMemory->GetByte(uiAddress) == 0xBF) && (pcMemory->GetByte(uiAddress + 4) == 0x85); iCopies++)
	{
		uiAddress += 6;
	}
	return iCopies;
}


//////////////////////////////////////////////////////////////////////////
//
// An aligned frame copies only the argument words its function reads: not
// an argument it ignores, nor a long's high word when only its low word is
// used.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AlignedFrameUnreadArguments(void)
{
	static const char*	szSource =
		"int Sum(int* p, int unused, int n) { int s = 0; for (int i = 0; i < n; i++) { s += p[i]; } return s; }\n"
		"int Low(int* p, long n) { int s = 0; for (int i = 0; i < (int)n; i++) { s += p[i]; } return s; }\n"
		"int All(int* p, int k, int n) { int s = 0; for (int i = 0; i < n; i++) { s += p[i] * k; } return s; }\n"
		"int (*volatile gpSum)(int*, int, int) = Sum;\n"
		"int (*volatile gpLow)(int*, long) = Low;\n"
		"int (*volatile gpAll)(int*, int, int) = All;\n"
		"int main()\n"
		"{\n"
		"	int v[50];\n"
		"	for (int i = 0; i < 50; i++) { v[i] = i; }\n"
		"	int a = gpSum(v, 99, 50);\n"
		"	int b = gpLow(v, 0x70000L + 10);\n"
		"	int c = gpAll(v, 2, 20);\n"
		"	return ((a == 1225) && (b == 45) && (c == 380)) ? 7 : 1;\n"
		"}\n";
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	CCPP65Builder		cBuilder;
	CAsm65Memory		cMemory;
	CFastW65C816			cFast;
	CChars				szProject;
	CFileUtil			cFileUtil;

	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());

	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(cBuilder.GetOutput());

	//p's two words and n; not unused.  p and n's low word.  All of p, k and n.
	AssertInt(3, TestCPP816AlignedCopies(&cBuilder, &cMemory, "_Sum"));
	AssertInt(3, TestCPP816AlignedCopies(&cBuilder, &cMemory, "_Low"));
	AssertInt(4, TestCPP816AlignedCopies(&cBuilder, &cMemory, "_All"));

	cMemory.InitFast65816(&cFast);
	cFast.Reset();
	TestCPP816RunChecked(&cMemory, &cFast, TEST_CPP816_STEPS);
	AssertTrue(cFast.IsStopped());
	AssertInt(7, (int16)cFast.GetC());
	cFast.Kill();
	cMemory.Kill();
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Integer constant expressions that overflow their type, and constants that
// an implicit conversion truncates, are warned about: the W65C816's 16 bit int
// turns code written for a 32 bit int quietly wrong ("12000 * 6").
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Warnings(void)
{
	//An int expression that doesn't fit 16 bits: not on the 65C64, where int is 32 bits.
	AssertInt(1, TestCPP816CountWarnings("int F(void) { return 12000 * 6; }\n", "Integer overflow"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { return 12000 * 6; }\n", "Integer overflow", CPPT_65C64));
	AssertInt(1, TestCPP816CountWarnings("int F(void) { return 30000 + 30000; }\n", "Integer overflow"));
	AssertInt(1, TestCPP816CountWarnings("int F(void) { return 1 << 15; }\n", "Integer overflow"));
	AssertInt(1, TestCPP816CountWarnings("unsigned F(void) { return 40000u * 2; }\n", "Integer overflow"));

	//What the code means is fine: a long expression, unsigned wrap by subtraction, a cast.
	AssertInt(0, TestCPP816CountWarnings("long F(void) { return (long)12000 * 6; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("long F(void) { return 12000L * 6; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("unsigned F(void) { return 0u - 1; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { return (int)70000L; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { return 32767; }\n", "overflow"));

	//A constant an implicit conversion truncates: parameters, initialisers, assignments, returns and globals.
	AssertInt(1, TestCPP816CountWarnings("void G(int i);\nvoid F(void) { G(89037124); }\n", "Integer conversion overflow"));
	AssertInt(0, TestCPP816CountWarnings("void G(int i);\nvoid F(void) { G(89037124); }\n", "Integer conversion overflow", CPPT_65C64));
	AssertInt(1, TestCPP816CountWarnings("int F(void) { int i = 70000; return i; }\n", "Integer conversion overflow"));
	AssertInt(1, TestCPP816CountWarnings("int F(void) { int i; i = 70000; return i; }\n", "Integer conversion overflow"));
	AssertInt(1, TestCPP816CountWarnings("int F(void) { return 70000; }\n", "Integer conversion overflow"));
	AssertInt(1, TestCPP816CountWarnings("int giX = 70000;\n", "Integer conversion overflow"));
	AssertInt(1, TestCPP816CountWarnings("char F(void) { return 300; }\n", "Integer conversion overflow"));

	//Bit patterns are fine: -1 to unsigned, 0xFFFF to a 16 bit int, 255 or -128 to a char.
	AssertInt(0, TestCPP816CountWarnings("unsigned F(void) { unsigned u = -1; return u; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { int i = 0xFFFF; return i; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("char F(void) { char c = 255; c = -128; return c; }\n", "overflow"));
	AssertInt(0, TestCPP816CountWarnings("unsigned long F(void) { unsigned long ul = 0xFFFFFFFF; return ul; }\n", "overflow"));

	//An expression looked at more than once is warned about once.
	AssertInt(1, TestCPP816CountWarnings("void G(int i);\nvoid F(void) { G(12000 * 6); }\n", "Integer overflow"));

	//An enum is as big as its enumerators need ([dcl.enum]), not always an int: TRISTATE's TRIERROR = 0xffffffff.
	AssertInt(0, TestCPP816CountWarnings("enum ETri { TRI_FALSE = 0, TRI_ERROR = 0xffffffff };\nETri F(void) { ETri e = TRI_ERROR; return e; }\n", "overflow"));
	AssertInt(2 + 4 * 8 + 1 * 64 + 2 * 512 + 4 * 4096, TestCPP816RunProject(NULL, "",
		"enum E1 { A1 = 1 };\n"
		"enum E2 { A2 = 0, B2 = 0xffffffff };\n"
		"enum E3 : unsigned char { A3 };\n"
		"enum class E4 { A4 };\n"
		"enum E5 { A5 = -1, B5 = 40000 };\n"
		"int main(void)\n"
		"{\n"
		"\treturn (int)(sizeof(E1) + sizeof(E2) * 8 + sizeof(E3) * 64 + sizeof(E4) * 512 + sizeof(E5) * 4096);\n"
		"}\n"));
	AssertInt(3, TestCPP816RunProject(NULL, "",
		"enum ETri { TRI_FALSE = 0, TRI_TRUE = 1, TRI_ERROR = 0xffffffff };\n"
		"ETri Get(int i) { return (i == 0) ? TRI_ERROR : TRI_TRUE; }\n"
		"int main(void)\n"
		"{\n"
		"\tETri e = Get(0);\n"
		"\treturn ((e == TRI_ERROR) ? 1 : 0) + ((unsigned long)e == 0xffffffffUL ? 2 : 0) + ((Get(1) == TRI_ERROR) ? 4 : 0);\n"
		"}\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A for loop's counter compared with a wider value that isn't a constant
// wraps before it can pass one that doesn't fit: a 16 bit i against a 32 bit
// count goes round forever once the count passes 65535.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816LoopCounterWarnings(void)
{
	const char*	szWarning = "Loop counter narrower than its bound";

	//i++, ++i, i += 2 and i-- against a 32 bit bound, on either side, in && and with a comma.
	AssertInt(1, TestCPP816CountWarnings("unsigned long N(void);\nint F(void) { unsigned i; int t = 0; for (i = 0; i < N(); i++) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; for (unsigned i = 0; n > i; ++i) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; for (unsigned i = 0; i <= n; i += 2) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; for (unsigned i = 10; i != n; i--) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long n, int b) { int t = 0; for (unsigned i = 0; b && (i < n); i++) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; unsigned j = 0; for (unsigned i = 0; i < n; i++, j++) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("struct S { unsigned m; int F(unsigned long n) { int t = 0; for (m = 0; m < n; m++) t++; return t; } };\nint G(void) { S s; return s.F(5); }\n", szWarning));

	//The same width, a wider counter, a constant bound, a converted bound, a variable that isn't stepped, and the 65C64 (32 bit ints).
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned n) { int t = 0; for (unsigned i = 0; i < n; i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned n) { int t = 0; for (unsigned long i = 0; i < n; i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { int t = 0; for (unsigned i = 0; i < 100L; i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(char b) { int t = 0; for (unsigned char i = 0; i < (b ? 7 : 3); i++) t++; return t; }\n", szWarning));
	AssertInt(1, TestCPP816CountWarnings("int F(char b, int n) { int t = 0; for (unsigned char i = 0; i < (b ? n : 3); i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; for (unsigned i = 0; i < (unsigned)n; i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; unsigned j = 0; for (unsigned i = 0; j < n; i++) t++; return t; }\n", szWarning));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned long n) { int t = 0; for (unsigned i = 0; i < n; i++) t++; return t; }\n", szWarning, CPPT_65C64));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned long long n) { int t = 0; for (unsigned i = 0; i < n; i++) t++; return t; }\n", szWarning, CPPT_65C64));
}


//////////////////////////////////////////////////////////////////////////
//
// A comparison with a constant outside the other side's range is always true
// or always false (a 16 bit size compared with a 32 bit "not found").  With
// CCPPCompileOptions::mbWarnNarrowing a non constant integer implicitly converted to a narrower
// type is warned about (a 32 bit index stored in a 16 bit one).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816RangeWarnings(void)
{
	//A 16 bit unsigned can't be 0xFFFFFFFF; in conditions and in values.
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned u) { return u == 0xFFFFFFFFUL; }\n", "Comparison is always false"));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned u) { return u != 0xFFFFFFFFUL; }\n", "Comparison is always true"));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned u) { if (u == 0xFFFFFFFFUL) return 1; return 0; }\n", "Comparison is always false"));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned u) { return u == 0xFFFFFFFFUL; }\n", "Comparison", CPPT_65C64));

	//At a type's limit: the smallest value of a signed char.
	AssertInt(1, TestCPP816CountWarnings("int F(signed char c) { return c >= -128; }\n", "Comparison is always true: the constant -128 is the smallest"));

	//A 16 bit product (it may have wrapped) widened afterwards; none when an operand is widened first.
	AssertInt(1, TestCPP816CountWarnings("long long F(unsigned u) { return u * 0x39e7; }\n", "Integer arithmetic widened"));
	AssertInt(0, TestCPP816CountWarnings("long long F(unsigned u) { return (long long)u * 0x39e7; }\n", "Integer arithmetic widened"));

	//Relational comparisons, the constant on either side.
	AssertInt(1, TestCPP816CountWarnings("int F(int i) { return i < 40000L; }\n", "Comparison is always true"));
	AssertInt(1, TestCPP816CountWarnings("int F(int i) { return 40000L < i; }\n", "Comparison is always false"));
	AssertInt(1, TestCPP816CountWarnings("int F(int i) { return i >= -40000L; }\n", "Comparison is always true"));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned char c) { return c >= 256; }\n", "Comparison is always false"));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned char c) { return 256 > c; }\n", "Comparison is always true"));

	//The constant is converted first: -1 is 0xFFFF as a 16 bit unsigned, but an int on the 65C64.
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned u) { return u == -1; }\n", "Comparison"));
	AssertInt(1, TestCPP816CountWarnings("int F(unsigned short u) { return u == -1; }\n", "Comparison is always false", CPPT_65C64));

	//In range, both sides variable or constant, and a signed side compared as unsigned.
	AssertInt(0, TestCPP816CountWarnings("int F(int i) { return i == 32767; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("int F(int i) { return i < 0; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned long ul) { return ul == 0xFFFFFFFFUL; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned u, long l) { return u == l; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { return 70000L == 0xFFFFFFFFUL; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("int F(int i) { return i == 0xFFFFFFFFUL; }\n", "Comparison"));
	AssertInt(0, TestCPP816CountWarnings("enum ETri { TRI_FALSE = 0, TRI_ERROR = 0xffffffff };\nint F(ETri e) { return e == TRI_ERROR; }\n", "Comparison"));

	//Narrowing is off by default.
	AssertInt(0, TestCPP816CountWarnings("int F(long l) { int i = l; return i; }\n", "narrows"));

	gcTestCPP816Options.mbWarnNarrowing = true;
	AssertInt(1, TestCPP816CountWarnings("int F(long l) { int i = l; return i; }\n", "Integer conversion narrows"));
	AssertInt(1, TestCPP816CountWarnings("int F(long l) { int i; i = l; return i; }\n", "Integer conversion narrows"));
	AssertInt(1, TestCPP816CountWarnings("int F(long l) { return l; }\n", "Integer conversion narrows"));
	AssertInt(1, TestCPP816CountWarnings("void G(int i);\nvoid F(long l) { G(l); }\n", "Integer conversion narrows"));
	AssertInt(1, TestCPP816CountWarnings("char F(int i) { return i; }\n", "Integer conversion narrows"));

	//Widening, explicit casts, same width and constants (which have their own warning) aren't.
	AssertInt(0, TestCPP816CountWarnings("long F(int i) { long l = i; return l; }\n", "narrows"));
	AssertInt(0, TestCPP816CountWarnings("int F(long l) { return (int)l; }\n", "narrows"));
	AssertInt(0, TestCPP816CountWarnings("unsigned F(int i) { return i; }\n", "narrows"));
	AssertInt(0, TestCPP816CountWarnings("int F(void) { int i = 7L; return i; }\n", "narrows"));
	AssertInt(0, TestCPP816CountWarnings("int F(long l) { return l; }\n", "narrows", CPPT_65C64));
	gcTestCPP816Options.mbWarnNarrowing = false;
}


//////////////////////////////////////////////////////////////////////////
//
// What making BaseLib's size 16 bits on the W65C816 (with a 32 bit lsize)
// turned up.  Libc65's printf read %ld as a 16 bit int.  MemSwp is stringW65C816.asm's (lsize length): when BaseLib
// declared it with a size length the call named a label nothing defined, and
// its stub was the 65C64's stp, which the W65C816 ran as something else: the
// sorts called address 0.  A missing name's stub now stops the W65C816.  A
// pointer to a 16 bit value passed for a pointer to a 32 bit one, and a call
// that matches two overloads (size and lsize), are errors that say so, not
// "doesn't support yet".  A 16 bit multiply widened to 64 bits (a key made
// from a 16 bit index) is warned about, and so is a comparison that can't
// change once the values are followed (a 16 bit "not found" widened and
// compared with the 32 bit one).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816SizeLsizeBugs(void)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;
	CAsm65Memory	cMemory;
	int64			lliAddress;

	//BaseLib's sorts swap with MemSwp.
	AssertInt(1 + 3 * 10, TestCPP816RunProject(NULL, "",
		"#include \"BaseLib/ArrayUint.h\"\n"
		"int main(void)\n"
		"{\n"
		"\tCArrayUint\tc;\n"
		"\tint\t\t\tr;\n"
		"\tc.Init();\n"
		"\tc.Add(3);\n"
		"\tc.Add(1);\n"
		"\tc.Add(2);\n"
		"\tc.BubbleSort();\n"
		"\tr = c[0] + c[2] * 10;\n"
		"\tc.QuickSort();\n"
		"\tc.Kill();\n"
		"\treturn r;\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));

	//printf's l is a long, not an int: an int is 16 bits on the W65C816.
	AssertInt(1, TestCPP816RunProject(NULL, "",
		"#include <stdio.h>\n"
		"#include <string.h>\n"
		"int main(void)\n"
		"{\n"
		"\tchar\tsz[64];\n"
		"\tsprintf(sz, \"%ld %lu %d %lx %u\", -65588L, 65588UL, 7, 0x12345678UL, 9u);\n"
		"\treturn strcmp(sz, \"-65588 65588 7 12345678 9\") == 0;\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));

	//TestMemoryQueue's FillQueueElement: uiSize - 1 is both widened for memset and a store's folded index (Y).  Its
	//store to the frame was dropped (the zext's lda right after took it from A), so Y was read from a slot never
	//written and the '\0' went somewhere else.
	AssertInt(1, TestCPP816RunProject(NULL, "",
		"#include <string.h>\n"
		"void Fill(void* pvData, unsigned short uiSize, char c)\n"
		"{\n"
		"\tif (pvData && uiSize > 1)\n"
		"\t{\n"
		"\t\tmemset(pvData, c, uiSize - 1);\n"
		"\t\t((char*)pvData)[uiSize - 1] = '\\0';\n"
		"\t}\n"
		"}\n"
		"int main(void)\n"
		"{\n"
		"\tchar\tac[40];\n"
		"\tmemset(ac, 'x', 40);\n"
		"\tFill(ac, 28, 'D');\n"
		"\treturn (ac[26] == 'D') && (ac[27] == '\\0') && (ac[28] == 'x');\n"
		"}\n", false, NULL, 0, TEST_CPP816_SLOW_STEPS, true));

	//No conversion from unsigned short* to unsigned long*.
	cBuilder.Init();
	szAssembly.Init();
	szErrors.Init();
	AssertFalse(CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp",
		"void F(unsigned long* pul) { *pul = 7; }\n"
		"int main(void) { unsigned short u; F(&u); return u; }\n", false, NULL, CPPT_W65C816));
	AssertNull(strstr(szErrors.Text(), "doesn't support"));
	AssertNull(strstr(szErrors.Text(), "Unknown instruction"));
	AssertNotNull(strstr(szErrors.Text(), "No function matches the arguments (unsigned short*): it takes (unsigned long*)"));
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();

	//An int matches a size and an lsize parameter as well.
	cBuilder.Init();
	szAssembly.Init();
	szErrors.Init();
	AssertFalse(CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp",
		"void M(void* p, unsigned short u);\n"
		"void M(void* p, unsigned long u);\n"
		"int main(void) { char a[4]; M(a, 6); return 0; }\n", false, NULL, CPPT_W65C816));
	AssertNull(strstr(szErrors.Text(), "doesn't support"));
	AssertNotNull(strstr(szErrors.Text(), "The call is ambiguous"));
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();

	//The stub a name nothing defines is linked to is the W65C816's stp ($DB), not the 65C64's.
	cBuilder.Init();
	cBuilder.SetTarget(CPPT_W65C816);
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	AssertTrue(cBuilder.AddSource("Test.cpp", "void Missing(void);\nint main(void) { Missing(); return 5; }\n"));
	AssertTrue(cBuilder.Link());
	AssertSize(1, cBuilder.NumMissing());
	AssertTrue(cBuilder.GetSymbol("_Missing", &lliAddress));
	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(cBuilder.GetOutput());
	AssertInt(0xDB, cMemory.GetByte((uint32)lliAddress));
	cMemory.Kill();
	cBuilder.Kill();

	//TestIndexPtr's keys: ui * 0x39e7 multiplied in 16 bits, then widened.
	AssertInt(1, TestCPP816CountWarnings("long long gl;\nvoid F(unsigned short ui) { gl = ui * 0x39e7; }\n", "widened after it may have wrapped"));
	AssertInt(1, TestCPP816CountWarnings("void* Remap(void* pv, long l);\nvoid* F(void* pv, unsigned short ui) { return Remap(pv, ui << 3); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("long long gl;\nvoid F(unsigned short ui) { gl = (long long)ui * 0x39e7; }\n", "widened after it may have wrapped"));
	AssertInt(1, TestCPP816CountWarnings("long long gl;\nvoid F(unsigned short ui) { gl = (long long)(ui * 0x39e7); }\n", "widened after it may have wrapped"));
	//SupportLib's row blitters: y * uiRowStride + x * uiPixelStride made a 16 bit offset, then widened to a ptrdiff.
	AssertInt(1, TestCPP816CountWarnings("void* Remap(void* pv, long l);\nvoid* F(void* pv, unsigned short y, unsigned short s, unsigned short x) { return Remap(pv, y * s + x * 3); }\n", "widened after it may have wrapped"));
	AssertInt(1, TestCPP816CountWarnings("long gl;\nvoid F(unsigned short a, unsigned short b) { gl = (a << 4) | b; }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("void* Remap(void* pv, long l);\nvoid* F(void* pv, unsigned short y, unsigned long s, unsigned short x) { return Remap(pv, y * s + x); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("long gl;\nvoid F(unsigned short a, unsigned short b) { gl = a + b; }\n", "widened after it may have wrapped"));

	//TestIndexUnknown: the cast doesn't make the multiply wider.
	AssertInt(1, TestCPP816CountWarnings("void Init(long l);\nvoid F(unsigned short ui) { Init((long)((ui + 1) * 7)); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("void Init(long l);\nvoid F(unsigned short ui) { Init(((long)ui + 1) * 7); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("long F(void) { return (long)(4 * 7); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("unsigned short F(unsigned short a, unsigned short b) { return (unsigned short)(a * b); }\n", "widened after it may have wrapped"));
	AssertInt(0, TestCPP816CountWarnings("long gl;\nvoid F(unsigned short ui) { gl = ui + 1; }\n", "widened after it may have wrapped"));

	//CChars::PassifyNewlines: ::PassifyNewlines' 16 bit MAX_SIZE in a 32 bit lsize, compared with MAX_LSIZE.
	AssertInt(1, TestCPP816CountWarnings("unsigned short Find(void);\nint F(void) { unsigned long ul; ul = Find(); if (ul != 0xFFFFFFFFUL) { return 1; } return 0; }\n", "Comparison is always true: a 16 bit value, zero extended, is compared with 4294967295"));
	AssertInt(0, TestCPP816CountWarnings("unsigned long Find(void);\nint F(void) { unsigned long ul; ul = Find(); if (ul != 0xFFFFFFFFUL) { return 1; } return 0; }\n", "Comparison is always"));
	AssertInt(0, TestCPP816CountWarnings("unsigned short Find(void);\nint F(void) { unsigned long ul; ul = Find(); if (ul != 0xFFFFUL) { return 1; } return 0; }\n", "Comparison is always"));

	//TestSound's every 16 bit value loop: i <= 32767 with a 16 bit int never ends.
	AssertInt(1, TestCPP816CountWarnings("int F(void) { int i; int n; n = 0; for (i = -32768; i <= 32767; i++) { n++; } return n; }\n", "Comparison is always true: the constant 32767 is the largest value of the 16 bit signed"));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned short u) { return 65535 < u; }\n", "Comparison is always"));
	AssertInt(1, TestCPP816CountWarnings("int F(short s) { return s >= -32768; }\n", "Comparison is always true"));
	AssertInt(0, TestCPP816CountWarnings("int F(unsigned short u) { return u >= 0; }\n", "Comparison is always"));
	AssertInt(0, TestCPP816CountWarnings("int F(int i) { return i < 32767; }\n", "Comparison is always"));
	AssertInt(0, TestCPP816CountWarnings("int F(long l) { return l <= 32767; }\n", "Comparison is always"));

	//SupportLib's Convert8BitColourTo32BitColour: uint8s promoted to a 16 bit int and shifted by 16 and 24 are zero.
	AssertInt(2, TestCPP816CountWarnings("unsigned long F(unsigned char r, unsigned char b, unsigned char a) { return r | (b << 16) | (a << 24); }\n", "Shift count"));
	AssertInt(1, TestCPP816CountWarnings("int F(int i) { return i >> 16; }\n", "Shift count 16 is not less than the 16 bit width"));
	//SupportLib's Set32BitColour(r, g, b): a constant 255 << 24 is shifted as a 16 bit int too.
	AssertInt(1, TestCPP816CountWarnings("unsigned long F(unsigned long ul) { ul |= 255 << 24; return ul; }\n", "Shift count 24"));
	AssertInt(0, TestCPP816CountWarnings("unsigned long F(unsigned char b) { return ((unsigned long)b << 16) | (b << 15); }\n", "Shift count"));
	AssertInt(0, TestCPP816CountWarnings("unsigned long F(unsigned char b, int i) { return (unsigned long)b << i; }\n", "Shift count"));
}


//////////////////////////////////////////////////////////////////////////
//
// The profile counts a register function's calls at its register entry
// (name.ax, after its stack entry), not only at its section's start: called
// straight and through a pointer (its stack entry, which calls the
// register entry: one call, not two, and not recursion).  A recursive one
// is recursive.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ProfileRegisterEntries(void)
{
	static const char*	szSource =
		"int Twice(int a, int b) { return a * 2 + b; }\n"
		"int Down(int n) { return (n == 0) ? 0 : 1 + Down(n - 1); }\n"
		"int (*volatile gpTwice)(int, int) = Twice;\n"
		"int main(void)\n"
		"{\n"
		"\tint r = 0;\n"
		"\tfor (int i = 0; i < 5; i++) { r += Twice(i, r & 7); }\n"
		"\tfor (int i = 0; i < 3; i++) { r += gpTwice(i, 1); }\n"
		"\treturn r + Down(4);\n"
		"}\n";
	CAsmW65C816Profile			cProfile;
	SAsmW65C816FunctionProfile*	psTwice;
	SAsmW65C816FunctionProfile*	psDown;
	uint64						uiCycles;

	//Not inlined: the calls are what is counted.
	gcTestCPP816Options.mbInline = false;
	cProfile.Init();
	TestCPP816RunProfiled(szSource, NULL, &cProfile, &uiCycles);
	gcTestCPP816Options.mbInline = true;
	psTwice = cProfile.Get("_Twice__Fss");
	psDown = cProfile.Get("_Down__Fs");
	AssertNotNull((void*)psTwice);
	AssertNotNull((void*)psDown);
	AssertLong(8, psTwice ? (int64)psTwice->ulliCalls : 0);
	AssertFalse(psTwice ? psTwice->bRecursive : true);
	AssertLong(5, psDown ? (int64)psDown->ulliCalls : 0);
	AssertTrue(psDown ? psDown->bRecursive : false);
	cProfile.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// Builds szSource with aszUnits for the W65C816 keeping its listing (Test.lst, into pszListing) and runs it.
// Returns main's int (or A where it stopped); puiResults gets uiResults longs from TEST_CPP816_RESULTS.  The stack
// must be back at its top unless !bReturns (a program that stops inside a function: _Exit).  The names nothing linked
// defines (stubbed with stp) are left in gszTestCPP816Missing.
//
//////////////////////////////////////////////////////////////////////////
char	gszTestCPP816Missing[1024];


int32 TestCPP816RunListed(const char* szSource, const char** aszUnits, CChars* pszListing, uint32* puiResults, size uiResults, bool bReturns = true)
{
	CChars				szMissing;
	CCPP65Builder	cBuilder;
	CAsm65Memory	cMemory;
	CFastW65C816		cFast;
	CChars			szProject;
	CFileUtil		cFileUtil;
	CTextFile		cFile;
	size			i;
	int32			iResult;

	iResult = -1000000;
	cFileUtil.RemoveDir("Output" _FS_ "CPP816Listing");
	cBuilder.Init();
	*cBuilder.GetOptions() = gcTestCPP816Options;
	cBuilder.SetTarget(CPPT_W65C816);
	cBuilder.SetListingDirectory("Output" _FS_ "CPP816Listing");
	TestCPP65AddIncludeDirectories(&cBuilder);
	AssertTrue(cBuilder.AddStartUp());
	szProject.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
	cFileUtil.FullPath(&szProject);
	cBuilder.AddMathTables(szProject.Text());
	szProject.Kill();
	AssertTrue(cBuilder.AddSource("Test.cpp", szSource));
	TestCPP65AddUnits(&cBuilder, aszUnits, false);
	AssertTrue(cBuilder.Link());
	AssertString("", cBuilder.GetErrors());
	szMissing.Init();
	cBuilder.GetMissingNames(&szMissing);
	StrCpySafe(gszTestCPP816Missing, szMissing.Text(), sizeof(gszTestCPP816Missing));
	szMissing.Kill();

	cFile.Init();
	AssertTrue(cFile.Read("Output" _FS_ "CPP816Listing" _FS_ "Test.lst"));
	pszListing->Init(cFile.Text() ? cFile.Text() : "");
	cFile.Kill();

	cMemory.Init(ASM65_MEMORY_LOW_SIZE);
	cMemory.Load(cBuilder.GetOutput());
	cMemory.InitFast65816(&cFast);
	cFast.Reset();
	AssertTrue(TestCPP816RunChecked(&cMemory, &cFast, TEST_CPP816_SLOW_STEPS) < TEST_CPP816_SLOW_STEPS);
	AssertTrue(cFast.IsStopped());
	AssertInt(0, (int)cMemory.NumBadAccesses());
	if (cFast.IsStopped())
	{
		iResult = (int16)cFast.GetC();
		if (bReturns)
		{
			AssertIntHex(GetCPPTarget(CPPT_W65C816)->GetStackTop(), (uint32)cFast.GetS());
		}
	}
	for (i = 0; i < uiResults; i++)
	{
		puiResults[i] = cMemory.GetLong(TEST_CPP816_RESULTS + 4 * (uint32)i);
	}
	cFast.Kill();
	cMemory.Kill();
	cBuilder.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the listing calls a label starting szPrefix with the register
// suffix szSuffix ("" for none: the plain label).
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP816Calls(const char* szListing, const char* szPrefix, const char* szSuffix)
{
	CChars		szCall;
	const char*	szAt;
	const char*	szEnd;
	const char*	szDot;

	szCall.Init("\t\tjsl ");
	szCall.Append(szPrefix);
	for (szAt = strstr(szListing, szCall.Text()); szAt; szAt = strstr(szAt + 1, szCall.Text()))
	{
		szEnd = strchr(szAt, '\n');
		szDot = (const char*)memchr(szAt + 6, '.', (size)(szEnd - (szAt + 6)));
		if (szSuffix[0] ? (szDot && ((size)(szEnd - szDot) == strlen(szSuffix)) && (strncmp(szDot, szSuffix, strlen(szSuffix)) == 0)) : (szDot == NULL))
		{
			szCall.Kill();
			return true;
		}
	}
	szCall.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Register arguments (W65C816.txt): the fixed rule's suffixes (A, X and Y
// taken in order by arguments whose words all fit, a later smaller one
// still fitting, the hidden result pointer first), extern "C" functions too,
// a variadic function pushing everything, a member function, recursion,
// narrow arguments, and each called through a function pointer (the plain
// label's stub).  Each bit of the result is a call that went wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816RegisterArguments(void)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	static const char*	szSource =
		"#include <stdarg.h>\n"
		"extern \"C\" long CAdd(long a, int b) { return a + b; }\n"
		"int Three(int a, int b, int c) { return a * 100 + b * 10 + c; }\n"
		"int Four(int a, int b, int c, int d) { return a * 1000 + b * 100 + c * 10 + d; }\n"
		"long Mixed(long a, long b, int c) { return a * 3 + b * 5 + c; }\n"
		"long long Wide(long long a, int b) { return a * b + 1; }\n"
		"long long Twice(int a, long long b) { return b * 2 + a; }\n"
		"int Sum(int n, ...) { va_list v; va_start(v, n); int s = 0; for (int i = 0; i < n; i++) { s += va_arg(v, int); } va_end(v); return s; }\n"
		"struct SValue { int m; int Get(int k, long l) { return m + k + (int)(l / 1000); } };\n"
		"long Recurse(int n, long acc) { return (n == 0) ? acc : Recurse(n - 1, acc + n); }\n"
		"char Narrow(char c, unsigned char u, bool b) { return (char)(c + u + (b ? 1 : 0)); }\n"
		"long (*volatile gpCAdd)(long, int) = CAdd;\n"
		"int (*volatile gpThree)(int, int, int) = Three;\n"
		"int (*volatile gpFour)(int, int, int, int) = Four;\n"
		"long (*volatile gpMixed)(long, long, int) = Mixed;\n"
		"long long (*volatile gpWide)(long long, int) = Wide;\n"
		"long long (*volatile gpTwice)(int, long long) = Twice;\n"
		"long (*volatile gpRecurse)(int, long) = Recurse;\n"
		"char (*volatile gpNarrow)(char, unsigned char, bool) = Narrow;\n"
		"volatile int gi = 3;\n"
		"int main(void)\n"
		"{\n"
		"	int w = 0;\n"
		"	SValue v;\n"
		"	v.m = 5;\n"
		"	if (CAdd(100000L, gi) != 100003L) { w |= 1; }\n"
		"	if (gpCAdd(100000L, gi) != 100003L) { w |= 2; }\n"
		"	if (Three(1, 2, gi) != 123) { w |= 4; }\n"
		"	if (gpThree(1, 2, gi) != 123) { w |= 8; }\n"
		"	if (Four(1, 2, gi, 4) != 1234) { w |= 16; }\n"
		"	if (gpFour(1, 2, gi, 4) != 1234) { w |= 32; }\n"
		"	if (Mixed(70000L, 80000L, gi) != 610003L) { w |= 64; }\n"
		"	if (gpMixed(70000L, 80000L, gi) != 610003L) { w |= 128; }\n"
		"	if (Wide(5000000000LL, gi) != 15000000001LL) { w |= 256; }\n"
		"	if (gpWide(5000000000LL, gi) != 15000000001LL) { w |= 512; }\n"
		"	if (Twice(gi, 5000000000LL) != 10000000003LL) { w |= 1024; }\n"
		"	if (gpTwice(gi, 5000000000LL) != 10000000003LL) { w |= 2048; }\n"
		"	if (Sum(4, 1, 2, gi, 4) != 10) { w |= 4096; }\n"
		"	if (v.Get(gi, 70000L) != 78) { w |= 8192; }\n"
		"	if ((Recurse(100, gi) != 5053L) || (gpRecurse(100, gi) != 5053L)) { w |= 16384; }\n"
		"	if ((Narrow(-3, 200, true) != (char)198) || (gpNarrow(-3, 200, true) != (char)198)) { w = -1; }\n"
		"	return w;\n"
		"}\n";
	CChars	szListing;

	AssertInt(0, TestCPP816RunListed(szSource, aszUnits, &szListing, NULL, 0));

	//The definitions' register labels (the plain labels are the stubs the function pointers call).
	AssertTrue(szListing.Contains("\n_CAdd.axy:\n"));
	AssertTrue(szListing.Contains("\n_CAdd:\n"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_Recurse__", ".axy"));
	AssertTrue(szListing.Contains(".axy:\n"));
	AssertTrue(szListing.Contains(".axssy:\n"));
	AssertTrue(szListing.Contains(".axssssy:\n"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_Sum__", ""));
	AssertFalse(TestCPP816Calls(szListing.Text(), "_Sum__", ".a"));
	szListing.Kill();

	//Every argument pushed (CCPPCompileOptions::mbRegisterArguments false, TestProject65's TEST_PROJECT65_STACK_ARGUMENTS):
	//no register labels, and the assembly's are called at their plain labels (the stack entries).
	gcTestCPP816Options.mbRegisterArguments = false;
	AssertInt(0, TestCPP816RunListed(szSource, aszUnits, &szListing, NULL, 0));
	AssertFalse(szListing.Contains(".axy"));
	AssertFalse(szListing.Contains(".ax\n"));
	gcTestCPP816Options.mbRegisterArguments = true;
	szListing.Kill();
}


//The float program, compiled natively for the expected results and for the W65C816 (TestCPP816FrameOperandFloats):
//arithmetic, compares (a NaN's too), every integer conversion each way, float and double conversions, operands that
//are constants, a function with only its parameters (no frame of its own) and calls through function pointers.
#define TEST_CPP816_FLOAT_GLOBALS \
	volatile double gd[4] = { 1.5, -2.25, 1e300, 3.0 }; \
	volatile float gf[4] = { 1.5f, -2.25f, 3.0e38f, 0.1f }; \
	volatile signed char gc = -100; \
	volatile unsigned char guc = 200; \
	volatile short gs = -30000; \
	volatile unsigned short gus = 60000; \
	volatile short gi = -12345; \
	volatile long gl = -2000000000L; \
	volatile unsigned long gul = 4000000000UL; \
	volatile long long gll = -5000000000000LL; \
	volatile unsigned long long gull = 17000000000000000000ULL; \
	double Add(double a, double b) { return a + b; } \
	float AddF(float a, float b) { return a + b; } \
	double (*volatile gpAdd)(double, double) = Add; \
	float (*volatile gpAddF)(float, float) = AddF;

#define TEST_CPP816_FLOAT_BODY \
	double a = gd[0]; \
	double b = gd[1]; \
	double c = gd[2]; \
	float f = gf[0]; \
	float g = gf[1]; \
	float h = gf[3]; \
	double n; \
	long long q; \
	unsigned long long uq; \
	PutD(a + b); PutD(a - b); PutD(a * b); PutD(a / b); PutD(c * c); PutD(a * 2.5); PutD(1.0 / b); PutD(gpAdd(a, b)); PutD(Add(c, a)); \
	PutF(f + g); PutF(f - g); PutF(f * g); PutF(f / g); PutF(f * 2.5f); PutF(1.0f / h); PutF(gpAddF(f, h)); PutF(AddF(g, h)); \
	PutI(a < b); PutI(a <= b); PutI(a == a); PutI(a != b); PutI(a > b); PutI(a >= b); PutI(f < g); PutI(f == f); \
	n = c * c - c * c; \
	PutI(n == n); PutI(n != n); PutI(n < a); PutI(a >= n); \
	PutD((double)gc); PutD((double)guc); PutD((double)gs); PutD((double)gus); PutD((double)gi); PutD((double)gl); PutD((double)gul); PutD((double)gll); PutD((double)gull); \
	PutF((float)gc); PutF((float)guc); PutF((float)gs); PutF((float)gus); PutF((float)gi); PutF((float)gl); PutF((float)gul); PutF((float)gll); PutF((float)gull); \
	PutI((signed char)(b * 10.0)); PutI((unsigned char)(a * 100.0)); PutI((short)(b * 1000.0)); PutI((unsigned short)(a * 1000.0)); PutI((short)(b * 1000.0)); \
	PutI((long)(b * 1000000.0)); PutI((long)(unsigned long)(a * 1000000000.0)); \
	q = (long long)(b * 1e12); PutI((long)q); PutI((long)(q >> 32)); \
	uq = (unsigned long long)(a * 1e18); PutI((long)uq); PutI((long)(uq >> 32)); \
	PutI((short)(g * 10.0f)); PutI((long)(f * 1e6f)); PutI((long)(unsigned long)(f * 1e9f)); \
	q = (long long)(g * 1e12f); PutI((long)q); PutI((long)(q >> 32)); \
	PutF((float)a); PutF((float)c); PutD((double)f); PutD((double)h);

//The native results are IEEE's: the project's /fp:fast lets an optimised build fold n == n (a NaN) to true.
#pragma float_control(precise, on, push)
namespace TestCPP816FloatNative
{
	uint32	gauiResults[256];
	int		giResults;
	void PutD(double d) { uint64 u; memcpy(&u, &d, 8); gauiResults[giResults++] = (uint32)u; gauiResults[giResults++] = (uint32)(u >> 32); }
	void PutF(float x) { uint32 u; memcpy(&u, &x, 4); gauiResults[giResults++] = u; }
	void PutI(long l) { gauiResults[giResults++] = (uint32)l; }
	TEST_CPP816_FLOAT_GLOBALS
	void Run(void) { TEST_CPP816_FLOAT_BODY }
}
#pragma float_control(pop)

#define TEST_CPP816_STRINGIZE2(...)	#__VA_ARGS__
#define TEST_CPP816_STRINGIZE(...)	TEST_CPP816_STRINGIZE2(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
//
// The compiler's float instructions call softfloatW65C816.asm's frame operand
// entries (name.dp: the operands' places in X and Y, a 64 bit result's in
// A): every result the same, bit for bit, as natively.  A function whose
// only frame use is through those places keeps its frame (DropFrame).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816FrameOperandFloats(void)
{
	static const char*	aszUnits[] = { "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", "Libc65/math.cpp", "Libc65/stringW65C816.asm", NULL };
	CChars				szSource;
	CChars				szListing;
	uint32				auiResults[256];
	int					i;

	TestCPP816FloatNative::giResults = 0;
	TestCPP816FloatNative::Run();

	szSource.Init("#define __cdecl\n#include <string.h>\n");
	szSource.Append("unsigned long* gpuiResult = (unsigned long*)0x100000;\n");
	szSource.Append("void PutD(double d) { unsigned long long u; memcpy(&u, &d, 8); *gpuiResult++ = (unsigned long)u; *gpuiResult++ = (unsigned long)(u >> 32); }\n");
	szSource.Append("void PutF(float x) { unsigned long u; memcpy(&u, &x, 4); *gpuiResult++ = u; }\n");
	szSource.Append("void PutI(long l) { *gpuiResult++ = (unsigned long)l; }\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_FLOAT_GLOBALS));
	szSource.Append("\nint main(void)\n{\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_FLOAT_BODY));
	szSource.Append("\n\treturn (int)(gpuiResult - (unsigned long*)0x100000);\n}\n");

	AssertInt(TestCPP816FloatNative::giResults, TestCPP816RunListed(szSource.Text(), aszUnits, &szListing, auiResults, TestCPP816FloatNative::giResults));
	for (i = 0; i < TestCPP816FloatNative::giResults; i++)
	{
		AssertIntHex(TestCPP816FloatNative::gauiResults[i], auiResults[i]);
	}
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleAdd", ".dp"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftFloatAdd", ".dp"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleCompare", ".dp"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleFromLongLong", ".dp"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftFloatToInt", ".dp"));
	AssertFalse(TestCPP816Calls(szListing.Text(), "_SoftDoubleAdd", ""));
	szListing.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With softfloat.cpp compiled in place of softfloatW65C816.asm (CCPP65Generator
// ::SetAssemblyUnits(false)) the float instructions call its functions'
// register entries (CIRW65C816Backend::SoftFloatCall), not their stubs: every
// result the same, bit for bit, as natively.  _SoftDoubleCompare takes
// nothing in registers (its plain label), _SoftFloatFromInt's int would
// (it is pushed, extended to 32 bits, to the plain label).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816RegisterFloats(void)
{
	static const char*	aszUnits[] = { "Libc65/softfloat.cpp", "Libc65/math.cpp", "Libc65/stringW65C816.asm", NULL };
	CChars				szSource;
	CChars				szListing;
	uint32				auiResults[256];
	int					i;

	TestCPP816FloatNative::giResults = 0;
	TestCPP816FloatNative::Run();

	szSource.Init("#define __cdecl\n#include <string.h>\n");
	szSource.Append("unsigned long* gpuiResult = (unsigned long*)0x100000;\n");
	szSource.Append("void PutD(double d) { unsigned long long u; memcpy(&u, &d, 8); *gpuiResult++ = (unsigned long)u; *gpuiResult++ = (unsigned long)(u >> 32); }\n");
	szSource.Append("void PutF(float x) { unsigned long u; memcpy(&u, &x, 4); *gpuiResult++ = u; }\n");
	szSource.Append("void PutI(long l) { *gpuiResult++ = (unsigned long)l; }\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_FLOAT_GLOBALS));
	szSource.Append("\nint main(void)\n{\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_FLOAT_BODY));
	szSource.Append("\n\treturn (int)(gpuiResult - (unsigned long*)0x100000);\n}\n");

	gcTestCPP816Options.SetAssemblyUnits(false);
	szListing.Init();
	AssertInt(TestCPP816FloatNative::giResults, TestCPP816RunListed(szSource.Text(), aszUnits, &szListing, auiResults, TestCPP816FloatNative::giResults));
	gcTestCPP816Options.SetAssemblyUnits(true);
	for (i = 0; i < TestCPP816FloatNative::giResults; i++)
	{
		AssertIntHex(TestCPP816FloatNative::gauiResults[i], auiResults[i]);
	}
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleAdd", ".ax"));
	AssertFalse(TestCPP816Calls(szListing.Text(), "_SoftDoubleAdd", ""));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftFloatAdd", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftFloatCompare", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleCompare", ""));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftDoubleFromLongLong", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_SoftFloatToInt", ".ax"));
	AssertFalse(TestCPP816Calls(szListing.Text(), "_SoftDoubleAdd", ".dp"));
	szListing.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Functions with an object passed by value keep their arguments on the
// stack (CCPP65IRFrontEnd::MarkFunctionSymbol): its pieces stay together
// where the caller pushed them, the called function taking its address.
// One first, then arguments that would fit in A, X and Y (as BaseLib's
// PrivateAssertFile(CChars, CChars, const char*, size, const char*)), a 6
// byte one whose last piece would fit in Y, and a member function, called
// straight and through pointers.  A function without one still takes
// registers.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ObjectArguments(void)
{
	static const char*	aszNone[] = { NULL };
	static const char*	szSource =
		"struct SBig { long a; long b; int c; char* p; };\n"
		"struct SSix { long a; int b; };\n"
		"int Big(SBig s, SBig t, const char* sz, unsigned short n, const char* sz2) { return (int)(s.a + s.b * 2 + s.c * 3 + s.p[0] + t.a * 5 + t.c) + sz[1] * 7 + n + sz2[0]; }\n"
		"int Six(int i, SSix s) { return i * 100 + (int)s.a * 10 + s.b; }\n"
		"class CHolder { public: int miBase; int Add(SSix s, int i) { return miBase + (int)s.a + s.b + i; } };\n"
		"int Plain(int i, int j) { return i - j; }\n"
		"int (*volatile gpBig)(SBig, SBig, const char*, unsigned short, const char*) = Big;\n"
		"int (*volatile gpSix)(int, SSix) = Six;\n"
		"int main(void)\n"
		"{\n"
		"\tSBig s; SBig t; SSix x; CHolder c; int r;\n"
		"\ts.a = 1; s.b = 2; s.c = 3; s.p = (char*)\"A\"; t.a = 4; t.b = 0; t.c = 6; t.p = 0;\n"
		"\tx.a = 7; x.b = 8; c.miBase = 1000;\n"
		"\tr = Big(s, t, \"xy\", 9, \"B\") - gpBig(s, t, \"xy\", 9, \"B\");\n"
		"\tr += Six(3, x) - gpSix(3, x);\n"
		"\treturn r * 10000 + Big(s, t, \"xy\", 9, \"B\") + Six(3, x) + c.Add(x, 5) + Plain(9, 4);\n"
		"}\n";
	CChars				szListing;
	int					iBig;

	//1 + 4 + 9 + 65 + 20 + 6 + 121 * 7 + 9 + 66 = 1027; 300 + 70 + 8 = 378; 1000 + 7 + 8 + 5 = 1020; 5.
	iBig = 1 + 2 * 2 + 3 * 3 + 'A' + 4 * 5 + 6 + 'y' * 7 + 9 + 'B';
	//Not inlined: the calls are what is tested.
	gcTestCPP816Options.mbInline = false;
	AssertInt(iBig + 378 + 1020 + 5, TestCPP816RunListed(szSource, aszNone, &szListing, NULL, 0));
	gcTestCPP816Options.mbInline = true;
	AssertTrue(szListing.Contains("jsl _Big__FC4SBigC4SBigPKctPKc\n"));
	AssertTrue(szListing.Contains("jsl _Six__FsC4SSix\n"));
	AssertTrue(szListing.Contains("jsl _Plain__Fss.ax"));
	szListing.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// C's _Exit(int) stops at once with its argument in A: called straight
// (the start up's register entry, __Exit.a) and through a pointer (__Exit).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Exit(void)
{
	static const char*	aszNone[] = { NULL };
	CChars				szListing;

	AssertInt(37, TestCPP816RunListed("extern \"C\" void _Exit(int iStatus);\nint main(void) { _Exit(37); return 1; }\n", aszNone, &szListing, NULL, 0, false));
	AssertTrue(szListing.Contains("jsl __Exit.a"));
	AssertString("", gszTestCPP816Missing);
	szListing.Kill();
	AssertInt(41, TestCPP816RunListed("extern \"C\" void _Exit(int iStatus);\nvoid (*volatile gpExit)(int) = _Exit;\nint main(void) { gpExit(41); return 1; }\n", aszNone, &szListing, NULL, 0, false));
	AssertString("", gszTestCPP816Missing);
	szListing.Kill();
}

//The functions of TestCPP816AlignedBigFrameArguments, natively for the expected result and for the W65C816.
#define TEST_CPP816_BIG_FRAME_FUNCTIONS \
	long Spread(int n, long add, long more, int k, char c) { int a[180]; for (int i = 0; i < 180; i++) { a[i] = i * k + n + c; } long s = add - more; for (int i = 0; i < 180; i++) { s += a[i]; } return s; } \
	long Pushed(long one, long two, long three, int k) { int a[180]; for (int i = 0; i < 180; i++) { a[i] = (int)(i + one) ^ k; } long s = two * 3 - three; for (int i = 0; i < 180; i++) { s += a[i]; } return s; } \
	long Run(void) { long w = 0; for (int i = 0; i < 12; i++) { w += gpSpread(i, 1000L + i, 7L * i, 3 + i, (char)(i * 5)); w += gpPushed(i * 11L, 100000L - i, 9L, i); } return w; }

namespace TestCPP816AlignedBigFrameNative
{
	long Spread(int n, long add, long more, int k, char c);
	long Pushed(long one, long two, long three, int k);
	long (*volatile gpSpread)(int, long, long, int, char) = Spread;
	long (*volatile gpPushed)(long, long, long, int) = Pushed;
	TEST_CPP816_BIG_FRAME_FUNCTIONS
}


//////////////////////////////////////////////////////////////////////////
//
// A frame past the page with register arguments and pushed ones gets D on a
// page: the pushed arguments, then (below the link bytes) the register words
// at the frame's top, are pushed into place (CIRW65C816Backend::AlignFrame).
// Spread takes A, X and Y and has two words pushed; Pushed has only its
// first argument in registers.  The results are as natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816AlignedBigFrameArguments(void)
{
	static const char*	aszPrefixes[] = { "_Spread", "_Pushed", NULL };
	CChars				szSource;
	CAsmW65C816Profile	cProfile;
	uint64				uiCycles;
	bool				abAligned[2];
	long				lExpected;

	lExpected = TestCPP816AlignedBigFrameNative::Run();
	szSource.Init("long Spread(int n, long add, long more, int k, char c);\nlong Pushed(long one, long two, long three, int k);\n");
	szSource.Append("long (*volatile gpSpread)(int, long, long, int, char) = Spread;\nlong (*volatile gpPushed)(long, long, long, int) = Pushed;\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_BIG_FRAME_FUNCTIONS));
	szSource.Append("\nint main() { long w = Run(); return (int)((w ^ (w >> 16)) & 0x7FFF); }\n");

	cProfile.Init();
	AssertInt((int)((lExpected ^ (lExpected >> 16)) & 0x7FFF), TestCPP816RunFrames(szSource.Text(), NULL, &cProfile, &uiCycles, aszPrefixes, abAligned));
	AssertTrue(abAligned[0]);
	AssertTrue(abAligned[1]);
	cProfile.Kill();
	szSource.Kill();
}

//The C string functions (stringW65C816.asm's register entries, and through function pointers their stack entries),
//compiled natively for the expected results and for the W65C816 (TestCPP816StringRegisters).
#define TEST_CPP816_STRING_GLOBALS \
	volatile int gi = 'w'; \
	void* (__cdecl *volatile gpMemcpy)(void*, const void*, size_t) = memcpy; \
	void* (__cdecl *volatile gpMemmove)(void*, const void*, size_t) = memmove; \
	void* (__cdecl *volatile gpMemset)(void*, int, size_t) = memset; \
	int (__cdecl *volatile gpMemcmp)(const void*, const void*, size_t) = memcmp; \
	void* (__cdecl *volatile gpMemchr)(const void*, int, size_t) = TestMemchr; \
	size_t (__cdecl *volatile gpStrlen)(const char*) = strlen; \
	char* (__cdecl *volatile gpStrcpy)(char*, const char*) = strcpy; \
	char* (__cdecl *volatile gpStrncpy)(char*, const char*, size_t) = strncpy; \
	char* (__cdecl *volatile gpStrcat)(char*, const char*) = strcat; \
	char* (__cdecl *volatile gpStrncat)(char*, const char*, size_t) = strncat; \
	int (__cdecl *volatile gpStrcmp)(const char*, const char*) = strcmp; \
	int (__cdecl *volatile gpStrncmp)(const char*, const char*, size_t) = strncmp; \
	char* (__cdecl *volatile gpStrchr)(const char*, int) = TestStrchr; \
	char* (__cdecl *volatile gpStrrchr)(const char*, int) = TestStrrchr;

#define TEST_CPP816_STRING_BODY \
	char a[64]; \
	char b[64]; \
	char c[16]; \
	int i; \
	for (i = 0; i < 64; i++) { a[i] = (char)('a' + i % 26); b[i] = (char)('A' + i % 26); } \
	a[40] = 0; \
	b[30] = 0; \
	strcpy(c, "abcabc"); \
	PutI((long)strlen(a)); PutI((long)gpStrlen(b)); PutI((long)strlen("")); \
	PutI((long)((char*)memcpy(a + 1, b + 2, 10) - a)); PutS(a); gpMemcpy(a + 3, b, 5); PutS(a); \
	memmove(a + 2, a, 20); PutS(a); memmove(a, a + 5, 20); PutS(a); gpMemmove(a + 1, a, 7); PutS(a); \
	PutI((long)((char*)memset(a + 4, 'z', 9) - a)); PutS(a); gpMemset(a + 10, gi, 3); PutS(a); memset(a, 'q', 0); PutS(a); \
	PutI(memcmp(a, b, 5) < 0); PutI(gpMemcmp(a, a, 30)); PutI(memcmp(b, a, 5) > 0); PutI(memcmp(a, b, 0)); PutI(memcmp(a + 1, b + 1, 1) != 0); \
	PutI((long)((char*)memchr(a, 'z', 30) - a)); PutI(gpMemchr(a, '#', 30) == 0); PutI((long)((char*)gpMemchr(a, gi, 30) - a)); \
	PutI((long)(strcpy(b + 40, "hello") - b)); PutS(b + 40); gpStrcpy(b + 50, "abc"); PutS(b + 50); \
	strncpy(b + 40, "xy", 5); PutI(b[44]); PutS(b + 40); gpStrncpy(b + 40, "longer", 3); PutS(b + 40); \
	strcat(b + 40, "end"); PutS(b + 40); gpStrcat(b + 40, "!"); PutS(b + 40); strncat(b + 40, "12345", 2); PutS(b + 40); gpStrncat(b + 40, "9", 5); PutS(b + 40); \
	PutI(strcmp("abc", "abd") < 0); PutI(gpStrcmp("abc", "abc")); PutI(strcmp("b", "a") > 0); PutI(strncmp("abcx", "abcy", 3)); PutI(gpStrncmp("abcx", "abcy", 4) < 0); \
	PutI((long)(strchr(a, 'z') - a)); PutI(gpStrchr(a, '@') == 0); PutI((long)(strchr(a, 0) - a)); \
	PutI((long)(strrchr(c, 'b') - c)); PutI((long)(gpStrrchr(c, 'a') - c)); PutI(strrchr(c, 'x') == 0);

namespace TestCPP816StringNative
{
	void* __cdecl TestMemchr(const void* pv, int c, size_t n) { return (void*)memchr(pv, c, n); }
	char* __cdecl TestStrchr(const char* sz, int c) { return (char*)strchr(sz, c); }
	char* __cdecl TestStrrchr(const char* sz, int c) { return (char*)strrchr(sz, c); }
	uint32	gauiResults[256];
	int		giResults;
	void PutI(long l) { gauiResults[giResults++] = (uint32)l; }
	void PutS(const char* sz) { uint32 h = 0; while (*sz) { h = h * 31 + (uint8)*sz++; } gauiResults[giResults++] = h; }
	TEST_CPP816_STRING_GLOBALS
	void Run(void) { TEST_CPP816_STRING_BODY }
}


//////////////////////////////////////////////////////////////////////////
//
// The C string functions in stringW65C816.asm and stringW65C816_16.asm with C
// linkage taking register arguments: each called straight (its register
// entry, name.ax or name.axy) and through a function pointer (its stack
// entry, which loads the registers), the same as natively.  Then the 16 bit
// count ones, memset's and memcpy's moves across a bank's end, and BaseLib's
// MemSwp16 and bit array finders (register entries and stack entries),
// checked on the W65C816 against plain loops.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816StringRegisters(void)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", "Libc65/stringW65C816_16.asm", NULL };
	static const char*	szSixteen =
		"#include <string.h>\n"
		"void MemSwp16(const void* pv1, const void* pv2, unsigned short uiLength);\n"
		"unsigned short FindFirstSetBit(void* pvArray, unsigned short iArraySize);\n"
		"unsigned short FindFirstClearBit(void* pvArray, unsigned short iArraySize);\n"
		"unsigned short FindLastSetBit(void* pvArray, unsigned short iArraySize);\n"
		"unsigned short FindLastClearBit(void* pvArray, unsigned short iArraySize);\n"
		"unsigned long FindFirstSetBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned long FindLastClearBitLarge(void* pvArray, unsigned long iArraySize);\n"
		"unsigned short FindFirstClearBit(unsigned long uiBits);\n"
		"void* (*volatile gpMemcpy16)(void*, const void*, unsigned short) = memcpy16;\n"
		"void* (*volatile gpMemmove16)(void*, const void*, unsigned short) = memmove16;\n"
		"void* (*volatile gpMemset16)(void*, int, unsigned short) = memset16;\n"
		"int (*volatile gpMemcmp16)(const void*, const void*, unsigned short) = memcmp16;\n"
		"unsigned short (*volatile gpStrlen16)(const char*) = strlen16;\n"
		"void (*volatile gpMemSwp16)(const void*, const void*, unsigned short) = MemSwp16;\n"
		"unsigned short (*volatile gpFindFirstSetBit)(void*, unsigned short) = FindFirstSetBit;\n"
		"unsigned short (*volatile gpFindLastClearBit)(void*, unsigned short) = FindLastClearBit;\n"
		"unsigned long (*volatile gpFindFirstSetBitLarge)(void*, unsigned long) = FindFirstSetBitLarge;\n"
		"unsigned long (*volatile gpFindLastClearBitLarge)(void*, unsigned long) = FindLastClearBitLarge;\n"
		"unsigned short (*volatile gpFindFirstClearBit32)(unsigned long) = FindFirstClearBit;\n"
		"unsigned char gauc[200];\n"
		"unsigned char gauc2[200];\n"
		"int Same(const unsigned char* p, const unsigned char* q, int n) { for (int i = 0; i < n; i++) { if (p[i] != q[i]) { return 0; } } return 1; }\n"
		"int main(void)\n"
		"{\n"
		"	int w = 0;\n"
		"	int i;\n"
		"	unsigned char* pBank = (unsigned char*)0x10FFF0;\n"
		"	for (i = 0; i < 200; i++) { gauc[i] = (unsigned char)(i * 7 + 1); gauc2[i] = 0; }\n"
		"	if (memcpy16(gauc2, gauc, 50) != gauc2 || !Same(gauc2, gauc, 50) || gauc2[50] != 0) { w |= 1; }\n"
		"	if (gpMemcpy16(gauc2 + 60, gauc, 33) != gauc2 + 60 || !Same(gauc2 + 60, gauc, 33)) { w |= 2; }\n"
		"	memmove16(gauc2 + 1, gauc2, 40);\n"
		"	if (!Same(gauc2 + 1, gauc, 40)) { w |= 4; }\n"
		"	gpMemmove16(gauc2, gauc2 + 1, 40);\n"
		"	if (!Same(gauc2, gauc, 40)) { w |= 8; }\n"
		"	if (memset16(gauc2, 0x5A, 21) != gauc2 || gauc2[20] != 0x5A || gauc2[21] != gauc[21]) { w |= 16; }\n"
		"	gpMemset16(gauc2, 0xA5, 3);\n"
		"	if (gauc2[2] != 0xA5 || gauc2[3] != 0x5A) { w |= 32; }\n"
		"	if (memcmp16(gauc, gauc, 100) != 0 || gpMemcmp16(gauc2, gauc, 5) <= 0 || memcmp16(gauc, gauc2, 5) >= 0) { w |= 64; }\n"
		"	if (strlen16(\"hello\") != 5 || gpStrlen16(\"\") != 0) { w |= 128; }\n"
		"	memcpy(gauc2, gauc, 200);\n"
		"	MemSwp16(gauc, gauc2 + 100, 31);\n"
		"	gpMemSwp16(gauc, gauc2 + 100, 31);\n"
		"	if (!Same(gauc, gauc2, 200)) { w |= 256; }\n"
		"	memset(pBank, 7, 32);\n"
		"	for (i = 0; i < 32; i++) { if (pBank[i] != 7) { w |= 512; } }\n"
		"	memcpy(pBank, gauc, 32);\n"
		"	if (!Same(pBank, gauc, 32)) { w |= 1024; }\n"
		"	memset(gauc2, 0, 40);\n"
		"	gauc2[13] = 0x10;\n"
		"	gauc2[17] = 0x81;\n"
		"	if (FindFirstSetBit(gauc2, 300) != 13 * 8 + 4 || gpFindFirstSetBit(gauc2, 300) != 13 * 8 + 4 || FindLastSetBit(gauc2, 300) != 17 * 8 + 7) { w |= 2048; }\n"
		"	if (FindFirstSetBitLarge(gauc2, 300) != 13 * 8 + 4 || gpFindFirstSetBitLarge(gauc2, 300) != 13 * 8 + 4) { w |= 4096; }\n"
		"	memset(gauc2, 0xFF, 40);\n"
		"	gauc2[5] = 0xEF;\n"
		"	if (FindFirstClearBit(gauc2, 300) != 5 * 8 + 4 || FindLastClearBit(gauc2, 300) != 5 * 8 + 4 || gpFindLastClearBit(gauc2, 300) != 5 * 8 + 4) { w |= 8192; }\n"
		"	if (FindLastClearBitLarge(gauc2, 300) != 5 * 8 + 4 || gpFindLastClearBitLarge(gauc2, 300) != 5 * 8 + 4) { w |= 16384; }\n"
		"	if (FindFirstClearBit(0xFFFEFFFFUL) != 16 || gpFindFirstClearBit32(0xFFFFFFFFUL) != 32) { w = -1; }\n"
		"	return w;\n"
		"}\n";
	CChars				szSource;
	CChars				szListing;
	uint32				auiResults[256];
	int					i;

	TestCPP816StringNative::giResults = 0;
	TestCPP816StringNative::Run();

	szSource.Init("#define __cdecl\n#define TestMemchr memchr\n#define TestStrchr strchr\n#define TestStrrchr strrchr\n#include <string.h>\n");
	szSource.Append("unsigned long* gpuiResult = (unsigned long*)0x100000;\n");
	szSource.Append("void PutI(long l) { *gpuiResult++ = (unsigned long)l; }\n");
	szSource.Append("void PutS(const char* sz) { unsigned long h = 0; while (*sz) { h = h * 31 + (unsigned char)*sz++; } *gpuiResult++ = h; }\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_STRING_GLOBALS));
	szSource.Append("\nint main(void)\n{\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_STRING_BODY));
	szSource.Append("\n\treturn (int)(gpuiResult - (unsigned long*)0x100000);\n}\n");

	AssertInt(TestCPP816StringNative::giResults, TestCPP816RunListed(szSource.Text(), aszUnits, &szListing, auiResults, TestCPP816StringNative::giResults));
	for (i = 0; i < TestCPP816StringNative::giResults; i++)
	{
		AssertIntHex(TestCPP816StringNative::gauiResults[i], auiResults[i]);
	}
	AssertTrue(TestCPP816Calls(szListing.Text(), "_strlen", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_strchr", ".axy"));
	szListing.Kill();
	szSource.Kill();

	AssertInt(0, TestCPP816RunListed(szSixteen, aszUnits, &szListing, NULL, 0));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_memcpy16", ".axssy"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_memset16", ".axy"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_FindFirstSetBit__", ".axy"));
	szListing.Kill();
}


//The math functions softfloatW65C816.asm has (register entries; through function pointers, the stack entries), compiled
//natively for the expected results and for the W65C816 (TestCPP816MathRegisters).
#define TEST_CPP816_MATH_GLOBALS \
	volatile double gd[4] = { 2.5, -7.75, 1e20, 0.3 }; \
	volatile float gf[3] = { 2.5f, -7.75f, 0.3f }; \
	double (__cdecl *volatile gpFabs)(double) = fabs; \
	float (__cdecl *volatile gpFabsf)(float) = fabsf; \
	double (__cdecl *volatile gpFloor)(double) = floor; \
	double (__cdecl *volatile gpRound)(double) = round; \
	float (__cdecl *volatile gpCeilf)(float) = ceilf; \
	double (__cdecl *volatile gpFmod)(double, double) = fmod; \
	double (__cdecl *volatile gpLdexp)(double, int) = ldexp; \
	double (__cdecl *volatile gpSqrt)(double) = sqrt; \
	float (__cdecl *volatile gpSqrtf)(float) = sqrtf;

#define TEST_CPP816_MATH_BODY \
	double a = gd[0]; \
	double b = gd[1]; \
	double c = gd[2]; \
	double d = gd[3]; \
	float f = gf[0]; \
	float g = gf[1]; \
	float h = gf[2]; \
	int e = 0; \
	double ip = 0; \
	PutD(fabs(b)); PutD(gpFabs(b)); PutF(fabsf(g)); PutF(gpFabsf(g)); \
	PutD(floor(b)); PutD(gpFloor(a)); PutD(ceil(b)); PutD(trunc(b)); PutD(round(a)); PutD(gpRound(b)); \
	PutF(floorf(g)); PutF(ceilf(f)); PutF(gpCeilf(g)); PutF(truncf(g)); PutF(roundf(f)); \
	PutD(fmod(c, d)); PutD(gpFmod(b, a)); PutF(fmodf(g, f)); \
	PutD(frexp(c, &e)); PutI(e); PutD(ldexp(a, 10)); PutD(gpLdexp(b, -3)); PutD(modf(b, &ip)); PutD(ip); \
	PutD(sqrt(a)); PutD(gpSqrt(c)); PutF(sqrtf(h)); PutF(gpSqrtf(f));

namespace TestCPP816MathNative
{
	uint32	gauiResults[256];
	int		giResults;
	void PutD(double x) { uint64 u; memcpy(&u, &x, 8); gauiResults[giResults++] = (uint32)u; gauiResults[giResults++] = (uint32)(u >> 32); }
	void PutF(float x) { uint32 u; memcpy(&u, &x, 4); gauiResults[giResults++] = u; }
	void PutI(long l) { gauiResults[giResults++] = (uint32)l; }
	TEST_CPP816_MATH_GLOBALS
	void Run(void) { TEST_CPP816_MATH_BODY }
}


//////////////////////////////////////////////////////////////////////////
//
// The math functions in softfloatW65C816.asm, with C linkage taking register
// arguments (the result pointer in A and X): straight and through function
// pointers (their stack entries), the same as natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816MathRegisters(void)
{
	static const char*	aszUnits[] = { "Libc65/softfloat.cpp", "Libc65/softfloatW65C816.asm", "Libc65/math.cpp", "Libc65/stringW65C816.asm", NULL };
	CChars				szSource;
	CChars				szListing;
	uint32				auiResults[256];
	int					i;

	TestCPP816MathNative::giResults = 0;
	TestCPP816MathNative::Run();

	szSource.Init("#define __cdecl\n#include <string.h>\n#include <math.h>\n");
	szSource.Append("unsigned long* gpuiResult = (unsigned long*)0x100000;\n");
	szSource.Append("void PutD(double d) { unsigned long long u; memcpy(&u, &d, 8); *gpuiResult++ = (unsigned long)u; *gpuiResult++ = (unsigned long)(u >> 32); }\n");
	szSource.Append("void PutF(float x) { unsigned long u; memcpy(&u, &x, 4); *gpuiResult++ = u; }\n");
	szSource.Append("void PutI(long l) { *gpuiResult++ = (unsigned long)l; }\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_MATH_GLOBALS));
	szSource.Append("\nint main(void)\n{\n");
	szSource.Append(TEST_CPP816_STRINGIZE(TEST_CPP816_MATH_BODY));
	szSource.Append("\n\treturn (int)(gpuiResult - (unsigned long*)0x100000);\n}\n");

	AssertInt(TestCPP816MathNative::giResults, TestCPP816RunListed(szSource.Text(), aszUnits, &szListing, auiResults, TestCPP816MathNative::giResults));
	for (i = 0; i < TestCPP816MathNative::giResults; i++)
	{
		AssertIntHex(TestCPP816MathNative::gauiResults[i], auiResults[i]);
	}
	AssertTrue(TestCPP816Calls(szListing.Text(), "_fabs", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_sqrt", ".ax"));
	AssertTrue(TestCPP816Calls(szListing.Text(), "_ldexp", ".axssssy"));
	szListing.Kill();
	szSource.Kill();
}



//A unit's functions only it calls, too big to inline, with constant arguments (TestCPP816ConstantArguments): Work
//with two counts (a copy each), Once always with the same (its parameter the constant) and Taken, whose address is
//taken (left as it is).
#define TEST_CPP816_CONSTANT_ARGUMENTS_STEP(n) \
	r ^= (x << iShift) + (x >> (iShift + n)); \
	x = x * (n + 3) + (r >> (64 - iShift)); \
	r += (x >> (iShift - 1)) ^ (unsigned long long)n;

#define TEST_CPP816_CONSTANT_ARGUMENTS_FUNCTIONS \
	static unsigned long long Work(unsigned long long x, int iShift) \
	{ \
		unsigned long long r = 0; \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(1) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(2) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(3) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(4) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(5) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(6) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(7) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(8) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(9) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(10) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(11) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(12) \
		return r ^ x; \
	} \
	static unsigned long long Once(unsigned long long x, int iShift) \
	{ \
		unsigned long long r = 0; \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(1) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(2) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(3) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(4) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(5) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(6) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(7) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(8) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(9) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(10) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(11) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(12) \
		return r + x; \
	} \
	static unsigned long long Taken(unsigned long long x, int iShift) \
	{ \
		unsigned long long r = 0; \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(1) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(2) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(3) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(4) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(5) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(6) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(7) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(8) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(9) \
		TEST_CPP816_CONSTANT_ARGUMENTS_STEP(10) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(11) TEST_CPP816_CONSTANT_ARGUMENTS_STEP(12) \
		return r - x; \
	} \
	unsigned long long (*volatile gpTaken)(unsigned long long, int) = Taken; \
	volatile unsigned long long gx = 0x9ABCDEF123456789ULL; \
	unsigned long long Run(void) \
	{ \
		return Work(gx, 12) ^ (Work(gx + 1, 40) << 1) ^ (Once(gx, 23) << 2) ^ (Once(gx + 5, 23) << 3) ^ (Taken(gx, 7) << 4) ^ (gpTaken(gx, 9) << 5); \
	}

namespace TestCPP816ConstantArgumentsNative
{
	TEST_CPP816_CONSTANT_ARGUMENTS_FUNCTIONS
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the listing defines a label starting szStart (a newline then
// the name's start) that isn't a specialised copy (no __K in it).
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP816Within(const char* szStart, const char* szEnd, const char* szFind);


bool TestCPP816DefinesUnspecialised(const char* szListing, const char* szStart)
{
	const char*	szAt;
	const char*	szEnd;
	const char*	szColon;

	for (szAt = strstr(szListing, szStart); szAt; szAt = strstr(szAt + 1, szStart))
	{
		szEnd = strchr(szAt + 1, '\n');
		szColon = szEnd ? szEnd - 1 : NULL;
		if (szColon && (*szColon == ':'))
		{
			if (!TestCPP816Within(szAt, szEnd, "__K"))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szFind is in the text from szStart to szEnd.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP816Within(const char* szStart, const char* szEnd, const char* szFind)
{
	size	uiFind;

	uiFind = strlen(szFind);
	for (; szStart + uiFind <= szEnd; szStart++)
	{
		if (strncmp(szStart, szFind, uiFind) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Interprocedural constant propagation (CIRInterprocedural::
// SpecialiseConstantArguments): a function only its unit calls, with a
// parameter that counts (a shift's count) a constant at every call, gets a
// copy for each value (name__K<parameter>_<value>) and goes; with one
// value its parameter is the constant; one whose address is taken is left.
// The results are the same as natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816ConstantArguments(void)
{
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	CChars				szSource;
	CChars				szListing;
	uint64				ulliExpected;

	ulliExpected = TestCPP816ConstantArgumentsNative::Run();
	szSource.Init(TEST_CPP816_STRINGIZE(TEST_CPP816_CONSTANT_ARGUMENTS_FUNCTIONS));
	szSource.Append("\nint main(void)\n{\n\tunsigned long long r = Run();\n\treturn (int)((r ^ (r >> 16) ^ (r >> 32) ^ (r >> 48)) & 0x7FFF);\n}\n");
	AssertInt((int)((ulliExpected ^ (ulliExpected >> 16) ^ (ulliExpected >> 32) ^ (ulliExpected >> 48)) & 0x7FFF), TestCPP816RunListed(szSource.Text(), aszUnits, &szListing, NULL, 0));

	//Work's two copies, its own code gone; Once's parameter the constant (no copy); Taken as it is.
	AssertTrue(szListing.Contains("__K1_12:"));
	AssertTrue(szListing.Contains("__K1_40:"));
	AssertFalse(TestCPP816DefinesUnspecialised(szListing.Text(), "\n_Work"));
	AssertFalse(szListing.Contains("__K1_23"));
	AssertTrue(TestCPP816DefinesUnspecialised(szListing.Text(), "\n_Once"));
	AssertTrue(szListing.Contains("jsl _Once"));
	AssertFalse(szListing.Contains("__K1_7"));
	AssertFalse(szListing.Contains("__K1_9"));
	szListing.Kill();
	szSource.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// 64 bit multiplies: of 32 bit halves (R816_UMulWide32, as softfloat's
// Multiply has them) and of any two (R816_Mul64), hashed, the same as
// natively.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816WideMultiplies(void)
{
	static const char*	szSource =
		"volatile unsigned long long gx = 0x9ABCDEF123456789ULL;\n"
		"volatile unsigned long long gy = 0xFEDCBA9876543210ULL;\n"
		"int main(void)\n"
		"{\n"
		"	unsigned long long x = gx;\n"
		"	unsigned long long y = gy;\n"
		"	unsigned long long h = 0;\n"
		"	for (int i = 0; i < 40; i++)\n"
		"	{\n"
		"		h = h * 33 + (x & 0xFFFFFFFF) * (y >> 32) + (x >> 32) * (y & 0xFFFFFFFF) + x * y;\n"
		"		x = x * 6364136223846793005ULL + 1442695040888963407ULL;\n"
		"		y = (y << 7) ^ (y >> 3) ^ x;\n"
		"	}\n"
		"	return (int)((h ^ (h >> 16) ^ (h >> 32) ^ (h >> 48)) & 0x7FFF);\n"
		"}\n";
	static const char*	aszUnits[] = { "Libc65/stringW65C816.asm", NULL };
	uint64	x;
	uint64	y;
	uint64	h;
	int		i;
	CChars	szListing;

	x = 0x9ABCDEF123456789ULL;
	y = 0xFEDCBA9876543210ULL;
	h = 0;
	for (i = 0; i < 40; i++)
	{
		h = h * 33 + (x & 0xFFFFFFFF) * (y >> 32) + (x >> 32) * (y & 0xFFFFFFFF) + x * y;
		x = x * 6364136223846793005ULL + 1442695040888963407ULL;
		y = (y << 7) ^ (y >> 3) ^ x;
	}
	AssertInt((int)((h ^ (h >> 16) ^ (h >> 32) ^ (h >> 48)) & 0x7FFF), TestCPP816RunListed(szSource, aszUnits, &szListing, NULL, 0));
	AssertTrue(szListing.Contains("jsl R816_UMulWide32"));
	AssertTrue(szListing.Contains("jsl R816_Mul64"));
	szListing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Namespaces run on the W65C816 too: qualified names, overloads, templates,
// argument-dependent lookup, operators and friends, and the NamespaceProject's
// units linked.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816Namespaces(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/NamespaceProject/Shapes.cpp", "Input/CPP65/NamespaceProject/Other.cpp", "Input/CPP65/NamespaceProject/Main.cpp", NULL };

	AssertInt(21, TestCPP816Run("namespace A { int F(int a) { return a + 10; } }\nnamespace B { int F(int a) { return a + 8; } int G() { return F(1); } }\nint F(int a) { return a; }\nint main() { return A::F(1) + B::G() + ::F(1); }\n"));
	AssertInt(13, TestCPP816Run("namespace N { template<class T> struct Box { T t; T Get() { return t; } }; template<class T> T Add(T a, T b) { return a + b; } }\nint main() { N::Box<int> b; b.t = 6; return b.Get() + N::Add<int>(3, 4); }\n"));
	AssertInt(1, TestCPP816Run("namespace N { struct S { int i; }; int Get(S s) { return s.i; } }\nint main() { N::S s; s.i = 1; return Get(s); }\n"));
	AssertInt(13, TestCPP816Run("namespace A { struct S { int i; }; int operator+(S a, int b) { return a.i + b; } }\nnamespace B { struct S { int i; }; int operator+(S a, int b) { return a.i - b; } }\nint main() { A::S a; B::S b; a.i = 10; b.i = 5; return (a + 1) + (b + 3); }\n"));
	AssertInt(10, TestCPP816Run("namespace N { struct S { int i; friend int Peek(S s) { return s.i; } }; }\nint main() { N::S s; s.i = 10; return Peek(s); }\n"));
	AssertInt(7, TestCPP816Run("namespace N { int x = 3; }\nnamespace { int x = 4; }\nint main() { return N::x + x; }\n"));
	AssertInt(9 + 4 + 3 + 10 + 7 + 11 + 2001 + 6 + 500 + 2, TestCPP816RunProject(aszUnits, "", NULL));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP816(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();
	CW65C816InstructionFactory::GetInstance()->Init();

	TestCPP816Basics();
	TestCPP816SignedToFloat();
	TestCPP816Functions();
	TestCPP816Classes();
	TestCPP816DelegatingConstructors();
	TestCPP816BitFields();
	TestCPP816Thunks();
	TestCPP816OverloadedComma();
	TestCPP816WideStrings();
	TestCPP816NestedConstants();
	TestCPP816SharedTables();
	TestCPP816Statements();
	TestCPP816NarrowSubscripts();
	TestCPP816GuardedIndices();
	TestCPP816ZeroWords();
	TestCPP816PointerCallsAndOrder();
	TestCPP816CounterDivisions();
	TestCPP816ByteMasksAndSignedCompares();
	TestCPP816SharedDivisions();
	TestCPP816Divide64();
	TestCPP816BitTables();
	TestCPP816CountZeros();
	TestCPP816BitTable64();
	TestCPP816ShiftCounts();
	TestCPP816IndexedWords();
	TestCPP816SmallShifts();
	TestCPP816ScratchOperands();
	TestCPP816PointerPlaceLoads();
	TestCPP816NarrowDivisors();
	TestCPP816WordShifts();
	TestCPP816Globals();
	TestCPP816Builtins();
	TestCPP816MemSwp();
	TestCPP816LoopIdioms();
	TestCPP816BaseLib();
	TestCPP816CompiledAgain();
	TestCPP816Sprites();
	TestCPP816TideVideo();
	TestCPP816Restrict();
	TestCPP816RestrictParameter();
	TestCPP816LocalSymbols();
	TestCPP816ExitValues();
	TestCPP816MovedCode();
	TestCPP816CompileBaseLibUnits();
	TestCPP816NewDelete();
	TestCPP816Floats();
	TestCPP816NoFloats();
	TestCPP816TestBaseLibBugs();
	TestCPP816ConstantShifts();
	TestCPP816VariableShifts();
	TestCPP816Warnings();
	TestCPP816RangeWarnings();
	TestCPP816LoopCounterWarnings();
	TestCPP816SizeLsizeBugs();
	TestCPP816DestructorCalls();
	TestCPP816DynamicCast();
	TestCPP816DynamicCastUnits();
	TestCPP816OverloadedFunctionPointers();
	TestCPP816MemberTemplateCallsPlainOverload();
	TestCPP816VariadicObjectParameters();
	TestCPP816ProfileGuidedInlining();
	TestCPP816ProfileRegisterEntries();
	TestCPP816ProfileGuidedDivisionTables();
	TestCPP816BiasedIndices();
	TestCPP816BCDBuiltins();
	TestCPP816ShiftedWordIndices();
	TestCPP816TypeTraits();
	TestCPP816MergedAddressPhis();
	TestCPP816InlineObjectParameters();
	TestCPP816ConstantDivisions();
	TestCPP816DivisionRanges();
	TestCPP816WideDivisors();
	TestCPP816ScaledIndices();
	TestCPP816BiasedIndicesInOneBlock();
	TestCPP816BoundedRemainders();
	TestCPP816MergeLoop();
	TestCPP816ReturnedObjectSlots();
	TestCPP816DivisibilityTests();
	TestCPP816IntIndexPointers();
	TestCPP816NarrowCounters();
	TestCPP816AlignedFrames();
	TestCPP816AlignedBigFrameArguments();
	TestCPP816AlignedFrameUnreadArguments();
	TestCPP816RegisterArguments();
	TestCPP816FrameOperandFloats();
	TestCPP816RegisterFloats();
	TestCPP816ObjectArguments();
	TestCPP816Exit();
	TestCPP816StringRegisters();
	TestCPP816MathRegisters();
	TestCPP816WideMultiplies();
	TestCPP816ConstantArguments();
	TestCPP816Namespaces();

	CW65C816InstructionFactory::GetInstance()->Kill();
	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

