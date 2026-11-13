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
// Each declaration written as a block, read into another tree and written
// again is the same block, and prints the same.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSyntaxBlockRoundTrip(void)
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

	AssertTrue(sBlock.Init("\
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
"));

	apcDeclarations.Init();
	TestSyntaxBlockDeclarations((CCPPSyntax*)sBlock.cTree.GetRoot(), &apcDeclarations);
	AssertTrue(apcDeclarations.NumElements() > 10);
	pcGlobal = sBlock.cTree.GetSymbols()->GetGlobalScope();

	uiSame = 0;
	for (i = 0; i < apcDeclarations.NumElements(); i++)
	{
		pcDeclaration = apcDeclarations.GetPtr(i);
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
	TestCPPSyntaxBlockWrong();

	TestStatistics();
}

