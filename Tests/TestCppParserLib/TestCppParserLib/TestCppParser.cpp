#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/CppTokeniser.h"
#include "CppParserLib/CppParser.h"
#include "CppParserLib/HeaderFileMap.h"
#include "BaseLib/FileUtil.h"
#include "TestLib/Assert.h"


//A translation unit parsed from text.
struct SParsedText
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	int					iErrors;

	bool	Init(char* szSource);
	void	Kill(void);
	char*	Print(CChars* psz);
	int		FindToken(char* szSpelling, int iOccurrence);
	CCPPSymbol*	GetSymbol(char* szSpelling, int iOccurrence);
	int		GetRole(char* szSpelling, int iOccurrence);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SParsedText::Init(char* szSource)
{
	CPreprocessor	cPreprocessor;
	CCPPParser		cParser;
	bool			bResult;

	cFile.Init("Parsed.cpp", NULL, false);
	cFile.SetContents(szSource);
	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	cPreprocessor.Kill();

	cTokeniser.Init();
	bResult &= cTokeniser.Tokenise(cFile.GetTokens());

	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	bResult &= cParser.Parse();
	iErrors = cParser.NumErrors();
	cParser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SParsedText::Kill(void)
{
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* SParsedText::Print(CChars* psz)
{
	cTree.PrettyPrint(psz);
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SParsedText::FindToken(char* szSpelling, int iOccurrence)
{
	size					i;
	SPreprocessedToken*		psSource;

	for (i = 0; i < cTokeniser.NumTokens(); i++)
	{
		psSource = cTokeniser.GetSource(cTokeniser.Get(i));
		if (psSource->Is(szSpelling))
		{
			if (iOccurrence == 0)
			{
				return (int)i;
			}
			iOccurrence--;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* SParsedText::GetSymbol(char* szSpelling, int iOccurrence)
{
	int		iToken;

	iToken = FindToken(szSpelling, iOccurrence);
	if (iToken == -1)
	{
		return NULL;
	}
	return cTree.GetTokenSymbol(iToken)->pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SParsedText::GetRole(char* szSpelling, int iOccurrence)
{
	int		iToken;

	iToken = FindToken(szSpelling, iOccurrence);
	if (iToken == -1)
	{
		return -1;
	}
	return cTree.GetTokenSymbol(iToken)->uiRole;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserDeclarations(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
typedef unsigned int uint;\n\
namespace N { extern \"C\" int f(int a, ...); }\n\
struct SPoint { int x, y : 3; static const int Z = 2; };\n\
enum EColour { RED = 1, GREEN = RED << 1, };\n\
void (*gpfCallback)(int);\n\
char gaszNames[2][8] = { \"A\", \"B\" };\n\
int (SPoint::*gpiMember);\n\
using namespace N;\n\
static_assert(sizeof(SPoint) > 4, \"Too small\");"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
typedef unsigned int uint;\n\
namespace N\n\
{\n\
	extern \"C\" int f(int a, ...);\n\
}\n\
struct SPoint\n\
{\n\
	int x, y : 3;\n\
	static const int Z = 2;\n\
};\n\
enum EColour\n\
{\n\
	RED = 1,\n\
	GREEN = RED << 1,\n\
};\n\
void (*gpfCallback)(int);\n\
char gaszNames[2][8] = { \"A\", \"B\" };\n\
int (SPoint::*gpiMember);\n\
using namespace N;\n\
static_assert(sizeof(SPoint) > 4, \"Too small\");\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Typedef, sParsed.GetSymbol("uint", 0)->meKind);
	AssertInt(CPPSK_Namespace, sParsed.GetSymbol("N", 0)->meKind);
	AssertInt(CPPSK_Function, sParsed.GetSymbol("f", 0)->meKind);
	AssertInt(CPPSK_Class, sParsed.GetSymbol("SPoint", 0)->meKind);
	AssertInt(CPPSK_Field, sParsed.GetSymbol("x", 0)->meKind);
	AssertInt(CPPSK_Enumerator, sParsed.GetSymbol("RED", 0)->meKind);
	AssertInt(CPPTR_Reference, sParsed.GetRole("RED", 1));
	AssertPointer(sParsed.GetSymbol("RED", 0), sParsed.GetSymbol("RED", 1));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("gpfCallback", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("SPoint", 0), sParsed.GetSymbol("SPoint", 2));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserAmbiguities(void)
{
	SParsedText		sParsed;
	CChars			sz;

	//[stmt.ambig] and [dcl.ambig.res]: what could be a declaration is one.
	AssertTrue(sParsed.Init("\
struct T { T(int); T operator*(T); };\n\
int a, b;\n\
T f(int);\n\
void g(void)\n\
{\n\
	T(x);\n\
	a * b;\n\
	T * p;\n\
	T t(a);\n\
	T u(int);\n\
	(T)a;\n\
	(a)*b;\n\
	T(a) * T(b);\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct T\n\
{\n\
	T(int);\n\
	T operator *(T);\n\
};\n\
int a, b;\n\
T f(int);\n\
void g(void)\n\
{\n\
	T (x);\n\
	a * b;\n\
	T *p;\n\
	T t(a);\n\
	T u(int);\n\
	(T)a;\n\
	(a) * b;\n\
	T(a) * T(b);\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	//T(x) declares x.  T u(int) declares a function.
	AssertInt(CPPTR_Declaration, sParsed.GetRole("x", 0));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("x", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("t", 0)->meKind);
	AssertInt(CPPSK_Function, sParsed.GetSymbol("u", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("p", 0)->meKind);
	AssertInt(CPPTR_Reference, sParsed.GetRole("a", 2));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserClasses(void)
{
	SParsedText		sParsed;
	CChars			sz;

	//Member function bodies see members declared after them.  [class.mem] 6
	AssertTrue(sParsed.Init("\
class CBase\n\
{\n\
public:\n\
	virtual ~CBase() {}\n\
	virtual int Get(void) = 0;\n\
protected:\n\
	int miBase;\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	CDerived() : miValue(0) {}\n\
	int Get(void) override { return miValue + miBase + Twice(); }\n\
	int Twice(void) const;\n\
private:\n\
	int miValue;\n\
};\n\
int CDerived::Twice(void) const\n\
{\n\
	return miValue * 2;\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
class CBase\n\
{\n\
public:\n\
	virtual ~CBase()\n\
	{\n\
	}\n\
	virtual int Get(void) = 0;\n\
protected:\n\
	int miBase;\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	CDerived() : miValue(0)\n\
	{\n\
	}\n\
	int Get(void) override\n\
	{\n\
		return miValue + miBase + Twice();\n\
	}\n\
	int Twice(void) const;\n\
private:\n\
	int miValue;\n\
};\n\
int CDerived::Twice(void) const\n\
{\n\
	return miValue * 2;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	//miValue in the constructor and Get are the member declared after them; miBase is in the base class.
	AssertNotNull(sParsed.GetSymbol("miValue", 2));
	AssertPointer(sParsed.GetSymbol("miValue", 2), sParsed.GetSymbol("miValue", 0));
	AssertPointer(sParsed.GetSymbol("miValue", 2), sParsed.GetSymbol("miValue", 1));
	AssertPointer(sParsed.GetSymbol("miValue", 2), sParsed.GetSymbol("miValue", 3));
	AssertNotNull(sParsed.GetSymbol("miBase", 0));
	AssertPointer(sParsed.GetSymbol("miBase", 0), sParsed.GetSymbol("miBase", 1));
	AssertPointer(sParsed.GetSymbol("Twice", 1), sParsed.GetSymbol("Twice", 0));
	AssertInt(CPPSK_Function, sParsed.GetSymbol("Twice", 2)->meKind);
	AssertInt(CPPSK_Class, sParsed.GetSymbol("CBase", 2)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserTemplates(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
template<class M> class CArray { public: M* Get(int i); M maData[4]; };\n\
template<class M> M* CArray<M>::Get(int i) { return &maData[i]; }\n\
template<> class CArray<bool> { };\n\
template class CArray<char>;\n\
template<class M, typename... Args> M* New(Args... args) { return new M(args...); }\n\
CArray<CArray<int>> gcArrays;\n\
int x = New<int>(3) != 0 && 1 > 0;\n\
bool b = sizeof(CArray<int>) >> 2 > 1;"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
template<class M>\n\
class CArray\n\
{\n\
public:\n\
	M *Get(int i);\n\
	M maData[4];\n\
};\n\
template<class M>\n\
M *CArray<M>::Get(int i)\n\
{\n\
	return &maData[i];\n\
}\n\
template<>\n\
class CArray<bool>\n\
{\n\
};\n\
template\n\
class CArray<char>;\n\
template<class M, typename... Args>\n\
M *New(Args ...args)\n\
{\n\
	return new M(args...);\n\
}\n\
CArray<CArray<int>> gcArrays;\n\
int x = New<int>(3) != 0 && 1 > 0;\n\
bool b = sizeof(CArray<int>) >> 2 > 1;\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_ClassTemplate, sParsed.GetSymbol("CArray", 0)->meKind);
	AssertInt(CPPSK_TypeParameter, sParsed.GetSymbol("M", 0)->meKind);
	AssertInt(CPPSK_FunctionTemplate, sParsed.GetSymbol("New", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("maData", 0), sParsed.GetSymbol("maData", 1));
	AssertPointer(sParsed.GetSymbol("Get", 0), sParsed.GetSymbol("Get", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserStatements(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
int f(int* pi, int n)\n\
{\n\
	int iTotal = 0;\n\
	for (int i = 0; i < n; i++)\n\
		iTotal += pi[i];\n\
	while (int j = n--) { if (j == 3) continue; else break; }\n\
	do { n++; } while (n < 10);\n\
	switch (n) { case 1: return 1; default: break; }\n\
	if (n > 0) goto done;\n\
done:\n\
	return iTotal ? iTotal : -1;\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
int f(int *pi, int n)\n\
{\n\
	int iTotal = 0;\n\
	for (int i = 0; i < n; i++)\n\
		iTotal += pi[i];\n\
	while (int j = n--)\n\
	{\n\
		if (j == 3)\n\
			continue;\n\
		else\n\
			break;\n\
	}\n\
	do\n\
	{\n\
		n++;\n\
	}\n\
	while (n < 10);\n\
	switch (n)\n\
	{\n\
	case 1:\n\
		return 1;\n\
	default:\n\
		break;\n\
	}\n\
	if (n > 0)\n\
		goto done;\n\
done:\n\
	return iTotal ? iTotal : -1;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Parameter, sParsed.GetSymbol("pi", 1)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("i", 2)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("j", 1)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserErrorRecovery(void)
{
	SParsedText		sParsed;
	CChars			sz;

	//A declaration that can't be parsed is kept as its tokens and parsing carries on.
	AssertFalse(sParsed.Init("\
int a = ;\n\
int b = 2;\n\
void f(void) { int c = 3 +; c++; }"));
	AssertInt(2, sParsed.iErrors);
	AssertInt(2, (int)sParsed.cTree.NumErrors());
	AssertString("Parsed.cpp (1, 9): Expected an expression. [;]", sParsed.cTree.GetError(0));
	AssertInt(sParsed.FindToken(";", 0), sParsed.cTree.GetErrorToken(0));
	AssertString("Parsed.cpp (3, 27): Expected an expression. [;]", sParsed.cTree.GetErrors()->Get(1));
	AssertInt(2, sParsed.cTree.GetErrors()->GetLine(1));

	sz.Init();
	AssertString("\
int a = ;\n\
int b = 2;\n\
void f(void)\n\
{\n\
	int c = 3 +;\n\
	c++;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("b", 0)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// try, catch, throw, dynamic exception specifications and noexcept as a
// specifier and as an expression.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserExceptions(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
void f(int n)\n\
{\n\
	try { if (n) throw 3; throw; } catch (int& e) { n = e; } catch (...) { }\n\
}\n\
void g() throw(int);\n\
void h() noexcept;\n\
void k() noexcept(true);\n\
bool b = noexcept(f(1));"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
void f(int n)\n\
{\n\
	try\n\
	{\n\
		if (n)\n\
			throw 3;\n\
		throw;\n\
	}\n\
	catch (int &e)\n\
	{\n\
		n = e;\n\
	}\n\
	catch (...)\n\
	{\n\
	}\n\
}\n\
void g() throw(int);\n\
void h() noexcept;\n\
void k() noexcept(true);\n\
bool b = noexcept(f(1));\n", sParsed.Print(&sz));
	sz.Kill();

	//The handler's parameter is in the handler's scope.
	AssertNotNull(sParsed.GetSymbol("e", 0));
	AssertInt(CPPTR_Declaration, sParsed.GetRole("e", 0));
	AssertPointer(sParsed.GetSymbol("e", 0), sParsed.GetSymbol("e", 1));
	AssertPointer(sParsed.GetSymbol("f", 0), sParsed.GetSymbol("f", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The named casts, typeid, decltype, __typeof__, alignof, sizeof... and
// the compiler's __is_ type traits.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserCastsAndTypeQueries(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
struct A { virtual ~A(); }; struct B : A { };\n\
void f(A* pa, const B* pb, long l)\n\
{\n\
	B* p1 = static_cast<B*>(pa);\n\
	B* p2 = dynamic_cast<B*>(pa);\n\
	char* p3 = reinterpret_cast<char*>(l);\n\
	B* p4 = const_cast<B*>(pb);\n\
	const char* sz = typeid(*pa).name();\n\
	decltype(l) m = l;\n\
	__typeof__(l) n = m;\n\
	unsigned s = alignof(A);\n\
}\n\
template<typename... T> int Count(T... t) { return sizeof...(T) + sizeof...(t); }\n\
bool b1 = __is_class(A);\n\
bool b2 = __is_base_of(A, B);\n\
bool b3 = __is_pod(int);\n\
bool b4 = __has_trivial_destructor(B);"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct A\n\
{\n\
	virtual ~A();\n\
};\n\
struct B : A\n\
{\n\
};\n\
void f(A *pa, const B *pb, long l)\n\
{\n\
	B *p1 = static_cast<B *>(pa);\n\
	B *p2 = dynamic_cast<B *>(pa);\n\
	char *p3 = reinterpret_cast<char *>(l);\n\
	B *p4 = const_cast<B *>(pb);\n\
	const char *sz = typeid(*pa).name();\n\
	decltype(l) m = l;\n\
	__typeof__(l) n = m;\n\
	unsigned s = alignof(A);\n\
}\n\
template<typename... T>\n\
int Count(T ...t)\n\
{\n\
	return sizeof...(T) + sizeof...(t);\n\
}\n\
bool b1 = __is_class(A);\n\
bool b2 = __is_base_of(A, B);\n\
bool b3 = __is_pod(int);\n\
bool b4 = __has_trivial_destructor(B);\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Class, sParsed.GetSymbol("B", 2)->meKind);
	AssertPointer(sParsed.GetSymbol("B", 0), sParsed.GetSymbol("B", 3));
	AssertPointer(sParsed.GetSymbol("pa", 0), sParsed.GetSymbol("pa", 3));
	AssertPointer(sParsed.GetSymbol("l", 0), sParsed.GetSymbol("l", 2));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("n", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("A", 0), sParsed.GetSymbol("A", 4));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Alias declarations, using-declarations, scoped enums and enum bases,
// unions, friends, inline, unnamed and aliased namespaces, linkage
// blocks, __extension__, alignas and [[attributes]].
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserModernDeclarations(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
namespace NA { int x; }\n\
using INT = int;\n\
using NA::x;\n\
enum class E : int { A, B = 2 };\n\
enum F : unsigned char { C };\n\
union U { int i; float f; };\n\
class G { friend class D; friend void H(G&); int m; };\n\
inline namespace V1 { int v; }\n\
namespace { int hidden; }\n\
namespace NB = NA;\n\
extern \"C\" { int cf(void); int cg(int); }\n\
__extension__ typedef long long ll;\n\
__extension__ int ext;\n\
struct SW { alignas(8) int w; alignas(SW) char c; };\n\
[[noreturn]] void Stop(void);\n\
[[deprecated(\"old\")]] INT Old(void);"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
namespace NA\n\
{\n\
	int x;\n\
}\n\
using INT = int;\n\
using NA::x;\n\
enum class E : int\n\
{\n\
	A,\n\
	B = 2\n\
};\n\
enum F : unsigned char\n\
{\n\
	C\n\
};\n\
union U\n\
{\n\
	int i;\n\
	float f;\n\
};\n\
class G\n\
{\n\
	friend class D;\n\
	friend void H(G &);\n\
	int m;\n\
};\n\
inline namespace V1\n\
{\n\
	int v;\n\
}\n\
namespace\n\
{\n\
	int hidden;\n\
}\n\
namespace NB = NA;\n\
extern \"C\"\n\
{\n\
	int cf(void);\n\
	int cg(int);\n\
}\n\
__extension__ typedef long long ll;\n\
__extension__ int ext;\n\
struct SW\n\
{\n\
	alignas(8) int w;\n\
	alignas(SW) char c;\n\
};\n\
[[noreturn]] void Stop(void);\n\
[[deprecated(\"old\")]] INT Old(void);\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Typedef, sParsed.GetSymbol("INT", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("INT", 0), sParsed.GetSymbol("INT", 1));
	AssertPointer(sParsed.GetSymbol("NA", 0), sParsed.GetSymbol("NA", 1));
	AssertInt(CPPSK_Enum, sParsed.GetSymbol("E", 0)->meKind);
	AssertInt(CPPSK_Enumerator, sParsed.GetSymbol("A", 0)->meKind);
	AssertInt(CPPSK_Enum, sParsed.GetSymbol("F", 0)->meKind);
	AssertInt(CPPSK_Class, sParsed.GetSymbol("U", 0)->meKind);
	AssertInt(CPPSK_Field, sParsed.GetSymbol("i", 0)->meKind);
	AssertInt(CPPSK_Namespace, sParsed.GetSymbol("V1", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("v", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("hidden", 0)->meKind);
	AssertInt(CPPSK_Function, sParsed.GetSymbol("cf", 0)->meKind);
	AssertInt(CPPSK_Typedef, sParsed.GetSymbol("ll", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("ext", 0)->meKind);
	AssertInt(CPPSK_Field, sParsed.GetSymbol("w", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("SW", 0), sParsed.GetSymbol("SW", 1));
	AssertInt(CPPSK_Function, sParsed.GetSymbol("Stop", 0)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Range-based for, and new and delete of arrays.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserRangeForNewAndDelete(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
int a[3];\n\
int f(int n)\n\
{\n\
	int t = 0;\n\
	for (int x : a) t += x;\n\
	for (auto& y : a) { y = 0; }\n\
	int* p = new int[n];\n\
	delete[] p;\n\
	int* q = new int(3);\n\
	delete q;\n\
	int** pp = new int*[n];\n\
	::delete[] pp;\n\
	return t;\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
int a[3];\n\
int f(int n)\n\
{\n\
	int t = 0;\n\
	for (int x : a)\n\
		t += x;\n\
	for (auto &y : a)\n\
	{\n\
		y = 0;\n\
	}\n\
	int *p = new int[n];\n\
	delete[] p;\n\
	int *q = new int(3);\n\
	delete q;\n\
	int **pp = new int *[n];\n\
	::delete[] pp;\n\
	return t;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Variable, sParsed.GetSymbol("x", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("x", 0), sParsed.GetSymbol("x", 1));
	AssertPointer(sParsed.GetSymbol("a", 0), sParsed.GetSymbol("a", 1));
	AssertPointer(sParsed.GetSymbol("y", 0), sParsed.GetSymbol("y", 1));
	AssertPointer(sParsed.GetSymbol("pp", 0), sParsed.GetSymbol("pp", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// operator(), operator new and delete, conversion operators, defaulted
// and deleted functions, and typename for a dependent type.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserOperators(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
struct F\n\
{\n\
	F() = default;\n\
	F(const F&) = delete;\n\
	int operator()(int i) const { return i; }\n\
	void* operator new(unsigned long s);\n\
	void operator delete(void* p);\n\
	operator int() const { return 1; }\n\
	explicit operator bool() const;\n\
	F& operator=(const F&) = default;\n\
	typedef int X;\n\
};\n\
template<class T> typename T::X g(T t) { typename T::X x = 0; return x; }"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct F\n\
{\n\
	F() = default;\n\
	F(const F &) = delete;\n\
	int operator ()(int i) const\n\
	{\n\
		return i;\n\
	}\n\
	void *operator new(unsigned long s);\n\
	void operator delete(void *p);\n\
	operator int() const\n\
	{\n\
		return 1;\n\
	}\n\
	explicit operator bool() const;\n\
	F &operator =(const F &) = default;\n\
	typedef int X;\n\
};\n\
template<class T>\n\
typename T::X g(T t)\n\
{\n\
	typename T::X x = 0;\n\
	return x;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Parameter, sParsed.GetSymbol("i", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("i", 0), sParsed.GetSymbol("i", 1));
	AssertInt(CPPSK_TypeParameter, sParsed.GetSymbol("T", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("T", 0), sParsed.GetSymbol("T", 1));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("x", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("x", 0), sParsed.GetSymbol("x", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The tree's syntaxes, their token ranges and children.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserTreeWalk(void)
{
	SParsedText		sParsed;
	CChars			sz;
	CCPPSyntax*		pcRoot;
	CCPPSyntax*		pcFunction;

	AssertTrue(sParsed.Init("int Twice(int i)\n{\n\treturn i * 2;\n}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	sParsed.cTree.TypePrint(&sz);
	AssertString("\
TranslationUnit [0, 12]\n\
  FunctionDefinition [0, 12]\n\
    DeclSpecifiers [0, 0]\n\
      KeywordSpecifier [0, 0]\n\
    Declarator [1, 5]\n\
      Name [1, 1]\n\
        NameComponent [1, 1]\n\
      FunctionSuffix [2, 5]\n\
        Parameter [3, 4]\n\
          DeclSpecifiers [3, 3]\n\
            KeywordSpecifier [3, 3]\n\
          Declarator [4, 4]\n\
            Name [4, 4]\n\
              NameComponent [4, 4]\n\
    CompoundStatement [6, 12]\n\
      ReturnStatement [7, 11]\n\
        BinaryExpression [8, 10]\n\
          IdExpression [8, 8]\n\
            Name [8, 8]\n\
              NameComponent [8, 8]\n\
          LiteralExpression [10, 10]\n", sz.Text());
	sz.Kill();

	pcRoot = (CCPPSyntax*)sParsed.cTree.GetRoot();
	AssertSize(1, pcRoot->NumChildren());
	pcFunction = pcRoot->GetChild(0);
	AssertString("FunctionDefinition", pcFunction->GetType());
	AssertSize(3, pcFunction->NumChildren());
	AssertString("CompoundStatement", pcFunction->GetChild(2)->GetType());
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The errors for szSource, one a line.  szDeclared, declared after the
// errors, shows parsing went on after them.
//
//////////////////////////////////////////////////////////////////////////
static int ParseErrors(char* szSource, char* szDeclared, CChars* pszErrors)
{
	SParsedText		sParsed;
	size			i;
	int				iErrors;

	AssertFalse(sParsed.Init(szSource));
	for (i = 0; i < sParsed.cTree.NumErrors(); i++)
	{
		pszErrors->Append(sParsed.cTree.GetError(i));
		pszErrors->AppendNewLine();
	}
	AssertNotNull(sParsed.GetSymbol(szDeclared, 0));
	AssertInt(CPPTR_Declaration, sParsed.GetRole(szDeclared, 0));
	iErrors = sParsed.iErrors;
	sParsed.Kill();
	return iErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Lambdas, compound literals and template template parameters are
// errors.  Parsing goes on after them.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserUnsupported(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertFalse(sParsed.Init("\
int f(int n)\n\
{\n\
	auto g = [n](int x) { return x + n; };\n\
	return n;\n\
}\n\
struct P { int x; };\n\
int h(void)\n\
{\n\
	P p = (P){ 1 };\n\
	return 0;\n\
}\n\
template<template<class> class T> class X { };\n\
int k = 1;"));
	AssertInt(3, sParsed.iErrors);
	AssertInt(3, (int)sParsed.cTree.NumErrors());
	AssertString("Parsed.cpp (3, 11): Lambda expressions are not supported. [[]", sParsed.cTree.GetError(0));
	AssertInt(sParsed.FindToken("[", 0), sParsed.cTree.GetErrorToken(0));
	AssertString("Parsed.cpp (9, 11): Compound literals are not C++. [{]", sParsed.cTree.GetError(1));
	AssertString("Parsed.cpp (12, 10): Template template parameters are not supported. [template]", sParsed.cTree.GetError(2));
	AssertInt(sParsed.FindToken("template", 1), sParsed.cTree.GetErrorToken(2));

	//The statements after the errors are parsed; the bad ones are kept as their tokens.
	sz.Init();
	AssertString("\
int f(int n)\n\
{\n\
	auto g = [n](int x) { return x + n; }\n\
	;\n\
	return n;\n\
}\n\
struct P\n\
{\n\
	int x;\n\
};\n\
int h(void)\n\
{\n\
	P p = (P){ 1 }\n\
	;\n\
	return 0;\n\
}\n\
template<template<class> class T> class X { };\n\
int k = 1;\n", sParsed.Print(&sz));
	sz.Kill();

	AssertPointer(sParsed.GetSymbol("n", 0), sParsed.GetSymbol("n", 3));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("k", 0)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The parser's errors, where they are, and that it goes on after them.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserErrors(void)
{
	CChars	sz;

	sz.Init();
	AssertInt(1, ParseErrors("void f(int, 3);\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 13): Expected a parameter. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("int (3);\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 5): Expected a declarator. [(]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("extern \"C\" 3;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 12): Expected a declarator. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(2, ParseErrors("int x = 1 2;\nint y = 2;\nvoid f() { x = 1 2; y = 3; }\nint q;", "q", &sz));
	AssertString("\
Parsed.cpp (1, 11): Expected ';'. [2]\n\
Parsed.cpp (3, 18): Expected ';'. [2]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(3, ParseErrors("class 3;\nenum 4;\nenum E { 5 };\nint q;", "q", &sz));
	AssertString("\
Parsed.cpp (1, 7): Expected a class name or a class body. [3]\n\
Parsed.cpp (2, 6): Expected an enum name or body. [4]\n\
Parsed.cpp (3, 10): Expected an enumerator. [5]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(2, ParseErrors("int f(int n)\n{\n\tgoto ;\n\treturn n;\n}\nint* p = new 3;\nint q;", "q", &sz));
	AssertString("\
Parsed.cpp (3, 7): Expected a label. [;]\n\
Parsed.cpp (6, 14): Expected a type after new. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("int f(int n)\n{\n\tint a = int;\n\treturn n;\n}\nint q;", "q", &sz));
	AssertString("Parsed.cpp (3, 13): Expected '(' or '{' after the type. [;]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("void f(void)\n{\n\ttry { }\n}\nint q;", "q", &sz));
	AssertString("Parsed.cpp (4, 1): Expected a catch handler. [}]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("template<class T;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 17): Expected '>' to end the template parameters. [;]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("int a = static_cast<int 3>(a);\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 25): Expected '>'. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("int a = static_cast<const>(1);\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 26): Expected a type. [>]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("int x = ::3;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 11): Expected a name. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("static_assert(1, 3);\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 18): Expected a string. [3]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("__declspec x int y;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 12): Expected '(' after the attribute. [x]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("struct S { void f() { int a = ; } int m; };\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 31): Expected an expression. [;]\n", sz.Text());
	sz.Kill();

	//An operator name at the end of the file is an error at the last token.
	sz.Init();
	AssertInt(1, ParseErrors("int q;\nint operator", "q", &sz));
	AssertString("Parsed.cpp (2, 5): Expected an operator. [operator]\n", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Errors in a system header are skimmed: counted, but not reported unless
// ReportSkimmedErrors asks for them.
//
//////////////////////////////////////////////////////////////////////////
static void ParseWithSystemHeader(bool bReport, int* piErrors, int* piSkimmed, CChars* pszErrors, CChars* pszPrint)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CHeaderFileMap		cHeaderFiles;
	CHeaderFiles		cSystem;
	CFileUtil			cFileUtil;
	CChars				szDirectory;
	size				i;

	//The header is made in memory, in a directory with no headers on disk.
	szDirectory.Init();
	cFileUtil.CurrentDirectory(&szDirectory);
	cFileUtil.AppendToPath(&szDirectory, "Input");
	cFileUtil.AppendToPath(&szDirectory, "SFT");
	cHeaderFiles.Init();
	cSystem.Init(szDirectory.Text(), &cHeaderFiles, false, true);
	cSystem.AddFile("System.h")->SetContents("int giSystem = ;\nint giAfter;\n");

	cFile.Init("Parsed.cpp", NULL, false);
	cFile.SetContents("#include <System.h>\nint a = giAfter;\nint b = ;\n");
	cPreprocessor.Init(NULL);
	cPreprocessor.AddIncludeDirectory(&cSystem);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.ReportSkimmedErrors(bReport);
	AssertFalse(cParser.Parse());
	*piErrors = cParser.NumErrors();
	*piSkimmed = cParser.NumSkimmedErrors();
	for (i = 0; i < cTree.NumErrors(); i++)
	{
		pszErrors->Append(cTree.GetError(i));
		pszErrors->AppendNewLine();
	}
	cTree.PrettyPrint(pszPrint);
	cParser.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	cSystem.Kill();
	cHeaderFiles.Kill();
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserSkimmedErrors(void)
{
	CChars	szErrors;
	CChars	szPrint;
	int		iErrors;
	int		iSkimmed;

	szErrors.Init();
	szPrint.Init();
	ParseWithSystemHeader(false, &iErrors, &iSkimmed, &szErrors, &szPrint);
	AssertInt(1, iErrors);
	AssertInt(1, iSkimmed);
	AssertString("Parsed.cpp (3, 9): Expected an expression. [;]\n", szErrors.Text());
	AssertString("\
int giSystem = ;\n\
int giAfter;\n\
int a = giAfter;\n\
int b = ;\n", szPrint.Text());
	szPrint.Kill();
	szErrors.Kill();

	//Reported skimmed errors are marked, and still not counted as errors.
	szErrors.Init();
	szPrint.Init();
	ParseWithSystemHeader(true, &iErrors, &iSkimmed, &szErrors, &szPrint);
	AssertInt(1, iErrors);
	AssertInt(1, iSkimmed);
	AssertString("\
System.h (1, 16): (skimmed) Expected an expression. [;]\n\
Parsed.cpp (3, 9): Expected an expression. [;]\n", szErrors.Text());
	szPrint.Kill();
	szErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Using-directives that name each other don't send name lookup round in
// a circle.  [namespace.udir] 4
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserCyclicUsingDirectives(void)
{
	SParsedText		sParsed;
	size			i;
	int				iUnresolved;

	AssertTrue(sParsed.Init("\
namespace B { int q; }\n\
namespace A { using namespace B; }\n\
namespace B { using namespace A; }\n\
int y = A::z;\n\
int w = A::q;\n\
int v = B::q;"));
	AssertInt(0, sParsed.iErrors);

	//z is the only name not found.  q is found through A's using-directive.
	iUnresolved = 0;
	for (i = 0; i < sParsed.cTokeniser.NumTokens(); i++)
	{
		if (sParsed.cTree.GetTokenSymbol((int)i)->uiRole == CPPTR_Unresolved)
		{
			iUnresolved++;
		}
	}
	AssertInt(1, iUnresolved);
	AssertInt(CPPTR_Unresolved, sParsed.GetRole("z", 0));
	AssertNull(sParsed.GetSymbol("z", 0));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("q", 1)->meKind);
	AssertPointer(sParsed.GetSymbol("q", 0), sParsed.GetSymbol("q", 1));
	AssertPointer(sParsed.GetSymbol("q", 0), sParsed.GetSymbol("q", 2));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// alignas between the class key and the class name.  [class] 1
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserClassAlignas(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
struct alignas(8) S { int i; };\n\
struct alignas(S) [[deprecated]] T { char c; };\n\
class alignas(4) U;\n\
S s;\n\
T t;"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct alignas(8) S\n\
{\n\
	int i;\n\
};\n\
struct alignas(S) [[deprecated]] T\n\
{\n\
	char c;\n\
};\n\
class alignas(4) U;\n\
S s;\n\
T t;\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Class, sParsed.GetSymbol("S", 0)->meKind);
	AssertInt(CPPTR_Declaration, sParsed.GetRole("S", 0));
	AssertPointer(sParsed.GetSymbol("S", 0), sParsed.GetSymbol("S", 1));
	AssertPointer(sParsed.GetSymbol("S", 0), sParsed.GetSymbol("S", 2));
	AssertInt(CPPSK_Class, sParsed.GetSymbol("T", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("T", 0), sParsed.GetSymbol("T", 1));
	AssertInt(CPPSK_Class, sParsed.GetSymbol("U", 0)->meKind);
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("t", 0)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A block-scope declaration can start with an attribute.  [[ is never a
// lambda.  [dcl.attr.grammar] 7
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserAttributedDeclarationStatements(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
struct P { int x; };\n\
int f(void)\n\
{\n\
	[[maybe_unused]] int x = 1;\n\
	[[maybe_unused]] const P p = { x };\n\
	[[gnu::unused]] [[maybe_unused]] P* pp = 0;\n\
	return x + p.x;\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct P\n\
{\n\
	int x;\n\
};\n\
int f(void)\n\
{\n\
	[[maybe_unused]] int x = 1;\n\
	[[maybe_unused]] const P p = { x };\n\
	[[gnu::unused]] [[maybe_unused]] P *pp = 0;\n\
	return x + p.x;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertInt(CPPSK_Variable, sParsed.GetSymbol("x", 1)->meKind);
	AssertInt(CPPTR_Declaration, sParsed.GetRole("x", 1));
	AssertPointer(sParsed.GetSymbol("x", 1), sParsed.GetSymbol("x", 2));
	AssertPointer(sParsed.GetSymbol("x", 1), sParsed.GetSymbol("x", 3));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("p", 0)->meKind);
	AssertPointer(sParsed.GetSymbol("p", 0), sParsed.GetSymbol("p", 1));
	AssertPointer(sParsed.GetSymbol("P", 0), sParsed.GetSymbol("P", 2));
	AssertInt(CPPSK_Variable, sParsed.GetSymbol("pp", 0)->meKind);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A statement that isn't a declaration can start with attributes too
// ([[fallthrough]];, [[likely]] if ...): they are kept with the statement
// (CCPPSyntaxAttributedStatement), not parsed as a lambda.  [stmt.pre]
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserAttributedStatements(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
int f(int a)\n\
{\n\
	int r = 0;\n\
	switch (a)\n\
	{\n\
	case 1:\n\
		r += 1;\n\
		[[fallthrough]];\n\
	case 2:\n\
		r += 2;\n\
		break;\n\
	}\n\
	[[likely]] if (a) r++;\n\
	[[gnu::hot]] [[likely]] return r;\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
int f(int a)\n\
{\n\
	int r = 0;\n\
	switch (a)\n\
	{\n\
	case 1:\n\
		r += 1;\n\
		[[fallthrough]]\n\
		;\n\
	case 2:\n\
		r += 2;\n\
		break;\n\
	}\n\
	[[likely]]\n\
	if (a)\n\
		r++;\n\
	[[gnu::hot]] [[likely]]\n\
	return r;\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertPointer(sParsed.GetSymbol("r", 0), sParsed.GetSymbol("r", 4));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// .template and ->template name a member template of a dependent type.
// [temp.names] 4
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserTemplateMemberAccess(void)
{
	SParsedText		sParsed;
	CChars			sz;

	AssertTrue(sParsed.Init("\
struct A { template<class T> T get(void); };\n\
template<class U> int f(U* p, U o)\n\
{\n\
	return p->template get<int>() + o.template get<long>() + p->U::template get<int>();\n\
}"));
	AssertInt(0, sParsed.iErrors);

	sz.Init();
	AssertString("\
struct A\n\
{\n\
	template<class T>\n\
	T get(void);\n\
};\n\
template<class U>\n\
int f(U *p, U o)\n\
{\n\
	return p->template get<int>() + o.template get<long>() + p->U::template get<int>();\n\
}\n", sParsed.Print(&sz));
	sz.Kill();

	AssertPointer(sParsed.GetSymbol("p", 0), sParsed.GetSymbol("p", 1));
	AssertPointer(sParsed.GetSymbol("o", 0), sParsed.GetSymbol("o", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The pack in sizeof...( pack ) is looked up.  [expr.sizeof] 5
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserSizeofPack(void)
{
	SParsedText		sParsed;

	AssertTrue(sParsed.Init("\
template<class... Args> int Count(Args... a) { return sizeof...(Args) + sizeof...(a); }"));
	AssertInt(0, sParsed.iErrors);

	AssertInt(CPPSK_TypeParameter, sParsed.GetSymbol("Args", 0)->meKind);
	AssertInt(CPPTR_Reference, sParsed.GetRole("Args", 2));
	AssertPointer(sParsed.GetSymbol("Args", 0), sParsed.GetSymbol("Args", 2));
	AssertInt(CPPSK_Parameter, sParsed.GetSymbol("a", 0)->meKind);
	AssertInt(CPPTR_Reference, sParsed.GetRole("a", 1));
	AssertPointer(sParsed.GetSymbol("a", 0), sParsed.GetSymbol("a", 1));
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Errors in declarations: each reported once.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserDeclarationErrors(void)
{
	CChars	sz;

	//A template argument list without its '>'.
	sz.Init();
	AssertInt(1, ParseErrors("template<int N> struct Y { };\nY<1 x;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (2, 5): Expected '>'. [x]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(2, ParseErrors("template<int N> struct Y { };\nstruct S { Y<1 m; };\nvoid f(void) { Y<1 y; }\nint q;", "q", &sz));
	AssertString("\
Parsed.cpp (2, 16): Expected '>'. [m]\n\
Parsed.cpp (3, 20): Expected '>'. [y]\n", sz.Text());
	sz.Kill();

	//operator without an operator or a conversion type.
	sz.Init();
	AssertInt(1, ParseErrors("int operator ;\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 14): Expected an operator. [;]\n", sz.Text());
	sz.Kill();

	sz.Init();
	AssertInt(1, ParseErrors("struct S { operator const(); };\nint q;", "q", &sz));
	AssertString("Parsed.cpp (1, 26): Expected a type. [(]\n", sz.Text());
	sz.Kill();

	//Declarations that declare nothing.  [dcl.dcl] 5
	sz.Init();
	AssertInt(4, ParseErrors("const int;\nstatic;\nstruct S { int; };\nvoid f(void) { const int; }\nint q;", "q", &sz));
	AssertString("\
Parsed.cpp (1, 1): The declaration does not declare anything. [const]\n\
Parsed.cpp (2, 1): The declaration does not declare anything. [static]\n\
Parsed.cpp (3, 12): The declaration does not declare anything. [int]\n\
Parsed.cpp (4, 16): The declaration does not declare anything. [const]\n", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations without declarators that do declare something.
//
//////////////////////////////////////////////////////////////////////////
void TestCppParserDeclarationsWithoutDeclarators(void)
{
	SParsedText		sParsed;

	AssertTrue(sParsed.Init("\
struct A;\n\
struct B { int b; };\n\
enum E { e0 };\n\
enum class F : int;\n\
static union { int u; };\n\
template<class T> class C { friend T; friend class A; };\n\
template class C<int>;\n\
[[deprecated]];\n\
;\n\
void f(void) { struct L; enum G { g0 }; }"));
	AssertInt(0, sParsed.iErrors);
	sParsed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppParser(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestCppParserDeclarations();
	TestCppParserAmbiguities();
	TestCppParserClasses();
	TestCppParserTemplates();
	TestCppParserStatements();
	TestCppParserErrorRecovery();
	TestCppParserExceptions();
	TestCppParserCastsAndTypeQueries();
	TestCppParserModernDeclarations();
	TestCppParserRangeForNewAndDelete();
	TestCppParserOperators();
	TestCppParserTreeWalk();
	TestCppParserUnsupported();
	TestCppParserErrors();
	TestCppParserSkimmedErrors();
	TestCppParserCyclicUsingDirectives();
	TestCppParserClassAlignas();
	TestCppParserAttributedDeclarationStatements();
	TestCppParserAttributedStatements();
	TestCppParserTemplateMemberAccess();
	TestCppParserSizeofPack();
	TestCppParserDeclarationErrors();
	TestCppParserDeclarationsWithoutDeclarators();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

