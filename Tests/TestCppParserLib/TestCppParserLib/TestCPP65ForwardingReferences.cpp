#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "CppParserLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


int32 TestCPP816Run(const char* szSource, bool bLockstep);  //TestCPP816.cpp


struct STestForwardingReference
{
	const char*	szName;
	int32		iExpected;
	const char*	szSource;
};


//////////////////////////////////////////////////////////////////////////
//
// Passing arguments through a function template with T&& and Args&&, as
// OMalloc would: each program returns what real C++ makes it return (they
// were all checked with MSVC).
//
//////////////////////////////////////////////////////////////////////////
static const STestForwardingReference gasTestForwardingReferences[] =
{
	{
		"LValueInt", 5,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 5; return Make(&t, i); }\n"
	},
	{
		"ConstLValueInt", 6,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; const int i = 6; return Make(&t, i); }\n"
	},
	{
		"LValueLong", 12,
		"struct CThing { int m; void Init(long a) { m = (a == 70000L) ? 12 : 0; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; long l = 70000L; return Make(&t, l); }\n"
	},
	{
		"IntByReference", 24,
		"struct CThing { int m; void Init(int& r) { r = 24; m = r; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 1; Make(&t, i); return i; }\n"
	},
	{
		"LiteralInt", 7,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, 7); }\n"
	},
	{
		"Expression", 8,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int a = 3; return Make(&t, a + 5); }\n"
	},
	{
		"CallResult", 9,
		"int F() { return 9; }\n"
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, F()); }\n"
	},
	{
		"Enum", 10,
		"enum E { eA = 10 };\n"
		"struct CThing { int m; void Init(E a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, eA); }\n"
	},
	{
		"CastChar", 11,
		"struct CThing { int m; void Init(char a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, (char)11); }\n"
	},
	{
		"LiteralLong", 13,
		"struct CThing { int m; void Init(long a) { m = (a == 70000L) ? 13 : 0; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, 70000L); }\n"
	},
	{
		"ThreeTemporaries", 123,
		"struct CThing { int m; void Init(int a, int b, int c) { m = a * 100 + b * 10 + c; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 1; return Make(&t, i + 0, i + 1, i + 2); }\n"
	},
	{
		"StaticConstMember", 38,
		"struct C { static const int k = 38; };\n"
		"const int C::k;\n"
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, C::k); }\n"
	},
	{
		"PointerVariable", 14,
		"struct CThing { int m; void Init(int* a) { m = *a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 14; int* q = &i; return Make(&t, q); }\n"
	},
	{
		"AddressOf", 15,
		"struct CThing { int m; void Init(int* a) { m = *a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 15; return Make(&t, &i); }\n"
	},
	{
		"This", 16,
		"struct CThing { int m; void Init(CThing* a) { m = a->m; } int Go(); };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int CThing::Go() { CThing t; return Make(&t, this); }\n"
		"int main() { CThing t; t.m = 16; return t.Go(); }\n"
	},
	{
		"StringLiteral", 66,
		"struct CThing { int m; void Init(const char* a) { m = a[1]; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, \"AB\"); }\n"
	},
	{
		"TwoStringLengths", 134,
		"struct CThing { int m; void Init(const char* a) { m = a[1]; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, \"AB\") + Make(&t, \"CDE\"); }\n"
	},
	{
		"CharArray", 89,
		"struct CThing { int m; void Init(char* a) { a[0] = 'Y'; m = 1; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; char ac[3]; ac[0] = 'Z'; Make(&t, ac); return ac[0]; }\n"
	},
	{
		"IntArray", 17,
		"struct CThing { int m; void Init(int* a) { m = a[1]; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int ai[2]; ai[1] = 17; return Make(&t, ai); }\n"
	},
	{
		"FunctionName", 18,
		"int F() { return 18; }\n"
		"struct CThing { int m; void Init(int (*f)()) { m = f(); } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, F); }\n"
	},
	{
		"ConstPointerOverload", 12,
		"struct CThing { int m; void Init(const char* a) { m = 1; } void Init(char* a) { m = 2; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; char ac[2]; const char* sz = ac; char* pc = ac; int a = Make(&t, sz); return a * 10 + Make(&t, pc); }\n"
	},
	{
		"ObjectByValue", 21,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; CObj o(2); int r = Make(&t, o); return r * 10 + giCopies; }\n"
	},
	{
		"ObjectByConstReference", 20,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(const CObj& c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; CObj o(2); int r = Make(&t, o); return r * 10 + giCopies; }\n"
	},
	{
		"ObjectByReference", 23,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj& c) { c.v = 23; m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; CObj o(2); Make(&t, o); return o.v; }\n"
	},
	{
		"ConstObjectOverload", 12,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(const CObj& c) { m = 1; } void Init(CObj& c) { m = 2; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; const CObj k(1); CObj o(2); int a = Make(&t, k); return a * 10 + Make(&t, o); }\n"
	},
	{
		"TemporaryByValue", 25,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, CObj(25)); }\n"
	},
	{
		"TemporaryByConstReference", 26,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(const CObj& c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, CObj(26)); }\n"
	},
	{
		"TemporaryByReference", 27,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj& c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, CObj(27)); }\n"
	},
	{
		"TemporaryLifetime", 2801,
		"int giDestroyed = 0;\n"
		"int giAtInit = 9;\n"
		"struct CObj { int v; CObj(int i) { v = i; } ~CObj() { giDestroyed++; } };\n"
		"struct CThing { int m; void Init(const CObj& c) { giAtInit = giDestroyed; m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int r = Make(&t, CObj(28)); return r * 100 + giAtInit * 10 + giDestroyed; }\n"
	},
	{
		"EmptyPack", 30,
		"struct CThing { int m; void Init(void) { m = 30; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t); }\n"
	},
	{
		"MixedPack", 106,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(int a, const char* b, CObj& c) { m = a + b[1] + c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int i = 1; CObj o(2); return Make(&t, i + 1, \"xe\", o) + i; }\n"
	},
	{
		"ExplicitFirst", 32,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<class M, typename ... Args> int New(Args&& ... args) { M c; c.Init(args...); return c.m; }\n"
		"int main() { int i = 1; return New<CThing>(31) + New<CThing>(i); }\n"
	},
	{
		"TwoLayers", 67,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"template<typename ... Args> int Outer(Args&& ... args) { CThing t; return Make(&t, args...); }\n"
		"int main() { int i = 33; return Outer(i + 1) + Outer(i); }\n"
	},
	{
		"TwoLayersByReference", 36,
		"struct CThing { int m; void Init(int& r) { r = 36; m = r; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"template<typename ... Args> int Outer(Args&& ... args) { CThing t; return Make(&t, args...); }\n"
		"int main() { int i = 1; Outer(i); return i; }\n"
	},
	{
		"DeclaredThenDefined", 37,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args);\n"
		"int Use(CThing* p);\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int Use(CThing* p) { return Make(p, 37); }\n"
		"int main() { CThing t; return Use(&t); }\n"
	},
	{
		"MemberTemplate", 39,
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"struct CMaker { template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; } };\n"
		"int main() { CMaker c; CThing t; int i = 39; return c.Make(&t, i); }\n"
	},
	{
		"NestedCallVariable", 41,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"template<class M, typename ... Args> M Get(Args&& ... args) { M o(0); o.Set(args...); return o; }\n"
		"struct CSet { int v; CSet(int i) { v = i; } CSet(const CSet& c) { v = c.v; } void Set(int i) { v = i; } operator CObj() { return CObj(v); } };\n"
		"int main() { CThing t; int i = 41; CSet s = Get<CSet>(i); return Make(&t, CObj(s.v)); }\n"
	},
	{
		"NestedCallLiteral", 40,
		"int giCopies = 0;\n"
		"struct CObj { int v; CObj(int i) { v = i; } CObj(const CObj& c) { v = c.v; giCopies++; } };\n"
		"struct CThing { int m; void Init(CObj c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"struct CSet { int v; CSet(int i) { v = i; } CSet(const CSet& c) { v = c.v; } void Set(int i) { v = i; } };\n"
		"template<class M, typename ... Args> M Get(Args&& ... args) { M o(0); o.Set(args...); return o; }\n"
		"int main() { CThing t; CSet s = Get<CSet>(40); return Make(&t, CObj(s.v)); }\n"
	},
	{
		"NestedInArguments", 46,
		"struct CSet { int v; CSet(int i) { v = i; } CSet(const CSet& c) { v = c.v; } void Set(int i) { v = i; } };\n"
		"struct CThing { int m; void Init(CSet c) { m = c.v; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"template<class M, typename ... Args> M Get(Args&& ... args) { M o(0); o.Set(args...); return o; }\n"
		"int main() { CThing t; int i = 46; return Make(&t, Get<CSet>(i)); }\n"
	},
	{
		"PtrLikeClass", 42,
		"int giCopies = 0;\n"
		"struct CP { int* mp; CP() { mp = 0; } CP(int* p) { mp = p; } CP(CP& c) { mp = c.mp; giCopies++; } CP(const CP& c) { mp = c.mp; giCopies++; } };\n"
		"template<class M> struct P : public CP { P() : CP() {} P(int* p) : CP(p) {} P(CP& c) : CP(c) {} P(const CP& c) : CP(c) {} P(P& c) : CP(c) {} P(const P& c) : CP(c) {} };\n"
		"struct CThing { int m; void Init(P<int> a, P<char> b) { m = *a.mp + *b.mp; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; int x = 40; int y = 2; P<int> a(&x); int r = Make(&t, a, P<char>(&y)); return r + (giCopies == 2 ? 0 : 1000); }\n"
	},
	{
		"PlainConstRefLiteral", 47,
		"int F(const int& i) { return i; }\n"
		"int main() { return F(47); }\n"
	},
	{
		"PlainConstRefExpression", 48,
		"int F(const int& i) { return i; }\n"
		"int main() { int a = 40; return F(a + 8); }\n"
	},
	{
		"PlainConstRefPointer", 49,
		"int F(const char* const& p) { return p[0]; }\n"
		"int main() { return F(\"1\"); }\n"
	},
	{
		"PlainArrayToPointer", 50,
		"int F(int* p) { return p[1]; }\n"
		"int main() { int ai[2]; int (&r)[2] = ai; ai[1] = 50; return F(r); }\n"
	},
	{
		"StaticConstRead", 61,
		"struct C { static const int k = 61; };\n"
		"const int C::k;\n"
		"int main() { return C::k; }\n"
	},
	{
		"StaticConstInOtherExpression", 62,
		"struct C { static const int k = 61; };\n"
		"const int C::k;\n"
		"int main() { int i = 1; return C::k + i; }\n"
	},
	{
		"ConstPointerRefToArray", 63,
		"int F(int* const& p) { return p[1]; }\n"
		"int main() { int ai[2]; ai[1] = 63; return F(ai); }\n"
	},
	{
		"PlainConstRefConverted", 51,
		"int F(const long& l) { return (l == 3) ? 51 : 0; }\n"
		"int main() { char c = 3; return F(c); }\n"
	},
	{
		"PlainConstRefLongLong", 52,
		"int F(const long long& l) { return (l == 5000000000LL) ? 52 : 0; }\n"
		"int main() { return F(5000000000LL); }\n"
	},
	{
		"PlainConstRefInLoop", 15,
		"int F(const int& i) { return i; }\n"
		"int main() { int t = 0; for (int i = 0; i < 5; i++) { t += F(i + 1); } return t; }\n"
	},
	{
		"PlainConstRefRecursive", 21,
		"int F(const int& n) { return (n == 0) ? 0 : n + F(n - 1); }\n"
		"int main() { return F(6); }\n"
	},
	{
		"PlainConstRefConditional", 53,
		"int F(const int& i) { return i; }\n"
		"int main() { int b = 1; return F(b ? 53 : 54); }\n"
	},
	{
		"PlainConstRefVariable", 55,
		"int F(const int& i) { return i; }\n"
		"int main() { int v = 55; return F(v); }\n"
	},
	{
		"PlainConstRefSeesVariable", 56,
		"int giV = 1;\n"
		"int F(const int& i) { giV = 56; return i; }\n"
		"int main() { return F(giV); }\n"
	},
	{
		"ConstructorConstRef", 57,
		"struct S { int v; S(const int& i) { v = i; } };\n"
		"int main() { S s(57); return s.v; }\n"
	},
	{
		"MemberConstRef", 58,
		"struct S { int v; void Set(const int& i) { v = i; } };\n"
		"int main() { S s; s.Set(50 + 8); return s.v; }\n"
	},
	{
		"LiteralPrefersConstRef", 12,
		"int F(int& i) { return 1; }\n"
		"int F(const int& i) { return 2; }\n"
		"int main() { int v = 0; return F(v) * 10 + F(5); }\n"
	},
	{
		"LiteralSkipsReferenceOverload", 59,
		"int F(int& i) { return 1; }\n"
		"int F(long l) { return 59; }\n"
		"int main() { return F(5); }\n"
	},
	{
		"ScalarTemporaryByReference", 60,
		"struct CThing { int m; void Init(int& r) { m = r + 60; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"int main() { CThing t; return Make(&t, 0); }\n"
	},
	{
		"SingleParameter", 43,
		"template<class T> int One(T&& t) { return t; }\n"
		"int main() { int i = 41; return One(i) + One(2); }\n"
	},
	{
		"ReferenceCollapsing", 9,
		"template<class T> int F(T&& t) { T x = t; x = 9; return t; }\n"
		"int main() { int i = 1; return F(i); }\n"
	},
	{
		"ForwardPattern", 44,
		"template<class T> struct R { typedef T Type; };\n"
		"template<class T> struct R<T&> { typedef T Type; };\n"
		"template<class T> T&& Forward(typename R<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct CThing { int m; void Init(int a) { m = a; } };\n"
		"template<typename ... Args> int Make(CThing* p, Args&& ... args) { p->Init(args...); return p->m; }\n"
		"template<typename ... Args> int F(CThing* p, Args&& ... args) { p->Init(Forward<Args>(args)...); return p->m; }\n"
		"int main() { CThing t; return F(&t, 44); }\n"
	},
};


