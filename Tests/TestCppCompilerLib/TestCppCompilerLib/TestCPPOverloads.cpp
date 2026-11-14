#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//////////////////////////////////////////////////////////////////////////
//
// The errors of compiling szSource have szExpected in them.
//
//////////////////////////////////////////////////////////////////////////
static void AssertOverloadError(const char* szExpected, const char* szSource)
{
	CChars	szAssembly;
	CChars	szErrors;

	szAssembly.Init();
	szErrors.Init();
	TestCPP65Generate(&szAssembly, &szErrors, szSource, true);
	AssertTrue(szErrors.Contains(szExpected));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The best candidate is the one whose every argument converts at least as well
// as any other's, and one better, wherever it is among them; with none, the
// call is ambiguous, and with no candidate that takes the arguments nothing
// matches.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloadsBest(void)
{
	AssertInt(2, TestCPP65Main("int F(char c) { return 1; }\nint F(int a) { return 2; }\nint F(long long a) { return 3; }\nint main() { return F(1000); }"));
	AssertInt(2, TestCPP65Main("int F(long long a) { return 3; }\nint F(char c) { return 1; }\nint F(int a) { return 2; }\nint main() { return F(1000); }"));
	AssertInt(2, TestCPP65Main("int F(int a) { return 2; }\nint F(long long a) { return 3; }\nint F(char c) { return 1; }\nint main() { return F(1000); }"));
	AssertInt(2, TestCPP65Main("int F(int a, long long b) { return 1; }\nint F(int a, int b) { return 2; }\nint F(long long a, long long b) { return 3; }\nint main() { return F(1, 1); }"));
	AssertInt(4, TestCPP65Main("int F(int a, long long b) { return 1; }\nint F(long long a, int b) { return 2; }\nint F(char a, char b) { return 3; }\nint F(int a, int b) { return 4; }\nint main() { return F(1, 1); }"));
	AssertInt(13, TestCPP65Main("int F(int a, int b = 3) { return a + b; }\nint F(char* p) { return 1; }\nint main() { return F(10); }"));
	AssertInt(2, TestCPP65Main("int F(int a, ...) { return 1; }\nint F(int a, int b) { return 2; }\nint main() { return F(1, 2); }"));
	AssertInt(1, TestCPP65Main("int F(int a, ...) { return 1; }\nint F(int a, char* p) { return 2; }\nint main() { return F(1, 2); }"));

	AssertOverloadError("The call is ambiguous", "int F(int a, long long b) { return 1; }\nint F(long long a, int b) { return 2; }\nint main() { return F(1, 1); }");
	AssertOverloadError("The call is ambiguous", "int F(int a, long long b) { return 1; }\nint F(long long a, int b) { return 2; }\nint F(char a, char b) { return 3; }\nint main() { return F(1, 1); }");
	AssertOverloadError("The call is ambiguous", "int F(char a, char b) { return 3; }\nint F(int a, long long b) { return 1; }\nint F(long long a, int b) { return 2; }\nint main() { return F(1, 1); }");
	AssertOverloadError("No function matches", "struct S { int i; };\nint F(int a) { return 1; }\nint F(int a, int b) { return 2; }\nint main() { S s; s.i = 1; return F(s); }");
}


//////////////////////////////////////////////////////////////////////////
//
// Between candidates whose arguments convert equally well, the one that adds
// less const to what a pointer or a reference refers to; a temporary binds
// better to a const reference.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloadsQualifiers(void)
{
	AssertInt(12, TestCPP65Main("int F(int* p) { return 1; }\nint F(const int* p) { return 2; }\nint main() { int a = 1; const int b = 2; return F(&a) * 10 + F(&b); }"));
	AssertInt(12, TestCPP65Main("int F(const int* p) { return 2; }\nint F(int* p) { return 1; }\nint main() { int a = 1; const int b = 2; return F(&a) * 10 + F(&b); }"));
	AssertInt(12, TestCPP65Main("struct S { int i; };\nint F(S& r) { return 1; }\nint F(const S& r) { return 2; }\nint main() { S a; a.i = 1; const S& b = a; return F(a) * 10 + F(b); }"));
	AssertInt(2, TestCPP65Main("struct S { int i; };\nS Make() { S s; s.i = 1; return s; }\nint F(S& r) { return 1; }\nint F(const S& r) { return 2; }\nint main() { return F(Make()); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// The object a member function is called on chooses between a const function
// and one that isn't: a const object takes only the const one, another prefers
// the one that isn't.  So with an operator, and through a pointer.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloadsCalledObject(void)
{
	AssertInt(12, TestCPP65Main("struct S { int i; int G() { return 1; } int G() const { return 2; } };\nint main() { S s; s.i = 0; const S& c = s; return s.G() * 10 + c.G(); }"));
	AssertInt(12, TestCPP65Main("struct S { int i; int G() const { return 2; } int G() { return 1; } };\nint main() { S s; s.i = 0; const S* p = &s; S* q = &s; return q->G() * 10 + p->G(); }"));
	AssertInt(12, TestCPP65Main("struct S { int i; int operator[](int a) { return 1; } int operator[](int a) const { return 2; } };\nint main() { S s; s.i = 0; const S& c = s; return s[0] * 10 + c[0]; }"));
	AssertInt(21, TestCPP65Main("struct S { int i; int G() { return 1; } int G() const { return 2; } int H() const { return G(); } int I() { return G(); } };\nint main() { S s; s.i = 0; return s.H() * 10 + s.I(); }"));
	AssertInt(2, TestCPP65Main("struct S { int i; int G(long long a) { return 1; } int G(int a) const { return 2; } };\nint main() { S s; s.i = 0; return s.G(1); }"));

	AssertOverloadError("No function matches", "struct S { int i; int G() { return 1; } };\nint main() { S s; s.i = 0; const S& c = s; return c.G(); }");
}


//////////////////////////////////////////////////////////////////////////
//
// A class's functions of a name hide its bases', which are looked in only when
// it has none; a using declaration makes a base's its overloads.  A conversion
// function is chosen by how well its result converts.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloadsBasesAndConversions(void)
{
	AssertInt(5, TestCPP65Main("struct B { int i; int F(int a) { return 5; } };\nstruct D : B { int G() { return F(1); } };\nint main() { D d; d.i = 0; return d.G(); }"));
	AssertInt(6, TestCPP65Main("struct B { int i; int F(int a) { return 5; } };\nstruct D : B { int F(char c) { return 6; } };\nint main() { D d; d.i = 0; return d.F(1000); }"));
	AssertInt(5, TestCPP65Main("struct B { int i; int F(int a) { return 5; } };\nstruct D : B { using B::F; int F(char c) { return 6; } };\nint main() { D d; d.i = 0; return d.F(1000); }"));
	AssertInt(7, TestCPP65Main("struct S { int i; operator int() { return 7; } };\nint main() { S s; s.i = 0; int a = s; return a; }"));
	AssertInt(8, TestCPP65Main("struct S { int i; operator char*() { return 0; } operator int() { return 8; } };\nint main() { S s; s.i = 0; int a = s; return a; }"));
}


//////////////////////////////////////////////////////////////////////////
//
// Of the function templates of a name, the one the call can use; of those, the
// one its arguments convert to best; then the one whose const is the object's.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloadsTemplates(void)
{
	AssertInt(12, TestCPP65Main("struct S { int i; template <class M> int Get(int a) { return 1; } template <class M> int Get(const char* sz) { return 2; } };\nint main() { S s; s.i = 0; return s.Get<int>(1) * 10 + s.Get<int>(\"a\"); }"));
	AssertInt(12, TestCPP65Main("struct S { int i; template <class T> int G(T t) { return 1; } template <class T> int G(T t) const { return 2; } };\nint main() { S s; s.i = 0; const S& c = s; return s.G(1) * 10 + c.G(1); }"));
	AssertInt(21, TestCPP65Main("struct S { int i; template <class T> int G(T t) const { return 2; } template <class T> int G(T t) { return 1; } };\nint main() { S s; s.i = 0; const S& c = s; return c.G(1) * 10 + s.G(1); }"));
	AssertInt(12, TestCPP65Main("template <class T> int F(T t, int a) { return 1; }\ntemplate <class T> int F(T t, const char* sz) { return 2; }\nint main() { return F(1, 1) * 10 + F(1, \"a\"); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// A function template's arguments are deduced from a call's: from a type, what
// a pointer or a reference refers to, a class template's instance (or a class
// derived from one), a pointer to a member function's class, a constant, and a
// pack; those given explicitly aren't deduced.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPDeduction(void)
{
	AssertInt(5, TestCPP65Main("template <class T> T Id(T t) { return t; }\nint main() { return Id(5); }"));
	AssertInt(31, TestCPP65Main("struct S { char a[3]; };\ntemplate <class T> int Size(T* p) { return sizeof(T); }\nint main() { S s; char c = 0; s.a[0] = 0; return Size(&s) * 10 + Size(&c); }"));
	AssertInt(31, TestCPP65Main("struct S { char a[3]; };\ntemplate <class T> int Size(const T& r) { return sizeof(T); }\nint main() { S s; char c = 0; s.a[0] = 0; return Size(s) * 10 + Size(c); }"));
	AssertInt(13, TestCPP65Main("struct S { char a[3]; };\ntemplate <class T, class U> int Size(T t, U u) { return sizeof(T) * 10 + sizeof(U); }\nint main() { S s; char c = 0; s.a[0] = 0; return Size(c, s); }"));
	AssertInt(3, TestCPP65Main("struct S { char a[3]; };\ntemplate <class T> struct Box { T t; };\ntemplate <class T> int Size(Box<T>& b) { return sizeof(T); }\nint main() { Box<S> b; b.t.a[0] = 0; return Size(b); }"));
	AssertInt(3, TestCPP65Main("struct S { char a[3]; };\ntemplate <class T> struct Box { T t; };\nstruct D : Box<S> { int i; };\ntemplate <class T> int Size(Box<T>& b) { return sizeof(T); }\nint main() { D d; d.i = 0; return Size(d); }"));
	AssertInt(7, TestCPP65Main("template <int N> struct A { int i; };\ntemplate <int N> int Count(A<N>& a) { return N; }\nint main() { A<7> a; a.i = 0; return Count(a); }"));
	AssertInt(4, TestCPP65Main("struct S { int i; int M() { return 4; } };\ntemplate <class C> int Call(C* p, int (C::*f)()) { return (p->*f)(); }\nint main() { S s; s.i = 0; return Call(&s, &S::M); }"));
	AssertInt(102, TestCPP65Main("int Take(int a, char c, int b) { return a + c + b; }\ntemplate <class... A> int Forward(A... a) { return Take(a...); }\nint main() { return Forward(1, 'c', 2); }"));
	AssertInt(44, TestCPP65Main("template <class R, class T> R Cast(T t) { return (R)t; }\nint main() { return Cast<char>(300); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// The specialisation of a class template an instance uses is the one whose
// pattern matches its arguments and names most of their structure; a full
// specialisation before any partial one, and two that match as much are
// ambiguous.
//
//////////////////////////////////////////////////////////////////////////
void TestCPPSpecialisationMatch(void)
{
	AssertInt(1234, TestCPP65Main("template <class T> struct K { enum { V = 1 }; };\ntemplate <class T> struct K<T*> { enum { V = 2 }; };\ntemplate <> struct K<int> { enum { V = 3 }; };\ntemplate <class T> struct K<const T> { enum { V = 4 }; };\nint main() { return K<char>::V * 1000 + K<char*>::V * 100 + K<int>::V * 10 + K<const char>::V; }"));
	AssertInt(123, TestCPP65Main("template <class T, class U> struct P { enum { V = 1 }; };\ntemplate <class T> struct P<T, T> { enum { V = 2 }; };\ntemplate <class T> struct P<T, int> { enum { V = 3 }; };\nint main() { return P<char, long long>::V * 100 + P<char, char>::V * 10 + P<char, int>::V; }"));
	AssertInt(39, TestCPP65Main("template <int N> struct F { enum { V = N }; };\ntemplate <> struct F<0> { enum { V = 9 }; };\nint main() { return F<3>::V * 10 + F<0>::V; }"));
	AssertInt(15, TestCPP65Main("template <class T> struct Box { T t; };\ntemplate <class T> struct K { enum { V = 1 }; };\ntemplate <class T> struct K<Box<T> > { enum { V = 5 }; };\nint main() { return K<int>::V * 10 + K<Box<int> >::V; }"));
	AssertInt(23, TestCPP65Main("template <class T> struct K { enum { V = 1 }; };\ntemplate <class T> struct K<T*> { enum { V = 2 }; };\ntemplate <class T> struct K<const T*> { enum { V = 3 }; };\nint main() { return K<char*>::V * 10 + K<const char*>::V; }"));

	AssertOverloadError("More than one specialisation of the class template matches equally", "template <class T, class U> struct P { enum { V = 1 }; };\ntemplate <class T> struct P<T, int> { enum { V = 2 }; };\ntemplate <class T> struct P<int, T> { enum { V = 3 }; };\nint main() { P<int, int> p; return sizeof(p); }");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPPOverloads(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();
	C65C64InstructionFactory::GetInstance()->Init();

	TestCPPOverloadsBest();
	TestCPPOverloadsQualifiers();
	TestCPPOverloadsCalledObject();
	TestCPPOverloadsBasesAndConversions();
	TestCPPOverloadsTemplates();
	TestCPPDeduction();
	TestCPPSpecialisationMatch();

	C65C64InstructionFactory::GetInstance()->Kill();
	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

