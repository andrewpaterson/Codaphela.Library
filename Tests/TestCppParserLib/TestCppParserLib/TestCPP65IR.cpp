#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "MicroprocessorLib/65C64InstructionFactory.h"
#include "CppParserLib/CPP65IRFrontEnd.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//////////////////////////////////////////////////////////////////////////
//
// Runs szSource with the generator writing function bodies as IR, and
// checks uiWritten of its functions were (not for a float result).
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65IRMain(const char* szSource, size uiWritten, bool bFloat = false)
{
	size	uiBefore;
	int32	iResult;

	CCPP65IRFrontEnd::sbPrint = getenv("CPP65_PRINT_IR") != NULL;
	uiBefore = CCPP65IRFrontEnd::suiWritten;
	iResult = bFloat ? TestCPP65MainFloat(szSource) : TestCPP65Main(szSource);
	if (!bFloat)
	{
		if (uiWritten != CCPP65IRFrontEnd::suiWritten - uiBefore)
		{
			fprintf(stderr, "Left at %s line %d %s in %.80s\n", CCPP65IRFrontEnd::sszLastUnsupportedFile, CCPP65IRFrontEnd::siLastUnsupportedLine, CCPP65IRFrontEnd::sszLastUnsupported, szSource);
		}
		AssertSize(uiWritten, CCPP65IRFrontEnd::suiWritten - uiBefore);
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRArithmetic(void)
{
	AssertInt(42, TestCPP65IRMain("int main() { return 42; }", 1));
	AssertInt(55, TestCPP65IRMain(
		"int Fibonacci(int n) { if (n < 2) return n; return Fibonacci(n - 1) + Fibonacci(n - 2); }\n"
		"int main() { return Fibonacci(10); }", 2));
	AssertInt(-56 + 200 + 7, TestCPP65IRMain(
		"int main() { char c = (char)200; unsigned char u = 200; short s = 7; return c + u + s; }", 1));
	AssertInt(-3 + -1 * 100 + 2147483644 / 1000, TestCPP65IRMain(
		"int main() { int a = -7; unsigned b = (unsigned)a; return a / 2 + (a % 2) * 100 + (int)(b / 2 / 1000); }", 1));
	AssertInt(1, TestCPP65IRMain(
		"int main() { long long a = 3000000000LL; long long b = a * 3; return (b == 9000000000LL) && (b > a) && (-b < 0); }", 1));
	AssertInt(0x5A ^ (0x0F << 4), TestCPP65IRMain(
		"int main() { int x = 0x5A; int y = 0x0F; x ^= y << 4; return x; }", 1));
	AssertInt(-2, TestCPP65IRMain("int main() { int x = -8; return x >> 2; }", 1));
	AssertInt(10 + 9 + 11 + 11, TestCPP65IRMain(
		"int main() { int i = 10; int a = i++; int b = i--; int c = ++i; return a + (b - 2) + c + i; }", 1));

	//Signed / and % by powers of 2 are shifts and adds (negative values round towards 0); the sum was worked out natively.
	AssertInt(7, TestCPP65IRMain(
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
		"}\n", 3));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRControl(void)
{
	AssertInt(5050, TestCPP65IRMain(
		"int main() { int s = 0; for (int i = 1; i <= 100; i++) { s += i; } return s; }", 1));
	AssertInt(1 + 3 + 5 + 7 + 9, TestCPP65IRMain(
		"int main() { int s = 0; int i = 0; while (true) { i++; if (i > 10) break; if ((i & 1) == 0) continue; s += i; } return s; }", 1));
	AssertInt(3, TestCPP65IRMain(
		"int main() { int n = 0; do { n++; } while (n < 3); return n; }", 1));
	AssertInt(20 + 30 + 99, TestCPP65IRMain(
		"int Pick(int x) { switch (x) { case 1: return 10; case 2: return 20; case 3: case 4: return 30; default: return 99; } }\n"
		"int main() { return Pick(2) + Pick(4) + Pick(7); }", 2));
	AssertInt(4, TestCPP65IRMain(
		"int main() { int n = 0; again: n++; if (n < 4) goto again; return n; }", 1));
	AssertInt(1 * 100 + 0 * 10 + 1, TestCPP65IRMain(
		"bool Between(int x, int a, int b) { return (x >= a) && !(x > b); }\n"
		"int main() { return Between(5, 1, 9) * 100 + Between(0, 1, 9) * 10 + (Between(9, 1, 9) || Between(0, 0, 0)); }", 2));
	AssertInt(7 + 3, TestCPP65IRMain(
		"int Max(int a, int b) { return a > b ? a : b; }\n"
		"int main() { int x = 3; return Max(x, 7) + (x ? x : 100); }", 2));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRMemory(void)
{
	AssertInt(10 + 20 + 30 + 40, TestCPP65IRMain(
		"int gTable[4] = { 10, 20, 30, 40 };\n"
		"int Sum(int* p, int n) { int s = 0; for (int i = 0; i < n; i++) s += p[i]; return s; }\n"
		"int main() { return Sum(gTable, 4); }", 2));
	AssertInt(5, TestCPP65IRMain(
		"int Length(const char* sz) { const char* p = sz; while (*p) p++; return (int)(p - sz); }\n"
		"int main() { return Length(\"hello\"); }", 2));
	AssertInt(1 + 2 + 3 + 0 + 0 + 'b', TestCPP65IRMain(
		"int main() { int a[5] = { 1, 2, 3 }; char s[4] = \"ab\"; int t = 0; for (int i = 0; i < 5; i++) t += a[i]; return t + s[1] + s[3]; }", 1));
	AssertInt(30, TestCPP65IRMain(
		"void Swap(int* a, int* b) { int t = *a; *a = *b; *b = t; }\n"
		"void Twice(int& x) { x *= 2; }\n"
		"int main() { int a = 5; int b = 10; Swap(&a, &b); Twice(b); return a * 2 + b; }", 3));
	AssertInt(3, TestCPP65IRMain(
		"int Count() { static int n = 0; n++; return n; }\n"
		"int main() { Count(); Count(); return Count(); }", 2));
	AssertInt(6, TestCPP65IRMain(
		"int gTotal;\n"
		"void Add(int x) { gTotal += x; }\n"
		"int main() { Add(1); Add(2); Add(3); return gTotal; }", 2));
	AssertInt(2 * 3 * 4, TestCPP65IRMain(
		"int m[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };\n"
		"int main() { int* p = &m[1][0]; return m[0][1] * m[0][2] * p[0]; }", 1));

	//In a[i] the array is worked out before the index.  [expr.sub] 1
	AssertInt(12, TestCPP65IRMain(
		"int gaiTable[3] = { 7, 8, 9 }; int giOrder = 0;\n"
		"int* Table() { giOrder = giOrder * 10 + 1; return gaiTable; }\n"
		"int Index() { giOrder = giOrder * 10 + 2; return 1; }\n"
		"int main() { Table()[Index()] = 0; return giOrder; }", 3));

	//In a = b the right is worked out before the left, an object's too.  [expr.ass] 1
	AssertInt(21, TestCPP65IRMain(
		"struct SPair { int a; int b; }; SPair gsLeft; SPair gsRight; int giOrder = 0;\n"
		"SPair& Left() { giOrder = giOrder * 10 + 1; return gsLeft; }\n"
		"SPair& Right() { giOrder = giOrder * 10 + 2; return gsRight; }\n"
		"int main() { Left() = Right(); return giOrder; }", 5));

	//The right before the left for an object's operator= and op= too; the object before the arguments in a call of
	//its member function, and the left before the right for <<.  [expr.call] 8, [over.match.oper] 2
	AssertInt(21 * 1000000 + 21 * 10000 + 12 * 100 + 12, TestCPP65IRMain(
		"int giOrder = 0;\n"
		"struct CValue { int mi; CValue& operator=(int i) { mi = i; return *this; } CValue& operator+=(int i) { mi += i; return *this; }\n"
		"int Set(int i) { mi = i; return i; } int operator<<(int i) { return mi << i; } };\n"
		"CValue gcValue;\n"
		"CValue& Left() { giOrder = giOrder * 10 + 1; return gcValue; }\n"
		"int Right() { giOrder = giOrder * 10 + 2; return 1; }\n"
		"int Order() { int i = giOrder; giOrder = 0; return i; }\n"
		"int main() { Left() = Right(); int a = Order(); Left() += Right(); int b = Order(); Left().Set(Right()); int c = Order(); Left() << Right(); int d = Order();\n"
		"return a * 1000000 + b * 10000 + c * 100 + d; }", 9));

	//A call through a pointer to a function or to a member function works out the callee (its object then the
	//pointer) before the arguments.  [expr.call] 8
	AssertInt(13 * 1000 + 123, TestCPP65IRMain(
		"int giOrder = 0;\n"
		"struct CValue { int mi; int Add(int i) { return mi + i; } };\n"
		"typedef int (*FTwice)(int);\n"
		"typedef int (CValue::*FAdd)(int);\n"
		"CValue gcValue;\n"
		"int Twice(int i) { return i * 2; }\n"
		"FTwice Pick() { giOrder = giOrder * 10 + 1; return Twice; }\n"
		"CValue& Object() { giOrder = giOrder * 10 + 1; return gcValue; }\n"
		"FAdd Member() { giOrder = giOrder * 10 + 2; return &CValue::Add; }\n"
		"int Argument() { giOrder = giOrder * 10 + 3; return 1; }\n"
		"int Order() { int i = giOrder; giOrder = 0; return i; }\n"
		"int main() { Pick()(Argument()); int a = Order(); (Object().*Member())(Argument()); int b = Order(); return a * 1000 + b; }", 9));
}


//////////////////////////////////////////////////////////////////////////
//
// Variadic functions read their arguments from argaddr; function pointers;
// floats.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRCalls(void)
{
	AssertInt(1 + 20 + 44, TestCPP65IRMain(
		"#include <stdarg.h>\n"
		"int SumAll(int count, ...) { va_list ap; va_start(ap, count); int s = 0; for (int i = 0; i < count; i++) s += va_arg(ap, int); va_end(ap); return s; }\n"
		"int main() { return SumAll(3, 1, 20, (char)300); }", 2));
	AssertInt(12 + 3, TestCPP65IRMain(
		"int Double(int x) { return x + x; }\n"
		"int Apply(int (*f)(int), int x) { return f(x); }\n"
		"int main() { int (*p)(int) = &Double; return Apply(p, 6) + p(1) + 1; }", 3));
	AssertInt(15, TestCPP65IRMain(
		"float Scale(float x, float s) { return x * s; }\n"
		"int main() { float a = 1.5f + 2.25f; double d = Scale(a, 4.0f); return (int)d; }", 0, true));
}


//////////////////////////////////////////////////////////////////////////
//
// Member functions written as IR, called from a main the generator writes
// (it has objects), and the other way round.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRMembers(void)
{
	AssertInt(10, TestCPP65IRMain(
		"struct CPoint { int x; int y; int Sum() { return x + y; } int Twice() { return Sum() * 2; } void Move(int d) { x += d; this->y += d; } };\n"
		"int main() { CPoint p; p.x = 1; p.y = 2; p.Move(1); CPoint* q = &p; return q->x + q->y + q->Twice() - p.Sum() + 7 * 2 - p.Sum() * 2 + 0 * 0 + p.Twice() - 2 * 7; }", 4));
	AssertInt(10 + 20 + 2, TestCPP65IRMain(
		"struct CShape { virtual int Area() { return 1; } int Twice() { return Area() * 2; } };\n"
		"struct CSquare : CShape { int miSide; virtual int Area() { return miSide * miSide; } };\n"
		"struct CLine : CShape { virtual int Area() { return 20; } };\n"
		"int Area(CShape* p) { return p->Area(); }\n"
		"int main() { CSquare s; s.miSide = 3; CLine l; CShape b; return Area(&s) + 1 + Area(&l) + b.Twice() + (s.Twice() - 18); }", 9));
	AssertInt(100 + 2 + 3, TestCPP65IRMain(
		"struct CA { int a; int GetA() { return a; } };\n"
		"struct CB { int b; int GetB() { return b; } };\n"
		"struct CC : CA, CB { int c; int All() { return GetA() + GetB() * 0 + c + b; } };\n"
		"CB* AsB(CC* p) { return p; }\n"
		"int main() { CC c; c.a = 100; c.b = 2; c.c = 3; CB* pb = AsB(&c); CC* n = 0; return c.All() + (AsB(n) == 0 ? 0 : 1000) + pb->GetB() - 2; }", 5));
	AssertInt(5, TestCPP65IRMain(
		"struct CPtr { int* mp; int* operator&() { return mp; } };\n"
		"struct CHolder { CPtr mcPtr; int* Get() { return &mcPtr; } };\n"
		"int main() { int x = 5; CHolder h; h.mcPtr.mp = &x; return *h.Get(); }", 3));
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ as a string; objects that need no constructor as locals,
// references to classes (a derived object bound to a reference to its
// second base is moved to it) and functions returning them.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRObjectPlaces(void)
{
	AssertInt('M' + 4, TestCPP65IRMain(
		"int Length(const char* sz) { int i = 0; while (sz[i]) i++; return i; }\n"
		"int Mine() { const char* sz = __func__; return sz[0] + Length(sz); }\n"
		"int main() { return Mine() + __func__[2] - 'i'; }", 3));
	AssertInt(3 + 40 + 7, TestCPP65IRMain(
		"struct CCounter { int miCount; void Init() { miCount = 0; } void Add(int i) { miCount += i; } int Get() const { return miCount; } };\n"
		"struct CPair { CCounter mcA; CCounter mcB; CCounter& First() { return mcA; } };\n"
		"int Total(const CCounter& c, CCounter& d) { d.Add(1); return c.Get() + d.Get(); }\n"
		"int main() { CCounter c; CPair p; CCounter ac[2]; c.Init(); c.Add(3); p.mcA.Init(); p.mcB.Init(); p.First().Add(40); ac[1].Init(); ac[1].Add(5);\n"
		"\treturn c.Get() + p.mcA.Get() + Total(ac[1], ac[1]) - 5; }", 6));
	AssertInt(2 + 30, TestCPP65IRMain(
		"struct CA { int a; };\n"
		"struct CB { int b; int GetB() const { return b; } };\n"
		"struct CC : CA, CB { int c; };\n"
		"int GetB(const CB& b) { return b.GetB(); }\n"
		"CB& AsB(CC& c) { return c; }\n"
		"int main() { CC c; c.a = 100; c.b = 2; c.c = 3; CB& rb = c; rb.b += 0; AsB(c).b = 30; return GetB(c) * 0 + 2 + GetB(rb); }", 4));
}


//////////////////////////////////////////////////////////////////////////
//
// Constructors (bases, member initialisers, fields' constructors, vtables)
// and destructors (fields then bases), local objects destroyed at the end
// of their scope and on return, break and continue, and new and delete.
// gaiLog records the order things happened in.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstructors(void)
{
	const char*	szLog =
		"int gaiLog[64]; int giLog = 0;\n"
		"void Log(int i) { gaiLog[giLog] = i; giLog++; }\n"
		"int LogValue() { int v = 0; for (int i = 0; i < giLog; i++) v = v * 10 + gaiLog[i]; return v; }\n";
	CChars		sz;

	//Base, field and own constructor in order; destructors in reverse, the field's after the body.
	sz.Init(szLog);
	sz.Append(
		"struct CPart { int miId; CPart(int i) : miId(i) { Log(i); } ~CPart() { Log(miId + 5); } };\n"
		"struct CBase { int miBase; CBase() { miBase = 1; Log(1); } ~CBase() { Log(9); } };\n"
		"struct CWhole : CBase { CPart mcPart; int miX; CWhole() : mcPart(2), miX(7) { Log(3); } ~CWhole() { Log(4); } };\n"
		"int main() { { CWhole w; if (w.miX != 7 || w.miBase != 1) return -1; } return LogValue(); }");
	AssertInt(123479, TestCPP65IRMain(sz.Text(), 9));
	sz.Kill();

	//Return, break and continue destroy what is living.
	sz.Init(szLog);
	sz.Append(
		"struct CGuard { int miId; CGuard(int i) { miId = i; } ~CGuard() { Log(miId); } };\n"
		"int Early(int x) { CGuard a(1); if (x) { CGuard b(2); return 5; } CGuard c(3); return 6; }\n"
		"void Loop() { for (int i = 0; i < 3; i++) { CGuard g(4); if (i == 0) continue; if (i == 2) break; } }\n"
		"int main() { int r = Early(1); Loop(); return LogValue() * 10 + r; }");
	AssertInt(214445, TestCPP65IRMain(sz.Text(), 7));
	sz.Kill();

	//A for statement's step and a switch's condition are full expressions: their temporaries are destroyed there,
	//the step's each time round.
	sz.Init(szLog);
	sz.Append(
		"struct CStep { int miId; CStep(int i) { miId = i; } ~CStep() { Log(miId); } int Get() { return 1; } };\n"
		"int main() { int t = 0; for (int i = 0; i < 2; i += CStep(1).Get()) { Log(2); }\n"
		"switch (CStep(3).Get()) { case 1: Log(4); t = 5; break; } return LogValue() * 10 + t; }");
	AssertInt(2121345, TestCPP65IRMain(sz.Text(), 6));
	sz.Kill();

	//new and delete through a virtual destructor; the vtable is the derived class's after construction.
	sz.Init(szLog);
	sz.Append(
		"unsigned char gacHeap[64]; int giHeap = 0;\n"
		"void* operator new(unsigned int uiSize) { void* p = &gacHeap[giHeap]; giHeap += (int)uiSize; return p; }\n"
		"void operator delete(void* p) { }\n"
		"struct CShape { CShape() { Log(1); } virtual ~CShape() { Log(2); } virtual int Sides() { return 0; } };\n"
		"struct CSquare : CShape { int* mpi; CSquare() { mpi = new int(4); Log(3); } ~CSquare() { delete mpi; Log(4); } virtual int Sides() { return *mpi; } };\n"
		"int main() { CShape* p = new CSquare(); int s = p->Sides(); delete p; CShape* q = 0; delete q; return LogValue() * 10 + s; }");
	AssertInt(13424, TestCPP65IRMain(sz.Text(), 11));
	sz.Kill();

	//Temporaries that need destroying go at the end of their full expression, last made first (+ works out its left
	//operand first); an object's operator+= is its member function.
	sz.Init(szLog);
	sz.Append(
		"struct CTemp { int miId; CTemp(int i) { miId = i; Log(i); } ~CTemp() { Log(miId + 5); } int Get() { return miId; } CTemp& operator+=(int i) { miId += i; return *this; } };\n"
		"CTemp Make(int i) { return CTemp(i); }\n"
		"int main() { int a = Make(1).Get() + Make(2).Get(); int r; { CTemp t(3); t += 1; r = t.Get(); } int b = CTemp(1).Get(); if ((a != 3) || (r != 4) || (b != 1)) return -1; return LogValue(); }");
	AssertInt(12763916, TestCPP65IRMain(sz.Text(), 8));
	sz.Kill();

	//Pointers to members: a member function (called on an object and through a pointer, a base's from a derived object,
	//a virtual one through the vtable's thunk) and a field's offset kept in locals.
	AssertInt(21 + 12 + 5 + 9, TestCPP65IRMain(
		"struct CBase { int miPad; int Base(int i) { return i + 1; } };\n"
		"struct CCalc : CBase { int miValue; int Add(int i) { return miValue + i; } virtual int Get() { return 9; } };\n"
		"int main() { CCalc c; c.miValue = 20; CCalc* p = &c; int (CCalc::*f)(int) = &CCalc::Add; int (CBase::*b)(int) = &CBase::Base;\n"
		"int (CCalc::*g)() = &CCalc::Get; int CCalc::*m = &CCalc::miValue; int r = (c.*f)(1); c.*m = 11; return r + (p->*f)(1) + (c.*b)(4) + (p->*g)(); }", 5));

	//A member pointer from 0 is null and false; == compares them; a base's field is the derived class's (its offset added).
	AssertInt(1 + 20 + 3, TestCPP65IRMain(
		"struct CBase { int miPad; int miBase; };\n"
		"struct CMore { int miMore; };\n"
		"struct CBoth : CMore, CBase { int miValue; int Get() { return 3; } };\n"
		"int main() { int (CBoth::*h)() = 0; if (h) return -1; h = &CBoth::Get; if (!(h == &CBoth::Get) || (h != &CBoth::Get) || !h) return -2;\n"
		"int CBase::*b = &CBase::miBase; int CBoth::*d = b; CBoth o; o.miBase = 20; o.miValue = 1; int CBoth::*v = &CBoth::miValue; return (o.*v) + (o.*d) + (o.*h)(); }", 2));

	//A static_cast of a pointer to a derived class's field back to the base's takes the base's offset off again.
	AssertInt(2 * 10 + 2, TestCPP65IRMain(
		"struct CBase { int miPad; int miBase; };\n"
		"struct CMore { int miMore; };\n"
		"struct CBoth : CMore, CBase { int miValue; };\n"
		"int main() { int CBoth::*d = &CBoth::miBase; int CBase::*b = static_cast<int CBase::*>(d); CBase c; c.miPad = 1; c.miBase = 2;\n"
		"CBoth o; o.miBase = 2; return (c.*b) * 10 + (o.*d); }", 1));

	//A pointer to a derived object compared with one to its base (not the first) is compared as the base's.
	AssertInt(1 * 100 + 1 * 10 + 0, TestCPP65IRMain(
		"struct CBase { int miBase; };\n"
		"struct CMore { int miMore; };\n"
		"struct CBoth : CMore, CBase { int miValue; };\n"
		"int main() { CBoth o; CBoth p; CBoth* po = &o; CBase* pb = &o; CBase* pq = &p; return (po == pb) * 100 + (pb == po) * 10 + (po == pq); }", 1));

	//A function's name, &f and a static member's name are its address; a cast picks one of an overload.
	AssertInt(3 + 20 + 100 + 7, TestCPP65IRMain(
		"int Three() { return 3; }\n"
		"int Seven(int i) { return 7; } int Seven(char c) { return 8; }\n"
		"struct CMath { static int Twice(int i) { return i * 2; } int Use() { int (*f)(int) = Twice; return f(10); } };\n"
		"int Apply(int (*f)(int), int i) { return f(i); }\n"
		"int main() { int (*a)() = Three; int (*b)(int) = &CMath::Twice; CMath m; int (*c)(int) = (int (*)(int))&Seven;\n"
		"return a() + m.Use() + Apply(b, 50) + c(1); }", 7));

	//Calls through a table of function pointers, *pf and a function a call returns; a field that is an array of objects.
	AssertInt(2 + 30 + 4 + 50, TestCPP65IRMain(
		"int Two(int i) { return 2; } int Times(int i) { return i * 10; }\n"
		"typedef int (*FInt)(int);\n"
		"FInt gaf[2] = { Two, Times };\n"
		"FInt Pick(int i) { return gaf[i]; }\n"
		"struct CCell { int miValue; }; struct CRow { int miCount; CCell macCells[3]; };\n"
		"int main() { FInt pf = Times; CRow r; r.macCells[2].miValue = 50; return gaf[0](9) + (*pf)(3) + Pick(0)(1) * 2 + r.macCells[2].miValue; }", 5));

	//f(args...) passes the pack's elements; object.pf() and pf() in a member call through a field.
	AssertInt(321 + 8 + 12, TestCPP65IRMain(
		"int Add3(int a, int b, int c) { return a + b * 10 + c * 100; }\n"
		"template<typename... T> int Forward(T... args) { return Add3(args...); }\n"
		"int Twice(int i) { return i * 2; }\n"
		"struct CHolder { int (*mpf)(int); int Use() { return mpf(6); } };\n"
		"int main() { CHolder h; h.mpf = Twice; return Forward(1, 2, 3) + h.mpf(4) + h.Use(); }", 5));

	//An object's operator-> gives the pointer for a call and for a field.
	AssertInt(42 + 7, TestCPP65IRMain(
		"struct CThing { int miValue; int Twice() { return miValue * 2; } };\n"
		"struct CHandle { CThing* mp; CThing* operator->() { return mp; } };\n"
		"int main() { CThing t; t.miValue = 21; CHandle h; h.mp = &t; int r = h->Twice(); h->miValue = 7; return r + t.miValue; }", 3));

	//Placement new makes the object at the address given.
	AssertInt(9 + 30, TestCPP65IRMain(
		"void* operator new(unsigned int uiSize, void* p) { return p; }\n"
		"struct CCell { int miValue; CCell(int i) { miValue = i * 10; } };\n"
		"int main() { int x = 0; int* p = new (&x) int(9); int ai[2]; CCell* c = new (ai) CCell(3); return x + c->miValue + *p - 9; }", 3));
}


//////////////////////////////////////////////////////////////////////////
//
// Objects by value: parameters (the bytes where the caller pushed them,
// destroyed by the function), results (made at the address passed first,
// in place for an object initialised from the call), temporaries,
// Class(arguments), copies (with a copy constructor, bytes otherwise, a
// derived object's base part) and assignment (keeping the destination's
// vtable).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRObjectsByValue(void)
{
	AssertInt(3 + 4 + 10 + 20 + 7, TestCPP65IRMain(
		"struct SPair { int a; short b; char c; };\n"
		"SPair Make(int a, int b) { SPair p; p.a = a; p.b = (short)b; p.c = 7; return p; }\n"
		"int Sum(SPair p) { p.a += 0; return p.a + p.b + p.c; }\n"
		"SPair Flip(SPair p) { SPair r = Make(p.b, p.a); return r; }\n"
		"struct CHolder { SPair mp; SPair Get() const { return mp; } };\n"
		"int main() { SPair p = Make(3, 4); SPair q; q = Flip(Make(20, 10)); CHolder h; h.mp = p; return Sum(h.Get()) - 7 + q.a + q.b + p.c; }", 5));

	AssertInt(5 * 100 + 2 * 10 + 1, TestCPP65IRMain(
		"int giCopies = 0; int giGone = 0;\n"
		"struct CCounted { int mi; CCounted(int i) { mi = i; } CCounted(const CCounted& c) { mi = c.mi; giCopies++; } };\n"
		"struct CBase { int mi; virtual int Who() { return 1; } };\n"
		"struct CDerived : CBase { int mj; virtual int Who() { return 2; } };\n"
		"CCounted Five() { return CCounted(5); }\n"
		"int main() { CCounted a = Five(); CCounted b = a; CDerived d; d.mi = 2; CBase c; c = d; return b.mi * 100 + c.mi * 10 + c.Who() + giCopies - 1; }", 8));

	//A derived object passed by value as its base is a base (its vtable is the base's); an object argument is copied
	//when it is worked out, before the arguments left of it change it.
	AssertInt(1 * 100 + 3, TestCPP65IRMain(
		"struct CB { int mx; virtual int F() { return 1; } int Call() { return F(); } };\n"
		"struct CD : CB { virtual int F() { return 2; } };\n"
		"int G(CB b) { return b.Call(); }\n"
		"int H(CB& r) { return G(r); }\n"
		"struct SA { int x; };\n"
		"int K(int n, SA a) { return a.x; }\n"
		"int main() { CD d; SA a; a.x = 3; return H(d) * 100 + K(a.x = 5, a); }", 9));

	//A copy constructor may have more parameters, all with defaults: a class copying its field copies it with that
	//constructor, the defaults given.
	AssertInt(1 + 5 + 10 * 2, TestCPP65IRMain(
		"struct CPart { int mi; CPart(int i) : mi(i) { } CPart(const CPart& c, int k = 5) : mi(c.mi + k) { } };\n"
		"struct CWhole { CPart mcPart; int mj; CWhole() : mcPart(1), mj(2) { } };\n"
		"int main() { CWhole a; CWhole b = a; return b.mcPart.mi + b.mj * 10; }", 4));
}


//////////////////////////////////////////////////////////////////////////
//
// memcpy and __builtin_memcpy: loads and stores for a small constant count,
// Libc65's memcpy otherwise.  Both return Dest.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRMemcpy(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	//memcpy of a small constant count is loads and stores (7 bytes: 4, 2 and 1); it returns Dest.
	AssertInt(0x07060504 + 0x0302 + 0x01 + 1, TestCPP65IRMain(
		"void* memcpy(void* pvDest, const void* pvSource, unsigned int uiCount);\n"
		"int main() { unsigned char ac[8] = { 4, 5, 6, 7, 2, 3, 1, 9 }; unsigned char ad[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; void* p = memcpy(ad, ac, 7);\n"
		"return *(int*)ad + ad[4] + ad[5] * 256 + ad[6] + ad[7] + (p == ad); }", 1));

	//Each width of ctz, clz and popcount, and expect, from the one builtin table.
	AssertInt(3 + 31 * 10 + 3 * 1000 + 63 * 10000, TestCPP65IRMain(
		"int main() { unsigned long long ull = 8ULL; unsigned long ul = 1UL; unsigned long ulp = 7UL; unsigned long long ullc = 1ULL;\n"
		"return __builtin_ctzll(ull) + __builtin_clzl(ul) * 10 + __builtin_popcountl(ulp) * 1000 + __builtin_expect(__builtin_clzll(ullc), 0) * 10000; }", 1));

	//__builtin_memcpy needs no declaration; a count that isn't a small constant calls Libc65's memcpy.
	AssertInt(3 + 69 + 10 + 1 + 79, TestCPP65IRMain(
		"int main() { unsigned char ac[80]; unsigned char ad[80]; int n = 70; for (int i = 0; i < 80; i++) { ac[i] = (unsigned char)i; ad[i] = 0; }\n"
		"__builtin_memcpy(ad, ac, 4); void* p = __builtin_memcpy(&ad[10], &ac[10], n);\n"
		"return ad[3] + ad[69] + ad[10] + (p == &ad[10]) + ad[80 - 1]; }", 1));
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, "void Copy(int* pi, int* pj) { __builtin_memcpy(pi, pj, 4); }"));
	AssertString("", szErrors.Text());
	AssertFalse(szAssembly.Contains("_memcpy"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function the front end can't write yet is an error that says which of
// its files (and line) found it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRUnsupportedReason(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "int Pick(int a) { switch (int x = a) { default: return x; } }"));
	AssertTrue(szErrors.Contains("doesn't support yet (CPP65IRFrontEndStatements.cpp line "));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A string literal's escapes are the tokeniser's (DecodeEscape): \x and
// octal are a byte each, \u and \U the character's UTF-8.  An unknown
// escape, or a \x too big for a char, is an error.  [lex.string]
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRStringEscapes(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	AssertInt(1, TestCPP65IRMain(
		"int main() { const char* s = \"\\x41\\u00e9\\101\\?\\x7e\"; return (s[0] == 65) && ((unsigned char)s[1] == 0xC3) && ((unsigned char)s[2] == 0xA9) && (s[3] == 65) && (s[4] == '?') && (s[5] == '~') && (s[6] == 0); }", 1));

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "const char* gsz = \"\\x141\";"));
	AssertString("Generated.cpp (1, 19): The escape sequence's value is too large for a char [\"\\x141\"]\n", szErrors.Text());
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's StringCompare.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRStringCompare(void)
{
	AssertInt(129, TestCPP65IRMain(
		"#define NULL 0\n"
		"int StringCompare(const char* ps1, const char* ps2)\n"
		"{\n"
		"	int s1;\n"
		"	int s2;\n"
		"	if ((ps1 == NULL) && (ps2 == NULL)) { return 0; }\n"
		"	if (ps1 == NULL) { return -1; }\n"
		"	if (ps2 == NULL) { return 1; }\n"
		"	do { s1 = *ps1++; s2 = *ps2++; if (s1 == 0) break; } while (s1 == s2);\n"
		"	return (s1 < s2) ? -1 : (s1 > s2);\n"
		"}\n"
		"int Code(int i) { return i + 1; }\n"
		"int main() { return Code(StringCompare(\"abc\", \"abc\")) * 0 + Code(StringCompare(\"abd\", \"abc\")) * 1 * 3 / 2 + Code(StringCompare(\"ab\", \"abc\")) * 2 * 9 + 3 * 27 * Code(StringCompare(\"b\", \"a\")) / 2 + Code(StringCompare(0, 0)) * 0 + 5 * 9 * Code(StringCompare(\"\", 0)) / 2; }", 3));
}


//////////////////////////////////////////////////////////////////////////
//
// A virtual call through a pointer to an object whose vtable was just
// stored: the vtable's entry is known, so the call is direct (and inlined).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRDevirtualise(void)
{
	const char*	szSource;
	CChars		szAssembly;
	CChars		szErrors;
	CChars		szUse;

	szSource =
		"struct CShape { virtual int Area() { return 1; } };\n"
		"struct CSquare : CShape { int miSide; virtual int Area() { return miSide * miSide; } };\n"
		"int Use(int i) { CSquare c; c.miSide = i; CShape* p = &c; return p->Area(); }\n"
		"int main() { return Use(5); }\n";
	AssertInt(25, TestCPP65IRMain(szSource, 6));

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	szUse.Init();
	szUse.AppendSubString(szAssembly, szAssembly.Find("_Use__Fi:"), szAssembly.Length());
	szUse.SetLength(szUse.Find("\t\tret"));
	AssertFalse(szUse.Contains("jsr"));
	szUse.Kill();
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function too big to inline is inlined when it is called with a
// constant it compares (the comparisons fold).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRInlineConstants(void)
{
	const char*	szSource;
	CChars		szAssembly;
	CChars		szErrors;
	CChars		szMain;

	szSource =
		"int ga = 7; int gb = 3;\n"
		"int Pick(int iKind, int a, int b)\n"
		"{\n"
		"	if (iKind == 0) return a + b; if (iKind == 1) return a - b; if (iKind == 2) return a * b;\n"
		"	if (iKind == 3) return a & b; if (iKind == 4) return a | b; if (iKind == 5) return a ^ b;\n"
		"	if (iKind == 6) return a << b; if (iKind == 7) return a >> b; if (iKind == 8) return a % b;\n"
		"	return 0;\n"
		"}\n"
		"int main() { return Pick(2, ga, gb) + Pick(8, ga, gb); }\n";
	AssertInt(22, TestCPP65IRMain(szSource, 2));

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	szMain.Init();
	szMain.AppendSubString(szAssembly, szAssembly.Find("_main:"), szAssembly.Length());
	szMain.SetLength(szMain.Find("\t\tret"));
	AssertFalse(szMain.Contains("jsr _Pick"));
	szMain.Kill();
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast: down from a first and a second base, across, to void*, a NULL
// pointer, up (a conversion), to a class the object isn't, and an ambiguous
// base (a diamond) which gives NULL.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRDynamicCast(void)
{
	AssertInt(8191, TestCPP65IRMain(
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
		"}\n", 12));
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable as CPP65Generator.h lays it out: a header (the distance to the
// object's start, the type info) before each part, the label after the
// first header, a secondary part's entries through a thunk that adjusts
// this.  Virtual calls through either part's pointer, and dynamic_cast
// from it, read it so.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRVtableLayout(void)
{
	const char*	szSource;
	CChars		szAssembly;
	CChars		szErrors;

	szSource =
		"struct CA { int ma; virtual int F() { return 1; } };\n"
		"struct CB { int mb; virtual int G() { return 2; } };\n"
		"struct CC : CA, CB { int mc; virtual int F() { return 10; } virtual int G() { return 20 + mc; } };\n"
		"int main() { CC c; c.mc = 3; CA* pa = &c; CB* pb = &c; return pa->F() + pb->G() * 100 + (dynamic_cast<CC*>(pb) == &c) * 10000; }\n";
	AssertInt(10 + 23 * 100 + 10000, TestCPP65IRMain(szSource, 8));

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	AssertString("", szErrors.Text());
	AssertTrue(szAssembly.Contains(
		"\t\t.section rodata._CC__2, comdat\n"
		"\t\t.global _CC__2\n"
		"\t\t.align 4\n"
		"\t\t.long 0\n"
		"\t\t.long _CC__3\n"
		"_CC__2:\n"
		"\t\t.long _CC__F\n"
		"\t\t.long _CC__G\n"
		"\t\t.long -8\n"
		"\t\t.long _CC__3\n"
		"\t\t.long _CC__G__t8\n"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// dynamic_cast across units: both write CSquare's vtable and type info and the
// linker keeps one of each, so the type info compared against is the object's.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRDynamicCastUnits(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/DynamicCastProject/Main.cpp", "Input/CPP65/DynamicCastProject/Square.cpp", NULL };

	AssertInt(1 + 400 + 1000, TestCPP65RunProject(aszUnits, ""));
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
void TestCPP65IROverloadedFunctionPointers(void)
{
	AssertInt(511, TestCPP65IRMain(
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
		"}\n", 8));
}


//////////////////////////////////////////////////////////////////////////
//
// Constant expressions (global initialisers, static_assert, array sizes)
// are worked out in their types: unsigned arithmetic wraps, compares and
// shifts as unsigned, and an int and an unsigned compare as unsigned.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstantTypes(void)
{
	AssertInt(1 + 2 + 4 + 8 + 16 + 32 + 64 + 128, TestCPP65IRMain(
		"unsigned guHalf = (0u - 1) / 2;\n"
		"int giAbove = (0u - 1) > 0;\n"
		"int giMixed = -1 < 1u;\n"
		"unsigned guShift = (0u - 1) >> 28;\n"
		"int giNarrow = (unsigned char)255 + 1;\n"
		"static_assert((0u - 1) > 0, \"Unsigned wraps\");\n"
		"int gaiSized[(0u - 1) % 7];\n"
		"long long glliNegative = -(1LL << 62) / -2;\n"
		"int giNot = !256 + ~0u % 3;\n"
		"int main() { return (guHalf == 0x7FFFFFFFu) + (giAbove == 1) * 2 + (giMixed == 0) * 4 + (guShift == 15) * 8 + (giNarrow == 256) * 16 +\n"
		"(sizeof(gaiSized) == 3 * sizeof(int)) * 32 + (glliNegative == (1LL << 61)) * 64 + (giNot == 0) * 128; }", 1));
}


//////////////////////////////////////////////////////////////////////////
//
// Functions that differ only by ..., and classes and enums of the same name
// in different classes, have different labels.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRMangledNames(void)
{
	const char*	szSource;
	CChars		szAssembly;
	CChars		szErrors;

	szSource =
		"int F(int a) { return a; }\n"
		"int F(int a, ...) { return a * 2; }\n"
		"struct CA { struct CIn { int Get() { return 1; } }; };\n"
		"struct CB { struct CIn { int Get() { return 2; } }; };\n"
		"struct CX { enum EKind { KIND }; }; struct CY { enum EKind { KIND }; };\n"
		"int G(CX::EKind e) { return 3; }\n"
		"int G(CY::EKind e) { return 4; }\n"
		"int main() { int (*pf)(int) = F; CA::CIn a; CB::CIn b; return pf(3) + F(4, 5) * 10 + a.Get() * 100 + b.Get() * 1000 + G(CX::KIND) * 10000 + G(CY::KIND) * 100000; }\n";
	AssertInt(3 + 80 + 100 + 2000 + 30000 + 400000, TestCPP65IRMain(szSource, 7));

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	AssertString("", szErrors.Text());
	AssertTrue(szAssembly.Contains("\n_F__Fi:"));
	AssertTrue(szAssembly.Contains("\n_F__Fiz:"));
	AssertTrue(szAssembly.Contains("\n_CA__CIn__Get:"));
	AssertTrue(szAssembly.Contains("\n_CB__CIn__Get:"));
	AssertTrue(szAssembly.Contains("\n_G__FN9CX__EKind:"));
	AssertTrue(szAssembly.Contains("\n_G__FN9CY__EKind:"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A member template of a class defined outside it after a function that
// calls it (StandardLib's CString::InitText): the call's instance has the
// definition, and so its type.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRMemberTemplateDefinedLater(void)
{
	AssertInt(5, TestCPP65IRMain(
		"template<class T> struct P { T* p; P(T* q) { p = q; } P(const P& o) { p = o.p; } };\n"
		"struct S { int v; template<class X> P<S> Make(X x); P<S> Init(int i); };\n"
		"P<S> S::Init(int i) { return Make(i); }\n"
		"template<class X> P<S> S::Make(X x) { v = (int)x; return this; }\n"
		"int main() { S s; P<S> p = s.Init(5); return p.p->v; }", 4));
}


//////////////////////////////////////////////////////////////////////////
//
// x / d and x % d with code between them (by a constant too) share one div
// on the 65C64: the optimiser puts the urem after the udiv
// (SetDivideRemainders) and the back end writes them with one div.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRSharedDivides(void)
{
	static const char*	szSource =
		"unsigned Digits(unsigned x, unsigned d) { unsigned q = x / d; unsigned y = q * 3; unsigned r = x % d; return y + r; }\n"
		"unsigned Tens(unsigned x) { unsigned r = x % 10; unsigned y = r * 7; unsigned q = x / 10; return y + q; }\n"
		"int main() { return (int)(Digits(1234, 100) + Tens(1234)); }";
	CChars				szAssembly;
	CChars				szErrors;
	const char*			sz;
	int					iDivs;

	AssertInt(12 * 3 + 34 + 4 * 7 + 123, TestCPP65IRMain(szSource, 3));
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	AssertString("", szErrors.Text());
	iDivs = 0;
	for (sz = strstr(szAssembly.Text(), "\t\tdiv\n"); sz; sz = strstr(sz + 1, "\t\tdiv\n"))
	{
		iDivs++;
	}
	AssertInt(2, iDivs);
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two anonymous objects in a function: each has its own slot (known by its
// class), and a member is at its offset in it.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRAnonymousLocals(void)
{
	AssertInt(1 + 2 * 10 + 3 * 100 + 4 * 1000, TestCPP65IRMain(
		"int main()\n"
		"{\n"
		"\tstruct { int a; short b; char c; };\n"
		"\tunion { int x; char y; };\n"
		"\ta = 1; b = 2; c = 3; x = 0x104;\n"
		"\treturn a + b * 10 + c * 100 + y * 1000;\n"
		"}\n", 1));
}


//////////////////////////////////////////////////////////////////////////
//
// const S named before S is defined (a parameter of a declaration): the
// const type has S's size once S is laid out, so an array of it is laid
// out by it (CCPP65Types::SetClassBytes).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstClassSize(void)
{
	AssertInt(3 * 100 + 4 * 10 + 8, TestCPP65IRMain(
		"struct S;\n"
		"int First(const S* ps);\n"
		"struct S { int a; int b; };\n"
		"const S gas[] = { { 1, 2 }, { 3, 4 } };\n"
		"int First(const S* ps) { return ps->a; }\n"
		"int main() { return First(&gas[1]) * 100 + gas[1].b * 10 + (int)sizeof(gas[0]); }\n", 3));
}


//////////////////////////////////////////////////////////////////////////
//
// const and volatile are part of a type: in a label (K and V, as the
// standard's mangling has them: const char* is PKc, const int& RKi), but
// not on a parameter itself (f(const int) is f(int)); a template deduces
// T* from a const int* as const int, which a const T specialisation tells
// from int.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstTypes(void)
{
	static const char*	szSource =
		"int F(const char* p) { return p[0]; }\n"
		"int G(const int& r) { return r; }\n"
		"int H(volatile int* p) { return *p; }\n"
		"int I(const int x) { return x + 1; }\n"
		"template<class T> struct SIsConst { static int V() { return 0; } };\n"
		"template<class T> struct SIsConst<const T> { static int V() { return 1; } };\n"
		"template<class T> int Check(T* p) { return SIsConst<T>::V(); }\n"
		"int main() { const int a = 2; int b = 3; volatile int c = 4; return F(\"A\") + G(a) * 10 + H(&c) * 100 + I(b) * 1000 + Check(&a) * 10000 + Check(&b) * 20000; }";
	CChars				szAssembly;
	CChars				szErrors;

	AssertInt('A' + 2 * 10 + 4 * 100 + 4 * 1000 + 10000, TestCPP65IRMain(szSource, 9));
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, true));
	AssertString("", szErrors.Text());
	AssertTrue(szAssembly.Contains("_F__FPKc:"));
	AssertTrue(szAssembly.Contains("_G__FRKi:"));
	AssertTrue(szAssembly.Contains("_H__FPVi:"));
	AssertTrue(szAssembly.Contains("_I__Fi:"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a const argument deduces T as [temp.deduct.call]: T and const T&
// and const T* take int (a value's own const doesn't count; the
// parameter's own is taken off), T& and T* const int.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstDeduction(void)
{
	AssertInt(1 * 10 + 1 * 1000 + 6 * 10000, TestCPP65IRMain(
		"template<class T> struct SIsConst { static int V() { return 0; } };\n"
		"template<class T> struct SIsConst<const T> { static int V() { return 1; } };\n"
		"template<class T> int ByValue(T x) { x = x + 1; return SIsConst<T>::V() * 10000 + x * 10000; }\n"
		"template<class T> int ByReference(T& r) { return SIsConst<T>::V() * 10; }\n"
		"template<class T> int ByConstReference(const T& r) { return SIsConst<T>::V() * 100; }\n"
		"template<class T> int ByPointer(T* p) { return SIsConst<T>::V() * 1000; }\n"
		"template<class T> int ByConstPointer(const T* p) { return SIsConst<T>::V() * 100000; }\n"
		"int main() { const int a = 5; return ByValue(a) + ByReference(a) + ByConstReference(a) + ByPointer(&a) + ByConstPointer(&a); }", 8));
}


//////////////////////////////////////////////////////////////////////////
//
// Overloads that differ only in const (and volatile) behind a pointer or
// reference: each is called for what it fits best, and a T* takes a const
// T* no longer (only const void*, not void*; a base of const Derived only
// when it is const).  Adding const is a worse match than none, but better
// than a conversion.  [over.ics.rank] 3.2.6
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstOverloads(void)
{
	AssertInt(1 + 2 * 10 + 3 * 100 + 4 * 1000 + 2 * 10000 + 2 * 100000 + 6 * 1000000, TestCPP65IRMain(
		"int F(int* p) { return 1; }\n"
		"int F(const int* p) { return 2; }\n"
		"int G(int& r) { return 3; }\n"
		"int G(const int& r) { return 4; }\n"
		"int OnlyConst(const int* p) { return 2; }\n"
		"int Void(void* p) { return 1; }\n"
		"int Void(const void* p) { return 2; }\n"
		"int Convert(const int* p) { return 6; }\n"
		"int Convert(void* p) { return 7; }\n"
		"int main() { int a = 0; const int b = 0; return F(&a) + F(&b) * 10 + G(a) * 100 + G(b) * 1000 + OnlyConst(&a) * 10000 + Void(&b) * 100000 + Convert(&a) * 1000000; }", 10));

	AssertInt(1 + 2 * 10 + 1 * 100 + 2 * 1000 + 2 * 10000, TestCPP65IRMain(
		"struct B { int i; };\n"
		"struct D : public B { int j; };\n"
		"int P(B* p) { return 1; }\n"
		"int P(const B* p) { return 2; }\n"
		"int R(B& r) { return 1; }\n"
		"int R(const B& r) { return 2; }\n"
		"int A(char* p) { return 1; }\n"
		"int A(const char* p) { return 2; }\n"
		"int main() { D d; const D c = d; const char s[2] = { 0, 0 }; return P(&d) + P(&c) * 10 + R(d) * 100 + R(c) * 1000 + A(s) * 10000; }", 7));
}


//////////////////////////////////////////////////////////////////////////
//
// A member of a const object is const, but for a mutable one; through a
// pointer or reference to const too; a member a reference is what it refers
// to.  Overloads show the type.  [expr.ref] 6.2
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstMembers(void)
{
	AssertInt(2 + 1 * 10 + 2 * 100 + 1 * 1000 + 1 * 10000 + 2 * 100000, TestCPP65IRMain(
		"int G(int& r) { return 1; }\n"
		"int G(const int& r) { return 2; }\n"
		"struct S { int i; mutable int m; int& r; const int k; S(int& x) : i(0), m(0), r(x), k(0) {} };\n"
		"int main()\n"
		"{\n"
		"\tint x = 0;\n"
		"\tconst S c(x);\n"
		"\tS s(x);\n"
		"\tconst S* p = &s;\n"
		"\treturn G(c.i) + G(s.i) * 10 + G(p->i) * 100 + G(c.m) * 1000 + G(c.r) * 10000 + G(s.k) * 100000;\n"
		"}\n", 4));
}


//////////////////////////////////////////////////////////////////////////
//
// szSource's assembly: compiled without errors, for what labels it has.
//
//////////////////////////////////////////////////////////////////////////
static void AssertNamespaceLabels(const char* szSource, const char** aszLabels, const char** aszNotLabels)
{
	CChars	szAssembly;
	CChars	szErrors;
	size	i;

	szAssembly.Init();
	szErrors.Init();
	TestCPP65Generate(&szAssembly, &szErrors, szSource, true);
	AssertString("", szErrors.Text());
	for (i = 0; aszLabels[i]; i++)
	{
		AssertTrue(szAssembly.Contains(aszLabels[i]));
	}
	for (i = 0; aszNotLabels[i]; i++)
	{
		AssertFalse(szAssembly.Contains(aszNotLabels[i]));
	}
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each type trait the compiler knows, of a class, a union, an enum and a type
// that is none of them.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRTypeTraits(void)
{
	static const char*	szTypes = "struct Plain { int i; };\nstruct Empty {};\nunion U { int i; char c; };\nenum E { A };\nstruct Made { int i; Made() : i(1) {} };\nstruct Gone { int i; ~Gone() {} };\nstruct Copied { int i; Copied() {} Copied(const Copied& c) : i(c.i) {} };\nstruct Virtual { virtual int F() { return 1; } };\nstruct Abstract { virtual int F() = 0; };\n";
	static const char*	aszTraits[] =
	{
		"__is_class(Plain) && __is_class(Empty) && !__is_class(U) && !__is_class(E) && !__is_class(int)",
		"__is_union(U) && !__is_union(Plain) && !__is_union(int)",
		"__is_enum(E) && !__is_enum(Plain) && !__is_enum(int)",
		"__is_empty(Empty) && !__is_empty(Plain) && !__is_empty(int)",
		"__is_polymorphic(Virtual) && __is_polymorphic(Abstract) && !__is_polymorphic(Plain) && !__is_polymorphic(int)",
		"__is_abstract(Abstract) && !__is_abstract(Virtual) && !__is_abstract(int)",
		"__is_trivially_constructible(Plain) && !__is_trivially_constructible(Made) && __has_trivial_constructor(int) && !__has_trivial_constructor(Made)",
		"__is_trivially_destructible(Plain) && !__is_trivially_destructible(Gone) && __has_trivial_destructor(int) && !__has_trivial_destructor(Gone)",
		"__is_pod(Plain) && __is_trivial(int) && __is_trivially_copyable(Plain) && !__is_pod(Made) && !__is_trivial(Gone) && !__is_trivially_copyable(Copied) && !__is_pod(Virtual)",
		NULL,
	};
	CChars	sz;
	size	i;

	for (i = 0; aszTraits[i]; i++)
	{
		sz.Init(szTypes);
		sz.Append("int main() { return (")->Append(aszTraits[i])->Append(") ? 7 : 1; }");
		AssertInt(7, TestCPP65Main(sz.Text()));
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// operator->* has its own label, so it and another operator can't meet in
// the assembly, and is called like any other binary operator.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IROperatorLabels(void)
{
	static const char*	szSource = "struct S { int i; int operator->*(int a) { return i + a; } };\nint Free(S s) { return s->*1; }\nint main() { S s; s.i = 10; return (s->*3) + Free(s); }";
	static const char*	aszLabels[] = { "_S__opArrowMember", NULL };
	static const char*	aszNotLabels[] = { "__opOperator", NULL };

	AssertNamespaceLabels(szSource, aszLabels, aszNotLabels);
	AssertInt(24, TestCPP65Main(szSource));

	//A pointer on the left is still the built in operator.
	AssertInt(5, TestCPP65Main("struct S { int i; int operator->*(int a) { return a; } };\nint main() { S s; s.i = 5; S* p = &s; int S::* m = &S::i; return p->*m; }"));
}


//////////////////////////////////////////////////////////////////////////
//
// What a namespace declares has the namespace in its label, and a parameter
// of a class in one too.  extern "C", the global main and what isn't in a
// namespace don't.  A namespace without a name is the unit's own.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceLabels(void)
{
	static const char*	aszNamed[] = { "_N__x:", "_N__F__Fi:", "_N__M__x:", "_N__M__G__Fi:", "_N__S__Get", "_N__Take__FC4N__S:", "_N__Inline__I__Fi:", NULL };
	static const char*	aszNotNamed[] = { "\n_x:", "\n_F__Fi:", NULL };
	static const char*	aszPlain[] = { "_N__main:", "\n_main:", "_Exported:", "_y:", "_Global__Fi:", NULL };
	static const char*	aszNotPlain[] = { "_N__Exported", "_N__y", NULL };
	static const char*	aszUnnamed[] = { "_x:", "_Unnamed0__x:", "_Unnamed0__F__Fi:", NULL };
	static const char*	aszNotUnnamed[] = { ".global _Unnamed0__x", ".global _Unnamed0__F__Fi", NULL };

	AssertNamespaceLabels("namespace N { int x = 1; int F(int a) { return a + x; }\nnamespace M { int x = 2; int G(int a) { return a + x; } }\nstruct S { int i; int Get() { return i; } };\nint Take(S s) { return s.Get(); }\ninline namespace Inline { int I(int a) { return a; } } }\nusing namespace N;\nusing namespace N::M;\nint main() { S s; s.i = 1; return F(1) + G(1) + Take(s) + I(1); }", aszNamed, aszNotNamed);
	AssertNamespaceLabels("namespace N { int main() { return 1; }\nextern \"C\" int Exported(int a) { return a; }\nextern \"C\" { int y = 1; } }\nint Global(int a) { return a; }\nusing N::Exported;\nusing N::y;\nint main() { return Exported(y) + Global(1); }", aszPlain, aszNotPlain);
	AssertNamespaceLabels("int x = 1;\nnamespace { int x2 = 1; }\nnamespace { int x = 2; int F(int a) { return a + 1; } }\nint main() { return ::x + x2 + F(1); }", aszUnnamed, aszNotUnnamed);
}


//////////////////////////////////////////////////////////////////////////
//
// What a namespace declares works from inside it and through a using
// directive or declaration, with its own globals' values.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceRuns(void)
{
	AssertInt(13, TestCPP65Main("namespace N { int x = 10; int F(int a) { return a + x; } struct S { int i; S() : i(1) {} int Get() { return i; } }; }\nusing namespace N;\nint main() { S s; return F(2) + s.Get(); }"));
	AssertInt(7, TestCPP65Main("namespace N { int x = 3; int F() { return x; } }\nnamespace { int y = 4; int G() { return y; } }\nusing N::F;\nint main() { return F() + G(); }"));
	AssertInt(9, TestCPP65Main("namespace { int F(int a); }\nnamespace { int F(int a) { return a + 1; } }\nint main() { return F(8); }"));
	AssertInt(5, TestCPP65Main("namespace N { enum E { A = 2, B }; typedef int I; }\nusing namespace N;\nint main() { I i = B; E e = A; return i + e; }"));
}


static void AssertConstError(const char* szExpected, const char* szSource);


//////////////////////////////////////////////////////////////////////////
//
// A name qualified by a namespace: calls, globals, types and addresses, with
// nested namespaces, aliases, ::, and definitions outside the namespace.  A
// class inside a class too.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceQualified(void)
{
	AssertInt(3, TestCPP65Main("namespace N { int F(int a) { return a + 1; } }\nint main() { return N::F(2); }"));
	AssertInt(5, TestCPP65Main("namespace N { namespace M { int F() { return 5; } } }\nint main() { return N::M::F(); }"));
	AssertInt(6, TestCPP65Main("namespace N { int F() { return 6; } }\nint main() { return ::N::F(); }"));
	AssertInt(7, TestCPP65Main("namespace N { int F() { return 7; } }\nnamespace A = N;\nint main() { return A::F(); }"));
	AssertInt(8, TestCPP65Main("namespace N { int x = 3; }\nint main() { N::x = N::x + 5; return N::x; }"));
	AssertInt(9, TestCPP65Main("namespace N { int F(int a); }\nint N::F(int a) { return a + 1; }\nint main() { return N::F(8); }"));
	AssertInt(10, TestCPP65Main("namespace N { extern int x; }\nint N::x = 10;\nint main() { return N::x; }"));
	AssertInt(11, TestCPP65Main("namespace N { struct S { int i; S(int a); int Get(); static int Twice(int a) { return a + a; } }; }\nN::S::S(int a) : i(a) {}\nint N::S::Get() { return i; }\nint main() { N::S s(1); return s.Get() + N::S::Twice(5); }"));
	AssertInt(12, TestCPP65Main("namespace N { int F(int a) { return a + 2; } }\nint main() { int (*p)(int) = N::F; int (*q)(int) = &N::F; return p(4) + q(4); }"));
	AssertInt(13, TestCPP65Main("namespace N { template<class T> struct Box { T t; T Get() { return t; } }; template<class T> T Add(T a, T b) { return a + b; } }\nint main() { N::Box<int> b; b.t = 6; return b.Get() + N::Add<int>(3, 4); }"));
	AssertInt(14, TestCPP65Main("namespace N { enum E { A = 14 }; typedef int I; }\nint main() { N::I i = N::A; N::E e = N::A; return (e == N::A) ? i : 0; }"));
	AssertInt(15, TestCPP65Main("struct Outer { struct Inner { int i; int Get(); static int Next(int a); }; };\nint Outer::Inner::Get() { return i; }\nint Outer::Inner::Next(int a) { return a + 1; }\nint main() { Outer::Inner n; n.i = 7; return n.Get() + Outer::Inner::Next(7); }"));
	AssertInt(16, TestCPP65Main("namespace N { struct Outer { struct Inner { int Get() { return 16; } }; }; }\nint main() { N::Outer::Inner n; return n.Get(); }"));
	AssertInt(18, TestCPP65Main("namespace N { int F() { return 9; } int x = 9; }\nint main() { using namespace N; return F() + x; }"));
	AssertInt(19, TestCPP65Main("namespace N { int F() { return 10; } int x = 9; }\nint main() { using N::F; using N::x; return F() + x; }"));
	AssertInt(20, TestCPP65Main("namespace N { int F() { return 20; } }\nint main() { namespace A = N; return A::F(); }"));
	AssertInt(21, TestCPP65Main("int main() { using I = int; I i = 21; return i; }"));
	AssertInt(22, TestCPP65Main("namespace N { using I = int; }\nint main() { N::I i = 22; return i; }"));
	AssertConstError("This declaration is not supported by the compiler yet", "asm(\"nop\");\nint main() { return 0; }");
	AssertInt(17, TestCPP65Main("namespace N { struct B { virtual int V() { return 1; } }; struct D : B { int V() { return 17; } }; }\nint main() { N::D d; N::B* p = &d; return p->V(); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// A call chooses between the functions its name can mean: those of the same
// name in other namespaces aren't overloads.  An inner name hides an outer one.
// What using directives nominate are overloads, ambiguous if they are the same.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceOverloads(void)
{
	AssertInt(21, TestCPP65Main("namespace A { int F(int a) { return a + 10; } }\nnamespace B { int F(int a) { return a + 8; } int G() { return F(1); } }\nint F(int a) { return a; }\nint main() { return A::F(1) + B::G() + ::F(1); }"));
	AssertInt(1, TestCPP65Main("int F(int a) { return 2; }\nnamespace N { int F(char c) { return 1; } int G() { return F(300); } }\nint main() { return N::G(); }"));
	AssertInt(2, TestCPP65Main("int F(int a) { return 2; }\nnamespace N { int F(char c) { return 1; } int G() { return ::F(300); } }\nint main() { return N::G(); }"));
	AssertInt(3, TestCPP65Main("namespace A { int F(int a) { return 3; } }\nnamespace B { int F(char c) { return 4; } }\nusing namespace A;\nusing namespace B;\nint main() { return F(1000); }"));
	AssertInt(4, TestCPP65Main("namespace A { int F(int a) { return 3; } }\nnamespace B { using namespace A; int F(char c) { return 4; } int G() { return F(1000); } }\nint main() { return B::G(); }"));
	AssertInt(5, TestCPP65Main("int F(char c) { return 6; }\nnamespace A { int F(int a) { return 5; } }\nint main() { using namespace A; return F(1000); }"));
	AssertInt(6, TestCPP65Main("namespace A { int F(int a) { return 6; } }\nnamespace B { using namespace A; }\nint main() { return B::F(1); }"));
	AssertInt(7, TestCPP65Main("namespace A { int F(int a) { return 8; } }\nnamespace B { using namespace A; int F(char c) { return 7; } }\nint main() { return B::F(1000); }"));
	AssertInt(8, TestCPP65Main("namespace N { int F(char c) { return 9; } inline namespace V { int F(int a) { return 8; } } }\nint main() { return N::F(1000); }"));
	AssertInt(9, TestCPP65Main("namespace N { int F(int a) { return 9; } }\nint F(char c) { return 1; }\nint main() { using N::F; return F(1000); }"));
	AssertInt(10, TestCPP65Main("namespace A { int F(int a) { return 10; } }\nnamespace B { int F(int a) { return 20; } }\nint main() { int (*p)(int) = A::F; return p(1); }"));
	AssertInt(11, TestCPP65Main("namespace A { int F(int a) { return 11; } }\nnamespace B { int F(int a) { return 20; } }\nint Call(int (*p)(int)) { return p(1); }\nint main() { return Call(A::F); }"));
	AssertInt(12, TestCPP65Main("namespace N { int G() { return 12; } int F(); struct S { int H(); int I() { return G(); } }; }\nint N::F() { return G(); }\nint main() { return N::F(); }"));
	AssertInt(13, TestCPP65Main("namespace N { int G() { return 13; } struct S { int H(); }; }\nint G() { return 1; }\nint N::S::H() { return G(); }\nint main() { N::S s; return s.H(); }"));
	AssertInt(14, TestCPP65Main("namespace N { struct S { int i; }; S operator+(S a, S b) { S s; s.i = a.i + b.i; return s; } }\nint main() { N::S a; N::S b; a.i = 6; b.i = 8; return (a + b).i; }"));

	AssertConstError("The call is ambiguous", "namespace A { int F(int a) { return 1; } }\nnamespace B { int F(int a) { return 2; } }\nusing namespace A;\nusing namespace B;\nint main() { return F(1); }");
	AssertConstError("No function matches", "namespace N { int F(int a) { return 1; } }\nnamespace M { int F() { return 2; } }\nint main() { return M::F(1); }");
}


//////////////////////////////////////////////////////////////////////////
//
// Templates of the same name in different namespaces are different templates,
// with different instances and labels.  Their members can be defined outside,
// and specialised.  An instance has the namespace of its template, wherever
// it is used from.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceTemplates(void)
{
	static const char*	aszLabels[] = { "_A__Box__T", "_B__Box__T", "_A__Add__T", "_B__Add__T", "_N__opAdd__T", NULL };
	static const char*	aszNone[] = { NULL };

	AssertInt(21, TestCPP65Main("namespace A { template<class T> struct Box { T t; T Get() { return t + 1; } }; }\nnamespace B { template<class T> struct Box { T t; T s; T Get() { return t + 10; } }; }\nint main() { A::Box<int> a; B::Box<int> b; a.t = 0; b.t = 10; return a.Get() + b.Get(); }"));
	AssertInt(4, TestCPP65Main("namespace A { template<class T> struct Box { T t; }; }\nnamespace B { template<class T> struct Box { T t; T s; }; }\nint main() { return sizeof(B::Box<char>) * 2 - sizeof(A::Box<char>) * 0; }"));
	AssertInt(3, TestCPP65Main("namespace A { template<class T> T Add(T a) { return a + 1; } }\nnamespace B { template<class T> T Add(T a) { return a + 2; } int Go() { return Add(1); } }\nint main() { return B::Go(); }"));
	AssertInt(5, TestCPP65Main("namespace A { template<class T> T Add(T a) { return a + 1; } }\nnamespace B { template<class T> T Add(T a) { return a + 2; } }\nint main() { return A::Add(1) + B::Add<int>(1); }"));
	AssertInt(6, TestCPP65Main("namespace N { template<class T> struct Box { T t; T Get(); }; }\ntemplate<class T> T N::Box<T>::Get() { return t; }\nint main() { N::Box<int> b; b.t = 6; return b.Get(); }"));
	AssertInt(7, TestCPP65Main("namespace N { template<class T> struct Box { T t; T Get(); }; template<class T> T Box<T>::Get() { return t + 1; } }\nint main() { N::Box<int> b; b.t = 6; return b.Get(); }"));
	AssertInt(8, TestCPP65Main("namespace M { template<class T> struct S { int V() { return 1; } }; }\nnamespace N { template<class T> struct S { int V() { return 2; } }; template<> struct S<int> { int V() { return 8; } }; }\nint main() { N::S<int> s; return s.V(); }"));
	AssertInt(9, TestCPP65Main("namespace M { template<class T> int Size(T a) { return 1; } }\nnamespace N { template<class T> int Size(T a) { return 2; } template<> int Size<int>(int a) { return 9; } }\nint main() { return N::Size<int>(1); }"));
	AssertInt(10, TestCPP65Main("namespace A { template<class T> struct In { T t; int V() { return 1; } }; }\nnamespace B { template<class T> struct In { T t; int V() { return 10; } }; template<class T> struct Out { In<T> i; int V() { return i.V(); } }; }\nint main() { B::Out<int> o; return o.V(); }"));
	AssertInt(11, TestCPP65Main("namespace A { template<class T> T F(T a) { return a + 1; } }\nnamespace B { template<class T> T F(T a, T b) { return a + b; } }\nusing namespace A;\nusing namespace B;\nint main() { return F(4) + F(3, 3); }"));
	AssertNamespaceLabels("namespace A { template<class T> struct Box { T Get() { T t = 0; return t; } }; template<class T> T Add(T a) { return a; } }\nnamespace B { template<class T> struct Box { T Get() { T t = 0; return t; } }; template<class T> T Add(T a) { return a; } }\nnamespace N { struct S { int i; }; template<class T> S operator+(S a, T b) { return a; } }\nint main() { A::Box<int> a; B::Box<int> b; N::S s; s.i = 0; s = s + 1; return a.Get() + b.Get() + A::Add<int>(1) + B::Add<int>(1) + s.i; }", aszLabels, aszNone);
}


//////////////////////////////////////////////////////////////////////////
//
// A class's own functions (SCPP65Class::mapsMembers), with another class's
// declared between them: its copy constructor and converting constructor are
// found, an explicit one isn't a conversion, and virtual functions override
// in its vtable and a secondary one.  Constructors inherited with using come
// from the base's alone, unless the class has its own of those parameters.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRClassMembers(void)
{
	AssertInt(7, TestCPP65Main(
		"struct O;\n"
		"struct A { int a; A(int x) : a(x) {} explicit A(char* p) : a(1) {} A(const A& s) : a(s.a + 1) {} virtual int F() { return a; } virtual int G() { return 1; } };\n"
		"struct O { int o; O(int x) : o(x + 100) {} O(const O& s) : o(s.o + 100) {} virtual int F() { return 50; } };\n"
		"struct B { virtual int H() { return 2; } };\n"
		"struct C : A, B { C(int x) : A(x) {} int H() { return 3; } int G() { return 2; } };\n"
		"int Take(A s) { return s.a; }\n"
		"int main() { C c(1); A* pa = &c; B* pb = &c; O o(0); O p(o); A b(4); A d(b); if (d.a != 5) return 1; if (Take(2) != 2) return 2; return pa->G() + pb->H() + (p.o == 200 ? 2 : 0); }"));
	AssertInt(23, TestCPP65Main(
		"struct B { int b; B(int x) : b(x) {} B(int x, int y) : b(x + y) {} };\n"
		"struct O { int o; O(int x) : o(0) {} };\n"
		"struct D : B { using B::B; D(int x) : B(x + 10) {} };\n"
		"int main() { D d(3); D e(4, 6); O o(1); return d.b + e.b + o.o; }"));
}


//////////////////////////////////////////////////////////////////////////
//
// The rest of a big array is zeroed all at once (ZeroBytes), not an element
// at a time, up to its end and no further; a small or volatile array's is
// still each element's.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRZeroedArrays(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	AssertInt(0, TestCPP65Main(
		"struct S { int iBefore; char a[61]; char cAfter; };\n"
		"int main()\n"
		"{\n"
		"\tchar acGuard[3]; char sz[256] = \"xy\"; int ai[50] = { 5 }; S s = { 1, \"q\", 9 }; long al[3][10] = { { 1 } }; int i;\n"
		"\tfor (i = 0; i < 3; i++) { acGuard[i] = 7; }\n"
		"\tif ((sz[0] != 'x') || (sz[1] != 'y') || (ai[0] != 5) || (al[0][0] != 1)) { return 1; }\n"
		"\tfor (i = 2; i < 256; i++) { if (sz[i]) { return 2; } }\n"
		"\tfor (i = 1; i < 50; i++) { if (ai[i]) { return 3; } }\n"
		"\tfor (i = 1; i < 30; i++) { if (al[i / 10][i % 10]) { return 4; } }\n"
		"\tif ((s.iBefore != 1) || (s.a[0] != 'q') || (s.cAfter != 9)) { return 5; }\n"
		"\tfor (i = 1; i < 61; i++) { if (s.a[i]) { return 6; } }\n"
		"\treturn 0;\n"
		"}\n"));

	//A few instructions, not 255 stores.
	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, "int Big() { volatile int i = 0; char sz[256] = \"x\"; return sz[i]; }"));
	AssertTrue(szAssembly.Count(10) < 100);
	szAssembly.Kill();
	szErrors.Kill();

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, "int Each() { volatile char sz[64] = { 1 }; return sz[0]; }"));
	AssertTrue(szAssembly.Count(10) > 63);
	szAssembly.Kill();
	szErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times szFind is in pszText.
//
//////////////////////////////////////////////////////////////////////////
static int TestCPP65IRCount(CChars* pszText, const char* szFind)
{
	const char*	sz;
	int			iCount;

	iCount = 0;
	for (sz = strstr(pszText->Text(), szFind); sz; sz = strstr(sz + 1, szFind))
	{
		iCount++;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A function template's instances (SCPP65Template::mapsInstances): the same
// arguments are the one instance however often it is called, others another,
// and a template of the same arguments has its own; a specialisation stands
// for its arguments.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRTemplateInstances(void)
{
	static const char*	szSource =
		"template<class T> T Same(T t) { return t + 1; }\n"
		"template<class T> T Other(T t) { return t + 2; }\n"
		"template<> short Same<short>(short t) { return t + 10; }\n"
		"int main() { char c = 1; short s = 5; return Same(1) + Same(2) + Same(c) + Other(3) + Other(4) + Same(s); }";
	CChars	szAssembly;
	CChars	szErrors;

	AssertInt(33, TestCPP65Main(szSource));

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource));
	AssertInt(3, TestCPP65IRCount(&szAssembly, ".section text._Same"));
	AssertInt(1, TestCPP65IRCount(&szAssembly, ".section text._Other"));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Member function bodies wait until their class is complete, first found
// first.  A class in one of them finishes the others (and its own) from its
// end, and the classes after them still have theirs parsed.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRDeferredBodies(void)
{
	AssertInt(21, TestCPP65Main(
		"struct A\n"
		"{\n"
		"\tint F() { struct L { int G() { return H(); } int H() { return 1; } }; L l; return l.G() + K(); }\n"
		"\tint J() { return K() + 1; }\n"
		"\tint K() { return 2; }\n"
		"};\n"
		"struct B { int M() { struct P { struct Q { int R() { return 4; } }; int S() { Q q; return q.R(); } }; P p; return p.S() + N(); } int N() { return 5; } };\n"
		"struct C { int T() { return U(); } int U() { return 6; } };\n"
		"int main() { A a; B b; C c; return a.F() + a.J() + b.M() + c.T(); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// The parser's and the generator's errors after a #line, and the macro use an
// error names, are where it says.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRLineErrors(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "#line 50 \"Mine.cpp\"\nint F() { return giNotDeclared; }\n"));
	AssertTrue(szErrors.Contains("Mine.cpp (50, "));
	szErrors.Kill();
	szAssembly.Kill();

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "#line 20 \"Mine.cpp\"\nint F( {\n"));
	AssertTrue(szErrors.Contains("Mine.cpp (20, "));
	szErrors.Kill();
	szAssembly.Kill();

	szAssembly.Init();
	szErrors.Init();
	AssertFalse(TestCPP65Generate(&szAssembly, &szErrors, "#define M giNotDeclared\n#line 30 \"Mine.cpp\"\nint F() { return M; }\n"));
	AssertTrue(szErrors.Contains("at Mine.cpp (30, "));
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A function that returns a function pointer needs no typedef: declared,
// defined, as a member, and called straight away.  A name in brackets is the
// name, and a pointer to one, or an array of them, is still a variable.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRFunctionsReturningFunctions(void)
{
	AssertInt(5 + 8 + 2 + 3 + 6 + 4 + 5 + 8 + 1 + 0, TestCPP65Main(
		"int Add1(int a) { return a + 1; }\n"
		"int Twice(int a) { return a * 2; }\n"
		"int (*Declared(int i))(int);\n"
		"int (*Pick(int i))(int) { return i ? Add1 : Twice; }\n"
		"int (*Declared(int i))(int) { return Pick(i); }\n"
		"struct S { int (*Get())(int) { return Add1; } static int (*Static(int i))(int) { return Pick(i); } };\n"
		"int (Bracketed)(int a) { return a + 3; }\n"
		"int ((Twoce))(int a) { return a + 4; }\n"
		"int (*gpf)(int) = Add1;\n"
		"int (*gapf[2])(int) = { Add1, Twice };\n"
		"int main() { S s; int (*pf)(int) = Pick(0);\n"
		"\treturn Pick(1)(4) + Pick(0)(4) + Declared(1)(1) + s.Get()(2) + pf(3) + (Bracketed)(1) + Twoce(1) + S::Static(0)(4) + gpf(0) + gapf[1](0); }"));
}


//////////////////////////////////////////////////////////////////////////
//
// The 65C64's thunks: a member pointer's to a virtual function goes through
// the vtable with its slot's offset (CPP65_VTABLE_ENTRY_BYTES apart), and one
// of a secondary base's vtable adjusts this.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRThunks(void)
{
	CChars	szAssembly;
	CChars	szErrors;

	szAssembly.Init();
	szErrors.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors,
		"struct A { int a; virtual int F() { return 1; } virtual int G() { return 2; } };\n"
		"struct B { int b; virtual int H() { return 3; } };\n"
		"struct C : A, B { int H() { return 4; } };\n"
		"int Call(A* p) { int (A::*f)() = &A::G; return (p->*f)(); }\n"
		"int Make(void) { C c; return c.H(); }\n"));
	AssertTrue(szAssembly.Contains(
		"__vcall_0_1:\n"
		"\t\tcpy A, [0 + X]\n"
		"\t\tjmp [4 + A]\n"));
	AssertTrue(szAssembly.Contains("\t\tadd X, -8\n\t\tjmp "));
	AssertString("", szErrors.Text());
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A call also looks in the namespaces of its arguments' types: their classes,
// bases, template arguments and enums, through pointers and references.  An
// operator does too, and a friend is in its class's namespace.  A qualified name
// doesn't.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRNamespaceArguments(void)
{
	AssertInt(1, TestCPP65Main("namespace N { struct S { int i; }; int Get(S s) { return s.i; } }\nint main() { N::S s; s.i = 1; return Get(s); }"));
	AssertInt(2, TestCPP65Main("namespace N { struct S { int i; }; int Get(S* p) { return p->i; } int Get(const S& r) { return r.i + 1; } }\nint main() { N::S s; s.i = 2; return Get(&s); }"));
	AssertInt(3, TestCPP65Main("namespace N { struct S { int i; }; int Get(const S& r) { return r.i + 1; } }\nint main() { N::S s; s.i = 2; return Get(s); }"));
	AssertInt(4, TestCPP65Main("namespace N { struct B { int i; }; int Get(B& b) { return b.i; } }\nstruct D : N::B { };\nint main() { D d; d.i = 4; return Get(d); }"));
	AssertInt(5, TestCPP65Main("namespace N { struct S { int i; }; template<class T> struct Box { T t; }; int Get(Box<S>& b) { return b.t.i; } }\ntemplate<class T> struct Own { T t; };\nnamespace M { struct A { int i; }; int Get(Own<A>& o) { return o.t.i; } }\nint main() { Own<M::A> o; o.t.i = 5; return Get(o); }"));
	AssertInt(6, TestCPP65Main("namespace N { enum E { A = 6 }; int Get(E e) { return e; } }\nint main() { return Get(N::A); }"));
	AssertInt(7, TestCPP65Main("namespace N { struct S { int i; }; template<class T> int Get(T t, S s) { return t + s.i; } }\nint main() { N::S s; s.i = 3; return Get(4, s); }"));
	AssertInt(8, TestCPP65Main("namespace N { struct S { int i; }; int Get(S s) { return s.i; } }\ntemplate<class T> int Call(T t) { return Get(t); }\nint main() { N::S s; s.i = 8; return Call(s); }"));
	AssertInt(9, TestCPP65Main("namespace N { struct S { int i; }; int F(S s) { return s.i; } }\nint F(int a) { return a; }\nint main() { N::S s; s.i = 4; return F(s) + F(5); }"));
	AssertInt(10, TestCPP65Main("namespace N { struct S { int i; friend int Peek(S s) { return s.i; } }; }\nint main() { N::S s; s.i = 10; return Peek(s); }"));
	AssertInt(11, TestCPP65Main("int Peek(int a) { return a; }\nnamespace N { struct S { int i; friend int Peek(S s) { return s.i + 1; } }; }\nint main() { N::S s; s.i = 10; return Peek(s); }"));
	AssertInt(12, TestCPP65Main("namespace N { struct S { int i; friend S operator+(S a, S b) { S s; s.i = a.i + b.i; return s; } }; }\nint main() { N::S a; N::S b; a.i = 5; b.i = 7; return (a + b).i; }"));
	AssertInt(13, TestCPP65Main("namespace A { struct S { int i; }; int operator+(S a, int b) { return a.i + b; } }\nnamespace B { struct S { int i; }; int operator+(S a, int b) { return a.i - b; } }\nint main() { A::S a; B::S b; a.i = 10; b.i = 5; return (a + 1) + (b + 3); }"));
	AssertInt(14, TestCPP65Main("namespace N { struct S { int i; }; template<class T> int operator+(S a, T b) { return a.i + b; } }\nint main() { N::S a; a.i = 10; return a + 4; }"));
	AssertInt(15, TestCPP65Main("namespace N { struct S { int i; }; bool operator==(S a, S b) { return a.i == b.i; } }\ntemplate<class T> int Same(T a, T b) { return (a == b) ? 15 : 0; }\nint main() { N::S a; N::S b; a.i = 1; b.i = 1; return Same(a, b); }"));
	AssertInt(16, TestCPP65Main("struct S { int i; };\nnamespace M { int operator-(S a, S b) { return a.i - b.i; } }\nusing namespace M;\nint main() { S a; S b; a.i = 20; b.i = 4; return a - b; }"));
	AssertInt(17, TestCPP65Main("struct S { int i; };\nint operator-(S a, S b) { return a.i - b.i; }\nnamespace N { int F() { S a; S b; a.i = 20; b.i = 3; return a - b; } }\nint main() { return N::F(); }"));

	AssertConstError("No function matches", "namespace N { struct S { int i; }; int Get(S s) { return s.i; } }\nint Get(int a) { return a; }\nint main() { N::S s; s.i = 1; return ::Get(s); }");
	AssertConstError("The call is ambiguous", "namespace A { struct S { int i; }; int Get(S s, int a) { return 1; } }\nnamespace B { struct T { int i; }; int Get(A::S s, T t) { return 2; } int Get(A::S s, int a) { return 3; } }\nint main() { A::S s; B::T t; using B::Get; return Get(s, 1); }");
}


//////////////////////////////////////////////////////////////////////////
//
// szSource compiled: szExpected is what its first error is, "" for none.
//
//////////////////////////////////////////////////////////////////////////
static void AssertConstError(const char* szExpected, const char* szSource)
{
	CChars	szAssembly;
	CChars	szErrors;

	szAssembly.Init();
	szErrors.Init();
	TestCPP65Generate(&szAssembly, &szErrors, szSource, true);
	if (szExpected[0] == '\0')
	{
		AssertString("", szErrors.Text());
	}
	else
	{
		AssertTrue(szErrors.Contains(szExpected));
	}
	szErrors.Kill();
	szAssembly.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What const forbids is an error: changing a const object, a pointer or
// reference that takes const away, and a function that isn't const called on a
// const object.  What it allows, and a cast, isn't.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IRConstErrors(void)
{
	static const char*	szChange = "A const object cannot be changed";
	static const char*	szPointer = "takes const away";
	static const char*	szBind = "cannot be bound to a reference";

	AssertConstError(szChange, "int main() { const int a = 1; a = 2; return a; }");
	AssertConstError(szChange, "int main() { const int a = 1; a += 2; return a; }");
	AssertConstError(szChange, "int main() { const int a = 1; a++; return a; }");
	AssertConstError(szChange, "int main() { const int a = 1; --a; return a; }");
	AssertConstError(szChange, "int main() { int x = 1; const int* p = &x; *p = 2; return x; }");
	AssertConstError(szChange, "int main() { int x = 1; int* const p = &x; p = 0; return x; }");
	AssertConstError(szChange, "int main() { const int a[2] = { 1, 2 }; a[0] = 3; return a[1]; }");
	AssertConstError(szChange, "int F(const int i) { i = 2; return i; }\nint main() { return F(1); }");
	AssertConstError(szChange, "int F(int* const p) { p = 0; return 1; }\nint main() { return F(0); }");
	AssertConstError(szChange, "struct S { int i; };\nint main() { S s; const S& r = s; r.i = 1; return 0; }");
	AssertConstError(szChange, "struct S { int i; int Set() const { i = 1; return i; } };\nint main() { S s; return s.Set(); }");
	AssertConstError(szChange, "struct S { int i; int Set() const { this->i = 1; return i; } };\nint main() { S s; return s.Set(); }");
	AssertConstError(szChange, "struct S { int i; };\nint main() { S s; const S c = s; c = s; return 0; }");

	AssertConstError(szPointer, "int main() { const int a = 1; int* p = &a; return *p; }");
	AssertConstError(szPointer, "int main() { const int a = 1; int* p; p = &a; return *p; }");
	AssertConstError(szPointer, "int main() { const int a = 1; void* p = &a; return p != 0; }");
	AssertConstError("No function matches the arguments (const long*): it takes (long*)", "int F(int* p) { return 1; }\nint main() { const int a = 1; return F(&a); }");
	AssertConstError(szPointer, "int* F(const int* p) { return p; }\nint main() { return F(0) != 0; }");
	AssertConstError(szPointer, "const int* F(const int* p) { return p; }\nint main() { int* q = F(0); return q != 0; }");
	AssertConstError("", "const int* F(const int* p) { return p; }\nint main() { const int* q = F(0); return q != 0; }");
	AssertConstError("", "char* const F(char* p) { return p; }\nint main() { char* q = F(0); return q != 0; }");
	AssertConstError(szPointer, "int main() { const char s[2] = { 0, 0 }; char* p = s; return p[0]; }");
	AssertConstError(szPointer, "int main() { char c = 0; char* p = &c; const char** q = &p; return q != 0; }");

	AssertConstError(szBind, "int main() { const int a = 1; int& r = a; return r; }");
	AssertConstError("No function matches the arguments (const long): it takes (long&)", "int F(int& r) { return r; }\nint main() { const int a = 1; return F(a); }");
	AssertConstError(szBind, "int& F(const int& r) { return r; }\nint main() { int a = 1; return F(a); }");

	AssertConstError("No function matches", "struct S { int F() { return 1; } };\nint main() { const S s; return s.F(); }");

	//Allowed.
	AssertConstError("", "struct S { mutable int m; int i; int Set() const { m = 1; return m; } S() : m(0), i(0) {} };\nint main() { const S s; return s.Set(); }");
	AssertConstError("", "struct S { int* p; };\nint main() { int x = 0; S s; s.p = &x; const S& r = s; *r.p = 1; return x; }");
	AssertConstError("", "int main() { int x = 1; const int* p = &x; p = 0; return x; }");
	AssertConstError("", "int main() { const int a = 1; int b = a; b = 2; return b; }");
	AssertConstError("", "int main() { const int a = 1; int* p = (int*)&a; *p = 1; return 0; }");
	AssertConstError("", "int main() { const int a = 1; int* p = const_cast<int*>(&a); return *p; }");
	AssertConstError("", "int main() { char c = 0; char* p = &c; const char* const* q = &p; return q != 0; }");
	AssertConstError("", "int main() { char* p = (char*)\"literal\"; const char* q = \"literal\"; return p == q; }");
	AssertConstError("", "struct S { int i; S() : i(1) {} };\nstruct T { const int k; const S s; T() : k(2) {} };\nint main() { T t; return t.k; }");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65IR(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();
	C65C64InstructionFactory::GetInstance()->Init();

	TestCPP65IRArithmetic();
	TestCPP65IRControl();
	TestCPP65IRMemory();
	TestCPP65IRCalls();
	TestCPP65IRMembers();
	TestCPP65IRObjectPlaces();
	TestCPP65IRConstructors();
	TestCPP65IRObjectsByValue();
	TestCPP65IRMemcpy();
	TestCPP65IRUnsupportedReason();
	TestCPP65IRStringEscapes();
	TestCPP65IRStringCompare();
	TestCPP65IRDevirtualise();
	TestCPP65IRInlineConstants();
	TestCPP65IRDynamicCast();
	TestCPP65IRVtableLayout();
	TestCPP65IRDynamicCastUnits();
	TestCPP65IROverloadedFunctionPointers();
	TestCPP65IRMangledNames();
	TestCPP65IRConstantTypes();
	TestCPP65IRMemberTemplateDefinedLater();
	TestCPP65IRSharedDivides();
	TestCPP65IRAnonymousLocals();
	TestCPP65IRConstClassSize();
	TestCPP65IRConstTypes();
	TestCPP65IRConstDeduction();
	TestCPP65IRConstOverloads();
	TestCPP65IRConstMembers();
	TestCPP65IRConstErrors();
	TestCPP65IRTypeTraits();
	TestCPP65IROperatorLabels();
	TestCPP65IRNamespaceLabels();
	TestCPP65IRNamespaceRuns();
	TestCPP65IRNamespaceQualified();
	TestCPP65IRNamespaceOverloads();
	TestCPP65IRNamespaceTemplates();
	TestCPP65IRNamespaceArguments();
	TestCPP65IRThunks();
	TestCPP65IRFunctionsReturningFunctions();
	TestCPP65IRLineErrors();
	TestCPP65IRTemplateInstances();
	TestCPP65IRDeferredBodies();
	TestCPP65IRClassMembers();
	TestCPP65IRZeroedArrays();

	C65C64InstructionFactory::GetInstance()->Kill();
	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}