//////////////////////////////////////////////////////////////////////////
//
// Appends the program's name if the compiler can't do it yet: (C) it didn't
// compile, (W) it ran and returned the wrong value.
//
//////////////////////////////////////////////////////////////////////////
static void TestForwardingReference(const STestForwardingReference* psCase, ECPPTarget eTarget, CChars* pszNotDone)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;
	bool			bCompiled;
	int32			iResult;

	cBuilder.Init();
	szAssembly.Init();
	szErrors.Init();
	bCompiled = CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", psCase->szSource, false, NULL, eTarget) && szErrors.Empty();
	if (!bCompiled)
	{
		pszNotDone->Append(psCase->szName);
		pszNotDone->Append("(C) ");
		if (getenv("TEST_CPP65_FORWARDING_VERBOSE"))
		{
			fprintf(stderr, "%s: %s\n", psCase->szName, szErrors.Text());
		}
	}
	else
	{
		iResult = (eTarget == CPPT_W65C816) ? TestCPP816Run(psCase->szSource, false) : TestCPP65Main(psCase->szSource);
		if (iResult != psCase->iExpected)
		{
			pszNotDone->Append(psCase->szName);
			pszNotDone->Append("(W) ");
			if (getenv("TEST_CPP65_FORWARDING_VERBOSE"))
			{
				fprintf(stderr, "%s: expected %d, got %d\n", psCase->szName, (int)psCase->iExpected, (int)iResult);
			}
		}
	}
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestForwardingReferences(ECPPTarget eTarget, const char* szExpectedNotDone)
{
	CChars	szNotDone;
	size	i;

	szNotDone.Init();
	for (i = 0; i < sizeof(gasTestForwardingReferences) / sizeof(gasTestForwardingReferences[0]); i++)
	{
		TestForwardingReference(&gasTestForwardingReferences[i], eTarget, &szNotDone);
	}
	AssertString(szExpectedNotDone, szNotDone.Text());
	szNotDone.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Only a const reference takes a value with no address, and a reference to
// a pointer isn't an array's address.
//
//////////////////////////////////////////////////////////////////////////
static void TestForwardingReferenceRejected(ECPPTarget eTarget)
{
	CCPP65Builder	cBuilder;
	CChars			szAssembly;
	CChars			szErrors;

	cBuilder.Init();
	szAssembly.Init();
	szErrors.Init();
	CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", "int F(int& i) { return i; }\nint main() { return F(5); }\n", false, NULL, eTarget);
	AssertFalse(szErrors.Empty());
	szErrors.Clear();
	CCPP65Builder::Generate(&szAssembly, &szErrors, cBuilder.GetIncludeDirectories(), "Test.cpp", "int F(int*& p) { return *p; }\nint main() { int ai[2]; ai[0] = 1; return F(ai); }\n", false, NULL, eTarget);
	AssertFalse(szErrors.Empty());
	szErrors.Kill();
	szAssembly.Kill();
	cBuilder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What the compiler can't do yet is listed here for each target: take a
// program off its list as the compiler does it.  ReferenceCollapsing and
// ForwardPattern need rvalue references as a type of their own (B10 in
// CppParserTodo.txt), which passing arguments through doesn't.  So does
// ScalarTemporaryByReference: T&& of a value is const here.
//
//////////////////////////////////////////////////////////////////////////
#define TEST_CPP65_FORWARDING_NOT_DONE		"ScalarTemporaryByReference(C) ReferenceCollapsing(W) ForwardPattern(C) "
#define TEST_CPP816_FORWARDING_NOT_DONE		"ScalarTemporaryByReference(C) ReferenceCollapsing(W) ForwardPattern(C) "

void TestCPP65ForwardingReferences(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestForwardingReferences(CPPT_65C64, TEST_CPP65_FORWARDING_NOT_DONE);
	TestForwardingReferences(CPPT_W65C816, TEST_CPP816_FORWARDING_NOT_DONE);
	TestForwardingReferenceRejected(CPPT_65C64);
	TestForwardingReferenceRejected(CPPT_W65C816);

	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

