#include "CppCompilerLib/CPPPreprocessor.h"
#include "CppCompilerLib/CppTokeniser.h"
#include "CppCompilerLib/CppParser.h"
#include "CppCompilerLib/CPPSyntaxes.h"
#include "CppCompilerLib/CPPSyntaxBlock.h"
#include "TestLib/Assert.h"


//The declarations of a parsed text, and a tree to read blocks into.
struct STestSyntaxBlock
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPSyntaxTree		cRead;		//The blocks are read into it (the same tokens).

	bool	Init(char* szSource);
	void	Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool STestSyntaxBlock::Init(char* szSource)
{
	CCPPPreprocessor	cPreprocessor;
	CCPPParser		cParser;
	bool			bResult;

	cFile.Init("Block.cpp", NULL, false);
	cFile.SetContents(szSource);
	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	cPreprocessor.Kill();
	cTokeniser.Init(NULL);
	bResult &= cTokeniser.Tokenise(cFile.GetTokens());
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	bResult &= cParser.Parse();
	bResult &= cParser.NumErrors() == 0;
	cParser.Kill();
	cRead.Init(&cTokeniser);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STestSyntaxBlock::Kill(void)
{
	cRead.Kill();
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The declarations, namespaces and linkage specifications looked into.
//
//////////////////////////////////////////////////////////////////////////
static void TestSyntaxBlockDeclarations(CCPPSyntax* pcNode, CArraySyntax* papcDeclarations)
{
	size		i;
	CCPPSyntax*	pcChild;
	ECPPSyntax	eKind;

	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		pcChild = pcNode->GetChild(i);
		if (!pcChild)
		{
			continue;
		}
		eKind = pcChild->GetKind();
		if ((eKind == CPPS_Namespace) || (eKind == CPPS_LinkageSpecification))
		{
			TestSyntaxBlockDeclarations(pcChild, papcDeclarations);
		}
		else if ((eKind != CPPS_Name) && (eKind != CPPS_NameComponent))
		{
			papcDeclarations->Add(pcChild);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The kinds of a node and of the nodes under it.
//
//////////////////////////////////////////////////////////////////////////
static void TestSyntaxBlockKinds(CCPPSyntax* pcNode, bool* pabKinds)
{
	size		i;
	CCPPSyntax*	pcChild;

	pabKinds[pcNode->GetKind()] = true;
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		pcChild = pcNode->GetChild(i);
		if (pcChild)
		{
			TestSyntaxBlockKinds(pcChild, pabKinds);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each declaration of the source written as a block, read into another tree
// and written again is the same block, and prints the same.  The kinds of
// node the declarations have are set in pabKinds, if it is given.
//
//////////////////////////////////////////////////////////////////////////
static void TestSyntaxBlockRoundTrips(char* szSource, bool* pabKinds)
{
	STestSyntaxBlock	sBlock;
	CArraySyntax		apcDeclarations;
	CArraySyntaxBlockDeclared	asDeclared;
	CArrayUint8			auiWritten;
	CArrayUint8			auiAgain;
	CCPPSyntax*			pcDeclaration;
	CCPPSyntax*			pcRead;
	CCPPScope*			pcGlobal;
	CChars				szWritten;
	CChars				szRead;
	size				i;
	size				uiSame;

	AssertTrue(sBlock.Init(szSource));

	apcDeclarations.Init();
	TestSyntaxBlockDeclarations((CCPPSyntax*)sBlock.cTree.GetRoot(), &apcDeclarations);
	AssertTrue(apcDeclarations.NumElements() > 10);
	pcGlobal = sBlock.cTree.GetSymbols()->GetGlobalScope();

	uiSame = 0;
	for (i = 0; i < apcDeclarations.NumElements(); i++)
	{
		pcDeclaration = apcDeclarations.GetPtr(i);
		if (pabKinds)
		{
			TestSyntaxBlockKinds(pcDeclaration, pabKinds);
		}
		auiWritten.Init();
		auiAgain.Init();
		AssertInt(CPPSB_Written, CCPPSyntaxBlockWriter::Write(&sBlock.cTree, pcDeclaration, pcGlobal, &auiWritten));
		asDeclared.Init();
		pcRead = CCPPSyntaxBlockReader::Read(auiWritten.GetData(), auiWritten.NumElements(), &sBlock.cRead, pcDeclaration->miFirstToken, pcGlobal, NULL, NULL, &asDeclared);
		AssertNotNull(pcRead);
		if (pcRead)
		{
			AssertInt(CPPSB_Written, CCPPSyntaxBlockWriter::Write(&sBlock.cRead, pcRead, pcGlobal, &auiAgain, &asDeclared));
			AssertSize(auiWritten.NumElements(), auiAgain.NumElements());
			if ((auiWritten.NumElements() == auiAgain.NumElements()) && (memcmp(auiWritten.GetData(), auiAgain.GetData(), auiWritten.NumElements()) == 0))
			{
				uiSame++;
			}

			szWritten.Init();
			szRead.Init();
			pcDeclaration->PrettyPrint(&szWritten, 0);
			pcRead->PrettyPrint(&szRead, 0);
			AssertString(szWritten.Text(), szRead.Text());
			szWritten.ReInit();
			szRead.ReInit();
			pcDeclaration->TypePrint(&szWritten, 0);
			pcRead->TypePrint(&szRead, 0);
			AssertString(szWritten.Text(), szRead.Text());
			szRead.Kill();
			szWritten.Kill();
		}
		asDeclared.Kill();
		auiAgain.Kill();
		auiWritten.Kill();
	}
	AssertSize(apcDeclarations.NumElements(), uiSame);

	apcDeclarations.Kill();
	sBlock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each declaration written as a block, read into another tree and written
// again is the same block, and prints the same.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSyntaxBlockRoundTrip(void)
{
	TestSyntaxBlockRoundTrips("\
typedef unsigned int uint;\n\
enum EColour { Red, Green = 3, Blue };\n\
enum class [[nodiscard]] EKind { One [[deprecated]] = 1, Two };\n\
struct SPoint { int x; int y; };\n\
struct SDerived : [[maybe_unused]] public SPoint { int z; };\n\
class CShape\n\
{\n\
public:\n\
	SPoint	msCentre;\n\
	virtual int Area(void) { return msCentre.x * msCentre.y; }\n\
	static const int kiSides = 4;\n\
	struct SInner { uint uiCount; } msInner;\n\
};\n\
int Add(int a, int b) { return a + b; }\n\
float Add(float a, float b) { return a + b; }\n\
template<class M> class CHolder { public: M mValue; M Get(void) { return mValue; } };\n\
template<class M> M Larger(M a, M b) { return (a > b) ? a : b; }\n\
namespace Space\n\
{\n\
	int giCount = 0;\n\
	void Count(int* pi, int n) { for (int i = 0; i < n; i++) { giCount += pi[i]; } }\n\
	class CDerived : public CShape { public: int Area(void) override { return Add(1, 2); } };\n\
}\n\
extern \"C\" { int Plain(char c); }\n\
using Space::giCount;\n\
int (*gpfAdd)(int, int) = &Add;\n\
CHolder<int> gcHolder;\n\
int Use(void) { EColour e = Green; CHolder<SPoint> c; return Larger(1, 2) + (int)e + c.mValue.x; }\n\
", NULL);
}


//The kinds' names, for saying which is missing.
#define TEST_SYNTAX_KIND_NAME(N)	#N,
static const char*	gaszTestSyntaxKinds[] = { CPP_SYNTAX_KINDS(TEST_SYNTAX_KIND_NAME) };
#undef TEST_SYNTAX_KIND_NAME


//////////////////////////////////////////////////////////////////////////
//
// Every kind of node is written as part of a block and read again the
// same: the source has each kind the parser makes of a text that is right
// (a new kind is added to it, or the test says which it has none of).  Not
// the tree's root, a namespace or a linkage specification, which are looked
// into and not written; what the parser makes of a text it can't parse
// (SkippedDeclaration, SkippedStatement); a declaration left in its header
// (LazyDeclaration); or what the semantic layer adds to a tree
// (ImplicitArgument).
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSyntaxBlockEveryKind(void)
{
	static const ECPPSyntax	aeNotWritten[] = { CPPS_TranslationUnit, CPPS_Namespace, CPPS_LinkageSpecification, CPPS_SkippedDeclaration, CPPS_SkippedStatement, CPPS_LazyDeclaration, CPPS_ImplicitArgument };
	bool					abKinds[CPPS_NumKinds];
	CChars					szMissing;
	size					i;

	memset(abKinds, 0, sizeof(abKinds));
	TestSyntaxBlockRoundTrips("\
typedef unsigned int uint;\n\
;\n\
struct SBase { int miBase; SBase(int i) : miBase(i) {} virtual ~SBase() {} virtual int Get(void) { return miBase; } };\n\
struct SForward;\n\
enum EColour { Red, Green = 3 };\n\
alignas(4) int giAligned = 0;\n\
decltype(giAligned) giSame = 1;\n\
struct SForward* gpsForward = nullptr;\n\
int gaiValues[3] = { 1, 2, 3 };\n\
int giParenthesised(4);\n\
[[noreturn]] void Stop(void);\n\
void Quiet(void) noexcept;\n\
void Throws(void) throw();\n\
using UInt = unsigned int;\n\
static_assert(sizeof(int) >= 2, \"int\");\n\
__extension__ template<class T> struct SExtended { T mValue; };\n\
template<class T, int N = 2> struct SArray { T maValues[N]; };\n\
template<class... A> int Count(A... a) { auto f = [a...] { return (int)sizeof...(a); }; return f(); }\n\
template<class... A> int Forward(A&&... a) { return Count(a...); }\n\
class CDerived : public SBase\n\
{\n\
public:\n\
	int miOwn;\n\
	CDerived(int i) : SBase(i), miOwn(i * 2) {}\n\
	int Get(void) override { return this->miOwn; }\n\
};\n\
namespace Space { int giCount = 0; }\n\
using namespace Space;\n\
using Space::giCount;\n\
int Statements(int a, int* pi)\n\
{\n\
	int i;\n\
	int aiLocal[2] = { 0, 1 };\n\
	i = 0;\n\
	if (a > 0) { i = 1; } else { i = 2; }\n\
	switch (a)\n\
	{\n\
		case 1:\n\
			i++;\n\
			[[fallthrough]];\n\
		default:\n\
			break;\n\
	}\n\
	while (i < 10) { i += 2; }\n\
	do { i--; } while (i > 5);\n\
	for (i = 0; i < 3; i++) { if (i == 1) { continue; } }\n\
	for (int v : aiLocal) { i += v; }\n\
	goto Done;\n\
Done:\n\
	try { throw 1; } catch (int e) { i = e; } catch (...) { }\n\
	return i + *pi;\n\
}\n\
int Expressions(CDerived* pc, int a)\n\
{\n\
	int*	pi = new int(3);\n\
	int*	pai = new int[4];\n\
	int		i = (a > 1) ? -a : (a + 1);\n\
	char	c = \"text\"[0];\n\
	long	l = (long)a + static_cast<long>(c) + int(2) + UInt{3};\n\
	bool	b = noexcept(Quiet()) && __is_class(SBase) && (typeid(int) == typeid(a));\n\
	i += pc->Get() + pc->miOwn + gaiValues[1] + sizeof(SBase) + sizeof a;\n\
	i += [](int x) { return x; }(2) + [&i, a] { return i + a; }();\n\
	delete pi;\n\
	delete[] pai;\n\
	return b ? i : (int)l;\n\
}\n\
", abKinds);

	for (i = 0; i < sizeof(aeNotWritten) / sizeof(ECPPSyntax); i++)
	{
		AssertFalse(abKinds[aeNotWritten[i]]);
		abKinds[aeNotWritten[i]] = true;
	}
	szMissing.Init();
	for (i = 0; i < (size)CPPS_NumKinds; i++)
	{
		if (!abKinds[i])
		{
			szMissing.Append(gaszTestSyntaxKinds[i]);
			szMissing.Append(' ');
		}
	}
	AssertString("", szMissing.Text());
	szMissing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A block that is cut short or isn't a block isn't read.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSyntaxBlockWrong(void)
{
	STestSyntaxBlock	sBlock;
	CArrayUint8			auiWritten;
	CCPPSyntax*			pcDeclaration;
	CCPPScope*			pcGlobal;

	AssertTrue(sBlock.Init("class CA { public: int miA; int Get(void) { return miA; } };\n"));
	pcDeclaration = (CCPPSyntax*)sBlock.cTree.GetRoot()->macpDeclarations.GetPtr(0);
	pcGlobal = sBlock.cTree.GetSymbols()->GetGlobalScope();
	auiWritten.Init();
	AssertInt(CPPSB_Written, CCPPSyntaxBlockWriter::Write(&sBlock.cTree, pcDeclaration, pcGlobal, &auiWritten));
	AssertNull(CCPPSyntaxBlockReader::Read(auiWritten.GetData(), auiWritten.NumElements() - 1, &sBlock.cRead, pcDeclaration->miFirstToken, pcGlobal, NULL, NULL));
	*auiWritten.Get(0) = 'X';
	AssertNull(CCPPSyntaxBlockReader::Read(auiWritten.GetData(), auiWritten.NumElements(), &sBlock.cRead, pcDeclaration->miFirstToken, pcGlobal, NULL, NULL));
	auiWritten.Kill();
	sBlock.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSyntaxBlock(void)
{
	BeginTests();

	TestCPPSyntaxBlockRoundTrip();
	TestCPPSyntaxBlockEveryKind();
	TestCPPSyntaxBlockWrong();

	TestStatistics();
}

