//The conformance cases of the clause by clause sweep of the standard (TestCPPConformance.cpp; section 6 of
//CppCompilerLib/CppParserTodo.txt): each case is a rule of a clause of N4140, named in the comment above it, and a
//group is a clause.  They were written to measure the compiler, not as it was made to do them, so most of what a
//group's lists name is what the compiler has yet to do.  A case that runs returns a sum of powers of two, one for each
//thing it checks, so what it returned says which of them went wrong; what depends on the target (an int's size, what
//an unsigned short promotes to) is worked out from sizeof in the case, so that one value is right on every target.
//
//The values are what g++ 13 gives (-std=c++14 -pedantic-errors, -m32 and -m64) and a refused case is one it refuses:
//the Microsoft compiler has not been run on them yet (Conformance/hostcheck.py).  A case that includes <typeinfo>
//for that compiler declares std::type_info itself for this one, as the stages' cases do.


//Clause 4, standard conversions [conv].
static const SConformanceCase gasConformanceClause4[] =
{
	//4 [conv] p4: contextual conversion to bool
	{
		"ConvContextualBool", 63,
		"struct B { explicit operator bool() const { return v != 0; } int v; };\n"
		"int main()\n"
		"{\n"
		"	B t = { 3 };\n"
		"	B f = { 0 };\n"
		"	int r = 0;\n"
		"	if (t) r += 1;\n"
		"	if (!f) r += 2;\n"
		"	if (t && !f) r += 4;\n"
		"	if (f || t) r += 8;\n"
		"	while (f) { r += 100; break; }\n"
		"	for (; f; ) { r += 100; break; }\n"
		"	r += t ? 16 : 0;\n"
		"	do { r += 32; } while (f);\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvContextualBoolInitRefused", CONFORMANCE_REFUSED,
		"struct B { explicit operator bool() const { return true; } };\n"
		"int main() { B b; bool x = b; return x ? 1 : 2; }\n"
	},
	{
		"ConvContextualBoolArithmeticRefused", CONFORMANCE_REFUSED,
		"struct B { explicit operator bool() const { return true; } };\n"
		"int main() { B b; return b + 1; }\n"
	},
	{
		"ConvContextualBoolArgumentRefused", CONFORMANCE_REFUSED,
		"struct B { explicit operator bool() const { return true; } };\n"
		"int f(bool b) { return b ? 1 : 2; }\n"
		"int main() { B b; return f(b); }\n"
	},
	//4 [conv] p5: contextual implicit conversion
	{
		"ConvContextualSwitch", 21,
		"struct I { operator int() const { return v; } int v; };\n"
		"enum E { E0, E1, E2 };\n"
		"struct F { operator E() const { return E2; } };\n"
		"int main()\n"
		"{\n"
		"	I i = { 2 };\n"
		"	F f;\n"
		"	int r = 0;\n"
		"	switch (i) { case 1: r += 100; break; case 2: r += 1; break; default: r += 200; }\n"
		"	switch (f) { case E1: r += 100; break; case E2: r += 20; break; default: r += 200; }\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvContextualSwitchTwoRefused", CONFORMANCE_REFUSED,
		"struct I { operator int() const { return 1; } operator long() const { return 2; } };\n"
		"int main() { I i; switch (i) { case 1: return 1; } return 2; }\n"
	},
	{
		"ConvContextualSwitchExplicitRefused", CONFORMANCE_REFUSED,
		"struct I { explicit operator int() const { return 1; } };\n"
		"int main() { I i; switch (i) { case 1: return 1; } return 2; }\n"
	},
	//4.1 [conv.lval]: lvalue to rvalue
	{
		"ConvPrvalueCvStripped", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int v; };\n"
		"const S MakeS();\n"
		"const int MakeI();\n"
		"int main()\n"
		"{\n"
		"	const int c = 5;\n"
		"	volatile int v = 6;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(static_cast<const int>(c)), int>::V) r += 1;\n"
		"	if (Same<decltype(MakeS()), const S>::V) r += 2;\n"
		"	if (Same<decltype(MakeI()), int>::V && Same<decltype(+v), int>::V) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvLvalueToRvalueIncompleteRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"extern I gI;\n"
		"int f(...);\n"
		"int main() { return f(gI); }\n"
	},
	{
		"ConvLvalueNotReadInSizeof", 3,
		"int gCalls = 0;\n"
		"int* Null() { gCalls++; return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(*Null()) == sizeof(int)) r += 1;\n"
		"	if (gCalls == 0) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//4.2 [conv.array]: array to pointer
	{
		"ConvArrayToPointer", 127,
		"extern int gu[];\n"
		"int First() { return *gu; }\n"
		"int gu[2] = { 4, 5 };\n"
		"int ga[3] = { 1, 2, 3 };\n"
		"struct S { int a[2]; };\n"
		"S Make() { S s = { { 6, 7 } }; return s; }\n"
		"int Sum(const int* p, int n) { int t = 0; for (int i = 0; i < n; i++) t += p[i]; return t; }\n"
		"int main()\n"
		"{\n"
		"	int* p = ga;\n"
		"	int (*pa)[3] = &ga;\n"
		"	const char* sz = \"abc\";\n"
		"	int r = 0;\n"
		"	if (p == &ga[0]) r += 1;\n"
		"	if (*(ga + 2) == 3) r += 2;\n"
		"	if (sizeof(ga) == 3 * sizeof(int) && (*pa)[1] == 2) r += 4;\n"
		"	if (First() == 4) r += 8;\n"
		"	if (sz[1] == 'b' && *\"xyz\" == 'x') r += 16;\n"
		"	if (Make().a[1] == 7) r += 32;\n"
		"	if (Sum(ga, 3) == 6) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvArrayToPointerMulti", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gm[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };\n"
		"int main()\n"
		"{\n"
		"	int (*p)[3] = gm;\n"
		"	int r = 0;\n"
		"	if (p[1][2] == 6) r += 1;\n"
		"	if (**gm == 1 && *(*(gm + 1) + 1) == 5) r += 2;\n"
		"	if (Same<decltype(+gm), int (*)[3]>::V) r += 4;\n"
		"	if (Same<decltype(*gm), int (&)[3]>::V && sizeof(*gm) == 3 * sizeof(int)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//4.3 [conv.func]: function to pointer
	{
		"ConvFunctionToPointer", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int f(int x) { return x + 1; }\n"
		"int Call(int (*p)(int), int x) { return p(x); }\n"
		"int main()\n"
		"{\n"
		"	int (*p)(int) = f;\n"
		"	int (*q)(int) = &f;\n"
		"	int (&rf)(int) = f;\n"
		"	int r = 0;\n"
		"	if (p == q) r += 1;\n"
		"	if (Call(f, 1) == 2) r += 2;\n"
		"	if ((*f)(2) == 3 && (**f)(2) == 3) r += 4;\n"
		"	if ((*p)(3) == 4 && (**p)(3) == 4) r += 8;\n"
		"	if (rf(4) == 5 && Call(rf, 5) == 6) r += 16;\n"
		"	if (Same<decltype(+f), int (*)(int)>::V) r += 32;\n"
		"	if (Same<decltype(f), int(int)>::V && Same<decltype(*p), int (&)(int)>::V) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvMemberFunctionNoAmpersandRefused", CONFORMANCE_REFUSED,
		"struct S { int f() { return 1; } };\n"
		"int main() { int (S::*p)() = S::f; S s; return (s.*p)(); }\n"
	},
	//4.4 [conv.qual]: qualification conversions
	{
		"ConvQualification", 31,
		"struct S { int m; };\n"
		"int main()\n"
		"{\n"
		"	int i = 3;\n"
		"	int* p = &i;\n"
		"	int** pp = &p;\n"
		"	const int* cp = p;\n"
		"	const volatile int* cvp = cp;\n"
		"	const int* const* cpp = pp;\n"
		"	const int* const* const* cppp = &cpp;\n"
		"	volatile int* vp = p;\n"
		"	int S::* m = &S::m;\n"
		"	const int S::* cm = m;\n"
		"	S s = { 9 };\n"
		"	int r = 0;\n"
		"	if (*cp == 3) r += 1;\n"
		"	if (*cvp == 3 && *vp == 3) r += 2;\n"
		"	if (**cpp == 3) r += 4;\n"
		"	if (***cppp == 3) r += 8;\n"
		"	if (s.*cm == 9) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvQualificationMultiLevelRefused", CONFORMANCE_REFUSED,
		"int main() { int** pp = 0; const int** cpp = pp; return cpp ? 1 : 2; }\n"
	},
	{
		"ConvQualificationMiddleConstRefused", CONFORMANCE_REFUSED,
		"int main() { int*** ppp = 0; const int** const* x = ppp; return x ? 1 : 2; }\n"
	},
	{
		"ConvQualificationDropConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int* cp = 0; int* p = cp; return p ? 1 : 2; }\n"
	},
	{
		"ConvQualificationDropVolatileRefused", CONFORMANCE_REFUSED,
		"int main() { volatile int* vp = 0; const int* p = vp; return p ? 1 : 2; }\n"
	},
	{
		"ConvQualificationMemberDropRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { const int S::* cm = &S::m; int S::* m = cm; S s; s.*m = 1; return 1; }\n"
	},
	{
		"ConvQualificationMixedMember", 3,
		"struct S { int* m; };\n"
		"int main()\n"
		"{\n"
		"	int i = 7;\n"
		"	S s = { &i };\n"
		"	int* S::* pm = &S::m;\n"
		"	const int* const S::* cpm = pm;\n"
		"	int* S::* * ppm = &pm;\n"
		"	const int* const S::* const* cppm = ppm;\n"
		"	int r = 0;\n"
		"	if (*(s.*cpm) == 7) r += 1;\n"
		"	if (*(s.**cppm) == 7) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//4.5 [conv.prom]: integral promotions
	{
		"ConvPromotionSmallTypes", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	char c = 1;\n"
		"	signed char sc = 1;\n"
		"	unsigned char uc = 1;\n"
		"	short s = 1;\n"
		"	unsigned short us = 1;\n"
		"	bool b = true;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(+c), int>::V) r += 1;\n"
		"	if (Same<decltype(+sc), int>::V && Same<decltype(+uc), int>::V) r += 2;\n"
		"	if (Same<decltype(+s), int>::V) r += 4;\n"
		"	if (sizeof(int) > sizeof(short) ? Same<decltype(+us), int>::V : Same<decltype(+us), unsigned int>::V) r += 8;\n"
		"	if (Same<decltype(+b), int>::V && +b == 1) r += 16;\n"
		"	if (Same<decltype(-uc), int>::V && -uc == -1) r += 32;\n"
		"	if (Same<decltype(~uc), int>::V && ~uc == -2) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPromotionUnsignedShortValue", 3,
		"int main()\n"
		"{\n"
		"	unsigned short us = 1;\n"
		"	unsigned char uc = 1;\n"
		"	int r = 0;\n"
		"	if ((us - 2 < 0) == (sizeof(int) > sizeof(short))) r += 1;\n"
		"	if (uc - 2 < 0) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPromotionCharTypes", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	char16_t c16 = u'a';\n"
		"	char32_t c32 = U'a';\n"
		"	wchar_t w = L'a';\n"
		"	int r = 0;\n"
		"	if (sizeof(int) > 2 ? Same<decltype(+c16), int>::V : Same<decltype(+c16), unsigned int>::V) r += 1;\n"
		"	if (sizeof(int) > 4 ? Same<decltype(+c32), int>::V : (sizeof(int) == 4 ? Same<decltype(+c32), unsigned int>::V : Same<decltype(+c32), unsigned long>::V)) r += 2;\n"
		"	if (sizeof(+w) >= sizeof(int) && +w == 97 && +c16 == 97 && +c32 == 97) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPromotionEnum", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum Small { S0, S1 = 100 };\n"
		"enum Negative { N0 = -1, N1 = 1 };\n"
		"enum Big { B0 = 0, B1 = 0xFFFFFFFFu };\n"
		"enum Huge { H0 = -1, H1 = 0x7FFFFFFFFFLL };\n"
		"enum Fixed : unsigned char { F0, F1 };\n"
		"enum FixedLong : long { L0, L1 };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(+S1), int>::V && Same<decltype(+N0), int>::V) r += 1;\n"
		"	if (sizeof(int) >= 4 ? Same<decltype(+B1), unsigned int>::V : Same<decltype(+B1), unsigned long>::V) r += 2;\n"
		"	if (sizeof(long) >= 8 ? Same<decltype(+H1), long>::V : Same<decltype(+H1), long long>::V) r += 4;\n"
		"	if (Same<decltype(+F1), int>::V) r += 8;\n"
		"	if (Same<decltype(+L1), long>::V) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPromotionScopedEnumRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { return +E::B; }\n"
	},
	{
		"ConvScopedEnumToIntRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { int i = E::B; return i + 1; }\n"
	},
	{
		"ConvPromotionBitField", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { unsigned int a : 3; unsigned int b : 15; signed int c : 4; bool d : 1; };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.a = 3; s.b = 1; s.c = -3; s.d = true;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(+s.a), int>::V && s.a - 4 < 0) r += 1;\n"
		"	if (Same<decltype(+s.b), int>::V && s.b - 2 < 0) r += 2;\n"
		"	if (Same<decltype(+s.c), int>::V && s.c == -3) r += 4;\n"
		"	if (Same<decltype(+s.d), int>::V && s.d + s.d == 2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//4.5 [conv.prom], 4.6 [conv.fpprom]: a promotion is better than a conversion
	{
		"ConvPromotionOverload", 2047,
		"enum E { E0, E1 };\n"
		"int f(int) { return 1; }\n"
		"int f(long) { return 2; }\n"
		"int f(unsigned int) { return 3; }\n"
		"int f(double) { return 4; }\n"
		"int f(long double) { return 5; }\n"
		"int f(char) { return 6; }\n"
		"int main()\n"
		"{\n"
		"	short s = 1;\n"
		"	unsigned char uc = 1;\n"
		"	signed char sc = 1;\n"
		"	unsigned short us = 1;\n"
		"	float fl = 1.0f;\n"
		"	int r = 0;\n"
		"	if (f(s) == 1) r += 1;\n"
		"	if (f(uc) == 1) r += 2;\n"
		"	if (f(sc) == 1) r += 4;\n"
		"	if (f(us) == (sizeof(int) > sizeof(short) ? 1 : 3)) r += 8;\n"
		"	if (f(true) == 1) r += 16;\n"
		"	if (f(E1) == 1) r += 32;\n"
		"	if (f(fl) == 4) r += 64;\n"
		"	if (f('a') == 6) r += 128;\n"
		"	if (f(1L) == 2 && f(1u) == 3) r += 256;\n"
		"	if (f(1.0) == 4 && f(1.0L) == 5) r += 512;\n"
		"	if (f(u'a') == (sizeof(int) > 2 ? 1 : 3)) r += 1024;\n"
		"	return r;\n"
		"}\n"
	},
	//4.6 [conv.fpprom]: floating point promotion
	{
		"ConvFloatPromotionVariadic", 3,
		"#include <stdarg.h>\n"
		"int First(int n, ...)\n"
		"{\n"
		"	va_list ap;\n"
		"	va_start(ap, n);\n"
		"	double d = va_arg(ap, double);\n"
		"	int i = va_arg(ap, int);\n"
		"	va_end(ap);\n"
		"	return (d == 2.5 ? 1 : 0) + (i == 7 ? 2 : 0);\n"
		"}\n"
		"int main() { float f = 2.5f; short s = 7; return First(2, f, s); }\n"
	},
	//4.7 [conv.integral]: integral conversions
	{
		"ConvIntegralUnsigned", 127,
		"int main()\n"
		"{\n"
		"	int i = 300;\n"
		"	int m = -1;\n"
		"	long big = 0x12345678L;\n"
		"	long long huge = 0x123456789ABCDEF0LL;\n"
		"	unsigned char uc = i;\n"
		"	unsigned short us = m;\n"
		"	unsigned int u = m;\n"
		"	unsigned long ul = m;\n"
		"	unsigned long long ull = m;\n"
		"	unsigned char uc2 = big;\n"
		"	unsigned short us2 = big;\n"
		"	unsigned long ul2 = huge;\n"
		"	int r = 0;\n"
		"	if (uc == 44) r += 1;\n"
		"	if (us == 65535u) r += 2;\n"
		"	if (u == ~0u && u > 0) r += 4;\n"
		"	if (ul == ~0ul && ul > 0) r += 8;\n"
		"	if (ull == ~0ull && ull > 0) r += 16;\n"
		"	if (uc2 == 0x78 && us2 == 0x5678) r += 32;\n"
		"	if (sizeof(long) == 8 || ul2 == 0x9ABCDEF0ul) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvIntegralSigned", 127,
		"enum E { E5 = 5 };\n"
		"int main()\n"
		"{\n"
		"	signed char sc = -2;\n"
		"	short s = -3;\n"
		"	unsigned char uc = 200;\n"
		"	unsigned short us = 60000u;\n"
		"	int i = -4;\n"
		"	bool t = true;\n"
		"	long l1 = sc;\n"
		"	long l2 = s;\n"
		"	long l3 = uc;\n"
		"	long l4 = us;\n"
		"	long long ll1 = i;\n"
		"	long long ll2 = us;\n"
		"	long long ll3 = l1;\n"
		"	unsigned long long ull = sc;\n"
		"	int b = t;\n"
		"	int e = E5;\n"
		"	int r = 0;\n"
		"	if (l1 == -2 && l2 == -3) r += 1;\n"
		"	if (l3 == 200 && l4 == 60000L) r += 2;\n"
		"	if (ll1 == -4 && ll1 < 0) r += 4;\n"
		"	if (ll2 == 60000LL && ll3 == -2) r += 8;\n"
		"	if (ull == 0xFFFFFFFFFFFFFFFEull) r += 16;\n"
		"	if (b == 1 && e == 5) r += 32;\n"
		"	if ((long)(unsigned long)us == 60000L && (long long)(unsigned char)uc == 200) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//4.7 [conv.integral]: integral conversions (implementation-defined: two's complement wrap)
	{
		"ConvIntegralNarrowSigned", 15,
		"int main()\n"
		"{\n"
		"	int i = 200;\n"
		"	long l = 0x12348765L;\n"
		"	long long ll = 0x1234567887654321LL;\n"
		"	unsigned long ul = 0xFFFFFFFEul;\n"
		"	signed char sc = (signed char)i;\n"
		"	short s = (short)l;\n"
		"	long l2 = (long)ll;\n"
		"	long l3 = (long)ul;\n"
		"	int r = 0;\n"
		"	if (sc == -56) r += 1;\n"
		"	if (s == (short)0x8765 && s < 0) r += 2;\n"
		"	if (sizeof(long) == 8 || l2 == (long)0x87654321L) r += 4;\n"
		"	if (sizeof(long) == 8 || l3 == -2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//4.8 [conv.double]: floating point conversions
	{
		"ConvFloating", 15,
		"int main()\n"
		"{\n"
		"	float f = 0.5f;\n"
		"	double d = f;\n"
		"	long double ld = d;\n"
		"	double third = 1.0 / 3.0;\n"
		"	float third_f = (float)third;\n"
		"	double back = third_f;\n"
		"	double big = 16777217.0;\n"
		"	float big_f = (float)big;\n"
		"	int r = 0;\n"
		"	if (d == 0.5 && ld == 0.5L) r += 1;\n"
		"	if ((float)ld == 0.5f && (double)ld == 0.5) r += 2;\n"
		"	if (back != third && back > 0.3333 && back < 0.3334) r += 4;\n"
		"	if (big_f == 16777216.0f || big_f == 16777218.0f) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//4.9 [conv.fpint]: floating-integral conversions
	{
		"ConvFloatToInt", 255,
		"int main()\n"
		"{\n"
		"	double d = 3.99;\n"
		"	double n = -3.99;\n"
		"	float f = 2.5f;\n"
		"	double big = 4000000000.0;\n"
		"	double huge = 1099511627776.0;\n"
		"	double small = 0.9;\n"
		"	int r = 0;\n"
		"	if ((int)d == 3) r += 1;\n"
		"	if ((int)n == -3) r += 2;\n"
		"	if ((int)f == 2 && (long)f == 2) r += 4;\n"
		"	if ((unsigned long)big == 4000000000ul) r += 8;\n"
		"	if ((long long)huge == 0x10000000000LL) r += 16;\n"
		"	if ((int)small == 0 && (int)-small == 0) r += 32;\n"
		"	if ((unsigned char)d == 3 && (short)n == -3 && (char)f == 2) r += 64;\n"
		"	if ((unsigned long long)huge == 0x10000000000ull && (long long)-huge == -0x10000000000LL) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvIntToFloat", 255,
		"int main()\n"
		"{\n"
		"	int i = 12345;\n"
		"	int m = -7;\n"
		"	unsigned long ul = 4000000000ul;\n"
		"	long long ll = 0x10000000000LL;\n"
		"	unsigned long long ull = 0x8000000000000000ull;\n"
		"	long inexact = 16777217L;\n"
		"	unsigned char uc = 200;\n"
		"	bool b = true;\n"
		"	int r = 0;\n"
		"	if ((double)i == 12345.0 && (float)i == 12345.0f) r += 1;\n"
		"	if ((double)m == -7.0 && (float)m == -7.0f) r += 2;\n"
		"	if ((double)ul == 4000000000.0 && (double)ul > 0) r += 4;\n"
		"	if ((double)ll == 1099511627776.0) r += 8;\n"
		"	if ((double)ull == 9223372036854775808.0 && (double)ull > 0) r += 16;\n"
		"	if ((float)inexact == 16777216.0f || (float)inexact == 16777218.0f) r += 32;\n"
		"	if ((double)uc == 200.0 && (float)uc == 200.0f) r += 64;\n"
		"	if ((double)b == 1.0 && (double)inexact == 16777217.0) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//4.10 [conv.ptr] p1: null pointer constants
	{
		"ConvNullPointerConstant", 63,
		"struct S { int m; };\n"
		"void g() {}\n"
		"int main()\n"
		"{\n"
		"	int* a = 0;\n"
		"	int* b = 0L;\n"
		"	int* c = nullptr;\n"
		"	char* d = 0u;\n"
		"	long* e = 0x0;\n"
		"	void (*f)() = 0;\n"
		"	void* v = 00;\n"
		"	const S* s = 0LL;\n"
		"	int i = 1;\n"
		"	int* p = &i;\n"
		"	int r = 0;\n"
		"	if (a == 0 && b == 0) r += 1;\n"
		"	if (c == nullptr && nullptr == c) r += 2;\n"
		"	if (!d && !e) r += 4;\n"
		"	if (f == 0 && v == 0 && s == nullptr) r += 8;\n"
		"	if (p != 0 && p != nullptr && 0 != p) r += 16;\n"
		"	p = 0;\n"
		"	f = g;\n"
		"	if (!p && f != 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvNullPointerConstVariableRefused", CONFORMANCE_REFUSED,
		"int main() { const int z = 0; int* p = z; return p ? 1 : 2; }\n"
	},
	{
		"ConvNullPointerExpressionRefused", CONFORMANCE_REFUSED,
		"int main() { int* p = (1 - 1); return p ? 1 : 2; }\n"
	},
	{
		"ConvIntegerToPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int* p = 1; return p ? 1 : 2; }\n"
	},
	{
		"ConvPointerToIntegerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = &i; return l ? 1 : 2; }\n"
	},
	{
		"ConvIntegerToPointerAssignRefused", CONFORMANCE_REFUSED,
		"int main() { int* p = 0; int i = 4; p = i; return p ? 1 : 2; }\n"
	},
	{
		"ConvIntegerToPointerArgumentRefused", CONFORMANCE_REFUSED,
		"int f(int* p) { return p ? 1 : 2; }\n"
		"int main() { int i = 0; return f(i); }\n"
	},
	{
		"ConvFalseToPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int* p = false; return p ? 1 : 2; }\n"
	},
	//4.10 [conv.ptr] p2: object pointer to void pointer
	{
		"ConvPointerToVoid", 31,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"struct D : A, B { int d; };\n"
		"int Same(const void* p, const void* q) { return p == q; }\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	const int ci = 6;\n"
		"	D d;\n"
		"	void* v = &i;\n"
		"	const void* cv = &ci;\n"
		"	const volatile void* cvv = v;\n"
		"	void* vd = &d;\n"
		"	B* pb = &d;\n"
		"	void* vb = pb;\n"
		"	int** pp = 0;\n"
		"	void* vpp = pp;\n"
		"	int r = 0;\n"
		"	if (*(int*)v == 5 && (int*)v == &i) r += 1;\n"
		"	if (*(const int*)cv == 6) r += 2;\n"
		"	if (cvv == v && Same(&i, v)) r += 4;\n"
		"	if (vd == (void*)(A*)&d && vb != vd) r += 8;\n"
		"	if (vpp == 0 && Same(&d, vd)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPointerToVoidDropConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int ci = 1; void* v = &ci; return v ? 1 : 2; }\n"
	},
	{
		"ConvFunctionPointerToVoidRefused", CONFORMANCE_REFUSED,
		"void f() {}\n"
		"int main() { void* v = f; return v ? 1 : 2; }\n"
	},
	{
		"ConvVoidToObjectPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; void* v = &i; int* p = v; return *p; }\n"
	},
	//4.10 [conv.ptr]: pointer conversions
	{
		"ConvUnrelatedPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long* p = &i; return p ? 1 : 2; }\n"
	},
	{
		"ConvUnrelatedClassPointerRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"int main() { A a; B* p = &a; return p ? 1 : 2; }\n"
	},
	//4.10 [conv.ptr] p3: derived to base
	{
		"ConvPointerDerivedToBase", 63,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"struct D : A, B { int d; };\n"
		"struct E : D { int e; };\n"
		"int ReadB(const B* p) { return p ? p->b : -1; }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	E e;\n"
		"	d.a = 1; d.b = 2; d.d = 3;\n"
		"	e.a = 4; e.b = 5; e.d = 6; e.e = 7;\n"
		"	A* pa = &d;\n"
		"	B* pb = &d;\n"
		"	D* nd = 0;\n"
		"	B* nb = nd;\n"
		"	B* eb = &e;\n"
		"	const B* cb = &d;\n"
		"	int r = 0;\n"
		"	if (pa->a == 1 && (void*)pa == (void*)&d) r += 1;\n"
		"	if (pb->b == 2 && (char*)pb != (char*)&d) r += 2;\n"
		"	if (nb == 0 && !nb) r += 4;\n"
		"	if (eb->b == 5 && cb->b == 2) r += 8;\n"
		"	if (ReadB(&d) == 2 && ReadB(&e) == 5) r += 16;\n"
		"	if (ReadB(nd) == -1) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPointerDerivedToVirtualBase", 15,
		"struct V { int v; };\n"
		"struct L : virtual V { int l; };\n"
		"struct R : virtual V { int r; };\n"
		"struct D : L, R { int d; };\n"
		"int ReadV(V* p) { return p ? p->v : -1; }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	d.v = 9; d.l = 1; d.r = 2; d.d = 3;\n"
		"	V* pv = &d;\n"
		"	L* pl = &d;\n"
		"	R* pr = &d;\n"
		"	V* lv = pl;\n"
		"	V* rv = pr;\n"
		"	D* nd = 0;\n"
		"	V* nv = nd;\n"
		"	int r = 0;\n"
		"	if (pv->v == 9) r += 1;\n"
		"	if (lv == pv && rv == pv) r += 2;\n"
		"	if (nv == 0) r += 4;\n"
		"	if (ReadV(&d) == 9 && ReadV(pr) == 9 && ReadV(nd) == -1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvPointerAmbiguousBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A* p = &d; return p ? 1 : 2; }\n"
	},
	{
		"ConvPointerInaccessibleBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D : private A { };\n"
		"int main() { D d; A* p = &d; return p ? 1 : 2; }\n"
	},
	{
		"ConvPointerBaseToDerivedRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D : A { };\n"
		"int main() { A a; D* p = &a; return p ? 1 : 2; }\n"
	},
	{
		"ConvPointerIncompleteDerivedRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D;\n"
		"D* Get();\n"
		"int main() { A* p = Get(); return p ? 1 : 2; }\n"
	},
	//4.11 [conv.mem]: pointer to member conversions
	{
		"ConvMemberPointer", 63,
		"struct A { int a; int f() { return a; } };\n"
		"struct B { int b; };\n"
		"struct D : B, A { int d; };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	d.a = 5; d.b = 6; d.d = 7;\n"
		"	int A::* pm = &A::a;\n"
		"	int D::* pd = pm;\n"
		"	int (A::*pf)() = &A::f;\n"
		"	int (D::*pdf)() = pf;\n"
		"	int A::* na = 0;\n"
		"	int D::* nd = na;\n"
		"	int (A::*nf)() = nullptr;\n"
		"	int (D::*ndf)() = nf;\n"
		"	int D::* pb = &B::b;\n"
		"	int r = 0;\n"
		"	if (d.*pd == 5) r += 1;\n"
		"	if ((d.*pdf)() == 5) r += 2;\n"
		"	if (nd == 0 && !nd && na == nullptr) r += 4;\n"
		"	if (ndf == 0 && !ndf) r += 8;\n"
		"	if (d.*pb == 6 && pd != pb) r += 16;\n"
		"	if (pd != 0 && pdf != 0 && pm != nullptr) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvMemberPointerDerivedToBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D : A { int d; };\n"
		"int main() { int D::* pd = &D::d; int A::* pa = pd; return pa ? 1 : 2; }\n"
	},
	{
		"ConvMemberPointerVirtualBaseRefused", CONFORMANCE_REFUSED,
		"struct V { int v; };\n"
		"struct D : virtual V { int d; };\n"
		"int main() { int D::* p = &V::v; return p ? 1 : 2; }\n"
	},
	{
		"ConvMemberPointerAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { int D::* p = &A::a; return p ? 1 : 2; }\n"
	},
	{
		"ConvMemberPointerFromIntegerRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"int main() { int A::* p = 1; return p ? 1 : 2; }\n"
	},
	{
		"ConvMemberPointerUnrelatedRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"int main() { int B::* p = &A::a; return p ? 1 : 2; }\n"
	},
	//4.12 [conv.bool]: boolean conversions
	{
		"ConvBoolean", 1023,
		"struct S { int m; int f() { return 1; } };\n"
		"enum E { E0, E2 = 2 };\n"
		"void g() {}\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	int* p = &i;\n"
		"	int* n = 0;\n"
		"	int S::* pm = &S::m;\n"
		"	int S::* nm = 0;\n"
		"	int (S::*pf)() = &S::f;\n"
		"	bool b1 = 5;\n"
		"	bool b2 = 256;\n"
		"	bool b3 = 0x10000L;\n"
		"	bool b4 = 0x100000000LL;\n"
		"	bool b5 = 0.5;\n"
		"	bool b6 = 0.0;\n"
		"	bool b7 = p;\n"
		"	bool b8 = n;\n"
		"	bool b9 = pm;\n"
		"	bool b10 = nm;\n"
		"	bool b11 = E2;\n"
		"	bool b12 = E0;\n"
		"	bool b13 = -0.0f;\n"
		"	bool b14 = pf;\n"
		"	bool b15(nullptr);\n"
		"	bool b16 = 0.25f;\n"
		"	int r = 0;\n"
		"	if (b1 && b1 == true && (int)b1 == 1) r += 1;\n"
		"	if (b2 && (int)b2 == 1) r += 2;\n"
		"	if (b3 && (int)b3 == 1) r += 4;\n"
		"	if (b4 && (int)b4 == 1) r += 8;\n"
		"	if (b5 && !b6 && !b13 && b16) r += 16;\n"
		"	if (b7 && !b8) r += 32;\n"
		"	if (b9 && !b10 && b14) r += 64;\n"
		"	if (b11 && !b12) r += 128;\n"
		"	if (!b15) r += 256;\n"
		"	if (sizeof(bool) == 1 && (int)b5 == 1 && (int)b7 == 1 && (int)b9 == 1) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvBooleanAssign", 31,
		"bool Is(bool b) { return b; }\n"
		"bool FromLong(long l) { return l; }\n"
		"bool FromDouble(double d) { return d; }\n"
		"int main()\n"
		"{\n"
		"	bool b = false;\n"
		"	long l = 0x10000L;\n"
		"	double d = 0.125;\n"
		"	const char* sz = \"a\";\n"
		"	int r = 0;\n"
		"	b = 0x4000;\n"
		"	if (b && (int)b == 1) r += 1;\n"
		"	b = l;\n"
		"	if (b && (int)b == 1) r += 2;\n"
		"	if (Is(0x100) && Is(l) && Is(d) && Is(sz)) r += 4;\n"
		"	if (FromLong(l) && !FromLong(0) && FromDouble(d) && !FromDouble(0.0)) r += 8;\n"
		"	b = d;\n"
		"	if ((int)b == 1 && (b + b) == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvBoolFromNullptrCopyRefused", CONFORMANCE_REFUSED,
		"int main() { bool b = nullptr; return b ? 1 : 2; }\n"
	},
	{
		"ConvBoolFromScopedEnumRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { bool b = E::B; return b ? 1 : 2; }\n"
	},
	//4.13 [conv.rank], 5 [expr] p10: usual arithmetic conversions
	{
		"ConvRank", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	char c = 1; short s = 1; int i = 1; unsigned int u = 1; long l = 1; unsigned long ul = 1; long long ll = 1; unsigned long long ull = 1;\n"
		"	float f = 1; double d = 1; long double ld = 1;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(c + c), int>::V && Same<decltype(s + c), int>::V) r += 1;\n"
		"	if (Same<decltype(i + u), unsigned int>::V && Same<decltype(i + l), long>::V) r += 2;\n"
		"	if (sizeof(long) > sizeof(int) ? Same<decltype(u + l), long>::V : Same<decltype(u + l), unsigned long>::V) r += 4;\n"
		"	if (Same<decltype(l + ul), unsigned long>::V && Same<decltype(ll + i), long long>::V) r += 8;\n"
		"	if (sizeof(long long) > sizeof(long) ? Same<decltype(ul + ll), long long>::V : Same<decltype(ul + ll), unsigned long long>::V) r += 16;\n"
		"	if (Same<decltype(ull + ll), unsigned long long>::V && Same<decltype(ull + c), unsigned long long>::V) r += 32;\n"
		"	if (Same<decltype(f + i), float>::V && Same<decltype(f + ull), float>::V && Same<decltype(d + f), double>::V) r += 64;\n"
		"	if (Same<decltype(ld + d), long double>::V && Same<decltype(ld + ll), long double>::V && Same<decltype(f + f), float>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConvRankValues", 31,
		"int main()\n"
		"{\n"
		"	int i = -1;\n"
		"	unsigned int u = 1;\n"
		"	long l = -1;\n"
		"	unsigned long ul = 1;\n"
		"	unsigned short us = 1;\n"
		"	long long ll = -1;\n"
		"	int r = 0;\n"
		"	if (i < u == false && i + u == 0) r += 1;\n"
		"	if ((l < u) == (sizeof(long) > sizeof(int))) r += 2;\n"
		"	if (!(l < ul) && !(ll < 1ull)) r += 4;\n"
		"	if ((i < us) == (sizeof(int) > sizeof(short))) r += 8;\n"
		"	if ((ll < ul) == (sizeof(long long) > sizeof(long)) && ll < u) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
};


//Clause 5, expressions [expr].
static const SConformanceCase gasConformanceClause5[] =
{
	//5 [expr] p10: usual arithmetic conversions, scoped enumerations
	{
		"ExprScopedEnumNoConversion", 7,
		"enum class E { A, B, C };\n"
		"int main()\n"
		"{\n"
		"	E a = E::A;\n"
		"	E c = E::C;\n"
		"	int r = 0;\n"
		"	if (a < c && c > a && a <= a && c >= a) r += 1;\n"
		"	if (a != c && a == E::A) r += 2;\n"
		"	if ((c == E::C ? 4 : 0) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ExprScopedEnumArithmeticRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { E e = E::B; return (e + 1) ? 1 : 2; }\n"
	},
	{
		"ExprScopedEnumMixedCompareRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"enum class F { A, B };\n"
		"int main() { return (E::A == F::A) ? 1 : 2; }\n"
	},
	{
		"ExprScopedEnumBitwiseRefused", CONFORMANCE_REFUSED,
		"enum class E { A = 1, B = 2 };\n"
		"int main() { E e = E::A | E::B; return e == E::A ? 1 : 2; }\n"
	},
	//5 [expr] p10: usual arithmetic conversions
	{
		"ExprUnscopedEnumArithmetic", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E1 = 1, E2 = 2 };\n"
		"enum F { F4 = 4 };\n"
		"int main()\n"
		"{\n"
		"	E e = E2;\n"
		"	int r = 0;\n"
		"	if (e + 1 == 3 && e * F4 == 8) r += 1;\n"
		"	if (Same<decltype(e + e), int>::V && Same<decltype(e | F4), int>::V) r += 2;\n"
		"	if (e < F4 && (E1 | E2) == 3) r += 4;\n"
		"	if (Same<decltype(e + 1.5), double>::V && e + 1.5 == 3.5) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5 [expr] p11: discarded-value expressions
	{
		"ExprDiscardedValue", 15,
		"int gCount = 0;\n"
		"struct S { S(int x) : v(x) { gCount += 10; } ~S() { gCount += 1; } int v; };\n"
		"int Call() { gCount += 100; return 5; }\n"
		"int main()\n"
		"{\n"
		"	volatile int v = 3;\n"
		"	int i = 4;\n"
		"	int a[2] = { 1, 2 };\n"
		"	int r = 0;\n"
		"	S(1);\n"
		"	if (gCount == 11) r += 1;\n"
		"	Call();\n"
		"	(void)Call();\n"
		"	if (gCount == 211) r += 2;\n"
		"	v;\n"
		"	i;\n"
		"	a;\n"
		"	Call;\n"
		"	i, v;\n"
		"	(void)a;\n"
		"	if (gCount == 211 && v == 3) r += 4;\n"
		"	S(2), S(3), Call();\n"
		"	if (gCount == 333) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5 [expr] p5: an expression's type is never a reference
	{
		"ExprReferenceAdjusted", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gi = 3;\n"
		"int& Ref() { return gi; }\n"
		"int&& RRef() { return static_cast<int&&>(gi); }\n"
		"int main()\n"
		"{\n"
		"	int& ri = gi;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(ri + 0), int>::V && sizeof(ri) == sizeof(int)) r += 1;\n"
		"	if (Same<decltype((ri)), int&>::V && Same<decltype(Ref()), int&>::V) r += 2;\n"
		"	if (Same<decltype(RRef()), int&&>::V && Same<decltype((RRef())), int&&>::V) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p1: literals
	{
		"PrimLiteralCategories", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	const char (*p)[4] = &\"abc\";\n"
		"	int r = 0;\n"
		"	if ((*p)[2] == 'c' && sizeof(*p) == 4) r += 1;\n"
		"	if (Same<decltype(\"abc\"), const char (&)[4]>::V) r += 2;\n"
		"	if (Same<decltype(5), int>::V && Same<decltype('a'), char>::V && Same<decltype(true), bool>::V) r += 4;\n"
		"	if (Same<decltype(nullptr), decltype(nullptr)>::V && Same<decltype(1.5f), float>::V && Same<decltype(L'a'), wchar_t>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"PrimAddressOfLiteralRefused", CONFORMANCE_REFUSED,
		"int main() { const int* p = &5; return *p; }\n"
	},
	//5.1.1 [expr.prim.general] p3: this
	{
		"PrimThisType", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S\n"
		"{\n"
		"	int m;\n"
		"	int a = 2;\n"
		"	int b = this->a + 5;\n"
		"	int Plain() { return Same<decltype(this), S*>::V; }\n"
		"	int Const() const { return Same<decltype(this), const S*>::V; }\n"
		"	int Volatile() volatile { return Same<decltype(this), volatile S*>::V; }\n"
		"	auto Trailing() const -> decltype(this->m) { return m; }\n"
		"	S* Self() { return this; }\n"
		"	static int Size() { return sizeof(m); }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.m = 4;\n"
		"	int r = 0;\n"
		"	if (s.Plain() && s.Const() && s.Volatile()) r += 1;\n"
		"	if (s.b == 7) r += 2;\n"
		"	if (s.Trailing() == 4 && Same<decltype(s.Trailing()), int>::V) r += 4;\n"
		"	if (s.Self() == &s && (*s.Self()).m == 4) r += 8;\n"
		"	if (S::Size() == sizeof(int)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"PrimThisOutsideRefused", CONFORMANCE_REFUSED,
		"int f() { return this ? 1 : 2; }\n"
		"int main() { return f(); }\n"
	},
	{
		"PrimThisInStaticRefused", CONFORMANCE_REFUSED,
		"struct S { int m; static int f() { return this->m; } };\n"
		"int main() { return S::f() + 1; }\n"
	},
	{
		"PrimThisAssignRefused", CONFORMANCE_REFUSED,
		"struct S { int m; void f(S* p) { this = p; } };\n"
		"int main() { S a; S b; a.f(&b); return 1; }\n"
	},
	{
		"PrimThisInConstWritesRefused", CONFORMANCE_REFUSED,
		"struct S { int m; void f() const { this->m = 1; } };\n"
		"int main() { S a; a.f(); return 1; }\n"
	},
	//5.1.1 [expr.prim.general] p6: parenthesised expressions
	{
		"PrimParentheses", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int f(int x) { return x + 1; }\n"
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int r = 0;\n"
		"	(x) = 3;\n"
		"	((x)) += 1;\n"
		"	if (x == 4 && &(x) == &x) r += 1;\n"
		"	if (Same<decltype(x), int>::V && Same<decltype((x)), int&>::V) r += 2;\n"
		"	if ((f)(1) == 2 && (*(&f))(2) == 3 && (a)[1] == 2 && sizeof((a)) == sizeof(a)) r += 4;\n"
		"	if (((1 + 2)) * (3) == 9 && (-(x)) == -4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p7-p11: qualified names
	{
		"PrimQualifiedNames", 63,
		"int v = 1;\n"
		"namespace N { int v = 2; namespace M { int v = 3; } int f() { return ::v * 10 + v; } }\n"
		"struct B { int m; int f() { return 4; } static int s; enum { E = 6 }; typedef int T; };\n"
		"int B::s = 5;\n"
		"struct D : B { int m; int f() { return 7; } int Both() { return B::m * 10 + m; } int BaseF() { return B::f() * 10 + f(); } };\n"
		"enum U { U0, U1 };\n"
		"int main()\n"
		"{\n"
		"	int v = 9;\n"
		"	D d;\n"
		"	d.m = 2;\n"
		"	d.B::m = 1;\n"
		"	int r = 0;\n"
		"	if (::v == 1 && N::v == 2 && N::M::v == 3 && ::N::M::v == 3 && v == 9) r += 1;\n"
		"	if (N::f() == 12) r += 2;\n"
		"	if (d.Both() == 12 && d.BaseF() == 47 && d.B::f() == 4) r += 4;\n"
		"	if (B::s == 5 && D::s == 5 && d.s == 5 && B::E == 6 && D::E == 6) r += 8;\n"
		"	if (U::U1 == 1 && ::U1 == 1 && sizeof(B::T) == sizeof(int) && sizeof(D::T) == sizeof(int)) r += 16;\n"
		"	if (decltype(d)::s == 5 && decltype(d)::E == 6) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p13: a non-static member's name outside its class
	{
		"PrimMemberOutsideClassRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { int x = S::m; return x + 1; }\n"
	},
	{
		"PrimMemberFunctionOutsideClassRefused", CONFORMANCE_REFUSED,
		"struct S { int f() { return 1; } };\n"
		"int main() { return S::f(); }\n"
	},
	{
		"PrimMemberUnevaluated", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; char c[5]; long f(); };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(S::m) == sizeof(int) && sizeof(S::c) == 5) r += 1;\n"
		"	if (Same<decltype(S::m), int>::V && Same<decltype(S::c), char[5]>::V) r += 2;\n"
		"	if (sizeof(((S*)0)->f()) == sizeof(long) && noexcept(S::m)) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p8: an enumerator is a prvalue
	{
		"PrimEnumeratorIsPrvalueRefused", CONFORMANCE_REFUSED,
		"enum E { E0, E1 };\n"
		"int main() { const int* p = &E1; return *p; }\n"
	},
	//5.1.1 [expr.prim.general] p8: naming a destructor
	{
		"PrimDestructorName", 7,
		"int gCount = 0;\n"
		"struct S { ~S() { gCount++; } };\n"
		"struct T : S { ~T() { gCount += 10; } };\n"
		"typedef S Alias;\n"
		"int main()\n"
		"{\n"
		"	char buffer[sizeof(T)];\n"
		"	S* p = (S*)buffer;\n"
		"	T* q = (T*)buffer;\n"
		"	int r = 0;\n"
		"	p->~S();\n"
		"	if (gCount == 1) r += 1;\n"
		"	p->S::~S();\n"
		"	(*p).~Alias();\n"
		"	if (gCount == 3) r += 2;\n"
		"	q->~T();\n"
		"	if (gCount == 14) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p3-p6: closure types
	{
		"LambdaClosureType", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int x = 3;\n"
		"	auto a = [] { return 1; };\n"
		"	auto b = [] { return 1; };\n"
		"	auto c = [x] { return x; };\n"
		"	auto d = a;\n"
		"	int (*pf)(int) = [](int v) { return v * 2; };\n"
		"	auto pg = +[](int v) { return v + 1; };\n"
		"	auto dbl = [] { return 2.5; };\n"
		"	auto lng = []() -> long { return 1; };\n"
		"	int r = 0;\n"
		"	if (!Same<decltype(a), decltype(b)>::V && Same<decltype(a), decltype(d)>::V) r += 1;\n"
		"	if (sizeof(c) >= sizeof(int) && c() == 3 && d() == 1) r += 2;\n"
		"	if (pf(4) == 8 && Same<decltype(pg), int (*)(int)>::V && pg(4) == 5) r += 4;\n"
		"	if (Same<decltype(a()), int>::V && Same<decltype(dbl()), double>::V) r += 8;\n"
		"	if ([](int p, int q) -> long { return p + q; }(1, 2) == 3 && Same<decltype(lng()), long>::V) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p4: the return type is deduced as for auto
	{
		"LambdaReturnDeduction", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int v; };\n"
		"int gi = 5;\n"
		"int main()\n"
		"{\n"
		"	const int ci = 3;\n"
		"	auto several = [](int v) { if (v > 0) return 1; else if (v < 0) return -1; return 0; };\n"
		"	auto copy = [&]() { return ci; };\n"
		"	auto nothing = [] { };\n"
		"	auto reference = []() -> int& { return gi; };\n"
		"	auto list = [] { return S{ 7 }; };\n"
		"	int r = 0;\n"
		"	if (several(5) == 1 && several(-5) == -1 && several(0) == 0) r += 1;\n"
		"	if (Same<decltype(copy()), int>::V && Same<decltype(nothing()), void>::V) r += 2;\n"
		"	reference() = 6;\n"
		"	if (gi == 6 && Same<decltype(reference()), int&>::V) r += 4;\n"
		"	if (list().v == 7) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"LambdaReturnDeductionMismatchRefused", CONFORMANCE_REFUSED,
		"int main() { auto f = [](int v) { if (v) return 1; return 2.5; }; return f(1) ? 1 : 2; }\n"
	},
	//5.1.2 [expr.prim.lambda] p8-p15: captures
	{
		"LambdaCaptureForms", 127,
		"int main()\n"
		"{\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int c = 3;\n"
		"	int r = 0;\n"
		"	auto byCopy = [a, b] { return a + b; };\n"
		"	auto byRef = [&a, &b] { a += 10; b += 10; };\n"
		"	auto mixed1 = [=, &c] { c = a + b; };\n"
		"	auto mixed2 = [&, a] { b = a + 100; };\n"
		"	a = 5;\n"
		"	if (byCopy() == 3) r += 1;\n"
		"	byRef();\n"
		"	if (a == 15 && b == 12) r += 2;\n"
		"	mixed1();\n"
		"	if (c == 3) r += 4;\n"
		"	a = 20;\n"
		"	mixed2();\n"
		"	if (b == 101) r += 8;\n"
		"	auto init = [s = a + b, &ra = a] { ra = 1; return s; };\n"
		"	if (init() == 121 && a == 1) r += 16;\n"
		"	auto counter = [n = 0]() mutable { return ++n; };\n"
		"	counter();\n"
		"	counter();\n"
		"	if (counter() == 3) r += 32;\n"
		"	auto copy = counter;\n"
		"	if (copy() == 4 && counter() == 4) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p8: captures
	{
		"LambdaCaptureEqualThenCopyRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [=, a] { return a; }; return f(); }\n"
	},
	{
		"LambdaCaptureAndThenReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [&, &a] { return a; }; return f(); }\n"
	},
	{
		"LambdaCaptureTwiceRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [a, a] { return a; }; return f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p10: only automatic variables are captured
	{
		"LambdaCaptureGlobalRefused", CONFORMANCE_REFUSED,
		"int g = 1;\n"
		"int main() { auto f = [g] { return g; }; return f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p12: an odr-used local must be captured
	{
		"LambdaUncapturedLocalRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [] { return a; }; return f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p5: the call operator is const unless mutable
	{
		"LambdaCopyIsConstRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [a] { a = 2; return a; }; return f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p9: this
	{
		"LambdaThisOutsideClassRefused", CONFORMANCE_REFUSED,
		"int main() { auto f = [this] { return 1; }; return f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p2: not in an unevaluated operand
	{
		"LambdaInUnevaluatedRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof([] { return 1; }) ? 1 : 2; }\n"
	},
	//5.1.2 [expr.prim.lambda] p13: default arguments
	{
		"LambdaDefaultArgumentCaptureRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; struct L { static int f(int v = [a] { return a; }()) { return v; } }; return L::f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p15-p19: types of captures
	{
		"LambdaCaptureTypes", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	const int cx = 2;\n"
		"	int& rx = x;\n"
		"	int a[3] = { 4, 5, 6 };\n"
		"	int r = 0;\n"
		"	auto f = [=] { return Same<decltype(x), int>::V + Same<decltype((x)), const int&>::V * 2; };\n"
		"	auto g = [=]() mutable { return Same<decltype((x)), int&>::V + Same<decltype((cx)), const int&>::V * 2; };\n"
		"	auto h = [rx]() mutable { rx = 9; return rx; };\n"
		"	auto arr = [a] { return a[0] + a[1] + a[2] + (int)sizeof(a) / (int)sizeof(int); };\n"
		"	auto ref = [&a] { a[0] = 7; return sizeof(a) / sizeof(int); };\n"
		"	if (f() == 3) r += 1;\n"
		"	if (g() == 3) r += 2;\n"
		"	if (h() == 9 && x == 1) r += 4;\n"
		"	a[1] = 50;\n"
		"	if (arr() == 18) r += 8;\n"
		"	if (ref() == 3 && a[0] == 7) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p9, p18: this
	{
		"LambdaThisCapture", 15,
		"struct S\n"
		"{\n"
		"	int m;\n"
		"	int Explicit() { auto f = [this] { return m + this->m; }; return f(); }\n"
		"	int ByCopyDefault() { auto f = [=] { m += 1; return m; }; return f(); }\n"
		"	int ByRefDefault() { auto f = [&] { m += 10; return m; }; return f(); }\n"
		"	int Nested() { auto f = [this] { auto g = [this] { return m * 2; }; return g(); }; return f(); }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.m = 3;\n"
		"	int r = 0;\n"
		"	if (s.Explicit() == 6) r += 1;\n"
		"	if (s.ByCopyDefault() == 4 && s.m == 4) r += 2;\n"
		"	if (s.ByRefDefault() == 14 && s.m == 14) r += 4;\n"
		"	if (s.Nested() == 28) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p5: generic lambdas
	{
		"LambdaGenericForms", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int Apply(int (*p)(int), int v) { return p(v); }\n"
		"int main()\n"
		"{\n"
		"	auto twice = [](auto v) { return v + v; };\n"
		"	auto first = [](auto a, auto b) { return a; };\n"
		"	auto byRef = [](auto& v) { v = v + 1; };\n"
		"	auto forward = [](auto&& v) -> decltype(auto) { return static_cast<decltype(v)>(v); };\n"
		"	int i = 5;\n"
		"	int r = 0;\n"
		"	if (twice(2) == 4 && twice(1.5) == 3.0 && Same<decltype(twice(1.5f)), float>::V) r += 1;\n"
		"	if (first(1, 2.5) == 1 && first('a', 1) == 'a' && Same<decltype(first('a', 1)), char>::V) r += 2;\n"
		"	byRef(i);\n"
		"	if (i == 6) r += 4;\n"
		"	if (Same<decltype(forward(i)), int&>::V && Same<decltype(forward(5)), int&&>::V) r += 8;\n"
		"	if (Apply(twice, 4) == 8 && Apply([](auto v) { return v - 1; }, 4) == 3) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p16: nested lambdas
	{
		"LambdaNestedCaptures", 15,
		"int main()\n"
		"{\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int r = 0;\n"
		"	auto outer = [=]() mutable { a += 10; auto inner = [=] { return a + b; }; return inner(); };\n"
		"	auto refs = [&] { auto inner = [&] { a += 100; }; inner(); return a; };\n"
		"	auto mixed = [&a, b] { return [&a, b] { a += b; return a; }(); };\n"
		"	if (outer() == 13 && a == 1) r += 1;\n"
		"	if (refs() == 101 && a == 101) r += 2;\n"
		"	if (mixed() == 103 && a == 103) r += 4;\n"
		"	auto maker = [](int n) { return [n](int v) { return n + v; }; };\n"
		"	if (maker(5)(6) == 11) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p20-p21: a closure's copy constructor and destructor
	{
		"LambdaCopyAndDestroy", 7,
		"int gCopies = 0;\n"
		"int gDestroyed = 0;\n"
		"struct S { S() { } S(const S&) { gCopies++; } ~S() { gDestroyed++; } int v = 4; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		S s;\n"
		"		auto f = [s] { return s.v; };\n"
		"		if (gCopies == 1 && f() == 4) r += 1;\n"
		"		auto g = f;\n"
		"		if (gCopies == 2 && g() == 4) r += 2;\n"
		"	}\n"
		"	if (gDestroyed == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p20: no default constructor, no copy assignment
	{
		"LambdaNotDefaultConstructibleRefused", CONFORMANCE_REFUSED,
		"int main() { auto f = [] { return 1; }; decltype(f) g; return g(); }\n"
	},
	{
		"LambdaNotAssignableRefused", CONFORMANCE_REFUSED,
		"int main() { auto f = [] { return 1; }; auto g = f; g = f; return g(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p6: only a lambda without captures becomes a function pointer
	{
		"LambdaCapturingNotFunctionPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; int (*p)() = [a] { return a; }; return p(); }\n"
	},
	//5.2.1 [expr.sub]: subscripting
	{
		"SubscriptForms", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int v; };\n"
		"int main()\n"
		"{\n"
		"	int a[4] = { 1, 2, 3, 4 };\n"
		"	int m[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };\n"
		"	S as[2] = { { 7 }, { 8 } };\n"
		"	int* p = a + 2;\n"
		"	long l = 1;\n"
		"	unsigned char uc = 3;\n"
		"	int r = 0;\n"
		"	if (a[2] == *(a + 2) && 2[a] == 3 && 1[m][2] == 6) r += 1;\n"
		"	a[1] = 20;\n"
		"	1[a] += 2;\n"
		"	if (a[1] == 22 && &a[1] == a + 1) r += 2;\n"
		"	if (p[-1] == 22 && p[-2] == 1 && (-1)[p] == 22) r += 4;\n"
		"	if (a[l] == 22 && a[uc] == 4 && a[true] == 22 && \"abc\"[1] == 'b') r += 8;\n"
		"	if (Same<decltype(a[0]), int&>::V && Same<decltype(m[0]), int (&)[3]>::V && Same<decltype(\"ab\"[0]), const char&>::V) r += 16;\n"
		"	if (as[1].v == 8 && (as + 1)->v == 8 && m[1][0] + m[0][2] == 7) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"SubscriptEnumIndex", 3,
		"enum E { E0, E1, E2 };\n"
		"enum class C : unsigned char { A, B };\n"
		"int main()\n"
		"{\n"
		"	int a[3] = { 5, 6, 7 };\n"
		"	E e = E2;\n"
		"	int r = 0;\n"
		"	if (a[e] == 7 && a[E1] == 6) r += 1;\n"
		"	if (a[(int)C::B] == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"SubscriptNotPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int j = 2; return i[j]; }\n"
	},
	{
		"SubscriptFloatIndexRefused", CONFORMANCE_REFUSED,
		"int main() { int a[3] = { 1, 2, 3 }; return a[1.0]; }\n"
	},
	{
		"SubscriptVoidPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int a[3] = { 1, 2, 3 }; void* p = a; p[1]; return 1; }\n"
	},
	//5.2.1 [expr.sub] p2: a braced list as the subscript of an overloaded operator
	{
		"SubscriptBracedOperator", 3,
		"struct P { int a; int b; };\n"
		"struct S { int operator[](P p) const { return p.a * 10 + p.b; } };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	int r = 0;\n"
		"	if (s[{1, 2}] == 12) r += 1;\n"
		"	if (s[P{3, 4}] == 34) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"SubscriptBracedBuiltInRefused", CONFORMANCE_REFUSED,
		"int main() { int a[3] = { 1, 2, 3 }; return a[{1}]; }\n"
	},
	//5.2.2 [expr.call] p1: function calls
	{
		"CallForms", 127,
		"struct S { int v; int Get() const { return v; } static int Stat(int x) { return x + 1; } int operator()(int x) const { return v + x; } };\n"
		"int f(int a, int b = 10) { return a + b; }\n"
		"int (*GetF())(int, int) { return f; }\n"
		"int (&RefF())(int, int) { return f; }\n"
		"int main()\n"
		"{\n"
		"	S s = { 5 };\n"
		"	S* ps = &s;\n"
		"	int (*p)(int, int) = f;\n"
		"	int (&rf)(int, int) = f;\n"
		"	int (S::*pm)() const = &S::Get;\n"
		"	int r = 0;\n"
		"	if (f(1, 2) == 3 && f(1) == 11) r += 1;\n"
		"	if (p(1, 2) == 3 && (*p)(2, 2) == 4 && rf(3, 3) == 6) r += 2;\n"
		"	if (GetF()(4, 4) == 8 && RefF()(5, 5) == 10) r += 4;\n"
		"	if (s.Get() == 5 && ps->Get() == 5 && (*ps).Get() == 5 && (s.*pm)() == 5 && (ps->*pm)() == 5) r += 8;\n"
		"	if (S::Stat(1) == 2 && s.Stat(2) == 3 && ps->Stat(3) == 4) r += 16;\n"
		"	if (s(1) == 6 && (*ps)(2) == 7) r += 32;\n"
		"	if ((&f)(1, 1) == 2 && (***f)(1, 1) == 2 && (true ? f : f)(2, 1) == 3) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call] p4: parameters are initialised from arguments
	{
		"CallArgumentInitialisation", 63,
		"int gCopies = 0;\n"
		"int gDestroyed = 0;\n"
		"struct S { S(int x) : v(x) { } S(const S& o) : v(o.v) { gCopies++; } ~S() { gDestroyed++; } int v; };\n"
		"struct B { int b; };\n"
		"struct D : B { int d; };\n"
		"int ByValue(S s) { s.v += 1; return s.v; }\n"
		"int ByRef(S& s) { s.v += 1; return s.v; }\n"
		"int ByConstRef(const S& s) { return s.v; }\n"
		"int Slice(B b) { return b.b; }\n"
		"long Widen(long l) { return l; }\n"
		"unsigned char Narrow(unsigned char c) { return c; }\n"
		"double ToDouble(double d) { return d; }\n"
		"int main()\n"
		"{\n"
		"	S s(1);\n"
		"	D d;\n"
		"	d.b = 7; d.d = 8;\n"
		"	int r = 0;\n"
		"	int v1 = ByValue(s);\n"
		"	if (v1 == 2 && s.v == 1 && gCopies == 1 && gDestroyed == 1) r += 1;\n"
		"	if (ByRef(s) == 2 && s.v == 2 && gCopies == 1) r += 2;\n"
		"	int v2 = ByConstRef(s) * 10 + ByConstRef(5);\n"
		"	if (v2 == 25 && gDestroyed == 2) r += 4;\n"
		"	int v3 = ByValue(9);\n"
		"	if (v3 == 10 && gCopies == 1 && gDestroyed == 3) r += 8;\n"
		"	if (Slice(d) == 7) r += 16;\n"
		"	if (Widen(-3) == -3L && Narrow(300) == 44 && ToDouble(3) == 3.0 && ToDouble(2.5f) == 2.5) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"CallArgumentCountRefused", CONFORMANCE_REFUSED,
		"int f(int a, int b) { return a + b; }\n"
		"int main() { return f(1); }\n"
	},
	{
		"CallTooManyArgumentsRefused", CONFORMANCE_REFUSED,
		"int f(int a, int b) { return a + b; }\n"
		"int main() { return f(1, 2, 3); }\n"
	},
	//5.2.2 [expr.call] p1: function calls
	{
		"CallNotAFunctionRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; return i(2); }\n"
	},
	//5.2.2 [expr.call] p4: parameters are initialised from arguments
	{
		"CallArgumentTypeRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"int f(S s) { return s.v; }\n"
		"int main() { return f(1); }\n"
	},
	{
		"CallIncompleteParameterRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"int f(I i);\n"
		"I* g();\n"
		"int main() { return f(*g()); }\n"
	},
	//5.2.2 [expr.call] p10: the value category of a call
	{
		"CallResultCategories", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gi = 1;\n"
		"struct S { int v; };\n"
		"S gs = { 2 };\n"
		"int Value() { return gi; }\n"
		"int& Lvalue() { return gi; }\n"
		"int&& Xvalue() { return static_cast<int&&>(gi); }\n"
		"const int& ConstLvalue() { return gi; }\n"
		"S& Object() { return gs; }\n"
		"S ObjectValue() { return gs; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Lvalue() = 5;\n"
		"	if (gi == 5 && &Lvalue() == &gi) r += 1;\n"
		"	Lvalue()++;\n"
		"	++Lvalue();\n"
		"	Lvalue() += 10;\n"
		"	if (gi == 17 && Value() == 17 && ConstLvalue() == 17) r += 2;\n"
		"	if (Same<decltype(Value()), int>::V && Same<decltype(Lvalue()), int&>::V && Same<decltype(Xvalue()), int&&>::V && Same<decltype(ConstLvalue()), const int&>::V) r += 4;\n"
		"	Object().v = 6;\n"
		"	if (gs.v == 6 && ObjectValue().v == 6 && &Object() == &gs) r += 8;\n"
		"	int&& rr = Xvalue();\n"
		"	rr = 20;\n"
		"	if (gi == 20) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"CallAssignToValueRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { f() = 2; return 1; }\n"
	},
	{
		"CallAddressOfValueRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { int* p = &f(); return *p; }\n"
	},
	//5.2.2 [expr.call] p11: a call that is a decltype operand need not have a complete type
	{
		"CallIncompleteReturnInDecltype", 3,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct I;\n"
		"I Make();\n"
		"template<class T> struct Holder { T t; };\n"
		"Holder<I> MakeHolder();\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(Make()), I>::V) r += 1;\n"
		"	if (Same<decltype(MakeHolder()), Holder<I> >::V) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"CallIncompleteReturnRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"I Make();\n"
		"int main() { Make(); return 1; }\n"
	},
	//5.2.2 [expr.call] p7: arguments without a parameter
	{
		"CallVariadicPromotions", 63,
		"#include <stdarg.h>\n"
		"enum E { E7 = 7 };\n"
		"int Check(int n, ...)\n"
		"{\n"
		"	va_list ap;\n"
		"	int r = 0;\n"
		"	va_start(ap, n);\n"
		"	if (va_arg(ap, int) == 'a') r += 1;\n"
		"	if (va_arg(ap, int) == -2) r += 2;\n"
		"	if (va_arg(ap, double) == 1.5) r += 4;\n"
		"	if (va_arg(ap, int) == 1) r += 8;\n"
		"	if (va_arg(ap, int) == 7) r += 16;\n"
		"	if (va_arg(ap, long) == 100000L && va_arg(ap, const char*)[1] == 'y' && va_arg(ap, void*) == 0) r += 32;\n"
		"	va_end(ap);\n"
		"	return r;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	char c = 'a';\n"
		"	short s = -2;\n"
		"	float f = 1.5f;\n"
		"	bool b = true;\n"
		"	return Check(8, c, s, f, b, E7, 100000L, \"xyz\", nullptr);\n"
		"}\n"
	},
	{
		"CallVariadicUnsignedShort", 3,
		"#include <stdarg.h>\n"
		"int Check(int n, ...)\n"
		"{\n"
		"	va_list ap;\n"
		"	int r = 0;\n"
		"	va_start(ap, n);\n"
		"	if (sizeof(int) > sizeof(short) ? (va_arg(ap, int) == 65535L) : (va_arg(ap, unsigned int) == 65535u)) r += 1;\n"
		"	if (va_arg(ap, int) == 200) r += 2;\n"
		"	va_end(ap);\n"
		"	return r;\n"
		"}\n"
		"int main() { unsigned short us = 65535u; unsigned char uc = 200; return Check(2, us, uc); }\n"
	},
	//5.2.2 [expr.call] p8: every argument is evaluated before the function is entered
	{
		"CallEvaluatedOnce", 15,
		"int gCalls = 0;\n"
		"int Next() { return ++gCalls; }\n"
		"int Sum(int a, int b, int c) { return gCalls * 100 + a + b + c; }\n"
		"int (*Pick())(int, int, int) { gCalls += 10; return Sum; }\n"
		"struct S { int v; S* Self() { gCalls += 20; return this; } int Add(int a) { return v + a + gCalls * 100; } };\n"
		"int main()\n"
		"{\n"
		"	S s = { 1 };\n"
		"	int r = 0;\n"
		"	if (Sum(Next(), Next(), Next()) == 306) r += 1;\n"
		"	if (Pick()(0, 0, 0) == 1300) r += 2;\n"
		"	if (s.Self()->Add(5) == 3306) r += 4;\n"
		"	if (gCalls == 33) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call] p1: a qualified name suppresses the virtual call
	{
		"CallVirtualAndQualified", 31,
		"struct B { virtual int f() { return 1; } int g() { return f(); } virtual ~B() { } };\n"
		"struct D : B { int f() override { return 2; } int h() { return B::f() * 10 + f(); } };\n"
		"struct E : D { int f() override { return 3; } };\n"
		"int main()\n"
		"{\n"
		"	E e;\n"
		"	B* pb = &e;\n"
		"	D* pd = &e;\n"
		"	B& rb = e;\n"
		"	int r = 0;\n"
		"	if (pb->f() == 3 && rb.f() == 3 && pd->f() == 3) r += 1;\n"
		"	if (pb->B::f() == 1 && pd->D::f() == 2 && pd->B::f() == 1 && rb.B::f() == 1) r += 2;\n"
		"	if (pb->g() == 3 && pd->h() == 13) r += 4;\n"
		"	if (e.B::f() == 1 && e.D::f() == 2 && e.E::f() == 3 && e.f() == 3) r += 8;\n"
		"	B b = e;\n"
		"	if (b.f() == 1 && static_cast<B>(e).f() == 1) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call] p4: this in a call of a base's member function
	{
		"CallThisAdjusted", 15,
		"struct A { int a; int GetA() { return a; } A* ThisA() { return this; } };\n"
		"struct B { int b; int GetB() { return b; } B* ThisB() { return this; } };\n"
		"struct D : A, B { int d; int Both() { return GetA() * 10 + GetB(); } };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	D* pd = &d;\n"
		"	d.a = 1; d.b = 2; d.d = 3;\n"
		"	int r = 0;\n"
		"	if (d.GetB() == 2 && pd->GetB() == 2 && d.GetA() == 1) r += 1;\n"
		"	if (d.ThisB() == static_cast<B*>(&d) && (void*)d.ThisB() != (void*)&d && (void*)d.ThisA() == (void*)&d) r += 2;\n"
		"	if (d.Both() == 12) r += 4;\n"
		"	if (pd->B::GetB() == 2 && (pd->*(&B::GetB))() == 2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call] p4: this in a call of a member function
	{
		"CallConstObjectNonConstMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int v; int f() { return v; } };\n"
		"int main() { const S s = { 1 }; return s.f(); }\n"
	},
	//5.2.3 [expr.type.conv]: explicit type conversion, functional notation
	{
		"FunctionalCastForms", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int a; int b; };\n"
		"struct C { C() : v(5) { } C(int a, int b) : v(a * 10 + b) { } explicit C(int a) : v(a) { } int v; };\n"
		"typedef unsigned char Byte;\n"
		"typedef int* IntPtr;\n"
		"template<class T> T Zero() { return T(); }\n"
		"int main()\n"
		"{\n"
		"	int i = 300;\n"
		"	int r = 0;\n"
		"	if (int(3.7) == 3 && double(1) / 2 == 0.5 && Byte(i) == 44 && long(i) == 300L) r += 1;\n"
		"	if (int() == 0 && double() == 0.0 && bool() == false && IntPtr() == 0 && char() == 0) r += 2;\n"
		"	if (S().a == 0 && S().b == 0 && C().v == 5) r += 4;\n"
		"	if (C(1, 2).v == 12 && C(7).v == 7) r += 8;\n"
		"	if (int{} == 0 && int{5} == 5 && S{1, 2}.b == 2 && C{3, 4}.v == 34 && Byte{200} == 200) r += 16;\n"
		"	if (Zero<int>() == 0 && Zero<long>() == 0 && Zero<IntPtr>() == 0 && Zero<S>().b == 0) r += 32;\n"
		"	if (Same<decltype(int(3.7)), int>::V && Same<decltype(Byte(1)), unsigned char>::V && Same<decltype(S()), S>::V && Same<decltype(void()), void>::V) r += 64;\n"
		"	if (IntPtr(&i) == &i && bool(i) && !bool(0.0) && int('a') == 97) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.3 [expr.type.conv] p2: T() for void
	{
		"FunctionalCastVoid", 3,
		"int gCalls = 0;\n"
		"void Nothing() { gCalls++; return void(); }\n"
		"template<class T> T Make() { return T(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Nothing();\n"
		"	Make<void>();\n"
		"	void();\n"
		"	if (gCalls == 1) r += 1;\n"
		"	if (sizeof(Make<char>()) == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.3 [expr.type.conv] p3: a braced list
	{
		"FunctionalCastNarrowingRefused", CONFORMANCE_REFUSED,
		"int main() { return int{3.5}; }\n"
	},
	{
		"FunctionalCastNarrowingCharRefused", CONFORMANCE_REFUSED,
		"int main() { return char{300} ? 1 : 2; }\n"
	},
	//5.2.3 [expr.type.conv] p1: only a simple type specifier or typename specifier
	{
		"FunctionalCastTwoWordTypeRefused", CONFORMANCE_REFUSED,
		"int main() { return unsigned int(3); }\n"
	},
	//5.2.3 [expr.type.conv] p1: equivalent to the cast expression
	{
		"FunctionalCastNoConversionRefused", CONFORMANCE_REFUSED,
		"struct S { int a; };\n"
		"struct T { int a; };\n"
		"int main() { S s = { 1 }; return T(s).a; }\n"
	},
	//5.2.4 [expr.pseudo]: pseudo destructor call
	{
		"PseudoDestructor", 15,
		"typedef int I;\n"
		"typedef int* P;\n"
		"typedef double F;\n"
		"enum E { E0 };\n"
		"template<class T> void Destroy(T* p) { p->~T(); }\n"
		"template<class T> void DestroyObject(T& t) { t.~T(); }\n"
		"template<class T> void DestroyQualified(T* p) { p->T::~T(); }\n"
		"int gCount = 0;\n"
		"struct S { ~S() { gCount++; } };\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	I* pi = &i;\n"
		"	P p = &i;\n"
		"	F f = 1.5;\n"
		"	E e = E0;\n"
		"	char buffer[sizeof(S)];\n"
		"	int r = 0;\n"
		"	i.~I();\n"
		"	pi->~I();\n"
		"	pi->I::~I();\n"
		"	p.~P();\n"
		"	f.~F();\n"
		"	if (i == 5 && f == 1.5) r += 1;\n"
		"	Destroy(&i);\n"
		"	Destroy(&p);\n"
		"	Destroy(&f);\n"
		"	Destroy(&e);\n"
		"	DestroyObject(i);\n"
		"	DestroyQualified(&i);\n"
		"	if (i == 5 && p == &i) r += 2;\n"
		"	Destroy((S*)buffer);\n"
		"	DestroyObject(*(S*)buffer);\n"
		"	DestroyQualified((S*)buffer);\n"
		"	if (gCount == 3) r += 4;\n"
		"	i.I::~I();\n"
		"	(&i)->~I(), r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"PseudoDestructorWrongTypeRefused", CONFORMANCE_REFUSED,
		"typedef int I;\n"
		"typedef long L;\n"
		"int main() { int i = 1; i.~L(); return 1; }\n"
	},
	{
		"PseudoDestructorNotCalledRefused", CONFORMANCE_REFUSED,
		"typedef int I;\n"
		"int main() { int i = 1; i.~I; return 1; }\n"
	},
	//5.2.5 [expr.ref]: class member access
	{
		"MemberAccessForms", 127,
		"struct In { int x; int y; };\n"
		"struct S\n"
		"{\n"
		"	int m;\n"
		"	In in;\n"
		"	int a[3];\n"
		"	static int s;\n"
		"	enum { E = 9 };\n"
		"	int Get() { return m; }\n"
		"	static int Stat() { return 8; }\n"
		"	In* Inner() { return &in; }\n"
		"};\n"
		"int S::s = 7;\n"
		"int gCalls = 0;\n"
		"S gs;\n"
		"S& Global() { gCalls++; return gs; }\n"
		"S* GlobalPointer() { gCalls++; return &gs; }\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	S* p = &s;\n"
		"	S** pp = &p;\n"
		"	int r = 0;\n"
		"	s.m = 1; s.in.x = 2; s.in.y = 3; s.a[0] = 4; s.a[2] = 5;\n"
		"	if (p->m == 1 && (*p).m == 1 && (*pp)->m == 1 && (**pp).m == 1 && (&s)->m == 1) r += 1;\n"
		"	if (s.in.x == 2 && p->in.y == 3 && p->Inner()->x == 2 && (&p->in)->y == 3) r += 2;\n"
		"	if (s.a[2] == 5 && p->a[0] == 4 && *p->a == 4) r += 4;\n"
		"	if (s.s == 7 && p->s == 7 && s.E == 9 && p->E == 9 && s.Stat() == 8 && p->Stat() == 8) r += 8;\n"
		"	p->m = 10;\n"
		"	(*p).in.x += 1;\n"
		"	p->a[1] = 6;\n"
		"	if (s.m == 10 && s.in.x == 3 && s.a[1] == 6 && s.Get() == 10) r += 16;\n"
		"	if (Global().s == 7 && GlobalPointer()->s == 7 && Global().E == 9 && Global().Stat() == 8 && gCalls == 4) r += 32;\n"
		"	s.s = 11;\n"
		"	if (S::s == 11 && &s.s == &S::s && &p->s == &gs.s) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.5 [expr.ref] p4: the type and value category of a member access
	{
		"MemberAccessCategories", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gi = 3;\n"
		"struct S { int m; mutable int mut; int& ref; const int c; static int s; int a[2]; };\n"
		"int S::s = 0;\n"
		"S Make() { S s = { 1, 2, gi, 4, { 5, 6 } }; return s; }\n"
		"int main()\n"
		"{\n"
		"	S s = { 1, 2, gi, 4, { 5, 6 } };\n"
		"	const S cs = { 1, 2, gi, 4, { 5, 6 } };\n"
		"	const S* pc = &cs;\n"
		"	volatile S* pv = 0;\n"
		"	int r = 0;\n"
		"	if (Same<decltype((s.m)), int&>::V && Same<decltype((cs.m)), const int&>::V && Same<decltype((pc->m)), const int&>::V) r += 1;\n"
		"	if (Same<decltype((cs.mut)), int&>::V && Same<decltype((cs.ref)), int&>::V && Same<decltype((s.c)), const int&>::V) r += 2;\n"
		"	if (Same<decltype((Make().m)), int&&>::V && Same<decltype((Make().ref)), int&>::V && Same<decltype((Make().s)), int&>::V) r += 4;\n"
		"	if (Same<decltype((pv->m)), volatile int&>::V && Same<decltype((cs.a)), const int (&)[2]>::V && Same<decltype((cs.s)), int&>::V) r += 8;\n"
		"	if (Same<decltype(s.m), int>::V && Same<decltype(cs.m), int>::V && Same<decltype(s.ref), int&>::V && Same<decltype(cs.c), const int>::V) r += 16;\n"
		"	cs.mut = 20;\n"
		"	cs.ref = 30;\n"
		"	if (cs.mut == 20 && gi == 30 && Make().m == 1 && Make().a[1] == 6) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"MemberAccessConstWriteRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { const S s = { 1 }; s.m = 2; return 1; }\n"
	},
	{
		"MemberAccessConstPointerWriteRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; const S* p = &s; p->m = 2; return 1; }\n"
	},
	//5.2.5 [expr.ref] p2: . takes an object, -> a pointer
	{
		"MemberAccessDotOnPointerRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; S* p = &s; return p.m; }\n"
	},
	{
		"MemberAccessArrowOnObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; return s->m; }\n"
	},
	//5.2.5 [expr.ref] p2: the class must be complete
	{
		"MemberAccessIncompleteRefused", CONFORMANCE_REFUSED,
		"struct S;\n"
		"S* Get();\n"
		"int main() { return Get()->m; }\n"
	},
	//5.2.5 [expr.ref] p2: the name must be a member
	{
		"MemberAccessNoSuchMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; return s.n; }\n"
	},
	//5.2.5 [expr.ref] p4: a nested type is not a member access
	{
		"MemberAccessNestedTypeRefused", CONFORMANCE_REFUSED,
		"struct S { typedef int T; int m; };\n"
		"int main() { S s = { 1 }; return sizeof(s.T); }\n"
	},
	//5.2.5 [expr.ref] p5: ambiguous base
	{
		"MemberAccessAmbiguousBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; return d.a; }\n"
	},
	//5.2.5 [expr.ref] p5: a qualified member name picks the base
	{
		"MemberAccessQualifiedBases", 15,
		"struct A { int a; int f() { return a; } };\n"
		"struct B : A { int b; };\n"
		"struct C : A { int c; };\n"
		"struct D : B, C { int a; };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	D* p = &d;\n"
		"	d.a = 1;\n"
		"	d.B::a = 2;\n"
		"	d.C::a = 3;\n"
		"	int r = 0;\n"
		"	if (d.a == 1 && p->a == 1 && d.D::a == 1) r += 1;\n"
		"	if (d.B::a == 2 && p->B::a == 2 && p->C::a == 3) r += 2;\n"
		"	if (d.B::f() == 2 && p->C::f() == 3) r += 4;\n"
		"	if (&d.B::a != &d.C::a && &d.B::a != &d.a) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.5 [expr.ref] p1: the object expression is evaluated
	{
		"MemberAccessObjectEvaluated", 7,
		"int gCalls = 0;\n"
		"struct S { int m; static int s; static int f() { return 3; } };\n"
		"int S::s = 2;\n"
		"S gs = { 1 };\n"
		"S* Next() { gCalls++; return &gs; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Next()->m == 1 && gCalls == 1) r += 1;\n"
		"	if (Next()->s == 2 && gCalls == 2) r += 2;\n"
		"	if (Next()->f() == 3 && (*Next()).f() == 3 && gCalls == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.6 [expr.post.incr]: increment and decrement
	{
		"PostIncrementForms", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { unsigned a : 3; int b : 4; };\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	long l = 70000L;\n"
		"	unsigned char uc = 255;\n"
		"	signed char sc = 5;\n"
		"	double d = 1.5;\n"
		"	float f = 2.5f;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int* p = a;\n"
		"	long long ll = 0xFFFFFFFFLL;\n"
		"	S s;\n"
		"	s.a = 7; s.b = -8;\n"
		"	int r = 0;\n"
		"	if (i++ == 5 && i == 6 && i-- == 6 && i == 5) r += 1;\n"
		"	if (l++ == 70000L && l == 70001L && l-- == 70001L && l == 70000L) r += 2;\n"
		"	if (uc++ == 255 && uc == 0 && uc-- == 0 && uc == 255 && sc-- == 5 && sc == 4) r += 4;\n"
		"	if (d++ == 1.5 && d == 2.5 && f-- == 2.5f && f == 1.5f) r += 8;\n"
		"	if (*p++ == 1 && *p == 2 && p == a + 1 && *p-- == 2 && p == a) r += 16;\n"
		"	if (ll++ == 0xFFFFFFFFLL && ll == 0x100000000LL && ll-- == 0x100000000LL && ll == 0xFFFFFFFFLL) r += 32;\n"
		"	if (s.a++ == 7 && s.a == 0 && s.b-- == -8) r += 64;\n"
		"	if (Same<decltype(i++), int>::V && Same<decltype(uc++), unsigned char>::V && Same<decltype(p--), int*>::V && Same<decltype(d++), double>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.6 [expr.post.incr] p1: bool is set to true (deprecated)
	{
		"PostIncrementBool", 3,
		"int main()\n"
		"{\n"
		"	bool b = false;\n"
		"	int r = 0;\n"
		"	if (b++ == false && b == true) r += 1;\n"
		"	if (b++ == true && b == true && (int)b == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.6 [expr.post.incr] p2: bool cannot be decremented
	{
		"PostDecrementBoolRefused", CONFORMANCE_REFUSED,
		"int main() { bool b = true; b--; return b ? 1 : 2; }\n"
	},
	//5.2.6 [expr.post.incr] p1: the result is a prvalue
	{
		"PostIncrementAssignedRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; i++ = 5; return i; }\n"
	},
	//5.2.6 [expr.post.incr] p1: the operand is a modifiable lvalue
	{
		"PostIncrementConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; i++; return i; }\n"
	},
	{
		"PostIncrementRvalueRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; (i + 1)++; return i; }\n"
	},
	//5.2.6 [expr.post.incr] p1: arithmetic or pointer type
	{
		"PostIncrementEnumRefused", CONFORMANCE_REFUSED,
		"enum E { E0, E1 };\n"
		"int main() { E e = E0; e++; return e + 1; }\n"
	},
	//5.2.6 [expr.post.incr] p1: a pointer to a complete object type
	{
		"PostIncrementVoidPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; void* p = &i; p++; return 1; }\n"
	},
	//5.2.6 [expr.post.incr] p1: a pointer moves by the size of what it points to
	{
		"PostIncrementPointerSizes", 7,
		"struct S { char c[7]; };\n"
		"int main()\n"
		"{\n"
		"	S as[3];\n"
		"	long al[3];\n"
		"	int m[3][5];\n"
		"	S* ps = as;\n"
		"	long* pl = al;\n"
		"	int (*pm)[5] = m;\n"
		"	int r = 0;\n"
		"	ps++; pl++; pm++;\n"
		"	if ((char*)ps - (char*)as == 7 && ps == &as[1]) r += 1;\n"
		"	if ((char*)pl - (char*)al == (int)sizeof(long) && pl == al + 1) r += 2;\n"
		"	if ((char*)pm - (char*)m == 5 * (int)sizeof(int) && pm-- == m + 1 && pm == m) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.7 [expr.dynamic.cast]: dynamic cast
	{
		"DynamicCastForms", 127,
		"struct A { virtual ~A() { } int a; };\n"
		"struct B { virtual ~B() { } int b; };\n"
		"struct C : A, B { int c; };\n"
		"struct E : A { int e; };\n"
		"struct N { int n; };\n"
		"struct M : N { int m; };\n"
		"int main()\n"
		"{\n"
		"	C c;\n"
		"	E e;\n"
		"	M m;\n"
		"	c.a = 4;\n"
		"	A* pa = &c;\n"
		"	B* pb = &c;\n"
		"	A* pe = &e;\n"
		"	A* null = 0;\n"
		"	const A* pca = &c;\n"
		"	int r = 0;\n"
		"	if (dynamic_cast<C*>(pa) == &c && dynamic_cast<C*>(pb) == &c) r += 1;\n"
		"	if (dynamic_cast<B*>(pa) == pb && dynamic_cast<A*>(pb) == pa) r += 2;\n"
		"	if (dynamic_cast<C*>(pe) == 0 && dynamic_cast<B*>(pe) == 0 && dynamic_cast<E*>(pe) == &e) r += 4;\n"
		"	if (dynamic_cast<C*>(null) == 0 && dynamic_cast<void*>(null) == 0) r += 8;\n"
		"	if (dynamic_cast<void*>(pb) == (void*)&c && dynamic_cast<const void*>(pca) == (const void*)&c) r += 16;\n"
		"	if (dynamic_cast<N*>(&m) == (N*)&m && dynamic_cast<A*>(&c) == pa && dynamic_cast<const C*>(pca) == &c) r += 32;\n"
		"	if (&dynamic_cast<C&>(*pa) == &c && &dynamic_cast<B&>(*pa) == pb && dynamic_cast<A&&>(static_cast<C&&>(c)).a == 4) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.7 [expr.dynamic.cast] p8: the run-time check
	{
		"DynamicCastAccessAndAmbiguity", 15,
		"struct A { virtual ~A() { } };\n"
		"struct B { virtual ~B() { } };\n"
		"struct Private : A, private B { B* AsB() { return this; } };\n"
		"struct Left : A { };\n"
		"struct Right : A { };\n"
		"struct Both : Left, Right { };\n"
		"struct V { virtual ~V() { } };\n"
		"struct VL : virtual V { };\n"
		"struct VR : virtual V { };\n"
		"struct VD : VL, VR { };\n"
		"int main()\n"
		"{\n"
		"	Private p;\n"
		"	Both b;\n"
		"	VD vd;\n"
		"	A* pa = &p;\n"
		"	B* pb = p.AsB();\n"
		"	Left* pl = &b;\n"
		"	A* la = pl;\n"
		"	V* pv = &vd;\n"
		"	int r = 0;\n"
		"	if (dynamic_cast<B*>(pa) == 0 && dynamic_cast<Private*>(pa) == &p) r += 1;\n"
		"	if (dynamic_cast<A*>(pb) == 0 && dynamic_cast<Private*>(pb) == 0) r += 2;\n"
		"	if (dynamic_cast<Both*>(la) == &b && dynamic_cast<Right*>(la) == static_cast<Right*>(&b) && dynamic_cast<Right*>(pl) == static_cast<Right*>(&b)) r += 4;\n"
		"	if (dynamic_cast<VD*>(pv) == &vd && dynamic_cast<VL*>(pv) == static_cast<VL*>(&vd) && dynamic_cast<VR*>(static_cast<VL*>(&vd)) == static_cast<VR*>(&vd)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.7 [expr.dynamic.cast] p6: the operand is polymorphic
	{
		"DynamicCastNotPolymorphicRefused", CONFORMANCE_REFUSED,
		"struct N { int n; };\n"
		"struct M : N { int m; };\n"
		"int main() { M m; N* p = &m; return dynamic_cast<M*>(p) ? 1 : 2; }\n"
	},
	//5.2.7 [expr.dynamic.cast] p1: constness is not cast away
	{
		"DynamicCastAwayConstRefused", CONFORMANCE_REFUSED,
		"struct A { virtual ~A() { } };\n"
		"struct D : A { };\n"
		"int main() { D d; const A* p = &d; return dynamic_cast<D*>(p) ? 1 : 2; }\n"
	},
	//5.2.7 [expr.dynamic.cast] p2: the classes are complete
	{
		"DynamicCastIncompleteRefused", CONFORMANCE_REFUSED,
		"struct A { virtual ~A() { } };\n"
		"struct I;\n"
		"int main() { A a; return dynamic_cast<I*>(&a) ? 1 : 2; }\n"
	},
	//5.2.7 [expr.dynamic.cast] p1: a pointer or reference to a class, or void pointer
	{
		"DynamicCastToNonClassRefused", CONFORMANCE_REFUSED,
		"struct A { virtual ~A() { } };\n"
		"int main() { A a; return dynamic_cast<int*>(&a) ? 1 : 2; }\n"
	},
	//5.2.7 [expr.dynamic.cast] p5: a cast to a base is the conversion
	{
		"DynamicCastUpAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { virtual ~A() { } };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; return dynamic_cast<A*>(&d) ? 1 : 2; }\n"
	},
	//5.2.8 [expr.typeid]: type identification
	{
		"TypeidRules", 127,
		"#if defined(_MSC_VER) || defined(CONFORMANCE_HOST)\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"struct P { virtual ~P() { } };\n"
		"struct Q : P { };\n"
		"struct N { int n; };\n"
		"struct M : N { };\n"
		"typedef int I;\n"
		"enum E { E0 };\n"
		"int gCalls = 0;\n"
		"N& Plain() { gCalls++; static M m; return m; }\n"
		"P& Poly() { gCalls += 10; static Q q; return q; }\n"
		"int main()\n"
		"{\n"
		"	Q q;\n"
		"	M m;\n"
		"	P& rp = q;\n"
		"	N& rn = m;\n"
		"	const P& crp = q;\n"
		"	int a[3];\n"
		"	int r = 0;\n"
		"	if (typeid(rp) == typeid(Q) && typeid(rn) == typeid(N) && typeid(rn) != typeid(M)) r += 1;\n"
		"	if (typeid(const int) == typeid(int) && typeid(volatile Q) == typeid(Q) && typeid(crp) == typeid(Q)) r += 2;\n"
		"	if (typeid(int&) == typeid(int) && typeid(const P&) == typeid(P) && typeid(I) == typeid(int)) r += 4;\n"
		"	if (typeid(Plain()) == typeid(N) && gCalls == 0) r += 8;\n"
		"	if (typeid(Poly()) == typeid(Q) && gCalls == 10) r += 16;\n"
		"	if (typeid(a) == typeid(int[3]) && typeid(a) != typeid(int*) && typeid(main) == typeid(int()) && typeid(E0) == typeid(E)) r += 32;\n"
		"	if (typeid(1 + 1L) == typeid(long) && typeid('a') == typeid(char) && typeid(const char*) != typeid(char*) && typeid(&rp) == typeid(P*)) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.8 [expr.typeid] p4: the class is complete
	{
		"TypeidIncompleteRefused", CONFORMANCE_REFUSED,
		"#if defined(_MSC_VER) || defined(CONFORMANCE_HOST)\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"struct I;\n"
		"int main() { return typeid(I) == typeid(int) ? 1 : 2; }\n"
	},
	//5.2.8 [expr.typeid] p6: <typeinfo> comes first
	{
		"TypeidWithoutHeaderRefused", CONFORMANCE_REFUSED,
		"int main() { return &typeid(int) ? 1 : 2; }\n"
	},
	//5.2.9 [expr.static.cast]: static cast
	{
		"StaticCastForms", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E0, E1, E5 = 5 };\n"
		"enum class C : unsigned char { A, B = 200 };\n"
		"struct X { explicit X(int x) : v(x) { } explicit operator int() const { return v * 2; } explicit operator bool() const { return v != 0; } int v; };\n"
		"int gCalls = 0;\n"
		"int Call() { return ++gCalls; }\n"
		"int main()\n"
		"{\n"
		"	int i = 65;\n"
		"	double d = 3.9;\n"
		"	void* v = &i;\n"
		"	const void* cv = &i;\n"
		"	int r = 0;\n"
		"	if (static_cast<int>(d) == 3 && static_cast<char>(i) == 'A' && static_cast<double>(i) / 2 == 32.5 && static_cast<unsigned char>(-1) == 255) r += 1;\n"
		"	if (static_cast<E>(1) == E1 && static_cast<int>(E5) == 5 && static_cast<E>(3) == 3 && static_cast<C>(200) == C::B && static_cast<int>(C::B) == 200) r += 2;\n"
		"	if (static_cast<C>(E1) == C::A || static_cast<E>(C::A) == E0) r += 4;\n"
		"	if (static_cast<int*>(v) == &i && *static_cast<const int*>(cv) == 65 && static_cast<void*>(&i) == v) r += 8;\n"
		"	static_cast<void>(Call());\n"
		"	static_cast<void>(i);\n"
		"	if (gCalls == 1) r += 16;\n"
		"	if (static_cast<X>(4).v == 4 && static_cast<int>(X(4)) == 8 && static_cast<bool>(X(4)) && !static_cast<bool>(X(0))) r += 32;\n"
		"	if (Same<decltype(static_cast<int&&>(i)), int&&>::V && Same<decltype(static_cast<int&>(i)), int&>::V && Same<decltype(static_cast<const int&>(i)), const int&>::V && Same<decltype(static_cast<long>(i)), long>::V) r += 64;\n"
		"	static_cast<int&>(i) = 7;\n"
		"	if (i == 7 && static_cast<bool>(i) && static_cast<bool>(0.5) && static_cast<int>(true) == 1) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.9 [expr.static.cast] p2, p11: base to derived
	{
		"StaticCastClasses", 127,
		"struct A { int a; };\n"
		"struct B { int b; int f() { return b; } };\n"
		"struct D : A, B { int d; };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	d.a = 1; d.b = 2; d.d = 3;\n"
		"	B* pb = &d;\n"
		"	B& rb = d;\n"
		"	const B* cb = &d;\n"
		"	B* nb = 0;\n"
		"	int B::* pmb = &B::b;\n"
		"	int D::* pmd = &D::b;\n"
		"	int (D::*pfd)() = &D::f;\n"
		"	int r = 0;\n"
		"	if (static_cast<D*>(pb) == &d && static_cast<D*>(pb)->d == 3) r += 1;\n"
		"	if (&static_cast<D&>(rb) == &d && static_cast<D&>(rb).a == 1) r += 2;\n"
		"	if (static_cast<D*>(nb) == 0 && static_cast<B*>(static_cast<D*>(0)) == 0) r += 4;\n"
		"	if (static_cast<const D*>(cb) == &d && static_cast<B*>(&d) == pb && static_cast<A*>(&d)->a == 1) r += 8;\n"
		"	if (d.*static_cast<int D::*>(pmb) == 2 && static_cast<int B::*>(pmd) == pmb) r += 16;\n"
		"	if ((static_cast<B&>(d).*static_cast<int (B::*)()>(pfd))() == 2) r += 32;\n"
		"	if (static_cast<B>(d).b == 2 && static_cast<B&&>(static_cast<D&&>(d)).b == 2 && static_cast<D&&>(rb).d == 3) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.9 [expr.static.cast] p1: constness is not cast away
	{
		"StaticCastAwayConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; const int* p = &i; int* q = static_cast<int*>(p); return *q; }\n"
	},
	//5.2.9 [expr.static.cast] p13: from a void pointer
	{
		"StaticCastVoidAwayConstRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; const void* p = &i; int* q = static_cast<int*>(p); return *q; }\n"
	},
	//5.2.9 [expr.static.cast]: static cast
	{
		"StaticCastUnrelatedPointersRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long* p = static_cast<long*>(&i); return p ? 1 : 2; }\n"
	},
	{
		"StaticCastIntToPointerRefused", CONFORMANCE_REFUSED,
		"int main() { long l = 16; int* p = static_cast<int*>(l); return p ? 1 : 2; }\n"
	},
	{
		"StaticCastPointerToIntRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = static_cast<long>(&i); return l ? 1 : 2; }\n"
	},
	{
		"StaticCastUnrelatedClassesRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"int main() { A a; B* p = static_cast<B*>(&a); return p ? 1 : 2; }\n"
	},
	//5.2.9 [expr.static.cast] p11: the base is accessible
	{
		"StaticCastPrivateBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D : private A { };\n"
		"int main() { D d; A* p = static_cast<A*>(&d); return p ? 1 : 2; }\n"
	},
	{
		"StaticCastFromPrivateBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct D : private A { A* Base() { return this; } };\n"
		"int main() { D d; D* p = static_cast<D*>(d.Base()); return p ? 1 : 2; }\n"
	},
	//5.2.9 [expr.static.cast] p11: the base is unambiguous
	{
		"StaticCastAmbiguousBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A* pa = static_cast<B*>(&d); return static_cast<D*>(pa) ? 1 : 2; }\n"
	},
	//5.2.9 [expr.static.cast] p4: as if the result were declared and initialised
	{
		"StaticCastNonConstReferenceToValueRefused", CONFORMANCE_REFUSED,
		"int main() { int& r = static_cast<int&>(5); return r; }\n"
	},
	//5.2.9 [expr.static.cast] p4: an overloaded function's address is chosen by the cast
	{
		"StaticCastFunctionOverload", 3,
		"int f(int v) { return v + 1; }\n"
		"int f(long v) { return (int)v + 2; }\n"
		"int f(int a, int b) { return a + b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (static_cast<int (*)(long)>(f)(1) == 3 && static_cast<int (*)(int)>(f)(1) == 2) r += 1;\n"
		"	if (static_cast<int (*)(int, int)>(&f)(1, 2) == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.10 [expr.reinterpret.cast]: reinterpret cast
	{
		"ReinterpretCastForms", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct A { int a; int f() { return a; } };\n"
		"struct B { long b; };\n"
		"enum E { E0, E7 = 7 };\n"
		"int Function(int v) { return v + 1; }\n"
		"typedef decltype(sizeof(0)) Size;\n"
		"int main()\n"
		"{\n"
		"	int i = 0x1234;\n"
		"	A a = { 5 };\n"
		"	int* p = &i;\n"
		"	int r = 0;\n"
		"	unsigned long long address = reinterpret_cast<unsigned long long>(p);\n"
		"	if (reinterpret_cast<int*>(address) == p && *reinterpret_cast<int*>(address) == 0x1234) r += 1;\n"
		"	if (reinterpret_cast<int*>(reinterpret_cast<B*>(p)) == p && reinterpret_cast<A*>(reinterpret_cast<char*>(&a))->a == 5) r += 2;\n"
		"	void (*other)() = reinterpret_cast<void (*)()>(Function);\n"
		"	if (reinterpret_cast<int (*)(int)>(other)(1) == 2 && reinterpret_cast<int (*)(int)>(other) == Function) r += 4;\n"
		"	reinterpret_cast<unsigned int&>(i) = 0x4321;\n"
		"	if (i == 0x4321 && &reinterpret_cast<char&>(i) == reinterpret_cast<char*>(&i)) r += 8;\n"
		"	int B::* otherMember = reinterpret_cast<int B::*>(&A::a);\n"
		"	int (B::*otherFunction)() = reinterpret_cast<int (B::*)()>(&A::f);\n"
		"	if (a.*reinterpret_cast<int A::*>(otherMember) == 5 && (a.*reinterpret_cast<int (A::*)()>(otherFunction))() == 5) r += 16;\n"
		"	if (reinterpret_cast<int>(i) == 0x4321 && reinterpret_cast<int*>(p) == p && reinterpret_cast<E>(E7) == E7) r += 32;\n"
		"	if (reinterpret_cast<unsigned long long>(nullptr) == 0 && reinterpret_cast<int*>(0) == 0 && reinterpret_cast<unsigned long long>((int*)0) == 0) r += 64;\n"
		"	if (Same<decltype(reinterpret_cast<long&>(i)), long&>::V && Same<decltype(reinterpret_cast<char*>(p)), char*>::V && Same<decltype(reinterpret_cast<int&&>(i)), int&&>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.10 [expr.reinterpret.cast] p7: object pointers
	{
		"ReinterpretCastBytes", 3,
		"int main()\n"
		"{\n"
		"	unsigned long l = 0x04030201ul;\n"
		"	unsigned char* p = reinterpret_cast<unsigned char*>(&l);\n"
		"	const unsigned short* ps = reinterpret_cast<const unsigned short*>(&l);\n"
		"	int r = 0;\n"
		"	if (p[0] + p[1] + p[2] + p[3] == 10 && (p[0] == 1 || p[0] == 4 || sizeof(long) == 8)) r += 1;\n"
		"	if (ps[0] == 0x0201 || ps[0] == 0x0403 || sizeof(long) == 8) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.10 [expr.reinterpret.cast] p2: constness is not cast away
	{
		"ReinterpretCastAwayConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; long* p = reinterpret_cast<long*>(&i); return p ? 1 : 2; }\n"
	},
	//5.2.10 [expr.reinterpret.cast] p4: a pointer to an integer large enough
	{
		"ReinterpretCastPointerToSmallIntegerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; char c = reinterpret_cast<char>(&i); return c ? 1 : 2; }\n"
	},
	//5.2.10 [expr.reinterpret.cast] p1: only the listed conversions
	{
		"ReinterpretCastFloatRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.5; long long l = reinterpret_cast<long long>(d); return l ? 1 : 2; }\n"
	},
	{
		"ReinterpretCastIntToLongRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = reinterpret_cast<long>(i); return l ? 1 : 2; }\n"
	},
	{
		"ReinterpretCastToNullptrRefused", CONFORMANCE_REFUSED,
		"int main() { decltype(nullptr) n = reinterpret_cast<decltype(nullptr)>(0); return n ? 1 : 2; }\n"
	},
	{
		"ReinterpretCastClassRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"int main() { A a = { 1 }; return reinterpret_cast<B>(a).b; }\n"
	},
	{
		"ReinterpretCastMemberToObjectPointerRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"int main() { int A::* pm = &A::a; int* p = reinterpret_cast<int*>(pm); return p ? 1 : 2; }\n"
	},
	//5.2.10 [expr.reinterpret.cast] p11: a glvalue to a reference
	{
		"ReinterpretCastRvalueToReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { long& r = reinterpret_cast<long&>(5); return r ? 1 : 2; }\n"
	},
	//5.2.11 [expr.const.cast]: const cast
	{
		"ConstCastForms", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; int Get() { return m; } };\n"
		"int main()\n"
		"{\n"
		"	int i = 1;\n"
		"	const int* cp = &i;\n"
		"	const int& cr = i;\n"
		"	const int* const* cpp = &cp;\n"
		"	volatile int* vp = &i;\n"
		"	S s = { 4 };\n"
		"	const S* cs = &s;\n"
		"	const int S::* cpm = &S::m;\n"
		"	const int* null = 0;\n"
		"	int r = 0;\n"
		"	*const_cast<int*>(cp) = 2;\n"
		"	if (i == 2 && const_cast<int*>(cp) == &i) r += 1;\n"
		"	const_cast<int&>(cr) = 3;\n"
		"	if (i == 3 && &const_cast<int&>(cr) == &i) r += 2;\n"
		"	**const_cast<int**>(cpp) = 5;\n"
		"	*const_cast<int*>(vp) += 1;\n"
		"	if (i == 6) r += 4;\n"
		"	if (const_cast<S*>(cs)->Get() == 4 && const_cast<S&>(*cs).Get() == 4) r += 8;\n"
		"	s.*const_cast<int S::*>(cpm) = 9;\n"
		"	if (s.m == 9 && const_cast<int*>(null) == 0) r += 16;\n"
		"	if (Same<decltype(const_cast<const int*>(&i)), const int*>::V && Same<decltype(const_cast<int&&>(i)), int&&>::V && Same<decltype(const_cast<const volatile int&>(i)), const volatile int&>::V) r += 32;\n"
		"	if (const_cast<int&&>(i) == 6 && *const_cast<const volatile int*>(&i) == 6 && const_cast<int*>(&i) == &i) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.11 [expr.const.cast] p1: only constness changes
	{
		"ConstCastChangesTypeRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; long* p = const_cast<long*>(&i); return p ? 1 : 2; }\n"
	},
	//5.2.11 [expr.const.cast] p1: a pointer, reference or pointer to member
	{
		"ConstCastToValueRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; return const_cast<int>(i); }\n"
	},
	//5.2.11 [expr.const.cast] p2: not function pointers
	{
		"ConstCastFunctionPointerRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { int (*p)() = f; return const_cast<int (*)()>(p)(); }\n"
	},
	//5.2.11 [expr.const.cast] p4: a prvalue only to an rvalue reference of class type
	{
		"ConstCastRvalueToLvalueReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { int& r = const_cast<int&>(5); return r; }\n"
	},
	//5.2.11 [expr.const.cast] p1: only constness changes
	{
		"ConstCastDerivedRefused", CONFORMANCE_REFUSED,
		"struct B { int b; };\n"
		"struct D : B { };\n"
		"int main() { D d; const D* p = &d; B* q = const_cast<B*>(p); return q ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p1: indirection
	{
		"UnaryIndirection", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct I;\n"
		"I* Through(I* p) { return &*p; }\n"
		"struct I { int v; };\n"
		"int f(int v) { return v + 1; }\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	int* p = &i;\n"
		"	int** pp = &p;\n"
		"	const int* cp = &i;\n"
		"	int (*pf)(int) = f;\n"
		"	I in = { 3 };\n"
		"	int r = 0;\n"
		"	*p = 6;\n"
		"	**pp += 1;\n"
		"	if (i == 7 && &*p == p && *&i == 7) r += 1;\n"
		"	if (Same<decltype(*p), int&>::V && Same<decltype(*cp), const int&>::V && Same<decltype(**pp), int&>::V && Same<decltype(*pp), int*&>::V) r += 2;\n"
		"	if ((*pf)(1) == 2 && (*****pf)(1) == 2 && Same<decltype(*pf), int (&)(int)>::V) r += 4;\n"
		"	if (Through(&in) == &in && Through(&in)->v == 3) r += 8;\n"
		"	if (*\"abc\" == 'a' && *(char*)\"xyz\" == 'x') r += 16;\n"
		"	*pp = 0;\n"
		"	if (p == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"UnaryIndirectionVoidRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; void* p = &i; *p; return 1; }\n"
	},
	{
		"UnaryIndirectionNotPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; return *i; }\n"
	},
	{
		"UnaryIndirectionConstWriteRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; const int* p = &i; *p = 2; return i; }\n"
	},
	//5.3.1 [expr.unary.op] p2-p6: address of
	{
		"UnaryAddressOf", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; mutable int mut; static int s; int f() { return m; } static int sf() { return 9; } int a[2]; };\n"
		"int S::s = 4;\n"
		"int g(int v) { return v + 1; }\n"
		"int g(long v) { return (int)v + 2; }\n"
		"int gi = 1;\n"
		"int& Ref() { return gi; }\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	const int ci = 6;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int& ri = i;\n"
		"	S s = { 1, 2, { 3, 4 } };\n"
		"	int (*pg)(long) = &g;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(&i), int*>::V && Same<decltype(&ci), const int*>::V && Same<decltype(&a), int (*)[3]>::V && Same<decltype(&a[1]), int*>::V) r += 1;\n"
		"	if (&ri == &i && &Ref() == &gi && &*a == a && (void*)&a == (void*)a && &a[1] == a + 1) r += 2;\n"
		"	if (Same<decltype(&S::m), int S::*>::V && Same<decltype(&S::mut), int S::*>::V && Same<decltype(&S::a), int (S::*)[2]>::V) r += 4;\n"
		"	if (Same<decltype(&S::s), int*>::V && Same<decltype(&S::sf), int (*)()>::V && Same<decltype(&S::f), int (S::*)()>::V) r += 8;\n"
		"	if (Same<decltype(&s.m), int*>::V && &s.m == (int*)&s && &s.a[1] == s.a + 1 && &s.s == &S::s) r += 16;\n"
		"	if (pg(1) == 3 && (&S::sf)() == 9 && *&S::s == 4) r += 32;\n"
		"	if (Same<decltype(&\"ab\"), const char (*)[3]>::V && Same<decltype(&main), int (*)()>::V) r += 64;\n"
		"	if (&(i) == &i && &((s).m) == &s.m && &*&*&i == &i) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op] p3: the operand is an lvalue or a qualified name
	{
		"UnaryAddressOfRvalueRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S* p = &S(); return p ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p4: not a bit-field
	{
		"UnaryAddressOfBitFieldRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 3; };\n"
		"int main() { S s; int* p = &s.b; return p ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p4: a pointer to member is formed only by & and an unparenthesised qualified name
	{
		"UnaryAddressOfParenthesisedMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { int S::* p = &(S::m); return p ? 1 : 2; }\n"
	},
	{
		"UnaryAddressOfUnqualifiedMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int S::* f() { return &m; } };\n"
		"int main() { S s; return s.f() ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p6: an overloaded function's address needs a target
	{
		"UnaryAddressOfOverloadAmbiguousRefused", CONFORMANCE_REFUSED,
		"int g(int v) { return v + 1; }\n"
		"int g(long v) { return (int)v + 2; }\n"
		"int main() { auto p = &g; return p ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p7-p8: unary plus and minus
	{
		"UnaryPlusMinus", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E3 = 3 };\n"
		"int main()\n"
		"{\n"
		"	unsigned int u = 1;\n"
		"	unsigned long ul = 1;\n"
		"	unsigned long long ull = 1;\n"
		"	unsigned char uc = 1;\n"
		"	short s = -5;\n"
		"	long long ll = 5;\n"
		"	double d = 2.5;\n"
		"	float f = -1.5f;\n"
		"	int a[2] = { 1, 2 };\n"
		"	int* p = a;\n"
		"	int r = 0;\n"
		"	if (-u == ~0u && -u > 0 && Same<decltype(-u), unsigned int>::V) r += 1;\n"
		"	if (-ul == ~0ul && -ull == ~0ull && -ull > 0) r += 2;\n"
		"	if (-uc == -1 && Same<decltype(-uc), int>::V && -s == 5 && Same<decltype(-s), int>::V) r += 4;\n"
		"	if (-ll == -5 && -(-ll) == 5 && Same<decltype(-ll), long long>::V && - -ll == 5) r += 8;\n"
		"	if (-d == -2.5 && -f == 1.5f && +d == 2.5 && Same<decltype(-f), float>::V) r += 16;\n"
		"	if (+p == a && Same<decltype(+p), int*>::V && Same<decltype(+a), int*>::V && +E3 == 3 && -E3 == -3 && Same<decltype(+E3), int>::V) r += 32;\n"
		"	if (-(-2147483647L - 1) / 2 != 0 && -0 == 0 && 3 - -2 == 5 && 3 + +2 == 5 && - - -1 == -1) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op] p8: minus takes arithmetic or unscoped enumeration
	{
		"UnaryMinusPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return -p ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p7: plus takes arithmetic, enumeration or pointer
	{
		"UnaryPlusClassRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; +s; return 1; }\n"
	},
	//5.3.1 [expr.unary.op] p9: logical negation
	{
		"UnaryNot", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; };\n"
		"enum E { E0, E1 };\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	int z = 0;\n"
		"	int* p = &i;\n"
		"	int* n = 0;\n"
		"	double d = 0.5;\n"
		"	double dz = 0.0;\n"
		"	int S::* pm = &S::m;\n"
		"	int S::* nm = 0;\n"
		"	long long big = 0x100000000LL;\n"
		"	int r = 0;\n"
		"	if (!z && !(!i) && !!i == 1 && Same<decltype(!i), bool>::V) r += 1;\n"
		"	if (!n && !(!p) && !nm && !(!pm)) r += 2;\n"
		"	if (!dz && !(!d) && !0.0f && !!0.25f) r += 4;\n"
		"	if (!(!big) && !E0 && !(!E1) && !false && !(!true)) r += 8;\n"
		"	if (!i + !z == 1 && Same<decltype(!i + !z), int>::V && !!big + !!d == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op] p10: complement
	{
		"UnaryComplement", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E1 = 1 };\n"
		"struct X { int v; X(int x) : v(x) { } int operator~() const { return v + 100; } };\n"
		"int main()\n"
		"{\n"
		"	unsigned char uc = 0xF0;\n"
		"	unsigned short us = 1;\n"
		"	unsigned int u = 0;\n"
		"	long l = 0;\n"
		"	unsigned long long ull = 0xFFull;\n"
		"	bool b = true;\n"
		"	int r = 0;\n"
		"	if (~0 == -1 && ~5 == -6 && ~-1 == 0) r += 1;\n"
		"	if (~uc == -241 && (unsigned char)~uc == 0x0F && Same<decltype(~uc), int>::V) r += 2;\n"
		"	if (~u == (unsigned int)-1 && ~l == -1L && Same<decltype(~l), long>::V && ~ull == 0xFFFFFFFFFFFFFF00ull) r += 4;\n"
		"	if (~b == -2 && ~false == -1 && ~E1 == -2 && Same<decltype(~E1), int>::V) r += 8;\n"
		"	if ((sizeof(int) > sizeof(short)) ? (~us < 0) : (~us == 65534u)) r += 16;\n"
		"	if (~X(5) == 105 && ~~7 == 7) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op] p10: complement takes integral or unscoped enumeration
	{
		"UnaryComplementFloatRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.5; return ~d ? 1 : 2; }\n"
	},
	{
		"UnaryComplementPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return ~p ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op] p9: the operand is contextually converted to bool
	{
		"UnaryNotScopedEnumRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { return !E::B ? 1 : 2; }\n"
	},
	//5.3.2 [expr.pre.incr]: prefix increment and decrement
	{
		"PreIncrementForms", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { unsigned a : 3; int b : 4; };\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	unsigned char uc = 255;\n"
		"	long long ll = 0xFFFFFFFFLL;\n"
		"	double d = 1.5;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int* p = a;\n"
		"	volatile int v = 1;\n"
		"	S s;\n"
		"	s.a = 7; s.b = 7;\n"
		"	int r = 0;\n"
		"	if (++i == 6 && i == 6 && --i == 5 && i == 5) r += 1;\n"
		"	++i = 10;\n"
		"	if (i == 10 && &++i == &i && i == 11) r += 2;\n"
		"	++++i;\n"
		"	(++i)++;\n"
		"	if (i == 15 && Same<decltype(++i), int&>::V && Same<decltype(--p), int*&>::V) r += 4;\n"
		"	if (++uc == 0 && --uc == 255 && ++ll == 0x100000000LL && --ll == 0xFFFFFFFFLL) r += 8;\n"
		"	if (++d == 2.5 && --d == 1.5 && *++p == 2 && *--p == 1 && p == a) r += 16;\n"
		"	++s.a;\n"
		"	--s.b;\n"
		"	if (s.a == 0 && s.b == 6 && ++s.a == 1) r += 32;\n"
		"	++v;\n"
		"	if (v == 2 && --v == 1) r += 64;\n"
		"	int j = 3;\n"
		"	int k = ++j + j;\n"
		"	if (k == 8 && (j += 1, ++j) == 6) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.2 [expr.pre.incr] p1: bool is set to true (deprecated)
	{
		"PreIncrementBool", 3,
		"int main()\n"
		"{\n"
		"	bool b = false;\n"
		"	int r = 0;\n"
		"	if (++b == true && b == true) r += 1;\n"
		"	if (++b == true && (int)b == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.2 [expr.pre.incr] p2: bool cannot be decremented
	{
		"PreDecrementBoolRefused", CONFORMANCE_REFUSED,
		"int main() { bool b = true; --b; return b ? 1 : 2; }\n"
	},
	//5.3.2 [expr.pre.incr] p1: the operand is a modifiable lvalue
	{
		"PreIncrementConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; ++i; return i; }\n"
	},
	{
		"PreIncrementRvalueRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; ++(i + 1); return i; }\n"
	},
	{
		"PreIncrementPostResultRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; ++i++; return i; }\n"
	},
	//5.3.2 [expr.pre.incr] p1: a pointer to a complete object type
	{
		"PreIncrementFunctionPointerTargetRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { int (*p)() = f; ++p; return 1; }\n"
	},
	//5.3.3 [expr.sizeof]: sizeof
	{
		"SizeofRules", 1023,
		"#include <stddef.h>\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct Empty { };\n"
		"struct S { char c; long l; char a[3]; };\n"
		"struct D : S { int d; };\n"
		"int Undefined(double d);\n"
		"long UndefinedLong();\n"
		"int Param(int a[10]) { return sizeof(a) == sizeof(int*); }\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	int a[7];\n"
		"	int m[2][3];\n"
		"	char& rc = *(char*)&i;\n"
		"	S s;\n"
		"	S& rs = s;\n"
		"	int* p = a;\n"
		"	int r = 0;\n"
		"	if (sizeof(char) == 1 && sizeof(signed char) == 1 && sizeof(unsigned char) == 1 && sizeof(bool) >= 1) r += 1;\n"
		"	if (sizeof(rc) == 1 && sizeof(char&) == 1 && sizeof(rs) == sizeof(S) && sizeof(S&) == sizeof(S)) r += 2;\n"
		"	if (sizeof a == 7 * sizeof(int) && sizeof a / sizeof a[0] == 7 && sizeof(m) == 6 * sizeof(int) && sizeof m[0] == 3 * sizeof(int) && sizeof(int[5]) == 5 * sizeof(int)) r += 4;\n"
		"	if (sizeof(\"abc\") == 4 && sizeof 'a' == 1 && sizeof(true ? 'a' : 'b') == 1 && sizeof('a' + 1) == sizeof(int) && sizeof(L\"ab\") == 3 * sizeof(wchar_t)) r += 8;\n"
		"	if (sizeof(i++) == sizeof(int) && sizeof(Undefined(1.5)) == sizeof(int) && sizeof UndefinedLong() == sizeof(long) && i == 5) r += 16;\n"
		"	if (Same<decltype(sizeof(int)), size_t>::V && Same<decltype(sizeof i), size_t>::V) r += 32;\n"
		"	if (sizeof(Empty) >= 1 && sizeof(D) >= sizeof(S) + sizeof(int) && sizeof(S) >= 4 + sizeof(long) && sizeof(S::a) == 3 && sizeof s.l == sizeof(long)) r += 64;\n"
		"	if (Param(a) && sizeof p == sizeof(void*) && sizeof *p == sizeof(int) && sizeof(int (*)[3]) == sizeof(void*) && sizeof(&a) == sizeof(void*)) r += 128;\n"
		"	if (sizeof i + 1 == sizeof(int) + 1 && sizeof(int) * 2 == 2 * sizeof(int) && sizeof -i == sizeof(int) && sizeof(short) + sizeof(char) == 3) r += 256;\n"
		"	char sized[sizeof(long) + sizeof(S)];\n"
		"	if (sizeof(sized) == sizeof(long) + sizeof(S) && sizeof(sizeof(int)) == sizeof(size_t)) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.3 [expr.sizeof] p1: not a function
	{
		"SizeofFunctionRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { return sizeof(f) ? 1 : 2; }\n"
	},
	//5.3.3 [expr.sizeof] p1: not an incomplete type
	{
		"SizeofIncompleteRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"int main() { return sizeof(I) ? 1 : 2; }\n"
	},
	{
		"SizeofIncompleteArrayRefused", CONFORMANCE_REFUSED,
		"extern int ga[];\n"
		"int main() { return sizeof(ga) ? 1 : 2; }\n"
	},
	{
		"SizeofVoidRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof(void) ? 1 : 2; }\n"
	},
	//5.3.3 [expr.sizeof] p1: not a bit-field
	{
		"SizeofBitFieldRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 3; };\n"
		"int main() { S s; return sizeof(s.b) ? 1 : 2; }\n"
	},
	//5.3.3 [expr.sizeof] p5: no type is defined in a sizeof
	{
		"SizeofDefinesTypeRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof(struct N { int a; }) ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p15-p17: what a new expression's object is initialised with
	{
		"NewInitialisation", 255,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct P { int a; int b; };\n"
		"struct C { C() : v(5) { } C(int a, int b) : v(a * 10 + b) { } int v; };\n"
		"struct M { int a = 3; int b; };\n"
		"int main()\n"
		"{\n"
		"	int i = 7;\n"
		"	int r = 0;\n"
		"	int* zero = new int();\n"
		"	int* five = new int(5);\n"
		"	int* six = new int{6};\n"
		"	int* empty = new int{};\n"
		"	int* plain = new int;\n"
		"	if (*zero == 0 && *five == 5 && *six == 6 && *empty == 0 && plain != 0 && plain != five) r += 1;\n"
		"	P* pz = new P();\n"
		"	P* pb = new P{1, 2};\n"
		"	P* pe = new P{};\n"
		"	if (pz->a == 0 && pz->b == 0 && pb->a == 1 && pb->b == 2 && pe->a == 0 && pe->b == 0) r += 2;\n"
		"	C* cd = new C;\n"
		"	C* cv = new C();\n"
		"	C* ca = new C(1, 2);\n"
		"	C* cb = new C{3, 4};\n"
		"	if (cd->v == 5 && cv->v == 5 && ca->v == 12 && cb->v == 34) r += 4;\n"
		"	if (Same<decltype(new const int(5)), const int*>::V && Same<decltype(new int*), int**>::V && Same<decltype(new auto(5L)), long*>::V && Same<decltype(new P), P*>::V) r += 8;\n"
		"	const int* constant = new const int(8);\n"
		"	int** pointer = new int*(&i);\n"
		"	auto deduced = new auto('c');\n"
		"	if (*constant == 8 && **pointer == 7 && *deduced == 'c' && sizeof(*deduced) == 1) r += 16;\n"
		"	double* d = new double(2.5);\n"
		"	long long* ll = new long long(0x123456789LL);\n"
		"	bool* b = new bool(true);\n"
		"	if (*d == 2.5 && *ll == 0x123456789LL && *b && guiLastSize == sizeof(bool)) r += 32;\n"
		"	M* m = new M;\n"
		"	M* mz = new M();\n"
		"	if (m->a == 3 && mz->a == 3 && mz->b == 0) r += 64;\n"
		"	if (giNews == 20 && giDeletes == 0) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p5-p7, p17: arrays
	{
		"NewArrays", 255,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gConstructed = 0;\n"
		"struct C { C() : v(++gConstructed) { } int v; };\n"
		"struct P { int a; int b; };\n"
		"int Three() { return 3; }\n"
		"int main()\n"
		"{\n"
		"	int n = 4;\n"
		"	int r = 0;\n"
		"	int* a = new int[n];\n"
		"	for (int i = 0; i < n; i++) a[i] = i * 2;\n"
		"	if (a[3] == 6 && guiLastSize == 4 * sizeof(int)) r += 1;\n"
		"	int* z = new int[n]();\n"
		"	int* e = new int[n]{};\n"
		"	if (z[0] == 0 && z[3] == 0 && e[0] == 0 && e[3] == 0) r += 2;\n"
		"	int* b = new int[5]{ 1, 2 };\n"
		"	if (b[0] == 1 && b[1] == 2 && b[2] == 0 && b[4] == 0) r += 4;\n"
		"	C* c = new C[Three()];\n"
		"	if (gConstructed == 3 && c[0].v == 1 && c[1].v == 2 && c[2].v == 3) r += 8;\n"
		"	int (*m)[3] = new int[n][3];\n"
		"	m[3][2] = 9;\n"
		"	if (guiLastSize == 12 * sizeof(int) && m[3][2] == 9 && Same<decltype(new int[n][3]), int (*)[3]>::V && Same<decltype(new int[n]), int*>::V) r += 16;\n"
		"	int* none = new int[0];\n"
		"	int* none2 = new int[n - 4];\n"
		"	if (none != 0 && none2 != 0) r += 32;\n"
		"	P* p = new P[2]{ { 1, 2 }, { 3, 4 } };\n"
		"	char* sz = new char[4]{ \"abc\" };\n"
		"	if (p[1].a == 3 && p[0].b == 2 && sz[2] == 'c' && sz[3] == 0) r += 64;\n"
		"	int** pp = new int*[n]();\n"
		"	int (**pf)() = new (int (*[2])());\n"
		"	pf[1] = Three;\n"
		"	if (pp[3] == 0 && pf[1]() == 3 && Same<decltype(new int*[n]), int**>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p6: the bound is converted to size_t
	{
		"NewArrayBound", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"enum E { E3 = 3 };\n"
		"struct Count { operator int() const { return 4; } };\n"
		"int gCalls = 0;\n"
		"int Next() { return ++gCalls + 1; }\n"
		"int main()\n"
		"{\n"
		"	unsigned char uc = 2;\n"
		"	long l = 5;\n"
		"	Count count;\n"
		"	int r = 0;\n"
		"	char* a = new char[uc];\n"
		"	if (a != 0 && guiLastSize == 2) r += 1;\n"
		"	a = new char[l];\n"
		"	if (guiLastSize == 5) r += 2;\n"
		"	a = new char[E3];\n"
		"	char* b = new char[count];\n"
		"	if (guiLastSize == 4 && a != b) r += 4;\n"
		"	a = new char[Next()];\n"
		"	if (guiLastSize == 2 && gCalls == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p11-p13: placement
	{
		"NewPlacementForms", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"struct Place { void* p; };\n"
		"int gTag = 0;\n"
		"size_t gSize = 0;\n"
		"char gacBuffer[64];\n"
		"void* operator new(size_t n, Place at) { gSize = n; return at.p; }\n"
		"void* operator new[](size_t n, Place at) { gSize = n; return at.p; }\n"
		"void* operator new(size_t n, int tag, char c) { gTag = tag + c; gSize = n; return gacBuffer + 32; }\n"
		"struct C { C(int x) : v(x) { } int v; char c[6]; };\n"
		"int main()\n"
		"{\n"
		"	Place at = { gacBuffer };\n"
		"	int r = 0;\n"
		"	int* p = new (at) int(5);\n"
		"	if ((char*)p == gacBuffer && *p == 5 && gSize == sizeof(int)) r += 1;\n"
		"	C* c = new (at) C(7);\n"
		"	if ((char*)c == gacBuffer && c->v == 7 && gSize == sizeof(C)) r += 2;\n"
		"	long* l = new (3, 'a') long(9);\n"
		"	if ((char*)l == gacBuffer + 32 && *l == 9 && gTag == 100 && gSize == sizeof(long)) r += 4;\n"
		"	char* a = new (at) char[10];\n"
		"	if (a == gacBuffer && gSize == 10) r += 8;\n"
		"	int* q = new (Place{ gacBuffer + 8 }) int{ 6 };\n"
		"	if ((char*)q == gacBuffer + 8 && *q == 6) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p9: the allocation function is looked up in the class first
	{
		"NewClassAllocator", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"char gacClass[64];\n"
		"int gClassNews = 0;\n"
		"int gClassDeletes = 0;\n"
		"struct C\n"
		"{\n"
		"	int v;\n"
		"	C() : v(4) { }\n"
		"	static void* operator new(size_t n) { gClassNews++; return gacClass; }\n"
		"	static void operator delete(void* p) { gClassDeletes++; }\n"
		"};\n"
		"struct D : C { int d; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C* c = new C;\n"
		"	if ((char*)c == gacClass && gClassNews == 1 && giNews == 0 && c->v == 4) r += 1;\n"
		"	delete c;\n"
		"	if (gClassDeletes == 1 && giDeletes == 0) r += 2;\n"
		"	C* g = ::new C;\n"
		"	if ((char*)g != gacClass && gClassNews == 1 && giNews == 1 && g->v == 4) r += 4;\n"
		"	::delete g;\n"
		"	C* a = new C[2];\n"
		"	if (gClassNews == 1 && giNews == 2 && giDeletes == 1 && a[1].v == 4) r += 8;\n"
		"	delete[] a;\n"
		"	D* d = new D;\n"
		"	delete d;\n"
		"	if (gClassNews == 2 && gClassDeletes == 2 && giDeletes == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p5: only the first bound may vary
	{
		"NewLaterBoundNotConstantRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int n = 3; int (*p)[3] = (int (*)[3])new int[2][n]; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p7: a constant bound is not negative
	{
		"NewNegativeBoundRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int* p = new int[-1]; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p6: the bound is integral
	{
		"NewFloatBoundRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int* p = new int[2.0]; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p1: a complete object type that is not abstract
	{
		"NewAbstractRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct A { virtual int f() = 0; };\n"
		"int main() { A* p = new A; return p ? 1 : 2; }\n"
	},
	{
		"NewIncompleteRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct I;\n"
		"int main() { I* p = new I; return p ? 1 : 2; }\n"
	},
	{
		"NewReferenceRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int* p = new int&; return p ? 1 : 2; }\n"
	},
	{
		"NewVoidRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { void* p = new void; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p2: auto needs one initialiser
	{
		"NewAutoNoInitialiserRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { auto p = new auto; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p17: the object is initialised by the initialiser
	{
		"NewTooManyInitialisersRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int* p = new int(1, 2); return *p; }\n"
	},
	{
		"NewPrivateConstructorRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct S { private: S() { } };\n"
		"int main() { S* p = new S; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p9: the allocation function is looked up in the class first
	{
		"NewDeletedAllocatorRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct S { static void* operator new(size_t n) = delete; };\n"
		"int main() { S* p = new S; return p ? 1 : 2; }\n"
	},
	//5.3.4 [expr.new] p12: the placement arguments choose the allocation function
	{
		"NewNoMatchingPlacementRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct T { };\n"
		"int main() { T t; int* p = new (t) int; return p ? 1 : 2; }\n"
	},
	//5.3.5 [expr.delete]: delete
	{
		"DeleteForms", 127,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gOrder = 0;\n"
		"int gCount = 0;\n"
		"struct C { int id; C() : id(0) { } ~C() { gCount++; gOrder = gOrder * 10 + id; } };\n"
		"struct B { virtual ~B() { gCount += 10; } };\n"
		"struct D : B { ~D() { gCount += 100; } };\n"
		"struct Handle { int* p; operator int*() const { return p; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int* i = new int(5);\n"
		"	delete i;\n"
		"	if (giDeletes == 1) r += 1;\n"
		"	C* c = new C;\n"
		"	delete c;\n"
		"	if (gCount == 1 && giDeletes == 2) r += 2;\n"
		"	C* a = new C[3];\n"
		"	a[0].id = 1; a[1].id = 2; a[2].id = 3;\n"
		"	gOrder = 0;\n"
		"	delete[] a;\n"
		"	if (gCount == 4 && gOrder == 321 && giDeletes == 3) r += 4;\n"
		"	C* null = 0;\n"
		"	int* nullInt = 0;\n"
		"	delete null;\n"
		"	delete[] null;\n"
		"	delete nullInt;\n"
		"	if (gCount == 4) r += 8;\n"
		"	B* b = new D;\n"
		"	delete b;\n"
		"	if (gCount == 114 && giDeletes == 4) r += 16;\n"
		"	const int* constant = new const int(3);\n"
		"	Handle h = { new int(4) };\n"
		"	delete constant;\n"
		"	delete h;\n"
		"	if (giDeletes == 6) r += 32;\n"
		"	if (Same<decltype(delete i), void>::V && Same<decltype(delete[] a), void>::V) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p10: the deallocation function and the size it is given
	{
		"DeleteClassSized", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"size_t gSize = 0;\n"
		"int gCalls = 0;\n"
		"struct B { virtual ~B() { } int b; static void operator delete(void* p, size_t n) { gSize = n; gCalls++; } };\n"
		"struct D : B { long d[3]; };\n"
		"struct P { char c[5]; static void operator delete(void* p, size_t n) { gSize = n; gCalls += 10; } };\n"
		"struct Both { static void operator delete(void* p) { gCalls += 100; } static void operator delete(void* p, size_t n) { gCalls += 1000; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B* b = new D;\n"
		"	delete b;\n"
		"	if (gSize == sizeof(D) && gCalls == 1) r += 1;\n"
		"	b = new B;\n"
		"	delete b;\n"
		"	if (gSize == sizeof(B) && gCalls == 2) r += 2;\n"
		"	P* p = new P;\n"
		"	delete p;\n"
		"	if (gSize == 5 && gCalls == 12) r += 4;\n"
		"	Both* both = new Both;\n"
		"	delete both;\n"
		"	if (gCalls == 112 && giDeletes == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p1: the operand is a pointer to an object type
	{
		"DeleteNotPointerRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main() { int i = 1; delete i; return 1; }\n"
	},
	{
		"DeleteFunctionPointerRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int f() { return 1; }\n"
		"int main() { int (*p)() = f; delete p; return 1; }\n"
	},
	//5.3.5 [expr.delete] p1: a class with one conversion to a pointer
	{
		"DeleteTwoConversionsRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct H { operator int*() const { return 0; } operator long*() const { return 0; } };\n"
		"int main() { H h; delete h; return 1; }\n"
	},
	//5.3.5 [expr.delete] p10: the destructor is accessible
	{
		"DeletePrivateDestructorRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct S { private: ~S() { } };\n"
		"int main() { S* p = new S; delete p; return 1; }\n"
	},
	//5.3.6 [expr.alignof]: alignof
	{
		"AlignofRules", 31,
		"#include <stddef.h>\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { char c; long l; };\n"
		"struct alignas(8) A8 { char c; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (alignof(char) == 1 && alignof(signed char) == 1 && alignof(unsigned char) == 1) r += 1;\n"
		"	if (alignof(int&) == alignof(int) && alignof(int[4]) == alignof(int) && alignof(S&) == alignof(S) && alignof(S[2][3]) == alignof(S)) r += 2;\n"
		"	if (alignof(S) >= alignof(long) && alignof(S) >= alignof(char) && sizeof(S) % alignof(S) == 0 && alignof(long) <= sizeof(long)) r += 4;\n"
		"	if (Same<decltype(alignof(int)), size_t>::V && alignof(A8) == 8 && sizeof(A8) == 8) r += 8;\n"
		"	char sized[alignof(long long) + alignof(A8)];\n"
		"	if (sizeof(sized) == alignof(long long) + 8 && (alignof(double) & (alignof(double) - 1)) == 0) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.6 [expr.alignof] p1: a complete object type
	{
		"AlignofIncompleteRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"int main() { return alignof(I) ? 1 : 2; }\n"
	},
	{
		"AlignofVoidRefused", CONFORMANCE_REFUSED,
		"int main() { return alignof(void) ? 1 : 2; }\n"
	},
	{
		"AlignofFunctionRefused", CONFORMANCE_REFUSED,
		"int main() { return alignof(int()) ? 1 : 2; }\n"
	},
	//5.3.6 [expr.alignof] p1: the operand is a type
	{
		"AlignofExpressionRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; return alignof(i) ? 1 : 2; }\n"
	},
	//5.3.7 [expr.unary.noexcept]: the noexcept operator
	{
		"NoexceptOperatorRules", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"void Never() noexcept;\n"
		"void May();\n"
		"int Value() noexcept(true);\n"
		"int Other() noexcept(false);\n"
		"struct S { S() noexcept; S(int); ~S(); void Never() noexcept; void May(); S& operator+=(int) noexcept; S operator+(int) const; };\n"
		"struct T { T(); ~T() noexcept(false); };\n"
		"template<class U> void Generic() noexcept(sizeof(U) == 1);\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	S* ps = 0;\n"
		"	void (*pointer)() = 0;\n"
		"	int r = 0;\n"
		"	if (noexcept(Never()) && !noexcept(May()) && noexcept(Value()) && !noexcept(Other())) r += 1;\n"
		"	if (noexcept(1 + 2) && noexcept(i++) && noexcept(i = 7) && i == 5 && noexcept(May) && noexcept(&May)) r += 2;\n"
		"	if (noexcept(S()) && !noexcept(S(1)) && noexcept(ps->Never()) && !noexcept(ps->May())) r += 4;\n"
		"	if (noexcept(*ps += 1) && !noexcept(*ps + 1) && noexcept(ps->~S()) && !noexcept(T())) r += 8;\n"
		"	if (!noexcept(pointer()) && noexcept(pointer) && noexcept(noexcept(May())) && noexcept(sizeof(May(), 1))) r += 16;\n"
		"	if (Same<decltype(noexcept(1)), bool>::V && noexcept(Generic<char>()) && !noexcept(Generic<long>())) r += 32;\n"
		"	char sized[noexcept(Never()) ? 3 : 5];\n"
		"	static_assert(noexcept(Never()) && !noexcept(May()), \"constant\");\n"
		"	if (sizeof(sized) == 3 && !noexcept(Never(), May()) && noexcept(true ? Value() : 2) && !noexcept(i ? Value() : Other())) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.4 [expr.cast]: explicit type conversion, cast notation
	{
		"CastNotation", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"struct D : A, B { int d; };\n"
		"enum E { E0, E3 = 3 };\n"
		"enum class C { A, B };\n"
		"int Function(int v) { return v + 1; }\n"
		"int gCalls = 0;\n"
		"int Call() { return ++gCalls; }\n"
		"int main()\n"
		"{\n"
		"	int i = 65;\n"
		"	const int ci = 7;\n"
		"	const int* cp = &i;\n"
		"	double d = 3.9;\n"
		"	D derived;\n"
		"	derived.a = 1; derived.b = 2; derived.d = 3;\n"
		"	const B* cb = &derived;\n"
		"	void* v = &i;\n"
		"	int r = 0;\n"
		"	if ((int)d == 3 && (char)i == 'A' && (unsigned char)-1 == 255 && (double)i / 2 == 32.5 && (long)(short)-3 == -3L && (bool)d) r += 1;\n"
		"	if ((E)3 == E3 && (int)E3 == 3 && (C)1 == C::B && (int)C::B == 1 && (E)C::A == E0) r += 2;\n"
		"	*(int*)cp = 66;\n"
		"	if (i == 66 && (int*)v == &i && (void*)cp == v && *(const char*)v == 66 + 0 * (int)sizeof(v)) r += 4;\n"
		"	if ((D*)cb == &derived && ((D*)cb)->d == 3 && &(D&)*cb == &derived && ((const D&)*cb).a == 1) r += 8;\n"
		"	(void)Call();\n"
		"	(void)i;\n"
		"	if (gCalls == 1 && (B*)&derived == cb && ((B)derived).b == 2 && ((A&)derived).a == 1) r += 16;\n"
		"	unsigned long long address = (unsigned long long)&i;\n"
		"	if ((int*)address == &i && ((int (*)(int))(void (*)())Function)(1) == 2 && *(unsigned char*)(long*)&i == (66 & 255) * (*(unsigned char*)&i == 66)) r += 32;\n"
		"	if (Same<decltype((long)i), long>::V && Same<decltype((int&)i), int&>::V && Same<decltype((int&&)i), int&&>::V && Same<decltype((const int)i), int>::V) r += 64;\n"
		"	(int&)ci;\n"
		"	(int&)i = 70;\n"
		"	if (i == 70 && (int)'a' == 97 && (int)3.99f == 3 && (float)1 / 2 == 0.5f) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.4 [expr.cast] p4: a cast to and from an inaccessible base
	{
		"CastNotationPrivateBase", 7,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"struct D : A, private B { D() { b = 7; } B* Base() { return this; } int d; };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	B* pb = (B*)&d;\n"
		"	int r = 0;\n"
		"	if (pb == d.Base() && pb->b == 7 && (void*)pb != (void*)&d) r += 1;\n"
		"	if ((D*)pb == &d && ((B&)d).b == 7) r += 2;\n"
		"	if (&(D&)*pb == &d) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.4 [expr.cast]: explicit type conversion, cast notation
	{
		"CastNotationToArrayRefused", CONFORMANCE_REFUSED,
		"int main() { int a[3] = { 1, 2, 3 }; return ((int[3])a)[0]; }\n"
	},
	{
		"CastNotationNoConversionRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { int i = 1; return ((S)i).m; }\n"
	},
	{
		"CastNotationClassToIntRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; return (int)s; }\n"
	},
	{
		"CastNotationFloatToPointerRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.5; int* p = (int*)d; return p ? 1 : 2; }\n"
	},
	{
		"CastNotationPointerToFloatRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; double d = (double)&i; return d ? 1 : 2; }\n"
	},
	{
		"CastNotationMemberPointerToPointerRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { int S::* pm = &S::m; int* p = (int*)pm; return p ? 1 : 2; }\n"
	},
	//5.5 [expr.mptr.oper]: pointer to member operators
	{
		"MemberPointerOperators", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct In { int x; };\n"
		"struct S\n"
		"{\n"
		"	int a; int b; In in; int arr[3]; int* ptr;\n"
		"	int Get() { return a; }\n"
		"	int GetConst() const { return b; }\n"
		"	int Add(int x, int y) { return a + x + y; }\n"
		"	virtual int Virtual() { return 1; }\n"
		"};\n"
		"struct D : S { int Virtual() override { return 2; } int d; };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	D d;\n"
		"	S* p = &s;\n"
		"	const S* cp = &s;\n"
		"	int target = 9;\n"
		"	s.a = 1; s.b = 2; s.in.x = 3; s.arr[1] = 4; s.ptr = &target;\n"
		"	d.a = 5; d.b = 6;\n"
		"	int S::* pa = &S::a;\n"
		"	int S::* pb = &S::b;\n"
		"	In S::* pin = &S::in;\n"
		"	int (S::* parr)[3] = &S::arr;\n"
		"	int* S::* pptr = &S::ptr;\n"
		"	int (S::*pf)() = &S::Get;\n"
		"	int (S::*pc)() const = &S::GetConst;\n"
		"	int (S::*padd)(int, int) = &S::Add;\n"
		"	int (S::*pv)() = &S::Virtual;\n"
		"	int r = 0;\n"
		"	if (s.*pa == 1 && p->*pb == 2 && (*p).*pa == 1 && cp->*pb == 2) r += 1;\n"
		"	s.*pa = 10;\n"
		"	p->*pb += 5;\n"
		"	if (s.a == 10 && s.b == 7 && &(s.*pa) == &s.a && &(p->*pb) == &s.b) r += 2;\n"
		"	if ((s.*pin).x == 3 && (p->*parr)[1] == 4 && *(s.*pptr) == 9) r += 4;\n"
		"	if (s.*pa + 1 == 11 && -(s.*pa) == -10 && s.*pa * 2 == 20 && 2 * s.*pa - s.*pb == 13 && !(s.*pa) == false) r += 8;\n"
		"	if ((s.*pf)() == 10 && (p->*pf)() == 10 && (cp->*pc)() == 7 && (s.*padd)(1, 2) == 13) r += 16;\n"
		"	if ((s.*pv)() == 1 && (d.*pv)() == 2 && (static_cast<S*>(&d)->*pv)() == 2 && d.*pa == 5 && (d.*pf)() == 5) r += 32;\n"
		"	if (Same<decltype((s.*pa)), int&>::V && Same<decltype((cp->*pa)), const int&>::V && Same<decltype((p->*pin)), In&>::V && Same<decltype((s.*pf)()), int>::V) r += 64;\n"
		"	pa = pb;\n"
		"	pf = pv;\n"
		"	if (s.*pa == 7 && (s.*pf)() == 1 && (d.*pf)() == 2) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.5 [expr.mptr.oper] p2-p3: the object's class is the member's class or derived from it
	{
		"MemberPointerSecondBase", 7,
		"struct A { int a; };\n"
		"struct B { int b; int GetB() { return b; } };\n"
		"struct D : A, B { int d; };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	D* p = &d;\n"
		"	d.a = 1; d.b = 2; d.d = 3;\n"
		"	int B::* pb = &B::b;\n"
		"	int (B::*pf)() = &B::GetB;\n"
		"	int r = 0;\n"
		"	if (d.*pb == 2 && p->*pb == 2) r += 1;\n"
		"	if ((d.*pf)() == 2 && (p->*pf)() == 2) r += 2;\n"
		"	d.*pb = 8;\n"
		"	if (d.b == 8 && d.a == 1 && d.d == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.5 [expr.mptr.oper] p5: constness of the object
	{
		"MemberPointerConstWriteRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { const S s = { 1 }; int S::* pm = &S::m; s.*pm = 2; return 1; }\n"
	},
	{
		"MemberPointerMutableConstRefused", CONFORMANCE_REFUSED,
		"struct S { mutable int m; };\n"
		"int main() { const S s = { 1 }; int S::* pm = &S::m; s.*pm = 2; return 1; }\n"
	},
	//5.5 [expr.mptr.oper] p6: a member function is only called
	{
		"MemberPointerNotCalledRefused", CONFORMANCE_REFUSED,
		"struct S { int f() { return 1; } };\n"
		"int main() { S s; int (S::*pf)() = &S::f; auto x = s.*pf; return 1; }\n"
	},
	//5.5 [expr.mptr.oper] p5: constness of the object
	{
		"MemberPointerConstObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int f() { return 1; } };\n"
		"int main() { const S s = S(); int (S::*pf)() = &S::f; return (s.*pf)(); }\n"
	},
	//5.5 [expr.mptr.oper] p2: .* takes an object, ->* a pointer
	{
		"MemberPointerOnPointerRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; S* p = &s; int S::* pm = &S::m; return p.*pm; }\n"
	},
	//5.5 [expr.mptr.oper] p3: .* takes an object, ->* a pointer
	{
		"MemberPointerArrowOnObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; int S::* pm = &S::m; return s->*pm; }\n"
	},
	//5.5 [expr.mptr.oper] p2: the object's class is the member's class or derived from it
	{
		"MemberPointerUnrelatedClassRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"struct T { int m; };\n"
		"int main() { T t = { 1 }; int S::* pm = &S::m; return t.*pm; }\n"
	},
	{
		"MemberPointerBaseObjectDerivedMemberRefused", CONFORMANCE_REFUSED,
		"struct B { int b; };\n"
		"struct D : B { int d; };\n"
		"int main() { B b = { 1 }; int D::* pm = &D::d; return b.*pm; }\n"
	},
	//5.6 [expr.mul]: multiplicative operators
	{
		"MultiplicativeIntegers", 255,
		"enum E { E3 = 3 };\n"
		"int main()\n"
		"{\n"
		"	int a = -7;\n"
		"	int b = 2;\n"
		"	int c = 7;\n"
		"	int d = -2;\n"
		"	unsigned int u = 7;\n"
		"	unsigned int minusOne = -1;\n"
		"	long l = 100000L;\n"
		"	long long ll = 10000000000LL;\n"
		"	unsigned long long ull = 0xFFFFFFFFFFFFFFFFull;\n"
		"	unsigned char uc = 100;\n"
		"	short s = -100;\n"
		"	int r = 0;\n"
		"	if (a / b == -3 && c / d == -3 && a / d == 3 && c / b == 3) r += 1;\n"
		"	if (a % b == -1 && c % d == 1 && a % d == -1 && c % b == 1) r += 2;\n"
		"	if ((a / b) * b + a % b == a && (c / d) * d + c % d == c && (a / d) * d + a % d == a) r += 4;\n"
		"	if (minusOne / u == ~0u / 7 && minusOne % u == ~0u % 7 && u * u == 49u && a * b == -14 && a * d == 14) r += 8;\n"
		"	if (l * 3 == 300000L && l / 7 == 14285L && l % 7 == 5 && -l / 7 == -14285L && -l % 7 == -5) r += 16;\n"
		"	if (ll / 3 == 3333333333LL && ll % 3 == 1 && l * (long long)l == ll && -ll / 7 == -1428571428LL && -ll % 7 == -4) r += 32;\n"
		"	if (ull / 10 == 1844674407370955161ull && ull % 10 == 5 && ull / 0x100000000ull == 0xFFFFFFFFull && ull * 2 == 0xFFFFFFFFFFFFFFFEull) r += 64;\n"
		"	if (uc * uc == 10000 && s * uc == -10000 && s / uc == -1 && s % 7 == -2 && E3 * 2 == 6 && 7 / E3 == 2 && 7 % E3 == 1) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"MultiplicativeFloating", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	double d = 7.0;\n"
		"	float f = 2.5f;\n"
		"	int i = 2;\n"
		"	long long ll = 3;\n"
		"	int r = 0;\n"
		"	if (d / 2 == 3.5 && 1 / 2 == 0 && 1 / 2.0 == 0.5 && d * 2 == 14.0 && -d / 2 == -3.5) r += 1;\n"
		"	if (f * f == 6.25f && f / 2 == 1.25f && f * i == 5.0f && d / f == 2.8 && f * d == 17.5) r += 2;\n"
		"	if (Same<decltype(f * i), float>::V && Same<decltype(f * d), double>::V && Same<decltype(d / ll), double>::V && Same<decltype(f * ll), float>::V) r += 4;\n"
		"	if (d / ll > 2.33 && d / ll < 2.34 && ll * f == 7.5f && 1.0 / 4 * 4 == 1.0 && 0.1 * 3 != 0.3) r += 8;\n"
		"	if (d * 0 == 0.0 && -d * -d == 49.0 && (d / 3) * 3 > 6.99 && (d / 3) * 3 < 7.01 && 1e10 * 1e10 == 1e20) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.6 [expr.mul] p2: the remainder's operands are integral
	{
		"MultiplicativeFloatRemainderRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 5.0; return (d % 2) ? 1 : 2; }\n"
	},
	//5.6 [expr.mul] p2: arithmetic or unscoped enumeration operands
	{
		"MultiplicativePointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p * 2) ? 1 : 2; }\n"
	},
	//5.6 [expr.mul] p4, 5.19 [expr.const]: a zero divisor is not a constant
	{
		"MultiplicativeConstantDivideByZeroRefused", CONFORMANCE_REFUSED,
		"int main() { constexpr int z = 1 / 0; return z + 1; }\n"
	},
	//5.7 [expr.add]: additive operators
	{
		"AdditiveArithmetic", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E3 = 3 };\n"
		"int main()\n"
		"{\n"
		"	unsigned int u = 0;\n"
		"	unsigned char uc = 200;\n"
		"	unsigned char uc2 = 100;\n"
		"	unsigned short us = 65535u;\n"
		"	long l = 0x7FFFL;\n"
		"	long long ll = 0xFFFFFFFFLL;\n"
		"	unsigned long ul = 0xFFFFFFFFul;\n"
		"	double d = 1.5;\n"
		"	float f = 0.25f;\n"
		"	int i = 3;\n"
		"	int r = 0;\n"
		"	if (u - 1 == ~0u && u - 1 > 0 && u + ~0u == ~0u) r += 1;\n"
		"	if (uc + uc2 == 300 && uc2 - uc == -100 && Same<decltype(uc + uc2), int>::V) r += 2;\n"
		"	if (l + 1 == 0x8000L && l + l == 0xFFFEL && ll + 1 == 0x100000000LL && ll - ll - 1 == -1) r += 4;\n"
		"	if (sizeof(long) == 8 || (ul + 1 == 0 && ul + ul == 0xFFFFFFFEul)) r += 8;\n"
		"	if (d + i == 4.5 && i - d == 1.5 && d + f == 1.75 && f - 1 == -0.75f && Same<decltype(f + 1), float>::V && Same<decltype(i - d), double>::V) r += 16;\n"
		"	if (sizeof(int) > sizeof(short) ? (us + 1 == 65536L) : (us + 1 == 0)) r += 32;\n"
		"	if (E3 + 1 == 4 && E3 - E3 == 0 && 1 - 2 - 3 == -4 && 1 - (2 - 3) == 2 && i + -i == 0 && i - -i == 6) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.7 [expr.add] p4-p7: pointer arithmetic
	{
		"AdditivePointers", 255,
		"#include <stddef.h>\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { char c[7]; };\n"
		"int main()\n"
		"{\n"
		"	int a[6] = { 10, 11, 12, 13, 14, 15 };\n"
		"	S as[4];\n"
		"	long al[4] = { 1, 2, 3, 4 };\n"
		"	int m[3][5];\n"
		"	int* p = a + 2;\n"
		"	int* q = &a[5];\n"
		"	const int* cp = a;\n"
		"	long idx = 3;\n"
		"	unsigned char small = 2;\n"
		"	short negative = -2;\n"
		"	int* null = 0;\n"
		"	int r = 0;\n"
		"	if (*(p + 1) == 13 && *(1 + p) == 13 && *(p - 2) == 10 && *(p + idx) == 15 && *(small + p) == 14 && *(p + negative) == 10) r += 1;\n"
		"	if (q - p == 3 && p - q == -3 && p - a == 2 && (a + 6) - a == 6 && q - cp == 5 && p - p == 0) r += 2;\n"
		"	if (Same<decltype(q - p), ptrdiff_t>::V && Same<decltype(p + 1), int*>::V && Same<decltype(cp + 1), const int*>::V && Same<decltype(a + 1), int*>::V) r += 4;\n"
		"	if (&as[3] - &as[0] == 3 && (char*)(as + 2) - (char*)as == 14 && (as + 3) - 2 == &as[1] && *(al + 2) == 3 && &al[3] - al == 3) r += 8;\n"
		"	if ((m + 2) - m == 2 && (char*)(m + 1) - (char*)m == 5 * (int)sizeof(int) && *(m + 1) + 2 == &m[1][2] && &m[2][4] - &m[2][0] == 4) r += 16;\n"
		"	if (null + 0 == 0 && null - null == 0 && p + 0 == p && p - 0 == p && (p + 3) - 3 == p) r += 32;\n"
		"	if (*(&a + 1) - a == 6 && (int*)(&a + 1) == a + 6 && a + 6 > a + 5 && &a[6] - &a[0] == 6) r += 64;\n"
		"	if ((p += 2) == a + 4 && (p -= idx) == a + 1 && p[1] == 12 && \"abc\" + 1 != 0 && *(\"abc\" + 1) == 'b') r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.7 [expr.add] p1: a pointer to a complete object type
	{
		"AdditiveVoidPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; void* p = &i; return (p + 1) ? 1 : 2; }\n"
	},
	{
		"AdditiveFunctionPointerRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { int (*p)() = f; return (p + 1) ? 1 : 2; }\n"
	},
	{
		"AdditiveIncompletePointerRefused", CONFORMANCE_REFUSED,
		"struct I;\n"
		"I* Get();\n"
		"int main() { return (Get() + 1) ? 1 : 2; }\n"
	},
	//5.7 [expr.add] p1: one operand is a pointer, the other integral
	{
		"AdditiveTwoPointersRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2 }; int* p = a; int* q = a + 1; return (p + q) ? 1 : 2; }\n"
	},
	//5.7 [expr.add] p2: both pointers to the same type
	{
		"AdditiveDifferentPointersRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = 2; return (&i - &l) ? 1 : 2; }\n"
	},
	//5.7 [expr.add] p2: the pointer is the left operand of a subtraction
	{
		"AdditiveIntegerMinusPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (1 - p) ? 1 : 2; }\n"
	},
	//5.7 [expr.add] p1: one operand is a pointer, the other integral
	{
		"AdditivePointerPlusFloatRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p + 1.0) ? 1 : 2; }\n"
	},
	//5.8 [expr.shift]: shift operators
	{
		"ShiftRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	char c = 1;\n"
		"	unsigned char uc = 0xFF;\n"
		"	int i = 1;\n"
		"	int negative = -8;\n"
		"	unsigned int u = ~0u;\n"
		"	long l = 1;\n"
		"	long long ll = 1;\n"
		"	unsigned long long ull = 0x8000000000000000ull;\n"
		"	int count = 3;\n"
		"	long longCount = 2;\n"
		"	unsigned char byteCount = 4;\n"
		"	int zero = 0;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(c << l), int>::V && Same<decltype(1 << 2ull), int>::V && Same<decltype(l << 1), long>::V && Same<decltype(uc >> 1), int>::V && Same<decltype(u << 1ll), unsigned int>::V) r += 1;\n"
		"	if (i << count == 8 && i << longCount == 4 && i << byteCount == 16 && i << zero == 1 && 64 >> count == 8 && 64 >> zero == 64) r += 2;\n"
		"	if (uc << 4 == 0xFF0 && (unsigned char)(uc << 4) == 0xF0 && uc >> 4 == 0x0F && c << 9 == 512) r += 4;\n"
		"	if (u >> 1 == ~0u / 2 && (u << 1) == ~0u - 1 && u >> (sizeof(int) * 8 - 1) == 1 && 1u << (sizeof(int) * 8 - 1) == ~0u / 2 + 1) r += 8;\n"
		"	if (negative >> 1 == -4 && negative >> 3 == -1 && negative >> 10 == -1 && -1L >> 5 == -1L && (long long)negative >> 40 == -1) r += 16;\n"
		"	if (l << 20 == 0x100000L && ll << 40 == 0x10000000000LL && ull >> 63 == 1 && ull >> 33 == 0x40000000ull && (ll << 33) >> 33 == 1 && 0x12345678L >> 16 == 0x1234) r += 32;\n"
		"	if (1 << 2 + 1 == 8 && 1 << 2 << 3 == 32 && 256 >> 2 >> 1 == 32 && (1 << 2) + 1 == 5 && 16 >> 1 + 1 == 4) r += 64;\n"
		"	if ((ll << count) == 8 && (ll << (count * 10 + longCount)) == 0x100000000LL && (ull >> (count + 60)) == 1 && (l << (zero + 16)) >> 16 == 1) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.8 [expr.shift] p1: integral or unscoped enumeration operands
	{
		"ShiftFloatRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.0; return (d << 2) ? 1 : 2; }\n"
	},
	{
		"ShiftByFloatRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; return (i << 2.0) ? 1 : 2; }\n"
	},
	{
		"ShiftPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p >> 1) ? 1 : 2; }\n"
	},
	//5.9 [expr.rel]: relational operators
	{
		"RelationalRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int a; int b; int c[3]; };\n"
		"enum E { E1 = 1, E2 = 2 };\n"
		"int main()\n"
		"{\n"
		"	int a[4] = { 1, 2, 3, 4 };\n"
		"	int* p = a + 1;\n"
		"	int* q = a + 3;\n"
		"	const int* cp = a + 2;\n"
		"	void* vp = p;\n"
		"	S s;\n"
		"	double d = 1.5;\n"
		"	float f = -1.5f;\n"
		"	long long big = 0x100000000LL;\n"
		"	long long negative = -1;\n"
		"	unsigned long long huge = 0x8000000000000000ull;\n"
		"	int i = 2;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(i < i), bool>::V && Same<decltype(d >= d), bool>::V && Same<decltype(p < q), bool>::V && (i < 3) == 1 && (i > 3) == 0) r += 1;\n"
		"	if (p < q && q > p && p <= p && p >= p && !(p < p) && !(q < p) && a + 4 > a + 3 && p < cp && cp < q && vp < (void*)q) r += 2;\n"
		"	if (&s.a < &s.b && &s.c[0] < &s.c[2] && (void*)&s <= (void*)&s.a && (void*)&s.c[2] > (void*)&s.b) r += 4;\n"
		"	if (d < 2.5 && f < -1.0 && f < d && !(d < d) && d <= d && -0.0 < 0.5 && !(-0.0 < 0.0) && d > i - 1 && f < i) r += 8;\n"
		"	if (big > 0xFFFFFFFFLL && negative < 0 && negative < big && huge > 1 && huge > 0x7FFFFFFFFFFFFFFFull && big >= big && -big < negative) r += 16;\n"
		"	if (1 < 2 < 3 && !(3 > 2 > 1) && (1 < 2) + (2 < 1) == 1 && 1 < 2 == 2 < 3 && 1 + 1 < 3 && 1 << 1 < 3) r += 32;\n"
		"	if (E1 < E2 && E2 > 1 && true > false && 'a' < 'b' && (unsigned char)200 > 100 && (signed char)-56 < 0) r += 64;\n"
		"	if (0x7FFFFFFFL > -0x7FFFFFFFL - 1 && -32767 - 1 < 32767 && 65535u > 1 && 0x80000000ul > 0x7FFFFFFFul && -1L < 1L) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.9 [expr.rel] p1: arithmetic, enumeration or pointer operands
	{
		"RelationalPointerIntegerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p < 5) ? 1 : 2; }\n"
	},
	//5.9 [expr.rel] p2: the pointers are brought to their composite pointer type
	{
		"RelationalUnrelatedPointersRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = 2; return (&i < &l) ? 1 : 2; }\n"
	},
	//5.9 [expr.rel] p1: arithmetic, enumeration or pointer operands
	{
		"RelationalMemberPointersRefused", CONFORMANCE_REFUSED,
		"struct S { int a; int b; };\n"
		"int main() { return (&S::a < &S::b) ? 1 : 2; }\n"
	},
	{
		"RelationalClassRefused", CONFORMANCE_REFUSED,
		"struct S { int a; };\n"
		"int main() { S s = { 1 }; S t = { 2 }; return (s < t) ? 1 : 2; }\n"
	},
	{
		"RelationalNullPointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p > nullptr) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq]: equality operators
	{
		"EqualityRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct A { int a; };\n"
		"struct B { int b; int f() { return 1; } int g() { return 2; } };\n"
		"struct D : A, B { int d; };\n"
		"int F(int v) { return v; }\n"
		"int G(int v) { return v; }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	D* pd = &d;\n"
		"	B* pb = &d;\n"
		"	A* pa = &d;\n"
		"	D* nd = 0;\n"
		"	B* nb = 0;\n"
		"	int i = 5;\n"
		"	void* v = &i;\n"
		"	const int* cp = &i;\n"
		"	int B::* pm = &B::b;\n"
		"	int D::* pdm = &B::b;\n"
		"	int B::* nm = 0;\n"
		"	int (B::*pf)() = &B::f;\n"
		"	decltype(nullptr) np = nullptr;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(i == i), bool>::V && Same<decltype(pb != pb), bool>::V && (i == 5) == 1 && (i != 5) == 0) r += 1;\n"
		"	if (pb == pd && pd == pb && pa == pd && !(pb != pd) && (void*)pb != (void*)pd) r += 2;\n"
		"	if (nb == nd && nd == 0 && 0 == nb && nb == nullptr && nullptr != pb && pd != 0) r += 4;\n"
		"	if (v == &i && &i == v && cp == &i && cp == v && np == nullptr && nullptr == nullptr && !(np != np)) r += 8;\n"
		"	if (F == F && F != G && &F == F && F != 0 && static_cast<int (*)(int)>(0) == nullptr) r += 16;\n"
		"	if (pm == &B::b && pm != nm && nm == 0 && nm == nullptr && pdm == pm && pm == pdm && pdm != 0) r += 32;\n"
		"	if (pf == &B::f && pf != &B::g && pf != 0 && !(pf == nullptr)) r += 64;\n"
		"	if (true == 1 && 1.0 == 1 && 0.5f == 0.5 && 'a' == 97 && (1 < 2 == 3 < 4) && (1 == 1 == 1) && !(2 == 2 == 2) && 0.1f != 0.1) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.10 [expr.eq] p1: the operands
	{
		"EqualityPointerIntegerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p == 5) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq] p2: the pointers are brought to their composite pointer type
	{
		"EqualityUnrelatedPointersRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long l = 2; return (&i == &l) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq] p1: the operands
	{
		"EqualityMemberAndObjectPointerRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s; int S::* pm = &S::m; int* p = &s.m; return (pm == p) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq] p3: pointers to members are brought to their composite type
	{
		"EqualityUnrelatedMemberPointersRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"struct T { int m; };\n"
		"int main() { return (&S::m == &T::m) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq] p1: the operands
	{
		"EqualityClassRefused", CONFORMANCE_REFUSED,
		"struct S { int a; };\n"
		"int main() { S s = { 1 }; S t = { 1 }; return (s == t) ? 1 : 2; }\n"
	},
	//5.10 [expr.eq] p2: the pointers are brought to their composite pointer type
	{
		"EqualityFunctionAndObjectPointerRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int main() { int i = 1; return (&i == f) ? 1 : 2; }\n"
	},
	//5.11 [expr.bit.and], 5.12 [expr.xor], 5.13 [expr.or]: bitwise operators
	{
		"BitwiseRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E { E1 = 1, E4 = 4 };\n"
		"int main()\n"
		"{\n"
		"	unsigned char uc = 0xFF;\n"
		"	int i = 0x5A;\n"
		"	int minusOne = -1;\n"
		"	unsigned int u = 0xF0u;\n"
		"	long l = 0x12345678L;\n"
		"	long long ll = 0x123456789ABCDEF0LL;\n"
		"	unsigned long long ull = 0xFF00FF00FF00FF00ull;\n"
		"	bool t = true;\n"
		"	bool f = false;\n"
		"	int r = 0;\n"
		"	if ((i & 0x0F) == 0x0A && (i | 0x05) == 0x5F && (i ^ 0xFF) == 0xA5 && (i ^ i) == 0 && (i & ~i) == 0 && (i | ~i) == -1) r += 1;\n"
		"	if ((uc | 0x100) == 0x1FF && (uc & 0x1F0) == 0xF0 && (uc ^ 0x1FF) == 0x100 && Same<decltype(uc & uc), int>::V) r += 2;\n"
		"	if ((t & t) == 1 && (t & f) == 0 && (t | f) == 1 && (t ^ t) == 0 && Same<decltype(t & f), int>::V && Same<decltype(t | f), int>::V) r += 4;\n"
		"	if ((minusOne & u) == 0xF0u && Same<decltype(minusOne & u), unsigned int>::V && (minusOne & l) == l && (minusOne | l) == -1L && Same<decltype(minusOne ^ l), long>::V) r += 8;\n"
		"	if ((l & 0xFFFF0000L) == 0x12340000L && (l | 0xFFL) == 0x123456FFL && (l ^ l) == 0 && (l & 0xFF) == 0x78) r += 16;\n"
		"	if ((ll & ull) == 0x120056009A00DE00ull && (ll | ~ll) == -1LL && (ll ^ ll) == 0 && (ull & 0xFFFFull) == 0xFF00ull) r += 32;\n"
		"	if ((E1 | E4) == 5 && (E4 & 5) == 4 && (E1 ^ E1) == 0 && Same<decltype(E1 | E4), int>::V) r += 64;\n"
		"	if ((1 | 2 ^ 3 & 4) == 3 && (6 & 3 == 3) == 0 && (1 ^ 3 | 4) == 6 && (~1 & 3) == 2 && (1 | 2 == 2) == 1 && (4 & 6 ^ 2) == 6) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.11 [expr.bit.and]: integral or unscoped enumeration operands
	{
		"BitwiseFloatRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.5; return (d & 1) ? 1 : 2; }\n"
	},
	//5.13 [expr.or]: integral or unscoped enumeration operands
	{
		"BitwisePointerRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int* p = &i; return (p | 1) ? 1 : 2; }\n"
	},
	//5.12 [expr.xor]: integral or unscoped enumeration operands
	{
		"BitwiseXorFloatRefused", CONFORMANCE_REFUSED,
		"int main() { float f = 1.5f; return (1 ^ f) ? 1 : 2; }\n"
	},
	//5.14 [expr.log.and], 5.15 [expr.log.or]: logical operators
	{
		"LogicalRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; };\n"
		"int gCalls = 0;\n"
		"bool Yes() { gCalls++; return true; }\n"
		"bool No() { gCalls += 10; return false; }\n"
		"int main()\n"
		"{\n"
		"	int i = 5;\n"
		"	int z = 0;\n"
		"	int* p = &i;\n"
		"	int* n = 0;\n"
		"	double d = 0.5;\n"
		"	int S::* pm = &S::m;\n"
		"	int S::* nm = 0;\n"
		"	long long big = 0x100000000LL;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(i && i), bool>::V && Same<decltype(i || z), bool>::V && (i && 7) == 1 && (z || 9) == 1 && (i && z) == 0 && (z || z) == 0) r += 1;\n"
		"	if (No() && Yes()) r += 1000;\n"
		"	if (gCalls == 10) r += 2;\n"
		"	if (Yes() || No()) r += 4;\n"
		"	if (gCalls == 11 && (Yes() && No()) == false && gCalls == 22 && (No() || Yes()) && gCalls == 33) r += 8;\n"
		"	if (p && !n && (p || n) && !(p && n) && d && (d || z) && (big && i) && pm && !nm && (nm || pm)) r += 16;\n"
		"	int j = 0;\n"
		"	if ((j++ || j == 1) && j == 1 && (++j && j++ == 2 && j == 3) && !(z && ++j) && j == 3 && (i || ++j) && j == 3) r += 32;\n"
		"	if ((1 || 0 && 0) == 1 && (0 && 0 || 1) == 1 && (!0 && 1) && (0 || !0) && (1 && 2 && 3) && !(1 && 0 && 3) && (0 || 0 || 5)) r += 64;\n"
		"	if ((i > 3 && i < 7) && (i < 3 || i > 4) && (i & 1 && i & 4) && !(i & 2 || z) && (5 && 7) + (0 || 3) == 2) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.14 [expr.log.and] p1: the operands are contextually converted to bool
	{
		"LogicalClassRefused", CONFORMANCE_REFUSED,
		"struct S { int m; };\n"
		"int main() { S s = { 1 }; return (s && true) ? 1 : 2; }\n"
	},
	//5.15 [expr.log.or] p1: the operands are contextually converted to bool
	{
		"LogicalScopedEnumRefused", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { return (E::B || false) ? 1 : 2; }\n"
	},
	//5.14 [expr.log.and] p1: the operands are contextually converted to bool
	{
		"LogicalVoidRefused", CONFORMANCE_REFUSED,
		"void f() { }\n"
		"int main() { return (true && f()) ? 1 : 2; }\n"
	},
	//5.16 [expr.cond] p6: the type of a conditional expression
	{
		"ConditionalTypes", 1023,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct B { int b; };\n"
		"struct A { int a; };\n"
		"struct D : A, B { int d; };\n"
		"enum E { E1 = 1, E2 };\n"
		"enum F { F1 = 1 };\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	bool n = false;\n"
		"	int i = 1;\n"
		"	long l = 2;\n"
		"	short s = 3;\n"
		"	char ch = 'a';\n"
		"	unsigned int u = 4;\n"
		"	D d;\n"
		"	D* pd = &d;\n"
		"	B* pb = 0;\n"
		"	int* p = &i;\n"
		"	const int* cp = &i;\n"
		"	void* v = 0;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(c ? 1 : 2.0), double>::V && Same<decltype(c ? ch : 1), int>::V && Same<decltype(c ? 1u : -1), unsigned int>::V && Same<decltype(c ? (short)1 : (char)2), int>::V) r += 1;\n"
		"	if (Same<decltype(c ? l : i), long>::V && Same<decltype(c ? true : false), bool>::V && Same<decltype(c ? s : s), short&>::V && Same<decltype(c ? 1.5f : 2), float>::V) r += 2;\n"
		"	if ((c ? 1 : 2.0) == 1.0 && (n ? 1 : 2.5) == 2.5 && (c ? -1 : u) > 0 && (n ? ch : 1000) == 1000 && sizeof(c ? ch : s) == sizeof(int)) r += 4;\n"
		"	if (Same<decltype(c ? p : 0), int*>::V && Same<decltype(c ? nullptr : p), int*>::V && Same<decltype(c ? p : cp), const int*>::V && Same<decltype(c ? p : v), void*>::V && Same<decltype(c ? nullptr : nullptr), decltype(nullptr)>::V) r += 8;\n"
		"	if ((n ? p : 0) == 0 && (c ? p : 0) == &i && (c ? nullptr : p) == 0 && (c ? p : v) == &i && (n ? cp : p) == &i) r += 16;\n"
		"	if (Same<decltype(c ? pd : pb), B*>::V && (c ? pd : pb) == static_cast<B*>(&d) && (void*)(c ? pd : pb) != (void*)pd && (n ? pd : pb) == 0) r += 32;\n"
		"	if (Same<decltype(c ? E1 : E2), E>::V && Same<decltype(c ? E1 : F1), int>::V && Same<decltype(c ? E1 : 1), int>::V && (n ? E1 : E2) == E2) r += 64;\n"
		"	if (Same<decltype(c ? i : i), int&>::V && Same<decltype(c ? i : 1), int>::V && Same<decltype(c ? i : l), long>::V && Same<decltype(c ? *cp : i), const int&>::V) r += 128;\n"
		"	if (Same<decltype(c ? \"ab\" : \"cd\"), const char (&)[3]>::V && Same<decltype(c ? \"ab\" : \"cde\"), const char*>::V && sizeof(c ? \"ab\" : \"cd\") == 3 && (n ? \"ab\" : \"cd\")[0] == 'c') r += 256;\n"
		"	if (Same<decltype(c ? static_cast<int&&>(i) : static_cast<int&&>(i)), int&&>::V && Same<decltype(c ? (void)0 : (void)1), void>::V && Same<decltype(c ? d : d), D&>::V && Same<decltype(c ? D() : d), D>::V) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p1: one of the second and third operands is evaluated
	{
		"ConditionalEvaluation", 127,
		"int gCalls = 0;\n"
		"int One() { gCalls += 1; return 1; }\n"
		"int Two() { gCalls += 10; return 2; }\n"
		"void Nothing() { gCalls += 100; }\n"
		"void NothingElse() { gCalls += 1000; }\n"
		"int Plus(int v) { return v + 1; }\n"
		"int Minus(int v) { return v - 1; }\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	bool n = false;\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int r = 0;\n"
		"	if ((c ? One() : Two()) == 1 && gCalls == 1 && (n ? One() : Two()) == 2 && gCalls == 11) r += 1;\n"
		"	c ? Nothing() : NothingElse();\n"
		"	n ? Nothing() : NothingElse();\n"
		"	if (gCalls == 1111) r += 2;\n"
		"	if ((c ? n ? 1 : 2 : 3) == 2 && (n ? 1 : c ? 2 : 3) == 2 && (n ? 1 : n ? 2 : 3) == 3 && (c ? 1 : c ? 2 : 3) == 1) r += 4;\n"
		"	if ((c ? Plus : Minus)(5) == 6 && (n ? Plus : Minus)(5) == 4) r += 8;\n"
		"	c ? a : b = 20;\n"
		"	n ? a : b = 30;\n"
		"	if (a == 1 && b == 30) r += 16;\n"
		"	a = c ? 5 : 6;\n"
		"	b = n ? a++ : a--;\n"
		"	if (a == 4 && b == 5 && (c ? a, b : 0) == 5) r += 32;\n"
		"	int i = 0;\n"
		"	int k = (i++ ? 100 : i);\n"
		"	if (k == 1 && i == 1 && (1 ? 2 : 3) + 1 == 3 && (0 ? 2 : 3) * 2 == 6 && (c || n ? 7 : 8) == 7) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p4: both operands lvalues of one type give an lvalue
	{
		"ConditionalLvalues", 127,
		"struct S { int v; };\n"
		"struct B { int b; };\n"
		"struct A { int a; };\n"
		"struct D : A, B { int d; };\n"
		"struct Bits { unsigned x : 3; unsigned y : 3; };\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	bool n = false;\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int a1[3] = { 1, 2, 3 };\n"
		"	int a2[3] = { 4, 5, 6 };\n"
		"	S s1 = { 1 };\n"
		"	S s2 = { 2 };\n"
		"	D d;\n"
		"	B base;\n"
		"	Bits bits;\n"
		"	bits.x = 1; bits.y = 2;\n"
		"	d.b = 7; base.b = 8;\n"
		"	int r = 0;\n"
		"	(c ? a : b) = 10;\n"
		"	(n ? a : b) = 20;\n"
		"	if (a == 10 && b == 20 && &(c ? a : b) == &a && &(n ? a : b) == &b) r += 1;\n"
		"	(c ? a : b)++;\n"
		"	++(n ? a : b);\n"
		"	(c ? a : b) += 5;\n"
		"	if (a == 16 && b == 21) r += 2;\n"
		"	if (sizeof(c ? a1 : a2) == 3 * sizeof(int) && (n ? a1 : a2)[1] == 5 && &(c ? a1 : a2) == &a1) r += 4;\n"
		"	(c ? s1 : s2).v = 30;\n"
		"	(n ? s1 : s2) = s1;\n"
		"	if (s1.v == 30 && s2.v == 30 && &(c ? s1 : s2) == &s1) r += 8;\n"
		"	if (&(c ? d : base) == static_cast<B*>(&d) && (c ? d : base).b == 7 && (n ? d : base).b == 8 && &(n ? d : base) == &base) r += 16;\n"
		"	(c ? bits.x : bits.y) = 5;\n"
		"	(n ? bits.x : bits.y) = 6;\n"
		"	if (bits.x == 5 && bits.y == 6) r += 32;\n"
		"	int& ref = n ? a : b;\n"
		"	ref = 40;\n"
		"	if (b == 40 && (c ? S{ 3 } : s1).v == 3 && (n ? S{ 3 } : s1).v == 30) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p3: an operand of class type is converted to the other's type
	{
		"ConditionalClassConversions", 15,
		"struct FromInt { FromInt(int x) : v(x) { } int v; };\n"
		"struct ToInt { operator int() const { return 7; } };\n"
		"struct B { int b; };\n"
		"struct D : B { int d; };\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	bool n = false;\n"
		"	FromInt f(1);\n"
		"	ToInt t;\n"
		"	D d;\n"
		"	d.b = 3; d.d = 4;\n"
		"	const B cb = { 5 };\n"
		"	int r = 0;\n"
		"	if ((c ? f : 9).v == 1 && (n ? f : 9).v == 9) r += 1;\n"
		"	if ((c ? t : 2) == 7 && (n ? t : 2) == 2 && sizeof(c ? t : 2) == sizeof(int)) r += 2;\n"
		"	if ((c ? d : cb).b == 3 && (n ? d : cb).b == 5) r += 4;\n"
		"	if ((c ? B() : d).b == 0 && (n ? B() : d).b == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p6: the operands have a common type
	{
		"ConditionalMismatchRefused", CONFORMANCE_REFUSED,
		"int main() { bool c = true; return *(c ? \"a\" : 1) ? 1 : 2; }\n"
	},
	//5.16 [expr.cond] p3: an operand of class type is converted to the other's type
	{
		"ConditionalUnrelatedClassesRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"struct T { int v; };\n"
		"int main() { bool c = true; S s = { 1 }; T t = { 2 }; return (c ? s : t).v; }\n"
	},
	//5.16 [expr.cond] p2: void operands
	{
		"ConditionalVoidAndValueRefused", CONFORMANCE_REFUSED,
		"void f() { }\n"
		"int main() { bool c = true; c ? f() : 1; return 1; }\n"
	},
	//5.16 [expr.cond] p4: both operands lvalues of one type give an lvalue
	{
		"ConditionalPrvalueAssignedRefused", CONFORMANCE_REFUSED,
		"int main() { bool c = true; int a = 1; (c ? a : 1) = 2; return a; }\n"
	},
	{
		"ConditionalMixedTypesAssignedRefused", CONFORMANCE_REFUSED,
		"int main() { bool c = true; int a = 1; long b = 2; (c ? a : b) = 2; return a; }\n"
	},
	//5.16 [expr.cond] p6: the operands have a common type
	{
		"ConditionalUnrelatedPointersRefused", CONFORMANCE_REFUSED,
		"int main() { bool c = true; int i = 1; long l = 2; return (c ? &i : &l) ? 1 : 2; }\n"
	},
	//5.16 [expr.cond] p3: an operand of class type is converted to the other's type
	{
		"ConditionalBothWaysRefused", CONFORMANCE_REFUSED,
		"struct T;\n"
		"struct S { S() { } S(const T&) { } };\n"
		"struct T { T() { } T(const S&) { } };\n"
		"int main() { bool c = true; S s; T t; c ? s : t; return 1; }\n"
	},
	//5.16 [expr.cond] p5: overload resolution picks the conversions
	{
		"ConditionalAmbiguousConversionRefused", CONFORMANCE_REFUSED,
		"struct S { S(int x) : v(x) { } operator int() const { return v; } int v; };\n"
		"int main() { bool c = true; S s(1); return (c ? s : 2) == 1 ? 1 : 2; }\n"
	},
	//5.17 [expr.ass]: assignment
	{
		"AssignmentRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int a; int b; };\n"
		"struct Bits { unsigned x : 3; int y : 4; };\n"
		"enum E { E0, E1 };\n"
		"int main()\n"
		"{\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int c = 3;\n"
		"	unsigned char uc = 0;\n"
		"	bool flag = false;\n"
		"	double d = 0;\n"
		"	float f = 0;\n"
		"	long long ll = 0;\n"
		"	S s1 = { 1, 2 };\n"
		"	S s2 = { 3, 4 };\n"
		"	Bits bits;\n"
		"	E e = E0;\n"
		"	int* p = 0;\n"
		"	int r = 0;\n"
		"	a = b = c;\n"
		"	if (a == 3 && b == 3) r += 1;\n"
		"	(a = 5) = 6;\n"
		"	if (a == 6 && &(a = 7) == &a && a == 7 && Same<decltype(a = b), int&>::V && Same<decltype(uc = 1), unsigned char&>::V) r += 2;\n"
		"	int stored = (uc = 300);\n"
		"	int truncated = (a = 2.9);\n"
		"	int boolean = (flag = 5);\n"
		"	if (stored == 44 && uc == 44 && truncated == 2 && a == 2 && boolean == 1 && flag == true) r += 4;\n"
		"	d = a;\n"
		"	f = 0.5;\n"
		"	ll = -a;\n"
		"	if (d == 2.0 && f == 0.5f && ll == -2 && (d = f) == 0.5 && (ll = 0x100000000LL) > 0xFFFFFFFFul * (sizeof(long) == 4)) r += 8;\n"
		"	s1 = s2;\n"
		"	s2.a = 9;\n"
		"	if (s1.a == 3 && s1.b == 4 && (s1 = s2).a == 9 && &(s1 = s2) == &s1) r += 16;\n"
		"	int field = (bits.x = 9);\n"
		"	bits.y = -3;\n"
		"	if (bits.x == 1 && field == 1 && bits.y == -3 && (bits.y = 7) == 7) r += 32;\n"
		"	e = E1;\n"
		"	p = &a;\n"
		"	*p = 11;\n"
		"	p = 0;\n"
		"	if (e == E1 && a == 11 && p == 0 && (p = &b) == &b && (*p = 12) == 12 && b == 12) r += 64;\n"
		"	int x;\n"
		"	int y;\n"
		"	x = y = a + 1;\n"
		"	x = (y = 3) + 4;\n"
		"	if (x == 7 && y == 3 && (x = y, y = 8, x + y) == 11) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.17 [expr.ass] p7: E1 op= E2 is E1 = E1 op E2 with E1 evaluated once
	{
		"CompoundAssignment", 1023,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct Bits { unsigned x : 3; int y : 5; };\n"
		"int gCalls = 0;\n"
		"int gValue = 10;\n"
		"int* Next() { gCalls++; return &gValue; }\n"
		"int main()\n"
		"{\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int i = 0;\n"
		"	unsigned char uc = 200;\n"
		"	signed char sc = -128;\n"
		"	unsigned short us = 65535u;\n"
		"	unsigned int u = 0;\n"
		"	int n = 3;\n"
		"	long l = 100000L;\n"
		"	long long ll = 1;\n"
		"	double d = 1.5;\n"
		"	float f = 2.5f;\n"
		"	bool flag = false;\n"
		"	int* p = a;\n"
		"	Bits bits;\n"
		"	bits.x = 6; bits.y = 15;\n"
		"	int r = 0;\n"
		"	a[i++] += 10;\n"
		"	*Next() += 1;\n"
		"	*Next() *= 2;\n"
		"	if (a[0] == 11 && a[1] == 2 && i == 1 && gValue == 22 && gCalls == 2) r += 1;\n"
		"	uc += 100;\n"
		"	sc >>= 1;\n"
		"	us += 1;\n"
		"	u -= 1;\n"
		"	if (uc == 44 && sc == -64 && us == 0 && u == ~0u) r += 2;\n"
		"	uc = 0x81;\n"
		"	uc >>= 1;\n"
		"	uc <<= 2;\n"
		"	uc *= 3;\n"
		"	if (uc == 0) r += 4;\n"
		"	n *= 1.5;\n"
		"	int first = n;\n"
		"	n /= 0.5;\n"
		"	int second = n;\n"
		"	n += 0.7 + 0.7;\n"
		"	if (first == 4 && second == 8 && n == 9 && (n -= 0.5) == 8 && (n *= 0.5) == 4) r += 8;\n"
		"	l *= 3;\n"
		"	l /= 7;\n"
		"	l %= 1000;\n"
		"	ll <<= 33;\n"
		"	ll += u;\n"
		"	ll -= 1;\n"
		"	if (l == 857 && ll == 0x200000000LL + ~0u - 1) r += 16;\n"
		"	d += 1;\n"
		"	d *= 2;\n"
		"	d -= 0.5;\n"
		"	d /= 3;\n"
		"	f *= 2;\n"
		"	f -= 1;\n"
		"	f /= 4;\n"
		"	if (d == 1.5 && f == 1.0f && (d += f) == 2.5) r += 32;\n"
		"	n = 13;\n"
		"	n %= 4;\n"
		"	n |= 8;\n"
		"	n &= 0xD;\n"
		"	n ^= 0xF;\n"
		"	n <<= 2;\n"
		"	n >>= 1;\n"
		"	if (n == 12) r += 64;\n"
		"	p += 2;\n"
		"	p -= 1;\n"
		"	p += (long)1;\n"
		"	p -= (unsigned char)2;\n"
		"	flag |= true;\n"
		"	flag &= true;\n"
		"	flag ^= true;\n"
		"	if (p == a && flag == false && (flag |= 4) == true && (int)flag == 1) r += 128;\n"
		"	bits.x += 5;\n"
		"	bits.y -= 20;\n"
		"	bits.y *= 2;\n"
		"	if (bits.x == 3 && bits.y == -10 && (bits.x <<= 2) == 4) r += 256;\n"
		"	n = 1;\n"
		"	(n += 1) += 1;\n"
		"	(n *= 2) -= 1;\n"
		"	if (n == 5 && &(n += 1) == &n && Same<decltype(n += 1), int&>::V && Same<decltype(uc *= 2), unsigned char&>::V && Same<decltype(p += 1), int*&>::V) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	//5.17 [expr.ass] p9: a braced list on the right
	{
		"AssignmentBraced", 15,
		"struct S { int a; int b; };\n"
		"struct C { C() : v(0) { } C(int a, int b) : v(a * 10 + b) { } int v; };\n"
		"int main()\n"
		"{\n"
		"	int i = 9;\n"
		"	double d = 9;\n"
		"	int* p = &i;\n"
		"	char c = 'x';\n"
		"	S s = { 9, 9 };\n"
		"	C k;\n"
		"	int r = 0;\n"
		"	i = { 5 };\n"
		"	d = { 1.5 };\n"
		"	c = { 'a' };\n"
		"	if (i == 5 && d == 1.5 && c == 'a') r += 1;\n"
		"	i = { };\n"
		"	p = { };\n"
		"	d = { };\n"
		"	if (i == 0 && p == 0 && d == 0.0) r += 2;\n"
		"	s = { 1, 2 };\n"
		"	k = { 3, 4 };\n"
		"	if (s.a == 1 && s.b == 2 && k.v == 34) r += 4;\n"
		"	s = { };\n"
		"	k = { };\n"
		"	if (s.a == 0 && s.b == 0 && k.v == 0 && (i = { 7 }) == 7) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.17 [expr.ass] p1: the left operand is a modifiable lvalue
	{
		"AssignToLiteralRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; 5 = i; return i; }\n"
	},
	{
		"AssignToSumRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; i + 1 = 2; return i; }\n"
	},
	{
		"AssignToConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; i = 2; return i; }\n"
	},
	{
		"AssignArrayRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2 }; int b[2] = { 3, 4 }; a = b; return a[0]; }\n"
	},
	{
		"AssignFunctionRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int g() { return 2; }\n"
		"int main() { f = g; return f(); }\n"
	},
	//5.17 [expr.ass] p3: the right operand is implicitly converted to the left's type
	{
		"AssignEnumFromIntRefused", CONFORMANCE_REFUSED,
		"enum E { E0, E1 };\n"
		"int main() { E e = E0; e = 1; return e + 1; }\n"
	},
	//5.17 [expr.ass] p7: E1 op= E2 is E1 = E1 op E2
	{
		"AssignEnumCompoundRefused", CONFORMANCE_REFUSED,
		"enum E { E0, E1 };\n"
		"int main() { E e = E0; e += 1; return e + 1; }\n"
	},
	//5.17 [expr.ass] p9: a braced list on the right
	{
		"AssignNarrowingBracedRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; i = { 1.5 }; return i; }\n"
	},
	{
		"AssignTwoInBracesRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; i = { 1, 2 }; return i; }\n"
	},
	//5.17 [expr.ass] p7: += and -= take a pointer and an integer
	{
		"AssignPointerPlusFloatRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2 }; int* p = a; p += 1.0; return *p; }\n"
	},
	{
		"AssignPointerTimesRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2 }; int* p = a; p *= 2; return *p; }\n"
	},
	//5.17 [expr.ass] p7: E1 op= E2 is E1 = E1 op E2
	{
		"AssignFloatRemainderRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 5.0; d %= 2; return d ? 1 : 2; }\n"
	},
	//5.17 [expr.ass] p4: a class's assignment is its operator=
	{
		"AssignClassFromIntRefused", CONFORMANCE_REFUSED,
		"struct S { int a; int b; };\n"
		"int main() { S s = { 1, 2 }; s = 5; return s.a; }\n"
	},
	{
		"AssignConstMemberClassRefused", CONFORMANCE_REFUSED,
		"struct S { const int a; };\n"
		"int main() { S s = { 1 }; S t = { 2 }; s = t; return s.a; }\n"
	},
	//5.18 [expr.comma]: comma operator
	{
		"CommaRules", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gCount = 0;\n"
		"int gOrder = 0;\n"
		"struct S { S(int x) : v(x) { gCount += 10; } ~S() { gCount += 1; } int v; };\n"
		"int Step(int v) { gOrder = gOrder * 10 + v; return v; }\n"
		"void Nothing() { gCount += 100; }\n"
		"int Twice(int v) { return v * 2; }\n"
		"int main()\n"
		"{\n"
		"	int a = 1;\n"
		"	int b = 2;\n"
		"	int arr[3] = { 5, 6, 7 };\n"
		"	int i;\n"
		"	int j;\n"
		"	int r = 0;\n"
		"	if ((a = 5, a + 1) == 6 && (Step(1), Step(2), Step(3)) == 3 && gOrder == 123) r += 1;\n"
		"	(a, b) = 9;\n"
		"	if (b == 9 && a == 5 && &(a, b) == &b) r += 2;\n"
		"	if (Same<decltype((a, b)), int&>::V && Same<decltype((a, 1.5)), double>::V && Same<decltype((1.5, a)), int&>::V && Same<decltype((a, Nothing())), void>::V) r += 4;\n"
		"	if (Twice((1, 2)) == 4 && arr[(0, 2)] == 7 && sizeof((0, arr)) == 3 * sizeof(int)) r += 8;\n"
		"	int total = 0;\n"
		"	for (i = 0, j = 10; i < j; i++, j--) total += j - i;\n"
		"	if (total == 30 && i == 5 && j == 5) r += 16;\n"
		"	int k = (Nothing(), 3);\n"
		"	a, Nothing();\n"
		"	if (k == 3 && gCount == 200) r += 32;\n"
		"	int v = (S(1), S(2), S(3).v);\n"
		"	if (v == 3 && gCount == 233) r += 64;\n"
		"	int x = 0;\n"
		"	int y = (x++, x++, x);\n"
		"	if (y == 2 && (x = 5, x += 1, x * 2) == 12 && (true ? x++, x : 0) == 7) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p3: where an integral constant expression is required
	{
		"ConstantContexts", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"const int gThree = 3;\n"
		"constexpr int Square(int v) { return v * v; }\n"
		"enum E { E0 = gThree + 1, E1 = Square(3), E2 = sizeof(long) * 2 };\n"
		"struct Bits { unsigned a : gThree; unsigned b : Square(2); };\n"
		"template<int N> struct Size { enum { V = N }; char c[N]; };\n"
		"struct Class { static const int scMember = 6; static constexpr int scExpr = scMember * 2; enum { Local = scExpr + 1 }; };\n"
		"struct alignas(1 << 2) Aligned { char c; };\n"
		"int Switch(int v)\n"
		"{\n"
		"	switch (v)\n"
		"	{\n"
		"		case gThree: return 1;\n"
		"		case Square(3) + 1: return 2;\n"
		"		case E1: return 3;\n"
		"		case Class::scExpr: return 4;\n"
		"		case 'a' + 1: return 5;\n"
		"		case sizeof(char) + 100: return 6;\n"
		"	}\n"
		"	return 0;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	const int local = 5;\n"
		"	constexpr int expr = local * 2;\n"
		"	int a[gThree];\n"
		"	int b[Square(4)];\n"
		"	int c[local + expr];\n"
		"	int d[E1];\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 3 * sizeof(int) && sizeof(b) == 16 * sizeof(int) && sizeof(c) == 15 * sizeof(int) && sizeof(d) == 9 * sizeof(int)) r += 1;\n"
		"	if (E0 == 4 && E1 == 9 && E2 == 2 * sizeof(long) && Class::Local == 13) r += 2;\n"
		"	Bits bits;\n"
		"	bits.a = 15; bits.b = 31;\n"
		"	if (bits.a == 7 && bits.b == 15) r += 4;\n"
		"	if (Size<gThree>::V == 3 && Size<Square(5)>::V == 25 && sizeof(Size<local * 2>) == 10 && Size<Class::scMember + E1>::V == 15 && Size<sizeof(int[4]) / sizeof(int)>::V == 4) r += 8;\n"
		"	if (Switch(3) == 1 && Switch(10) == 2 && Switch(9) == 3 && Switch(12) == 4 && Switch('b') == 5 && Switch(101) == 6 && Switch(0) == 0) r += 16;\n"
		"	static_assert(gThree == 3 && Square(gThree) == 9 && local == 5 && expr == 10, \"constants\");\n"
		"	static_assert(sizeof(Aligned) == 4 && alignof(Aligned) == 4, \"alignment\");\n"
		"	if (alignof(Aligned) == 4 && noexcept(Square(2)) == false) r += 32;\n"
		"	if (Same<Size<2 + 1>, Size<gThree> >::V && Same<Size<Square(2)>, Size<E0> >::V && !Same<Size<1>, Size<2> >::V) r += 64;\n"
		"	char e[local > 3 ? 2 : 1][expr / 5];\n"
		"	if (sizeof(e) == 4 && sizeof(e[0]) == 2) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2: a const integer initialised with a constant is a constant
	{
		"ConstantConstVariables", 15,
		"enum E { E5 = 5 };\n"
		"const int gA = 4;\n"
		"const unsigned char gB = 200;\n"
		"const long gC = gA * 100000L;\n"
		"const E gE = E5;\n"
		"const bool gFlag = true;\n"
		"const char gChar = 'x';\n"
		"static const int gsStatic = gA + 1;\n"
		"struct S { static const int scIn = 7; static const int scOut; };\n"
		"const int S::scOut = 8;\n"
		"int main()\n"
		"{\n"
		"	const int local = gA + gB;\n"
		"	int a[gA];\n"
		"	int b[gB];\n"
		"	int c[local];\n"
		"	int d[gE + gFlag + (gChar == 'x')];\n"
		"	int e[gsStatic + S::scIn + S::scOut];\n"
		"	char f[gC / 100000L];\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 4 * sizeof(int) && sizeof(b) == 200 * sizeof(int) && sizeof(c) == 204 * sizeof(int)) r += 1;\n"
		"	if (sizeof(d) == 7 * sizeof(int) && sizeof(e) == 20 * sizeof(int) && sizeof(f) == 4) r += 2;\n"
		"	constexpr long product = gC * 2;\n"
		"	constexpr int fromEnum = gE * 3;\n"
		"	if (product == 800000L && fromEnum == 15) r += 4;\n"
		"	static_assert(gB == 200 && gC == 400000L && S::scOut == 8, \"constants\");\n"
		"	const int& reference = gA;\n"
		"	if (&reference == &gA && *&gA == 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConstantRunTimeConstRefused", CONFORMANCE_REFUSED,
		"int Get() { return 3; }\n"
		"int main() { const int n = Get(); constexpr int m = n; return m; }\n"
	},
	//5.19 [expr.const] p2: lvalue to rvalue conversion
	{
		"ConstantNonConstVariableRefused", CONFORMANCE_REFUSED,
		"int main() { int n = 3; constexpr int m = n; return m; }\n"
	},
	//5.19 [expr.const] p2: only a const of integral or enumeration type is read
	{
		"ConstantConstDoubleRefused", CONFORMANCE_REFUSED,
		"int main() { const double d = 1.5; constexpr double e = d; return e > 1 ? 1 : 2; }\n"
	},
	//5.19 [expr.const] p2: a volatile object is not read
	{
		"ConstantVolatileRefused", CONFORMANCE_REFUSED,
		"int main() { const volatile int v = 1; constexpr int m = v; return m; }\n"
	},
	//5.19 [expr.const] p3: where an integral constant expression is required
	{
		"ConstantArrayBoundNotConstantRefused", CONFORMANCE_REFUSED,
		"int main() { int n = 3; int a[n]; return sizeof(a) ? 1 : 2; }\n"
	},
	{
		"ConstantCaseNotConstantRefused", CONFORMANCE_REFUSED,
		"int main() { int n = 3; int v = 3; switch (v) { case n: return 1; } return 2; }\n"
	},
	{
		"ConstantEnumeratorNotConstantRefused", CONFORMANCE_REFUSED,
		"int gN = 3;\n"
		"enum E { E0 = gN };\n"
		"int main() { return E0; }\n"
	},
	{
		"ConstantTemplateArgumentNotConstantRefused", CONFORMANCE_REFUSED,
		"template<int N> struct Size { enum { V = N }; };\n"
		"int main() { int n = 3; return Size<n>::V; }\n"
	},
	{
		"ConstantBitFieldWidthNotConstantRefused", CONFORMANCE_REFUSED,
		"int gN = 3;\n"
		"struct S { int b : gN; };\n"
		"int main() { S s; s.b = 1; return s.b; }\n"
	},
	//5.19 [expr.const] p3: a converted constant expression does not narrow
	{
		"ConstantConvertedNarrowingRefused", CONFORMANCE_REFUSED,
		"template<unsigned char C> struct Byte { enum { V = C }; };\n"
		"int main() { return Byte<300>::V; }\n"
	},
	{
		"ConstantConvertedNegativeRefused", CONFORMANCE_REFUSED,
		"template<unsigned int N> struct Count { enum { V = 1 }; };\n"
		"int main() { return Count<-1>::V; }\n"
	},
	//5.19 [expr.const] p3: a converted constant expression is not converted from floating point
	{
		"ConstantConvertedFloatRefused", CONFORMANCE_REFUSED,
		"template<int N> struct Size { enum { V = N }; };\n"
		"int main() { return Size<1.0>::V; }\n"
	},
	//5.19 [expr.const] p3: a converted constant expression does not narrow
	{
		"ConstantEnumeratorNarrowingRefused", CONFORMANCE_REFUSED,
		"enum E : unsigned char { E0 = 256 };\n"
		"int main() { return E0 + 1; }\n"
	},
	{
		"ConstantCaseNarrowingRefused", CONFORMANCE_REFUSED,
		"int main() { unsigned int v = 1; switch (v) { case -1: return 1; } return 2; }\n"
	},
	//5.19 [expr.const] p3: a converted constant expression is not converted from floating point
	{
		"ConstantArrayBoundFloatRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2.0]; return sizeof(a) ? 1 : 2; }\n"
	},
	//5.19 [expr.const] p3: a class with a constexpr conversion function
	{
		"ConstantClassConversion", 15,
		"struct Three { constexpr operator int() const { return 3; } };\n"
		"struct Count { constexpr Count(int x) : v(x) { } constexpr operator unsigned char() const { return v; } unsigned char v; };\n"
		"template<int N> struct Size { enum { V = N }; };\n"
		"enum E { E0 = Three(), E1 = Count(7) };\n"
		"int main()\n"
		"{\n"
		"	constexpr Three three = Three();\n"
		"	constexpr Count five(5);\n"
		"	int a[three];\n"
		"	int b[Count(4)];\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 3 * sizeof(int) && sizeof(b) == 4 * sizeof(int)) r += 1;\n"
		"	if (E0 == 3 && E1 == 7 && Size<three>::V == 3 && Size<five>::V == 5) r += 2;\n"
		"	switch (5) { case five: r += 4; break; case three: r += 100; break; }\n"
		"	static_assert(three == 3 && five + 1 == 6, \"conversions\");\n"
		"	if (three + five == 8 && sizeof(char[three * five]) == 15) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2: only what is evaluated need be constant
	{
		"ConstantOnlyEvaluatedMatters", 31,
		"int NotConstant() { return 7; }\n"
		"int gVariable = 1;\n"
		"constexpr int Pick(int v) { return v > 0 ? v * 2 : NotConstant(); }\n"
		"constexpr int Guarded(int v) { if (v < 0) { return gVariable; } return v + 1; }\n"
		"constexpr int Divide(int a, int b) { return b == 0 ? 0 : a / b; }\n"
		"int main()\n"
		"{\n"
		"	constexpr int a = true ? 1 : NotConstant();\n"
		"	constexpr int b = false && NotConstant();\n"
		"	constexpr int c = true || NotConstant();\n"
		"	constexpr int d = Pick(3);\n"
		"	constexpr int e = Guarded(4);\n"
		"	constexpr int f = Divide(7, 0) + Divide(7, 2);\n"
		"	constexpr int g = sizeof(NotConstant()) + sizeof(gVariable);\n"
		"	int r = 0;\n"
		"	if (a == 1 && b == 0 && c == 1) r += 1;\n"
		"	if (d == 6 && e == 5 && f == 3) r += 2;\n"
		"	if (g == 2 * sizeof(int)) r += 4;\n"
		"	if (Pick(0) == 7 && Guarded(-1) == 1) r += 8;\n"
		"	gVariable = 9;\n"
		"	if (Guarded(-1) == 9 && Pick(-5) == 7 && Divide(9, 3) == 3) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"ConstantUnevaluatedBranchTakenRefused", CONFORMANCE_REFUSED,
		"int NotConstant() { return 7; }\n"
		"constexpr int Pick(int v) { return v > 0 ? v * 2 : NotConstant(); }\n"
		"int main() { constexpr int a = Pick(0); return a; }\n"
	},
	//5.19 [expr.const] p2: a constexpr function not yet defined
	{
		"ConstantUndefinedFunctionRefused", CONFORMANCE_REFUSED,
		"constexpr int Later(int v);\n"
		"constexpr int gValue = Later(1);\n"
		"constexpr int Later(int v) { return v + 1; }\n"
		"int main() { return gValue; }\n"
	},
	{
		"ConstantDefinedBeforeUse", 3,
		"constexpr int Later(int v);\n"
		"constexpr int Caller(int v) { return Later(v) * 2; }\n"
		"constexpr int Later(int v) { return v + 1; }\n"
		"constexpr int gValue = Caller(2);\n"
		"int main()\n"
		"{\n"
		"	int a[gValue];\n"
		"	int r = 0;\n"
		"	if (gValue == 6) r += 1;\n"
		"	if (sizeof(a) == 6 * sizeof(int)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2: a lambda expression is not constant
	{
		"ConstantLambdaRefused", CONFORMANCE_REFUSED,
		"int main() { constexpr int a = [] { return 1; }(); return a; }\n"
	},
	//5.19 [expr.const] p2: reinterpret_cast is not constant
	{
		"ConstantReinterpretCastRefused", CONFORMANCE_REFUSED,
		"int gi = 1;\n"
		"int main() { constexpr unsigned long long a = reinterpret_cast<unsigned long long>(&gi); return a ? 1 : 2; }\n"
	},
	//5.19 [expr.const] p2: only objects created in the evaluation are changed
	{
		"ConstantModifiesGlobalRefused", CONFORMANCE_REFUSED,
		"int gi = 1;\n"
		"constexpr int Bump() { return ++gi; }\n"
		"int main() { constexpr int a = Bump(); return a; }\n"
	},
	{
		"ConstantAssignsGlobalRefused", CONFORMANCE_REFUSED,
		"int gi = 1;\n"
		"constexpr int Set(int v) { gi = v; return v; }\n"
		"int main() { constexpr int a = Set(2); return a; }\n"
	},
	//5.19 [expr.const] p2: a new expression is not constant
	{
		"ConstantNewRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"constexpr int Make() { return *new int(5); }\n"
		"int main() { constexpr int a = Make(); return a; }\n"
	},
	//5.19 [expr.const] p2: undefined behaviour is not constant
	{
		"ConstantNullDereferenceRefused", CONFORMANCE_REFUSED,
		"constexpr int Read(const int* p) { return *p; }\n"
		"int main() { constexpr int a = Read(nullptr); return a; }\n"
	},
	{
		"ConstantShiftTooFarRefused", CONFORMANCE_REFUSED,
		"int main() { constexpr int a = 1 << (sizeof(int) * 8); return a + 1; }\n"
	},
	{
		"ConstantNegativeShiftRefused", CONFORMANCE_REFUSED,
		"constexpr int Shift(int v, int n) { return v << n; }\n"
		"int main() { constexpr int a = Shift(1, -1); return a + 1; }\n"
	},
	//5.19 [expr.const] p2: a union member that is not active is not read
	{
		"ConstantInactiveUnionMemberRefused", CONFORMANCE_REFUSED,
		"union U { int i; long l; constexpr U(int v) : i(v) { } };\n"
		"int main() { constexpr U u(1); constexpr long a = u.l; return a ? 1 : 2; }\n"
	},
	//5.19 [expr.const] p2: a comparison whose result is unspecified is not constant
	{
		"ConstantUnrelatedPointerCompareRefused", CONFORMANCE_REFUSED,
		"int ga;\n"
		"int gb;\n"
		"int main() { constexpr bool less = &ga < &gb; return less ? 1 : 2; }\n"
	},
	//5.19 [expr.const] p2: this, outside a constexpr member function being evaluated
	{
		"ConstantThisOutsideRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int f() { constexpr S* p = this; return p->m; } };\n"
		"int main() { S s = { 1 }; return s.f(); }\n"
	},
	//5.19 [expr.const] p5: address and reference constant expressions
	{
		"ConstantAddresses", 127,
		"struct S { int m; int a[3]; int f() const { return m; } };\n"
		"int gi = 5;\n"
		"int ga[4] = { 1, 2, 3, 4 };\n"
		"S gs = { 6, { 7, 8, 9 } };\n"
		"int Function(int v) { return v + 1; }\n"
		"constexpr int* gp = &gi;\n"
		"constexpr int* gq = &ga[2];\n"
		"constexpr int* gm = &gs.a[1];\n"
		"constexpr const char* gsz = \"abc\";\n"
		"constexpr int (*gpf)(int) = Function;\n"
		"constexpr int S::* gpm = &S::m;\n"
		"constexpr int (S::*gpmf)() const = &S::f;\n"
		"constexpr int& gr = gi;\n"
		"constexpr S& grs = gs;\n"
		"constexpr int* gnull = nullptr;\n"
		"constexpr decltype(nullptr) gnp = nullptr;\n"
		"int main()\n"
		"{\n"
		"	static int sLocal = 3;\n"
		"	constexpr int* ps = &sLocal;\n"
		"	int r = 0;\n"
		"	if (*gp == 5 && gp == &gi && *gq == 3 && *gm == 8 && gq - ga == 2) r += 1;\n"
		"	if (gsz[1] == 'b' && gpf(1) == 2 && gs.*gpm == 6 && (gs.*gpmf)() == 6) r += 2;\n"
		"	gr = 10;\n"
		"	grs.m = 11;\n"
		"	if (gi == 10 && gs.m == 11 && &gr == &gi && &grs == &gs) r += 4;\n"
		"	if (gnull == 0 && gnp == nullptr && *ps == 3 && ps == &sLocal) r += 8;\n"
		"	constexpr long difference = &ga[3] - &ga[1];\n"
		"	constexpr bool before = &ga[1] < &ga[2];\n"
		"	constexpr bool same = &ga[1] == ga + 1;\n"
		"	constexpr bool notNull = &gi != nullptr;\n"
		"	if (difference == 2 && before && same && notNull) r += 16;\n"
		"	static_assert(gp != nullptr && gq == ga + 2 && gpf == Function && gnull == nullptr, \"addresses\");\n"
		"	constexpr char second = \"abc\"[1];\n"
		"	constexpr char last = gsz[3];\n"
		"	if (second == 'b' && last == 0 && sizeof(char[gsz[0] - 'a' + 2]) == 2) r += 32;\n"
		"	constexpr const int* const* gpp = &gp;\n"
		"	if (**gpp == 10 && *gpp == &gi) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p5: an address constant is of an object with static storage
	{
		"ConstantAddressOfLocalRefused", CONFORMANCE_REFUSED,
		"int main() { int local = 1; constexpr int* p = &local; return *p; }\n"
	},
	//5.19 [expr.const] p5: a reference constant refers to an object with static storage
	{
		"ConstantReferenceToLocalRefused", CONFORMANCE_REFUSED,
		"int main() { int local = 1; constexpr int& r = local; return r; }\n"
	},
	//5.19 [expr.const] p2: literal objects and their subobjects are read
	{
		"ConstantObjects", 255,
		"struct P { int a; int b; };\n"
		"struct Line { P from; P to; constexpr int Length() const { return (to.a - from.a) + (to.b - from.b); } };\n"
		"struct C\n"
		"{\n"
		"	constexpr C(int x) : v(x), twice(x * 2) { }\n"
		"	constexpr int Sum() const { return v + twice; }\n"
		"	int v;\n"
		"	int twice;\n"
		"};\n"
		"constexpr P gp = { 1, 2 };\n"
		"constexpr Line gl = { { 1, 2 }, { 4, 6 } };\n"
		"constexpr int gt[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };\n"
		"constexpr C gc(5);\n"
		"constexpr P gap[2] = { { 7, 8 }, { 9, 10 } };\n"
		"constexpr const P* gpp = &gap[1];\n"
		"constexpr double gd = 2.5;\n"
		"constexpr float gf = 0.5f;\n"
		"template<int N> struct Size { enum { V = N }; };\n"
		"int main()\n"
		"{\n"
		"	constexpr P local = { 3, 4 };\n"
		"	constexpr C made = C(4);\n"
		"	constexpr P copy = gp;\n"
		"	constexpr Line moved = { gl.to, gl.from };\n"
		"	int a[gp.b + local.a];\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 5 * sizeof(int) && Size<gl.to.b>::V == 6 && Size<gt[1][2]>::V == 6) r += 1;\n"
		"	if (Size<gc.twice>::V == 10 && Size<gc.Sum()>::V == 15 && Size<made.Sum()>::V == 12 && Size<C(2).Sum()>::V == 6) r += 2;\n"
		"	if (Size<gl.Length()>::V == 7 && Size<gap[1].a + gap[0].b>::V == 17 && Size<gpp->b>::V == 10 && Size<(*gpp).a>::V == 9) r += 4;\n"
		"	if (copy.a == 1 && copy.b == 2 && Size<copy.b>::V == 2 && moved.from.a == 4 && Size<moved.to.b>::V == 2) r += 8;\n"
		"	constexpr int fromDouble = (int)(gd * 2);\n"
		"	constexpr double sum = gd + gf;\n"
		"	constexpr int rounded = (int)(sum * sum * 100);\n"
		"	if (fromDouble == 5 && sum == 3.0 && Size<rounded>::V == 900 && Size<(int)(gd * gd * 4)>::V == 25) r += 16;\n"
		"	constexpr int total = gt[0][0] + gt[0][1] + gt[0][2] + gt[1][0] + gt[1][1] + gt[1][2];\n"
		"	constexpr unsigned long long size = sizeof(gt) / sizeof(gt[0]) + sizeof(gt[0]) / sizeof(int);\n"
		"	if (total == 21 && size == 5 && Size<sizeof(gap) / sizeof(P)>::V == 2) r += 32;\n"
		"	static_assert(gp.a == 1 && gl.to.a == 4 && gt[1][0] == 4 && gc.v == 5 && gap[0].a == 7, \"objects\");\n"
		"	const P* runtime = &gap[0];\n"
		"	if (runtime[1].b == 10 && gc.Sum() == 15 && gl.Length() == 7 && &gap[1] == gpp) r += 64;\n"
		"	constexpr bool compare = gp.a < gp.b && gd > 2 && gf == 0.5f && gpp == &gap[1];\n"
		"	if (compare) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2: objects created in the evaluation may be changed
	{
		"ConstantEvaluationChangesLocals", 255,
		"struct P { int a; int b; constexpr void Swap() { int t = a; a = b; b = t; } constexpr P& Add(int v) { a += v; b += v; return *this; } };\n"
		"constexpr int SumTo(int n) { int total = 0; for (int i = 1; i <= n; i++) total += i; return total; }\n"
		"constexpr void Increment(int& r) { r++; }\n"
		"constexpr void SetThrough(int* p, int v) { *p = v; }\n"
		"constexpr int ThroughReference(int v) { Increment(v); Increment(v); int& r = v; r *= 2; return v; }\n"
		"constexpr int ThroughPointer(int v) { int local = 0; int* p = &local; SetThrough(p, v + 1); *p += 1; return local; }\n"
		"constexpr int LocalArray(int n) { int a[5] = { 0, 0, 0, 0, 0 }; for (int i = 0; i < 5; i++) a[i] = i * n; int total = 0; for (int i = 4; i >= 0; i--) total += a[i]; return total; }\n"
		"constexpr int LocalObject(int v) { P p = { v, v + 1 }; p.Swap(); p.Add(10).Add(1); return p.a * 100 + p.b; }\n"
		"constexpr int Fibonacci(int n) { int a = 0; int b = 1; while (n-- > 0) { int t = a + b; a = b; b = t; } return a; }\n"
		"constexpr int Collatz(int n) { int steps = 0; while (n != 1) { if (n % 2 == 0) n /= 2; else n = n * 3 + 1; steps++; } return steps; }\n"
		"constexpr int Switch(int v) { int r = 0; switch (v) { case 1: r += 1; case 2: r += 2; break; case 3: r = 30; break; default: r = -1; } return r; }\n"
		"constexpr int Compound(int v) { int r = v; r += 3; r *= 2; r -= 1; r /= 3; r %= 5; r <<= 2; r >>= 1; r |= 1; r &= 7; r ^= 2; return r; }\n"
		"constexpr P MakeSwapped(int a, int b) { P p = { a, b }; p.Swap(); return p; }\n"
		"template<int N> struct Size { enum { V = N }; };\n"
		"int main()\n"
		"{\n"
		"	constexpr P swapped = MakeSwapped(1, 2);\n"
		"	int r = 0;\n"
		"	if (Size<SumTo(10)>::V == 55 && Size<SumTo(100)>::V == 5050) r += 1;\n"
		"	if (Size<ThroughReference(3)>::V == 10 && Size<ThroughPointer(5)>::V == 7) r += 2;\n"
		"	if (Size<LocalArray(3)>::V == 30 && Size<LocalObject(1)>::V == 1312) r += 4;\n"
		"	if (Size<Fibonacci(20)>::V == 6765 && Size<Collatz(27)>::V == 111) r += 8;\n"
		"	if (Size<Switch(1)>::V == 3 && Size<Switch(2)>::V == 2 && Size<Switch(3)>::V == 30 && Switch(9) == -1) r += 16;\n"
		"	if (Size<Compound(4)>::V == Compound(4) && Compound(4) == 3) r += 32;\n"
		"	if (swapped.a == 2 && swapped.b == 1 && Size<swapped.a>::V == 2) r += 64;\n"
		"	int n = 10;\n"
		"	if (SumTo(n) == 55 && Fibonacci(n) == 55 && LocalArray(n) == 100 && ThroughReference(n) == 24 && LocalObject(n) == 2221) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p4: a constant has the value the expression has when the program runs
	{
		"ConstantAgreesWithRunTime", 1023,
		"constexpr int Div(int a, int b) { return a / b; }\n"
		"constexpr int Mod(int a, int b) { return a % b; }\n"
		"constexpr long ShiftRight(long v, int n) { return v >> n; }\n"
		"constexpr unsigned char ToByte(int v) { return (unsigned char)v; }\n"
		"constexpr signed char ToSignedByte(int v) { return (signed char)v; }\n"
		"constexpr long Widen(short v) { return v; }\n"
		"constexpr unsigned long WidenUnsigned(unsigned short v) { return v; }\n"
		"constexpr unsigned int Negate(unsigned int v) { return -v; }\n"
		"constexpr long long Multiply(long long a, long long b) { return a * b; }\n"
		"constexpr unsigned long long DivideUnsigned(unsigned long long a, unsigned long long b) { return a / b; }\n"
		"constexpr int Truncate(double d) { return (int)d; }\n"
		"constexpr double ToDouble(long long v) { return (double)v; }\n"
		"constexpr bool ToBool(long v) { return v; }\n"
		"constexpr int Compare(int a, unsigned int b) { return a < b; }\n"
		"constexpr int Promote(unsigned char a, unsigned char b) { return a + b; }\n"
		"constexpr int Complement(unsigned char v) { return ~v; }\n"
		"constexpr double Scale(double d, int n) { return d * n / 4; }\n"
		"int main()\n"
		"{\n"
		"	volatile int seven = 7;\n"
		"	volatile int two = 2;\n"
		"	volatile int big = 300;\n"
		"	volatile int negative = -1;\n"
		"	volatile long long million = 1000000;\n"
		"	volatile double fraction = -3.99;\n"
		"	int r = 0;\n"
		"	constexpr int d1 = Div(-7, 2);\n"
		"	constexpr int m1 = Mod(-7, 2);\n"
		"	constexpr int m2 = Mod(7, -2);\n"
		"	if (d1 == -3 && m1 == -1 && m2 == 1 && Div(-seven, two) == d1 && Mod(-seven, two) == m1 && Mod(seven, -two) == m2) r += 1;\n"
		"	constexpr long s1 = ShiftRight(-8L, 1);\n"
		"	constexpr long s2 = ShiftRight(-1L, 20);\n"
		"	if (s1 == -4 && s2 == -1 && ShiftRight(-8L * (two / 2), two - 1) == s1 && ShiftRight(negative, seven * 2 + 6) == s2) r += 2;\n"
		"	constexpr unsigned char b1 = ToByte(300);\n"
		"	constexpr signed char b2 = ToSignedByte(200);\n"
		"	if (b1 == 44 && b2 == -56 && ToByte(big) == b1 && ToSignedByte(big - 100) == b2) r += 4;\n"
		"	constexpr long w1 = Widen(-2);\n"
		"	constexpr unsigned long w2 = WidenUnsigned(65535u);\n"
		"	constexpr unsigned int n1 = Negate(1);\n"
		"	if (w1 == -2L && w2 == 65535ul && n1 == ~0u && Widen((short)(negative * two)) == w1 && WidenUnsigned((unsigned short)negative) == w2 && Negate(-negative) == n1) r += 8;\n"
		"	constexpr long long p1 = Multiply(1000000, 1000000);\n"
		"	constexpr unsigned long long q1 = DivideUnsigned(0xFFFFFFFFFFFFFFFFull, 10);\n"
		"	if (p1 == 1000000000000LL && q1 == 1844674407370955161ull && Multiply(million, million) == p1 && DivideUnsigned((unsigned long long)(long long)negative, seven + 3) == q1) r += 16;\n"
		"	constexpr int t1 = Truncate(-3.99);\n"
		"	constexpr int t2 = Truncate(3.99);\n"
		"	constexpr double f1 = ToDouble(1000000000000LL);\n"
		"	if (t1 == -3 && t2 == 3 && f1 == 1e12 && Truncate(fraction) == t1 && Truncate(-fraction) == t2 && ToDouble(million * million) == f1) r += 32;\n"
		"	constexpr bool c1 = ToBool(0x10000L);\n"
		"	constexpr int c2 = Compare(-1, 1);\n"
		"	if (c1 && c2 == 0 && ToBool(million * 0 + 0x10000L) == c1 && Compare(negative, 1u) == c2) r += 64;\n"
		"	constexpr int a1 = Promote(200, 100);\n"
		"	constexpr int a2 = Complement(0xF0);\n"
		"	if (a1 == 300 && a2 == -241 && Promote((unsigned char)(big - 100), 100) == a1 && Complement((unsigned char)(big - 60)) == a2) r += 128;\n"
		"	constexpr double x1 = Scale(2.5, 3);\n"
		"	constexpr double x2 = 1.0 / 3;\n"
		"	constexpr int x3 = (int)(x2 * 3000);\n"
		"	volatile double one = 1.0;\n"
		"	if (x1 == 1.875 && Scale(2.5, seven - 4) == x1 && x2 == one / 3 && (x3 == 1000 || x3 == 999) && (int)(one / 3 * 3000) == x3) r += 256;\n"
		"	constexpr long long shifted = 1LL << 40;\n"
		"	constexpr unsigned long long mask = ~0ull >> 4;\n"
		"	constexpr int precedence = 1 + 2 * 3 << 1 & 0xFE | 1;\n"
		"	if (shifted == 0x10000000000LL && mask == 0x0FFFFFFFFFFFFFFFull && precedence == 15 && (1LL << (seven * 5 + 5)) == shifted && (1 + two * 3 << 1 & 0xFE | 1) == precedence) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call] p1: a virtual function's final overrider in the object's dynamic type is called
	{
		"CallVirtualThroughReference", 127,
		"struct B { virtual int f() { return 1; } virtual int g(int x) const { return x + 1; } virtual ~B() { } };\n"
		"struct D : B { int f() override { return 2; } int g(int x) const override { return x + 2; } };\n"
		"struct E : D { int f() override { return 3; } };\n"
		"struct Holder { B& b; int Call() { return b.f(); } };\n"
		"int ByReference(B& b) { return b.f(); }\n"
		"int ByConstReference(const B& b) { return b.g(10); }\n"
		"int ByRvalueReference(B&& b) { return b.f(); }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	E e;\n"
		"	B b;\n"
		"	B& rd = d;\n"
		"	B& re = e;\n"
		"	D& rde = e;\n"
		"	const B& crd = d;\n"
		"	B* pd = &d;\n"
		"	B& fromPointer = *pd;\n"
		"	Holder holder = { e };\n"
		"	int r = 0;\n"
		"	if (rd.f() == 2 && re.f() == 3 && rde.f() == 3) r += 1;\n"
		"	if (crd.g(10) == 12 && fromPointer.f() == 2) r += 2;\n"
		"	if (ByReference(d) == 2 && ByReference(e) == 3 && ByReference(b) == 1) r += 4;\n"
		"	if (ByConstReference(d) == 12 && ByConstReference(b) == 11) r += 8;\n"
		"	if (ByRvalueReference(D()) == 2 && ByRvalueReference(static_cast<B&&>(e)) == 3) r += 16;\n"
		"	if (holder.Call() == 3 && holder.b.f() == 3) r += 32;\n"
		"	if (b.f() == 1 && d.f() == 2 && e.f() == 3 && (*pd).f() == 2 && pd->f() == 2 && rd.B::f() == 1) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"CallVirtualOperatorThroughReference", 7,
		"struct B { virtual int operator+(int x) const { return x + 1; } virtual int operator-() const { return 1; } virtual int operator[](int x) const { return x; } virtual ~B() { } };\n"
		"struct D : B { int operator+(int x) const override { return x + 2; } int operator-() const override { return 2; } int operator[](int x) const override { return x * 2; } };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	const B& rb = d;\n"
		"	int r = 0;\n"
		"	if (rb + 10 == 12) r += 1;\n"
		"	if (-rb == 2) r += 2;\n"
		"	if (rb[5] == 10) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.3 [expr.type.conv] p1: T(x) is the cast (T)x
	{
		"FunctionalCastFloating", 31,
		"int main()\n"
		"{\n"
		"	int i = 3;\n"
		"	long l = 7;\n"
		"	int r = 0;\n"
		"	if (double(3) / 2 == 1.5 && double(i) / 2 == 1.5) r += 1;\n"
		"	if (float(3) / 2 == 1.5f && float(i) / 2 == 1.5f) r += 2;\n"
		"	if (double(3.5) == 3.5 && float(0.25f) == 0.25f && double(0.5f) == 0.5) r += 4;\n"
		"	if (double(l) / 2 == 3.5 && (int)(double(i) * 2.5) == 7) r += 8;\n"
		"	double d = double(i) + double(1) / 4;\n"
		"	float f = float(l) / float(2);\n"
		"	if (d == 3.25 && f == 3.5f) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p6: operands of one type give that type
	{
		"ConditionalKeepsSmallTypes", 15,
		"int f(char) { return 1; }\n"
		"int f(int) { return 2; }\n"
		"int f(bool) { return 3; }\n"
		"int f(short) { return 4; }\n"
		"int f(unsigned char) { return 5; }\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	char a = 'a';\n"
		"	char b = 'b';\n"
		"	short s = 1;\n"
		"	unsigned char uc = 2;\n"
		"	int r = 0;\n"
		"	if (f(c ? a : b) == 1 && f(c ? 'x' : 'y') == 1) r += 1;\n"
		"	if (f(c ? true : false) == 3 && f(c ? c : c) == 3) r += 2;\n"
		"	if (f(c ? s : s) == 4 && f(c ? uc : uc) == 5) r += 4;\n"
		"	if (f(c ? a : s) == 2 && f(c ? a : 1) == 2 && sizeof(c ? a : b) == 1 && sizeof(c ? s : s) == sizeof(short)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//4.11 [conv.mem] p1, 5.10 [expr.eq] p3: a null member pointer is distinct from every pointer to a member
	{
		"MemberPointerNull", 127,
		"struct B { int b; int n; };\n"
		"struct A { int a; };\n"
		"struct D : B, A { int d; };\n"
		"int main()\n"
		"{\n"
		"	int B::* first = &B::b;\n"
		"	int B::* second = &B::n;\n"
		"	int B::* null = 0;\n"
		"	int A::* nullA = nullptr;\n"
		"	int D::* nullD = nullA;\n"
		"	int D::* firstD = first;\n"
		"	int D::* aD = &A::a;\n"
		"	int r = 0;\n"
		"	if (first && first != 0 && first != nullptr && first != null) r += 1;\n"
		"	if (second && second != null && first != second) r += 2;\n"
		"	if (!null && null == 0 && null == nullptr && !nullA) r += 4;\n"
		"	if (nullD == 0 && !nullD) r += 8;\n"
		"	if (firstD != 0 && firstD == first && aD != 0 && aD != firstD) r += 16;\n"
		"	bool b1 = first;\n"
		"	bool b2 = null;\n"
		"	if (b1 && !b2 && (first ? 1 : 2) == 1 && (null ? 1 : 2) == 2) r += 32;\n"
		"	null = first;\n"
		"	first = 0;\n"
		"	if (null && !first && null != first) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p15, 3.3.7 [basic.scope.class]: a parameter hides the member of its name (found by the clause 5 cases)
	{
		"ParameterNamedAsMember", 31,
		"struct S { S(int v) : v(v) { } int v; };\n"
		"struct T { T(int a, int b) : b(a), a(b) { } int a; int b; };\n"
		"struct U { U(int v) : twice(v * 2), v(v + 1) { } int v; int twice; };\n"
		"struct W { W(int v) { this->v = v; } int Get(int v) { return v + this->v; } int v; };\n"
		"struct X { constexpr X(int v) : v(v) { } constexpr int Get() const { return v; } int v; };\n"
		"constexpr X gx(6);\n"
		"int main()\n"
		"{\n"
		"	S s(7);\n"
		"	T t(7, 5);\n"
		"	U u(4);\n"
		"	W w(3);\n"
		"	X x(9);\n"
		"	int r = 0;\n"
		"	if (s.v == 7) r += 1;\n"
		"	if (t.a == 5 && t.b == 7) r += 2;\n"
		"	if (u.v == 5 && u.twice == 8) r += 4;\n"
		"	if (w.v == 3 && w.Get(10) == 13) r += 8;\n"
		"	if (x.Get() == 9 && gx.Get() == 6 && gx.v == 6) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p17, 8.5.2 [dcl.init.string]: a new array of characters from a string in braces
	{
		"NewCharArrayFromString", 7,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"int main()\n"
		"{\n"
		"	char* sz = new char[4]{ \"abc\" };\n"
		"	char* longer = new char[6]{ \"ab\" };\n"
		"	wchar_t* wide = new wchar_t[3]{ L\"xy\" };\n"
		"	int r = 0;\n"
		"	if (sz[0] == 'a' && sz[2] == 'c' && sz[3] == 0) r += 1;\n"
		"	if (longer[1] == 'b' && longer[2] == 0 && longer[5] == 0) r += 2;\n"
		"	if (wide[0] == L'x' && wide[1] == L'y' && wide[2] == 0) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p10: a class's operator delete that takes a size is given the class's
	{
		"DeleteClassSizedExact", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"size_t gSize = 0;\n"
		"int gCalls = 0;\n"
		"struct P { char c[5]; static void operator delete(void* p, size_t n) { gSize = n; gCalls += 1; } };\n"
		"struct Q : P { long l; };\n"
		"struct Both { static void operator delete(void* p) { gCalls += 100; } static void operator delete(void* p, size_t n) { gCalls += 1000; } };\n"
		"struct Hides : P { static void operator delete(void* p) { gCalls += 10; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	P* p = new P;\n"
		"	delete p;\n"
		"	if (gSize == 5 && gCalls == 1 && giDeletes == 0) r += 1;\n"
		"	Q* q = new Q;\n"
		"	delete q;\n"
		"	if (gSize == sizeof(Q) && gCalls == 2) r += 2;\n"
		"	Both* both = new Both;\n"
		"	delete both;\n"
		"	if (gCalls == 102) r += 4;\n"
		"	Hides* hides = new Hides;\n"
		"	delete hides;\n"
		"	if (gCalls == 112 && giDeletes == 0) r += 8;\n"
		"	p = new P;\n"
		"	::delete p;\n"
		"	if (gCalls == 112 && giDeletes == 1) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.5 [expr.ref] p5, 10.2 [class.member.lookup]: a member of a base the class has twice, named through each
	{
		"MemberAccessRepeatedBase", 63,
		"struct A { int a; int f() { return a; } static int s() { return 3; } };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct V : virtual A { };\n"
		"struct W : virtual A { };\n"
		"struct D : B, C { int k() { return B::a * 10 + C::f(); } int m() { return C::a * 10 + B::f() + s(); } };\n"
		"struct E : V, W { int k() { return a * 10 + f(); } };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	E e;\n"
		"	d.B::a = 1; d.C::a = 2; e.a = 4;\n"
		"	A* p = &e;\n"
		"	A& ra = e;\n"
		"	int r = 0;\n"
		"	if (d.k() == 12) r += 1;\n"
		"	if (d.m() == 24) r += 2;\n"
		"	if (d.B::a == 1 && d.C::f() == 2 && d.s() == 3) r += 4;\n"
		"	if (e.k() == 44 && p->a == 4 && ra.a == 4) r += 8;\n"
		"	if (static_cast<A*>(static_cast<B*>(&d))->a == 1 && static_cast<A&>(static_cast<C&>(d)).a == 2) r += 16;\n"
		"	if (static_cast<E*>(static_cast<V*>(&e))->k() == 44) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.5 [expr.ref] p5, 10.2 [class.member.lookup]: ambiguous base
	{
		"MemberAccessAmbiguousBaseOnThisRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { int g() { return a; } };\n"
		"int main() { D d; return d.g(); }\n"
	},
	{
		"MemberCallAmbiguousBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int f() { return 1; } };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; return d.f(); }\n"
	},
	{
		"MemberCallAmbiguousBaseOnThisRefused", CONFORMANCE_REFUSED,
		"struct A { int f() { return 1; } };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { int h() { return f(); } };\n"
		"int main() { D d; return d.h(); }\n"
	},
	//5.3.5 [expr.delete] p3, p9, p10: the object's own class frees it, whatever base the pointer is to
	{
		"DeleteThroughBase", 63,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void* gFreed = 0;\n"
		"size_t gSize = 0;\n"
		"int gLog = 0;\n"
		"struct A { int a; virtual ~A() { gLog = gLog * 10 + 1; } };\n"
		"struct B { int b; virtual ~B() { gLog = gLog * 10 + 2; } };\n"
		"struct D : A, B { long d; ~D() { gLog = gLog * 10 + 3; } static void operator delete(void* p, size_t n) { gFreed = p; gSize = n; } };\n"
		"struct E : D { char e[7]; static void operator delete(void* p) { gFreed = p; gSize = 77; } };\n"
		"struct V { int v; virtual ~V() { gLog = gLog * 10 + 4; } };\n"
		"struct W : virtual V { int w; static void operator delete(void* p, size_t n) { gFreed = p; gSize = n; } };\n"
		"struct G : A { int g; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D* d = new D;\n"
		"	B* b = d;\n"
		"	delete b;\n"
		"	if (gFreed == d && gSize == sizeof(D) && gLog == 321 && giDeletes == 0) r += 1;\n"
		"	gLog = 0;\n"
		"	E* e = new E;\n"
		"	b = e;\n"
		"	delete b;\n"
		"	if (gFreed == e && gSize == 77 && gLog == 321) r += 2;\n"
		"	gLog = 0;\n"
		"	W* w = new W;\n"
		"	V* v = w;\n"
		"	delete v;\n"
		"	if (gFreed == w && gSize == sizeof(W) && gLog == 4) r += 4;\n"
		"	gLog = 0;\n"
		"	gFreed = 0;\n"
		"	A* a = new G;\n"
		"	delete a;\n"
		"	if (gFreed == 0 && giDeletes == 1 && gLog == 1) r += 8;\n"
		"	gLog = 0;\n"
		"	d = new D;\n"
		"	a = d;\n"
		"	::delete a;\n"
		"	if (gFreed == 0 && giDeletes == 2 && gLog == 321) r += 16;\n"
		"	b = 0;\n"
		"	delete b;\n"
		"	if (giDeletes == 2) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p6, p9: an array of objects is destroyed and freed by its class's operator delete[]
	{
		"DeleteArrayClassOwn", 7,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void* gGiven = 0;\n"
		"void* gFreed = 0;\n"
		"int gCalls = 0;\n"
		"int gDestroyed = 0;\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	S() : v(1) { }\n"
		"	~S() { gDestroyed++; }\n"
		"	static void* operator new[](size_t n) { gGiven = Allocate(n); return gGiven; }\n"
		"	static void operator delete[](void* p) { gFreed = p; gCalls++; }\n"
		"};\n"
		"struct T { int v; ~T() { gDestroyed += 10; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S* s = new S[3];\n"
		"	delete[] s;\n"
		"	if (gFreed == gGiven && gGiven != 0 && gCalls == 1 && gDestroyed == 3 && giDeletes == 0) r += 1;\n"
		"	s = new S[2];\n"
		"	::delete[] s;\n"
		"	if (gCalls == 1 && gDestroyed == 5 && giDeletes == 1) r += 2;\n"
		"	T* t = new T[2];\n"
		"	delete[] t;\n"
		"	if (gDestroyed == 25 && giDeletes == 2 && gCalls == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p3, 9.3.2 [class.this]: this in a volatile member function, and which of the const and volatile overloads an object takes
	{
		"PrimThisVolatile", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S\n"
		"{\n"
		"	int m;\n"
		"	int f() { return 1; }\n"
		"	int f() const { return 2; }\n"
		"	int f() volatile { return 4; }\n"
		"	int f() const volatile { return 8; }\n"
		"	int g() volatile { return Same<decltype(this), volatile S*>::V + Same<decltype(m), int>::V + Same<decltype((m)), volatile int&>::V; }\n"
		"	int h() const volatile { return Same<decltype(this), const volatile S*>::V; }\n"
		"	int k() const { return 16; }\n"
		"	int k() const volatile { return 32; }\n"
		"};\n"
		"typedef int (S::*PV)() volatile;\n"
		"typedef int (S::*PP)();\n"
		"int main()\n"
		"{\n"
		"	S s; const S c = S(); volatile S v; const volatile S cv = S();\n"
		"	volatile S* pv = &s;\n"
		"	PV p = &S::g;\n"
		"	int r = 0;\n"
		"	if (s.f() == 1 && c.f() == 2 && v.f() == 4 && cv.f() == 8 && pv->f() == 4) r += 1;\n"
		"	if (s.g() == 3 && v.g() == 3 && s.h() == 1 && cv.h() == 1) r += 2;\n"
		"	if (s.k() == 16 && v.k() == 32) r += 4;\n"
		"	if ((v.*p)() == 3 && (s.*p)() == 3) r += 8;\n"
		"	if (!Same<PV, PP>::V && Same<decltype(&S::g), PV>::V) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//9.3.2 [class.this] p4: a volatile object takes only volatile member functions
	{
		"PrimThisVolatileObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int f() { return m; } };\n"
		"int main() { volatile S s; s.m = 1; return s.f(); }\n"
	},
	//13.3.3.2 [over.ics.rank] p3: neither of const and volatile is the better for an object that is neither
	{
		"PrimThisQualifierAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int f() const { return 1; } int f() volatile { return 2; } };\n"
		"int main() { S s; return s.f(); }\n"
	},
	//5.5 [expr.mptr.oper] p5: a volatile object's member function through a pointer to one that isn't volatile
	{
		"MemberPointerVolatileObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int f() { return m; } };\n"
		"int main() { volatile S s; int (S::*p)() = &S::f; s.m = 1; return (s.*p)(); }\n"
	},
	//5.1.1 [expr.prim.general] p8, 7.1.6.2 [dcl.type.simple]: decltype(e):: as a name's first qualifier, and decltype(e)(x)
	{
		"PrimDecltypeQualifier", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct B { int m; int f() { return 4; } static int s; static int g(int x) { return x + 1; } enum { E = 6 }; typedef long T; struct In { int v; }; };\n"
		"int B::s = 5;\n"
		"struct D : B { int m; };\n"
		"template<class X> struct W { typedef X Type; static X Twice(X x) { return x + x; } };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	W<int> w;\n"
		"	int r = 0;\n"
		"	if (decltype(d)::s == 5 && decltype(d)::E == 6) r += 1;\n"
		"	decltype(d)::T t = 3;\n"
		"	decltype(d)::In in;\n"
		"	in.v = 2;\n"
		"	if (Same<decltype(t), long>::V && t == 3 && in.v == 2 && sizeof(decltype(d)::T) == sizeof(long)) r += 2;\n"
		"	if (decltype(d)::g(2) == 3 && decltype(w)::Twice(4) == 8 && Same<decltype(w)::Type, int>::V) r += 4;\n"
		"	decltype(d)::s = 9;\n"
		"	if (B::s == 9 && decltype(t)(7) == 7 && Same<decltype(decltype(t)(7)), long>::V) r += 8;\n"
		"	d.m = 3;\n"
		"	d.decltype(d)::m = 4;\n"
		"	if (d.m == 4 && &decltype(d)::s == &B::s) r += 16;\n"
		"	if ((decltype(d)::T)5 == 5 && static_cast<decltype(d)::T>(6) == 6 && alignof(decltype(d)::T) == alignof(long)) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p8: decltype(e):: needs a class or an enumeration
	{
		"PrimDecltypeQualifierNotClassRefused", CONFORMANCE_REFUSED,
		"struct B { int m; };\n"
		"int main() { int i = 0; return decltype(i)::m; }\n"
	},
	//5.1.1 [expr.prim.general] p8: decltype((b)) is a reference, which has no members to name
	{
		"PrimDecltypeQualifierReferenceRefused", CONFORMANCE_REFUSED,
		"struct B { int m; static int s; };\n"
		"int B::s = 2;\n"
		"int main() { B b; return decltype((b))::s; }\n"
	},
	//5.1.1 [expr.prim.general] p13: a non-static member named in an operand that isn't evaluated, with no object
	{
		"PrimMemberUnevaluatedOperands", 7,
		"struct S\n"
		"{\n"
		"	int m;\n"
		"	char c[5];\n"
		"	long f();\n"
		"	static int g() { return sizeof(m + 1) == sizeof(int) && noexcept(m + 1) && sizeof(c[1]) == 1; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(((S*)0)->f()) == sizeof(long)) r += 1;\n"
		"	if (noexcept(S::m + 1) && noexcept(S::c[2]) && sizeof(S::m + 1) == sizeof(int)) r += 2;\n"
		"	if (S::g()) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p13: a non-static member read in a static member function
	{
		"PrimMemberInStaticRefused", CONFORMANCE_REFUSED,
		"struct S { int m; static int g() { return m + 1; } };\n"
		"int main() { return S::g(); }\n"
	},
	//5.3.1 [expr.unary.op] p7-p10, 13.6 [over.built]: +, - and ~ of an object no operator function takes are the built-in ones on what its conversion function gives
	{
		"UnaryObjectConversions", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct C { int v; operator int*() { return &v; } };\n"
		"struct D { operator int() const { return 5; } };\n"
		"struct E { operator bool() const { return true; } };\n"
		"int main()\n"
		"{\n"
		"	auto pg = +[](int v) { return v + 1; };\n"
		"	C c; c.v = 3;\n"
		"	D d; E e;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(pg), int (*)(int)>::V && pg(4) == 5) r += 1;\n"
		"	if (*+c == 3 && Same<decltype(+c), int*>::V) r += 2;\n"
		"	if (+d == 5 && -d == -5 && ~d == ~5 && !d == false && Same<decltype(-d), int>::V) r += 4;\n"
		"	if (!e == false && +e == 1 && Same<decltype(+e), int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op], 13.6 [over.built]: - of an object that converts only to a pointer
	{
		"UnaryMinusPointerObjectRefused", CONFORMANCE_REFUSED,
		"struct C { int v; operator int*() { return &v; } };\n"
		"int main() { C c; c.v = 3; return -c ? 1 : 2; }\n"
	},
	//5.3.1 [expr.unary.op], 13.6 [over.built]: an object that converts to two arithmetic types
	{
		"UnaryObjectTwoConversionsRefused", CONFORMANCE_REFUSED,
		"struct X { operator int() const { return 1; } operator double() const { return 2.0; } };\n"
		"int main() { X x; return (int)-x; }\n"
	},
	//5.3.1 [expr.unary.op], 13.6 [over.built]: ~ of an object that converts only to a floating type
	{
		"UnaryComplementFloatObjectRefused", CONFORMANCE_REFUSED,
		"struct F { operator double() const { return 2.0; } };\n"
		"int main() { F f; return (int)~f; }\n"
	},
	//5.3.1 [expr.unary.op], 13.6 [over.built]: + of an object with no operator and no conversion function
	{
		"UnaryPlusObjectNoConversionRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"int main() { S s; s.v = 1; return +s ? 1 : 2; }\n"
	},
	//5.1.2 [expr.prim.lambda] p13: a lambda in a default argument that captures nothing, and one inside it that captures its local
	{
		"LambdaInDefaultArgument", 6,
		"int g = 3;\n"
		"int f(int v = [] { return g + 1; }()) { return v; }\n"
		"int h(int v = [] { int z = 2; return [z] { return z; }(); }()) { return v; }\n"
		"int main() { return f() + h(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p13: a lambda in a default argument captures nothing, by a default capture either
	{
		"LambdaDefaultArgumentDefaultCaptureRefused", CONFORMANCE_REFUSED,
		"int main() { int a = 1; struct L { static int f(int v = [=] { return a; }()) { return v; } }; return L::f(); }\n"
	},
	//5.1.2 [expr.prim.lambda] p15: an array captured by copy is the closure's own array, each element copied
	{
		"LambdaArrayCapture", 31,
		"int gCopies = 0;\n"
		"int gDestroyed = 0;\n"
		"struct P { int v; P() : v(1) { } P(const P& o) : v(o.v + 10) { gCopies++; } ~P() { gDestroyed++; } };\n"
		"int Sum(const int* p, int n) { int s = 0; for (int i = 0; i < n; i++) s += p[i]; return s; }\n"
		"int main()\n"
		"{\n"
		"	int a[3] = { 4, 5, 6 };\n"
		"	int m[2][2] = { { 1, 2 }, { 3, 4 } };\n"
		"	int r = 0;\n"
		"	auto arr = [a] { return a[0] + a[1] + a[2] + (int)sizeof(a) / (int)sizeof(int); };\n"
		"	auto mut = [a]() mutable { a[0] = 20; return a[0] + a[1]; };\n"
		"	auto mat = [m] { return m[0][0] + m[1][1] * 10 + Sum(m[1], 2) * 100; };\n"
		"	auto all = [=] { return Sum(a, 3) + m[0][1]; };\n"
		"	a[1] = 50;\n"
		"	m[1][1] = 9;\n"
		"	if (arr() == 18) r += 1;\n"
		"	if (mut() == 25 && a[0] == 4) r += 2;\n"
		"	if (mat() == 741 && all() == 61 + 2 - 46 + 0) r += 4;\n"
		"	{\n"
		"		P p[2];\n"
		"		auto objs = [p] { return p[0].v + p[1].v; };\n"
		"		auto again = objs;\n"
		"		if (objs() == 22 && again() == 42 && gCopies == 4) r += 8;\n"
		"	}\n"
		"	if (gDestroyed == 6) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p18, 3.2 [basic.def.odr] p3: decltype of a captured variable, and of a constant the lambda needn't capture
	{
		"LambdaConstantDecltype", 4095,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	const int cx = 2;\n"
		"	auto g = [=]() mutable { return Same<decltype((x)), int&>::V + Same<decltype((cx)), const int&>::V * 2 + Same<decltype(cx), const int>::V * 4 + Same<decltype(x), int>::V * 8; };\n"
		"	auto h = [=]() mutable { int y = x + cx; return Same<decltype((cx)), const int&>::V + Same<decltype((x)), int&>::V * 2 + y * 4; };\n"
		"	auto k = [=]() { int y = x + cx; return Same<decltype((cx)), const int&>::V + Same<decltype((x)), const int&>::V * 2 + y * 4; };\n"
		"	return g() + h() * 16 + k() * 256;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p6: a generic lambda that captures nothing converts to a pointer to a function of each type its operator() can be
	{
		"LambdaGenericConversion", 14,
		"int Apply(int (*p)(int), int v) { return p(v); }\n"
		"int main()\n"
		"{\n"
		"	auto twice = [](auto v) { return v + v; };\n"
		"	int (*p)(int) = twice;\n"
		"	long (*q)(long) = twice;\n"
		"	return Apply(twice, 4) + p(1) + (int)q(2);\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p5, 7.1.6.4 [dcl.spec.auto]: -> decltype(auto) is what the return gives, a reference where that is one
	{
		"LambdaTrailingDeducedReturn", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	auto forward = [](auto&& v) -> decltype(auto) { return static_cast<decltype(v)>(v); };\n"
		"	auto ref = [](auto&& v) { return Same<decltype(v), int&>::V + Same<decltype(v), int&&>::V * 2; };\n"
		"	auto plain = [](int& v) -> decltype(auto) { return static_cast<int&>(v); };\n"
		"	auto moved = [](int& v) -> decltype(auto) { return static_cast<int&&>(v); };\n"
		"	int i = 5;\n"
		"	int r = 0;\n"
		"	if (Same<decltype(forward(i)), int&>::V) r += 1;\n"
		"	if (Same<decltype(forward(5)), int&&>::V) r += 2;\n"
		"	if (ref(i) == 1 && ref(5) == 2) r += 4;\n"
		"	if (Same<decltype(plain(i)), int&>::V) r += 8;\n"
		"	if (Same<decltype(moved(i)), int&&>::V) r += 16;\n"
		"	forward(i) = 7;\n"
		"	if (i == 7) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p6: the function pointed to must return what the lambda does for those parameters
	{
		"LambdaGenericConversionReturnRefused", CONFORMANCE_REFUSED,
		"int main() { int (*p)(double) = [](auto v) { return v + v; }; return p(1.0); }\n"
	},
	//5.1.2 [expr.prim.lambda] p6: a lambda that captures has no conversion to a pointer to a function
	{
		"LambdaGenericConversionCaptureRefused", CONFORMANCE_REFUSED,
		"int main() { int k = 1; int (*p)(int) = [k](auto v) { return v + k; }; return p(1); }\n"
	},
	//5.1.2 [expr.prim.lambda] p6: auto& is no parameter of a function that takes an int
	{
		"LambdaGenericConversionReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { int (*p)(int) = [](auto& v) { return v + 1; }; return p(1); }\n"
	},
	//5.2.2 [expr.call] p11, 7.1.6.2 [dcl.type.simple] p5: the call in brackets, and as the right of a comma that is decltype's operand
	{
		"CallIncompleteReturnDecltypeForms", 3,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct I;\n"
		"I Make();\n"
		"I& Ref();\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype((Make())), I>::V && Same<decltype(0, Make()), I>::V) r += 1;\n"
		"	if (Same<decltype(Ref()), I&>::V && Same<decltype((0, (Make()))), I>::V) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.18 [expr.comma] p1: a comma expression is an lvalue or an rvalue as its right operand is
	{
		"CommaValueCategory", 7,
		"struct S { int v; };\n"
		"int Take(S&) { return 1; }\n"
		"int Take(S&&) { return 2; }\n"
		"S Make() { S s; s.v = 3; return s; }\n"
		"int main()\n"
		"{\n"
		"	S a;\n"
		"	a.v = 1;\n"
		"	int r = 0;\n"
		"	if (Take((0, a)) == 1) r += 1;\n"
		"	if (Take((0, Make())) == 2) r += 2;\n"
		"	S&& moved = (a.v++, Make());\n"
		"	S& same = (a.v++, a);\n"
		"	if (moved.v == 3 && &same == &a && a.v == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p5: a type-id that is an array by itself, new (T[N]) and new A for a typedef of one, makes the array and gives a pointer to its first element
	{
		"NewParenthesisedArray", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gConstructed = 0;\n"
		"int gDestroyed = 0;\n"
		"struct C { C() : v(++gConstructed) { } ~C() { gDestroyed++; } int v; };\n"
		"typedef int A[4];\n"
		"typedef C Pair[2];\n"
		"int Three() { return 3; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int* a = new (int[3]);\n"
		"	a[2] = 5;\n"
		"	if (a[2] == 5 && guiLastSize == 3 * sizeof(int) && Same<decltype(new (int[3])), int*>::V) r += 1;\n"
		"	int (**pf)() = new (int (*[2])());\n"
		"	pf[1] = Three;\n"
		"	int (*m)[2] = new (int[3][2]);\n"
		"	m[2][1] = 7;\n"
		"	if (pf[1]() == 3 && m[2][1] == 7 && guiLastSize == 6 * sizeof(int) && Same<decltype(new (int[3][2])), int (*)[2]>::V) r += 2;\n"
		"	int* t = new A;\n"
		"	t[3] = 9;\n"
		"	int* z = new A();\n"
		"	if (t[3] == 9 && z[0] == 0 && z[3] == 0 && guiLastSize == 4 * sizeof(int) && Same<decltype(new A), int*>::V) r += 4;\n"
		"	C* c = new Pair;\n"
		"	C* d = new (C[3]);\n"
		"	if (gConstructed == 5 && c[1].v == 2 && d[2].v == 5) r += 8;\n"
		"	delete[] c;\n"
		"	delete[] d;\n"
		"	if (gDestroyed == 5 && giDeletes == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.4 [expr.new] p1: an array of unknown bound can't be made
	{
		"NewParenthesisedArrayNoBoundRefused", CONFORMANCE_REFUSED,
		"void* operator new[](decltype(sizeof(0)) n);\n"
		"int main() { int* p = new (int[]); return p != 0; }\n"
	},
	//14.5.6 [temp.fct], 3.2 [basic.def.odr] p4: a function template that is only declared is called where the call is not evaluated, and is its definition once it has one
	{
		"TemplateDeclaredOnly", 7,
		"template<class T> char Test(T*);\n"
		"template<class T> long Test(...);\n"
		"template<class T> T Declared(T) noexcept;\n"
		"template<class T, class U> U Pick(T, U);\n"
		"struct S { };\n"
		"template<class T> T f(T);\n"
		"typedef decltype(f(1)) X;\n"
		"template<class U> U f(U u) { return u + 1; }\n"
		"template<class T> T f(T);\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(Test<S>((S*)0)) == 1 && sizeof(Test<S>(1, 2)) == sizeof(long)) r += 1;\n"
		"	if (noexcept(Declared(1)) && sizeof(Declared('a')) == 1 && sizeof(Pick(1, 2L)) == sizeof(long)) r += 2;\n"
		"	X x = f(1);\n"
		"	if (x == 2 && f(2L) == 3 && sizeof(X) == sizeof(int)) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.16 [expr.cond] p4: c ? x : y of bit-fields, or of one and a variable of its type, is the one picked: assigned, counted and read
	{
		"ConditionalBitFields", 15,
		"struct Bits { unsigned x : 3; unsigned y : 5; int s : 4; int t : 6; unsigned whole; };\n"
		"int main()\n"
		"{\n"
		"	bool c = true;\n"
		"	bool n = false;\n"
		"	Bits b;\n"
		"	unsigned plain = 9;\n"
		"	b.x = 1; b.y = 2; b.s = -3; b.t = -20; b.whole = 100;\n"
		"	int r = 0;\n"
		"	(c ? b.x : b.y) = 5;\n"
		"	(n ? b.x : b.y) = 30;\n"
		"	if (b.x == 5 && b.y == 30 && b.s == -3 && b.t == -20 && b.whole == 100) r += 1;\n"
		"	(c ? b.x : b.y) += 2;\n"
		"	(n ? b.x : b.y)--;\n"
		"	++(c ? b.s : b.t);\n"
		"	if (b.x == 7 && b.y == 29 && b.s == -2 && (n ? b.s : b.t) == -20 && (c ? b.s : b.t) == -2) r += 2;\n"
		"	(c ? b.x : plain) = 3;\n"
		"	(n ? b.x : plain) = 7000;\n"
		"	if (b.x == 3 && plain == 7000 && (c ? plain : b.y) == 7000 && (n ? plain : b.y) == 29) r += 4;\n"
		"	if (((n ? b.x : b.y) = 0x7F) == 31 && b.y == 31 && b.x == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.1 [expr.unary.op] p4: the address of c ? x : y that picks a bit-field
	{
		"ConditionalBitFieldAddressRefused", CONFORMANCE_REFUSED,
		"struct Bits { unsigned x : 3; unsigned y : 3; };\n"
		"int main() { Bits b; b.x = 1; b.y = 2; unsigned* p = &(b.x ? b.x : b.y); return *p; }\n"
	},
	//5.16 [expr.cond] p3: each operand converts to the other's type
	{
		"ConditionalBothConvertRefused", CONFORMANCE_REFUSED,
		"struct X { X(int v) : v(v) { } operator int() const { return v; } int v; };\n"
		"int main() { X x(1); bool c = true; return (c ? x : 2) == 1; }\n"
	},
	//5.19 [expr.const] p2, p5, 7.1.5 [dcl.constexpr] p9: a constexpr local of a class, a floating or a pointer type is a constant, read by what follows
	{
		"ConstantLocalObjects", 7,
		"struct P { int x; int y; constexpr int Sum() const { return x + y; } };\n"
		"constexpr double Half(double v) { return v / 2; }\n"
		"int gA = 5;\n"
		"int gArr[4];\n"
		"constexpr int Twice(int v) { return v * 2; }\n"
		"int main()\n"
		"{\n"
		"	constexpr P p = { 3, 4 };\n"
		"	constexpr double d = 2.5;\n"
		"	constexpr double e = d * 2 + Half(3.0);\n"
		"	constexpr int* pg = &gA;\n"
		"	constexpr int& rg = gA;\n"
		"	constexpr const int* pe = &gArr[p.x];\n"
		"	constexpr P q = { p.y, Twice(p.x) };\n"
		"	static constexpr int sLocal = 9;\n"
		"	constexpr const int* ps = &sLocal;\n"
		"	int a[p.x];\n"
		"	int b[p.Sum() + q.y];\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 3 * sizeof(int) && sizeof(b) == 13 * sizeof(int)) r += 1;\n"
		"	if (e == 6.5 && *pg == 5 && &rg == &gA && pe == &gArr[3]) r += 2;\n"
		"	if (q.x == 4 && q.y == 6 && *ps == 9) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p5: a constexpr pointer to a member function that nothing else names
	{
		"ConstantMemberFunctionPointerData", 6,
		"struct S { int m; int f() const { return m; } };\n"
		"S gs = { 6 };\n"
		"constexpr int (S::*gpmf)() const = &S::f;\n"
		"int main() { return (gs.*gpmf)(); }\n"
	},
	//5.19 [expr.const] p2: a static const member is a constant from its definition after the class
	{
		"ConstantStaticMemberOutside", 15,
		"struct S { static const int scIn = 7; static const int scOut; static const int scLate; };\n"
		"const int S::scOut = 8;\n"
		"int main()\n"
		"{\n"
		"	int e[S::scIn + S::scOut];\n"
		"	return sizeof(e) / sizeof(int);\n"
		"}\n"
	},
	//13.6 [over.built], 5.6-5.13: a built-in binary operator on an object no operator function takes is done on what the object's conversion function gives
	{
		"BuiltinOperatorsOnObjects", 63,
		"struct T { operator int() const { return 7; } };\n"
		"struct U { U(int x) : v(x) { } operator long() const { return v; } long v; };\n"
		"struct Q { operator int*() const { return p; } int* p; };\n"
		"int ga[9];\n"
		"int main()\n"
		"{\n"
		"	T t;\n"
		"	U u(3);\n"
		"	Q q;\n"
		"	q.p = ga + 2;\n"
		"	int x = 1;\n"
		"	int* p = ga;\n"
		"	int r = 0;\n"
		"	if (t + 1 == 8 && 1 + t == 8 && t * t == 49 && t - u == 4 && t / 2.0 == 3.5) r += 1;\n"
		"	if (t % 4 == 3 && t << 1 == 14 && (t & 6) == 6 && (t | 8) == 15 && (t ^ u) == 4 && u >> 1 == 1) r += 2;\n"
		"	if (t < 9 && !(t == u) && t != u && t >= u && u <= t && t > 6L) r += 4;\n"
		"	if (t + p == p + 7 && q + 1 == ga + 3 && q - p == 2 && q == ga + 2 && q > p) r += 8;\n"
		"	x += t;\n"
		"	x <<= u;\n"
		"	x -= u;\n"
		"	p += t;\n"
		"	if (x == 61 && p == ga + 7) r += 16;\n"
		"	if ((t && u) && (t || u) && sizeof(t + u) == sizeof(long) && sizeof(t + 1) == sizeof(int)) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.6 [over.built]: an object with no operator function and no conversion function
	{
		"BuiltinOperatorObjectNoConversionRefused", CONFORMANCE_REFUSED,
		"struct T { operator int() const { return 7; } };\n"
		"struct S { int v; };\n"
		"int main() { T t; S s; s.v = 1; return t + s; }\n"
	},
	//13.6 [over.built]: an object that converts to two arithmetic types
	{
		"BuiltinOperatorObjectTwoConversionsRefused", CONFORMANCE_REFUSED,
		"struct X { operator int() const { return 1; } operator double() const { return 2.0; } };\n"
		"int main() { X x; return (int)(x + 1); }\n"
	},
	//13.6 [over.built]: * of an object that converts only to a pointer
	{
		"BuiltinOperatorPointerObjectTimesRefused", CONFORMANCE_REFUSED,
		"struct Q { operator int*() const { return 0; } };\n"
		"int main() { Q q; return q * 2; }\n"
	},
	//5.19 [expr.const] p3, 14.3.2 [temp.arg.nontype], 7.2 [dcl.enum] p5: template arguments and enumerators whose values are of the type they are converted to
	{
		"ConstantConvertedFits", 7,
		"template<unsigned char C> struct Byte { enum { V = C }; };\n"
		"template<unsigned int N> struct Count { enum { V = 1 }; };\n"
		"template<class T, T v> struct Of { static const T V = v; };\n"
		"template<long L> struct Long { enum { V = L > 0 }; };\n"
		"template<bool B> struct Flag { enum { V = B }; };\n"
		"enum E : unsigned char { E0 = 255, E1 = 0 };\n"
		"enum class F : short { F0 = -32768, F1 = 32767 };\n"
		"const int gFits = 200;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Byte<255>::V == 255 && Byte<gFits>::V == 200 && Byte<'a'>::V == 'a' && Count<65535u>::V == 1) r += 1;\n"
		"	if (Of<unsigned char, 255>::V == 255 && Of<signed char, -128>::V == -128 && Long<70000L>::V && Flag<(3 > 2)>::V) r += 2;\n"
		"	if (E0 == 255 && (int)F::F0 == -32768 && (int)F::F1 == 32767) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2: an address cast to an integer is not a constant
	{
		"ConstantAddressAsNumberRefused", CONFORMANCE_REFUSED,
		"int gi = 1;\n"
		"int main() { constexpr unsigned long long a = (unsigned long long)&gi; return a ? 1 : 2; }\n"
	},
	//5.3.5 [expr.delete] p10, 3.7.4.2 [basic.stc.dynamic.deallocation] p2: of the global operator delete that takes the pointer and the one that also takes the size, an object of a complete type is freed by the one that takes the size
	{
		"DeleteGlobalSized", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[256];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiNew = 0;\n"
		"size_t guiSized = 0;\n"
		"int giPlain = 0;\n"
		"int giSized = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; guiHeap += (n + 7) & ~(size_t)7; guiNew = n; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { giPlain++; }\n"
		"void operator delete[](void* p) noexcept { giPlain++; }\n"
		"void operator delete(void* p, size_t n) noexcept { giSized++; guiSized = n; }\n"
		"void operator delete[](void* p, size_t n) noexcept { giSized++; guiSized = n; }\n"
		"struct S { long a; long b; };\n"
		"struct D { int v; ~D() { } };\n"
		"struct V { long x; virtual ~V() { } };\n"
		"struct W : V { long y; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S* s = new S;\n"
		"	delete s;\n"
		"	if (giSized == 1 && guiSized == sizeof(S) && giPlain == 0) r += 1;\n"
		"	int* i = new int;\n"
		"	delete i;\n"
		"	if (giSized == 2 && guiSized == sizeof(int) && giPlain == 0) r += 2;\n"
		"	D* d = new D[3];\n"
		"	delete[] d;\n"
		"	if (giSized == 3 && guiSized == guiNew && guiSized > 3 * sizeof(D) && giPlain == 0) r += 4;\n"
		"	S* a = new S[2];\n"
		"	delete[] a;\n"
		"	if (giSized + giPlain == 4 && (giPlain == 1 || guiSized == guiNew)) r += 8;\n"
		"	giPlain = 0;\n"
		"	giSized = 0;\n"
		"	V* v = new W;\n"
		"	delete v;\n"
		"	if (giSized + giPlain == 1 && (giPlain == 1 || guiSized == sizeof(W))) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p10, 12.5 [class.free] p5: a class's operator delete[] that takes a size is given the size its array was allocated with
	{
		"DeleteArraySizedClass", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[256];\n"
		"size_t guiNew = 0;\n"
		"size_t guiDeleted = 0;\n"
		"int giDestroyed = 0;\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	S() : v(1) { }\n"
		"	~S() { giDestroyed++; }\n"
		"	static void* operator new[](size_t n) { guiNew = n; return gacHeap; }\n"
		"	static void operator delete[](void* p, size_t n) { guiDeleted = n; }\n"
		"};\n"
		"struct T : S { int w; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S* p = new S[3];\n"
		"	if (guiNew > 3 * sizeof(S)) r += 1;\n"
		"	delete[] p;\n"
		"	if (giDestroyed == 3) r += 2;\n"
		"	if (guiDeleted == guiNew) r += 4;\n"
		"	T* q = new T[2];\n"
		"	delete[] q;\n"
		"	if (giDestroyed == 5 && guiDeleted == guiNew && guiNew > 2 * sizeof(T)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.2 [expr.call], 13.5 [over.oper] p4: an operator function is called by its name, operator+(a, b), as another function is
	{
		"CallOperatorFunctionByName", 31,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[64];\n"
		"int giDeleted = 0;\n"
		"void* operator new(size_t n) { return gacHeap; }\n"
		"void operator delete(void* p) noexcept { giDeleted++; }\n"
		"struct V { int v; V operator-() const { V r = { -v }; return r; } int operator[](int i) const { return v + i; } };\n"
		"V operator+(const V& a, const V& b) { V r = { a.v + b.v }; return r; }\n"
		"bool operator==(const V& a, const V& b) { return a.v == b.v; }\n"
		"template<class T> T operator*(const V& a, T t) { return (T)(a.v * t); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	V a = { 3 };\n"
		"	V b = { 4 };\n"
		"	void* p = operator new(8);\n"
		"	if (p == gacHeap) r += 1;\n"
		"	operator delete(p);\n"
		"	if (giDeleted == 1) r += 2;\n"
		"	if (operator+(a, b).v == 7 && operator==(a, a)) r += 4;\n"
		"	if (a.operator-().v == -3 && a.operator[](2) == 5) r += 8;\n"
		"	if (operator*(a, 5L) == 15 && sizeof(operator*(a, 5L)) == sizeof(long)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p24: a capture followed by an ellipsis is a pack expansion, and a default capture captures a pack the body expands
	{
		"LambdaCapturesPack", 31,
		"int Sum(int a, int b, int c) { return a + b + c; }\n"
		"template<class... A> int ByCopy(A... a) { auto l = [a...] { return Sum(a...); }; return l(); }\n"
		"template<class... A> int ByReference(A&... a) { auto l = [&a...] { int x[] = { (a += 1)... }; return x[0] + x[1] + x[2]; }; return l(); }\n"
		"template<class... A> int ByDefault(A... a) { auto l = [=] { return Sum(a...) * 2; }; return l(); }\n"
		"template<class... A> int Count(A... a) { int n = 5; auto l = [n, a...] { return n + (int)sizeof...(a); }; return l(); }\n"
		"template<class... A> int Mutable(A... a) { auto l = [a...]() mutable { int x[] = { (a *= 2)... }; return x[0] + x[1] + x[2]; }; return l() + Sum(a...); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = 1;\n"
		"	int j = 2;\n"
		"	int k = 3;\n"
		"	if (ByCopy(1, 2, 3) == 6) r += 1;\n"
		"	if (ByReference(i, j, k) == 9 && i == 2 && j == 3 && k == 4) r += 2;\n"
		"	if (ByDefault(1, 2, 3) == 12) r += 4;\n"
		"	if (Count() == 5 && Count(1, 'c') == 7) r += 8;\n"
		"	if (Mutable(1, 2, 3) == 18) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p5: a generic lambda's parameter-declaration that is a function parameter pack makes its operator() template's last parameter a pack
	{
		"LambdaGenericPack", 15,
		"int Sum(int a, int b, int c) { return a + b + c; }\n"
		"template<class... A> int Count(A... a) { return (int)sizeof...(a); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	auto all = [](auto... a) { return Sum(a...); };\n"
		"	auto rest = [](int x, auto... a) { int t[] = { a..., 0 }; return x + t[0]; };\n"
		"	auto count = [](auto... a) { return (int)sizeof...(a) * 10 + Count(a...); };\n"
		"	auto sizes = [](auto... a) { int s[] = { (int)sizeof(a)..., 0 }; return s[0] * 10 + s[1]; };\n"
		"	if (all(1, 2, 3) == 6) r += 1;\n"
		"	if (rest(4, 5, 6) == 9 && rest(7) == 7) r += 2;\n"
		"	if (count() == 0 && count(1, 'c', 2L) == 33) r += 4;\n"
		"	if (sizes('c', \"ab\") == 10 + (int)sizeof(const char*)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
};


//Clause 8, declarators [dcl.decl].
static const SConformanceCase gasConformanceClause8[] =
{
	//8.1 [dcl.name]: type names
	{
		"DclTypeNames", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef int* IntPointer;\n"
		"typedef int Row[3];\n"
		"typedef int Function(double);\n"
		"int Twice(double d) { return (int)(d * 2); }\n"
		"int main()\n"
		"{\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int r = 0;\n"
		"	if (sizeof(int*) == sizeof(IntPointer) && sizeof(int[3]) == sizeof(Row) && sizeof(int*[3]) == 3 * sizeof(int*)) r += 1;\n"
		"	if (sizeof(int (*)[3]) == sizeof(void*) && sizeof(int (*)(double)) == sizeof(Function*)) r += 2;\n"
		"	if (Same<int (*)[3], Row*>::V && Same<int (*)(double), Function*>::V && Same<int* (*)(), IntPointer (*)()>::V) r += 4;\n"
		"	if (!Same<int*[3], int (*)[3]>::V && !Same<int* (), int (*)()>::V && Same<int[3], Row>::V) r += 8;\n"
		"	if (((int (*)(double))Twice)(2.5) == 5 && (*(int (*)[3])&a)[2] == 3 && static_cast<const int*>(a)[1] == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//8.2 [dcl.ambig.res] p1: what can be a declaration is one
	{
		"DclAmbiguityFunctionOrObject", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { S(int x) : v(x) { } S() : v(9) { } int v; };\n"
		"int ga = 4;\n"
		"int main()\n"
		"{\n"
		"	int a = 3;\n"
		"	S x(int(a));\n"
		"	S y((int)a);\n"
		"	S z(int(ga) + 1);\n"
		"	S w(S());\n"
		"	int r = 0;\n"
		"	if (Same<decltype(x), S(int)>::V) r += 1;\n"
		"	if (y.v == 3) r += 2;\n"
		"	if (z.v == 5) r += 4;\n"
		"	if (Same<decltype(w), S(S (*)())>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//8.2 [dcl.ambig.res], 6.8 [stmt.ambig]: a statement that can be a declaration is one
	{
		"DclAmbiguityStatement", 7,
		"struct T { T(int v) : m(v) { } T* operator->() { return this; } int m; };\n"
		"int gCalls = 0;\n"
		"int Call(int v) { gCalls += v; return v; }\n"
		"int main()\n"
		"{\n"
		"	int a = 1;\n"
		"	int r = 0;\n"
		"	{\n"
		"		int(x);\n"
		"		x = 5;\n"
		"		if (x == 5) r += 1;\n"
		"	}\n"
		"	T(a)->m = 7;\n"
		"	int(Call(3));\n"
		"	if (gCalls == 3) r += 2;\n"
		"	{\n"
		"		T(b)(4);\n"
		"		if (b.m == 4) r += 4;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//8.2 [dcl.ambig.res] p2: what can be a type is one
	{
		"DclAmbiguityTypeId", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class F> struct Holder { enum { IsFunction = Same<F, int()>::V }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Holder<int()>::IsFunction) r += 1;\n"
		"	if (sizeof(int(*)()) == sizeof(void*)) r += 2;\n"
		"	if ((int)(1.5) == 1 && (int)1.5 + int(2.5) == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.1 [dcl.ptr]: pointers
	{
		"DclPointers", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int i = 1;\n"
		"	int j = 2;\n"
		"	const int ci = 3;\n"
		"	int* p = &i;\n"
		"	const int* pc = &ci;\n"
		"	int* const cp = &i;\n"
		"	const int* const cpc = &ci;\n"
		"	int** pp = &p;\n"
		"	int* const* pcp = &cp;\n"
		"	volatile int* pv = &i;\n"
		"	int r = 0;\n"
		"	*cp = 5;\n"
		"	pc = &i;\n"
		"	p = &j;\n"
		"	if (i == 5 && *pc == 5 && *p == 2 && **pp == 2) r += 1;\n"
		"	if (*cpc == 3 && **pcp == 5 && *pv == 5) r += 2;\n"
		"	if (Same<decltype(cp), int* const>::V && Same<decltype(pc), const int*>::V && Same<decltype(cpc), const int* const>::V) r += 4;\n"
		"	if (Same<decltype(*cp), int&>::V && Same<decltype(*pc), const int&>::V && Same<decltype(*pcp), int* const&>::V) r += 8;\n"
		"	int *a, b, *c;\n"
		"	a = &i; b = 7; c = &b;\n"
		"	if (*a == 5 && *c == 7 && sizeof(b) == sizeof(int)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"DclConstPointerAssignedRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int j = 2; int* const cp = &i; cp = &j; return *cp; }\n"
	},
	{
		"DclPointerToConstWrittenRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; const int* pc = &i; *pc = 2; return i; }\n"
	},
	//8.3.1 [dcl.ptr] p4: no pointers to references
	{
		"DclPointerToReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int&* p = 0; return i; }\n"
	},
	//8.3.2 [dcl.ref]: references
	{
		"DclReferences", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef int& LRef;\n"
		"typedef int&& RRef;\n"
		"typedef const int CInt;\n"
		"int gi = 4;\n"
		"int& Global() { return gi; }\n"
		"void Bump(int& r) { r++; }\n"
		"int main()\n"
		"{\n"
		"	int i = 1;\n"
		"	int& r = i;\n"
		"	int& rr = r;\n"
		"	const int& cr = i;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int (&ra)[3] = a;\n"
		"	int* p = &i;\n"
		"	int*& rp = p;\n"
		"	int r0 = 0;\n"
		"	r = 5;\n"
		"	rr += 1;\n"
		"	if (i == 6 && &r == &i && &rr == &i && cr == 6) r0 += 1;\n"
		"	Bump(i);\n"
		"	Bump(r);\n"
		"	Global() = 9;\n"
		"	if (i == 8 && gi == 9) r0 += 2;\n"
		"	ra[1] = 20;\n"
		"	rp = &gi;\n"
		"	if (a[1] == 20 && sizeof(ra) == sizeof(a) && p == &gi && *rp == 9) r0 += 4;\n"
		"	if (Same<LRef&, int&>::V && Same<LRef&&, int&>::V && Same<RRef&, int&>::V && Same<RRef&&, int&&>::V) r0 += 8;\n"
		"	if (Same<CInt&, const int&>::V && sizeof(LRef) == sizeof(int) && sizeof(r) == sizeof(int)) r0 += 16;\n"
		"	LRef& collapsed = i;\n"
		"	collapsed = 30;\n"
		"	if (i == 30 && Same<decltype(collapsed), int&>::V) r0 += 32;\n"
		"	return r0;\n"
		"}\n"
	},
	//8.3.2 [dcl.ref] p5: a reference is initialised
	{
		"DclReferenceNoInitialiserRefused", CONFORMANCE_REFUSED,
		"int main() { int& r; return 1; }\n"
	},
	//8.3.2 [dcl.ref] p1: no references to void
	{
		"DclReferenceToVoidRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; void& r = i; return 1; }\n"
	},
	//8.3.2 [dcl.ref] p5: no arrays of references
	{
		"DclArrayOfReferencesRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int& a[2] = { i, i }; return a[0]; }\n"
	},
	//8.3.2 [dcl.ref] p5: no references to references
	{
		"DclReferenceToReferenceRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int& & r = i; return r; }\n"
	},
	//8.3.2 [dcl.ref] p1: a reference has no cv-qualifiers of its own
	{
		"DclConstReferenceQualifierRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int& const r = i; return r; }\n"
	},
	//8.3.3 [dcl.mptr]: pointers to members
	{
		"DclMemberPointers", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int a; int b; const int c; int f() { return a; } int g() const { return b; } static int s; static int sf() { return 7; } S() : a(1), b(2), c(3) { } };\n"
		"int S::s = 4;\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	int S::* pa = &S::a;\n"
		"	const int S::* pc = &S::c;\n"
		"	int (S::*pf)() = &S::f;\n"
		"	int (S::*pg)() const = &S::g;\n"
		"	int* ps = &S::s;\n"
		"	int (*psf)() = &S::sf;\n"
		"	int S::* const cpa = &S::b;\n"
		"	int r = 0;\n"
		"	if (s.*pa == 1 && s.*pc == 3 && (s.*pf)() == 1 && (s.*pg)() == 2) r += 1;\n"
		"	if (*ps == 4 && psf() == 7 && s.*cpa == 2) r += 2;\n"
		"	if (Same<decltype(pa), int S::*>::V && Same<decltype(pc), const int S::*>::V && Same<decltype(&S::g), int (S::*)() const>::V) r += 4;\n"
		"	if (!Same<int S::*, int*>::V && !Same<int (S::*)(), int (S::*)() const>::V && Same<decltype(&S::s), int*>::V) r += 8;\n"
		"	typedef int S::* MemberPointer;\n"
		"	MemberPointer table[2] = { &S::a, &S::b };\n"
		"	if (s.*table[0] + s.*table[1] == 3 && sizeof(table) == 2 * sizeof(MemberPointer)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.3 [dcl.mptr] p3: not to a static member
	{
		"DclMemberPointerToStaticRefused", CONFORMANCE_REFUSED,
		"struct S { static int s; };\n"
		"int S::s = 1;\n"
		"int main() { int S::* p = &S::s; return p ? 1 : 2; }\n"
	},
	//8.3.3 [dcl.mptr]: pointers to members
	{
		"DclMemberPointerToConstFunctionRefused", CONFORMANCE_REFUSED,
		"struct S { int g() const { return 1; } };\n"
		"int main() { int (S::*p)() = &S::g; S s; return (s.*p)(); }\n"
	},
	//8.3.4 [dcl.array]: arrays
	{
		"DclArrays", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef int Unbounded[];\n"
		"extern int gLater[];\n"
		"int Peek() { return gLater[1]; }\n"
		"int gLater[3] = { 4, 5, 6 };\n"
		"const int gSize = 4;\n"
		"enum { Five = 5 };\n"
		"int main()\n"
		"{\n"
		"	int a[] = { 1, 2, 3 };\n"
		"	int m[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };\n"
		"	int n[][2] = { { 1, 2 }, { 3, 4 }, { 5, 6 } };\n"
		"	Unbounded u = { 7, 8 };\n"
		"	char sized[gSize * 2 + Five];\n"
		"	const int c[2] = { 9, 10 };\n"
		"	int (*rows)[3] = m;\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 3 * sizeof(int) && sizeof(u) == 2 * sizeof(int) && sizeof(sized) == 13) r += 1;\n"
		"	if (sizeof(m) == 6 * sizeof(int) && sizeof(m[0]) == 3 * sizeof(int) && sizeof(n) == 6 * sizeof(int) && sizeof(n) / sizeof(n[0]) == 3) r += 2;\n"
		"	if (m[1][2] == 6 && n[2][0] == 5 && rows[1][0] == 4 && *(*(m + 1) + 1) == 5) r += 4;\n"
		"	if (Peek() == 5 && sizeof(gLater) == 3 * sizeof(int)) r += 8;\n"
		"	if (Same<decltype(a), int[3]>::V && Same<decltype(m), int[2][3]>::V && Same<decltype(c), const int[2]>::V && Same<decltype(c[0]), const int&>::V) r += 16;\n"
		"	if (Same<decltype(m[0]), int (&)[3]>::V && Same<decltype(&m), int (*)[2][3]>::V && Same<decltype(&m[0]), int (*)[3]>::V) r += 32;\n"
		"	int t[2][2][2] = { { { 1, 2 }, { 3, 4 } }, { { 5, 6 }, { 7, 8 } } };\n"
		"	if (t[1][0][1] == 6 && sizeof(t[0]) == 4 * sizeof(int) && c[1] == 10 && &a[3] - &a[0] == 3) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.4 [dcl.array] p1: the bound is greater than zero
	{
		"DclArrayNegativeBoundRefused", CONFORMANCE_REFUSED,
		"int main() { int a[-1]; return sizeof(a) ? 1 : 2; }\n"
	},
	{
		"DclArrayZeroBoundRefused", CONFORMANCE_REFUSED,
		"int main() { int a[0]; return 1; }\n"
	},
	//8.3.4 [dcl.array] p1: not of void, references or functions
	{
		"DclArrayOfVoidRefused", CONFORMANCE_REFUSED,
		"int main() { void a[2]; return 1; }\n"
	},
	{
		"DclArrayOfFunctionsRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2](); return 1; }\n"
	},
	//8.3.4 [dcl.array] p3: only the first bound may be left out
	{
		"DclArraySecondBoundMissingRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2][] = { { 1 }, { 2 } }; return a[0][0]; }\n"
	},
	//8.3.4 [dcl.array] p3: a bound left out comes from the initialiser
	{
		"DclArrayNoBoundNoInitialiserRefused", CONFORMANCE_REFUSED,
		"int main() { int a[]; return 1; }\n"
	},
	//8.3.4 [dcl.array]: an array is not a modifiable lvalue
	{
		"DclArrayAssignedRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2 }; int b[2] = { 3, 4 }; a = b; return a[0]; }\n"
	},
	//8.3.5 [dcl.fct]: functions
	{
		"DclFunctionTypes", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef int Binary(int, int);\n"
		"Binary Add;\n"
		"int Add(int a, int b) { return a + b; }\n"
		"int ArrayParameter(int a[10]) { return sizeof(a) == sizeof(int*) ? a[1] : -1; }\n"
		"int FunctionParameter(int f(int), int v) { return f(v); }\n"
		"int Twice(int v) { return v * 2; }\n"
		"int ConstParameter(const int v);\n"
		"int ConstParameter(int v) { return v + 1; }\n"
		"int NoParameters(void) { return 7; }\n"
		"int Unnamed(int, int b) { return b; }\n"
		"auto Trailing(int v) -> long { return v; }\n"
		"int Variadic(int n, ...) { return n; }\n"
		"int (*Returned(int which))(int) { return which ? Twice : 0; }\n"
		"int main()\n"
		"{\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	Binary* p = Add;\n"
		"	int r = 0;\n"
		"	if (Add(1, 2) == 3 && p(3, 4) == 7) r += 1;\n"
		"	if (ArrayParameter(a) == 2 && FunctionParameter(Twice, 4) == 8) r += 2;\n"
		"	if (ConstParameter(1) == 2 && NoParameters() == 7 && Unnamed(1, 2) == 2) r += 4;\n"
		"	if (Trailing(5) == 5 && Same<decltype(Trailing(5)), long>::V && Variadic(3, 1, 2.5, \"x\") == 3) r += 8;\n"
		"	if (Returned(1)(6) == 12 && Returned(0) == 0) r += 16;\n"
		"	if (Same<decltype(ArrayParameter), int(int*)>::V && Same<decltype(FunctionParameter), int(int (*)(int), int)>::V && Same<decltype(ConstParameter), int(int)>::V) r += 32;\n"
		"	if (Same<decltype(NoParameters), int()>::V && Same<Binary, int(int, int)>::V && !Same<int(int), int(long)>::V && Same<decltype(Variadic), int(int, ...)>::V) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.5 [dcl.fct] p6: a member function's const and reference qualifiers
	{
		"DclFunctionMemberTypes", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	int Get() { return v; }\n"
		"	int Get() const { return v + 10; }\n"
		"	int Ref() & { return 1; }\n"
		"	int Ref() && { return 2; }\n"
		"	int Both() const & { return 3; }\n"
		"	typedef int Getter() const;\n"
		"	Getter Typed;\n"
		"};\n"
		"int S::Typed() const { return v + 20; }\n"
		"int main()\n"
		"{\n"
		"	S s = { 1 };\n"
		"	const S cs = { 2 };\n"
		"	int r = 0;\n"
		"	if (s.Get() == 1 && cs.Get() == 12) r += 1;\n"
		"	if (s.Ref() == 1 && S().Ref() == 2 && s.Both() == 3 && S().Both() == 3) r += 2;\n"
		"	if (cs.Typed() == 22 && s.Typed() == 21) r += 4;\n"
		"	if (!Same<int (S::*)(), int (S::*)() const>::V && Same<decltype(&S::Typed), int (S::*)() const>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.5 [dcl.fct] p8: a function returns no array or function
	{
		"DclFunctionReturnsArrayRefused", CONFORMANCE_REFUSED,
		"typedef int Row[3];\n"
		"Row f();\n"
		"int main() { return 1; }\n"
	},
	{
		"DclFunctionReturnsFunctionRefused", CONFORMANCE_REFUSED,
		"typedef int F(int);\n"
		"F g();\n"
		"int main() { return 1; }\n"
	},
	//8.3.5 [dcl.fct] p4: void is the whole parameter list or not there
	{
		"DclVoidParameterWithOthersRefused", CONFORMANCE_REFUSED,
		"int f(void, int v) { return v; }\n"
		"int main() { return f(1); }\n"
	},
	{
		"DclVoidParameterNamedRefused", CONFORMANCE_REFUSED,
		"int f(void v) { return 1; }\n"
		"int main() { return f(); }\n"
	},
	//8.3.5 [dcl.fct]: the return type is not part of what tells overloads apart
	{
		"DclFunctionRedeclaredDifferentReturnRefused", CONFORMANCE_REFUSED,
		"int f(int v);\n"
		"long f(int v) { return v; }\n"
		"int main() { return (int)f(1); }\n"
	},
	//8.3.5 [dcl.fct] p6: only a member function has cv-qualifiers
	{
		"DclConstFreeFunctionRefused", CONFORMANCE_REFUSED,
		"int f() const { return 1; }\n"
		"int main() { return f(); }\n"
	},
	//8.3.6 [dcl.fct.default]: default arguments
	{
		"DclDefaultArguments", 127,
		"int gBase = 10;\n"
		"int gCalls = 0;\n"
		"int Next() { return ++gCalls; }\n"
		"int One(int a, int b = 2, int c = 3) { return a * 100 + b * 10 + c; }\n"
		"int Later(int a, int b);\n"
		"int Later(int a, int b = 5);\n"
		"int Later(int a, int b) { return a + b; }\n"
		"int More(int a, int b, int c = 3);\n"
		"int More(int a, int b = 2, int c);\n"
		"int More(int a, int b, int c) { return a * 100 + b * 10 + c; }\n"
		"int Global(int a = gBase) { return a; }\n"
		"int Counted(int a = Next()) { return a; }\n"
		"int Expression(int a = sizeof(long) * 2 + One(1)) { return a; }\n"
		"struct S { static int s; int m; int Member(int a = s) { return a + m; } static int Static(int a = 7) { return a; } };\n"
		"int S::s = 4;\n"
		"struct B { virtual int F(int a = 1) { return a; } virtual ~B() { } };\n"
		"struct D : B { int F(int a = 2) override { return a + 10; } };\n"
		"template<class T> T Templated(T a, T b = T(6)) { return a + b; }\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	D d;\n"
		"	B* pb = &d;\n"
		"	s.m = 1;\n"
		"	int r = 0;\n"
		"	if (One(1) == 123 && One(1, 5) == 153 && One(1, 5, 6) == 156) r += 1;\n"
		"	if (Later(1) == 6 && Later(1, 2) == 3 && More(1) == 123 && More(1, 4) == 143) r += 2;\n"
		"	gBase = 20;\n"
		"	if (Global() == 20 && Global(3) == 3) r += 4;\n"
		"	if (Counted() == 1 && Counted() == 2 && Counted(9) == 9 && gCalls == 2) r += 8;\n"
		"	if (Expression() == (int)sizeof(long) * 2 + 123 && s.Member() == 5 && s.Member(2) == 3 && S::Static() == 7) r += 16;\n"
		"	if (pb->F() == 11 && d.F() == 12 && pb->F(5) == 15) r += 32;\n"
		"	if (Templated(1) == 7 && Templated(1.5) == 7.5 && Templated(1, 2) == 3) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.6 [dcl.fct.default] p4: after a default argument every parameter has one
	{
		"DclDefaultArgumentGapRefused", CONFORMANCE_REFUSED,
		"int f(int a = 1, int b) { return a + b; }\n"
		"int main() { return f(1, 2); }\n"
	},
	//8.3.6 [dcl.fct.default] p4: a default argument is given once
	{
		"DclDefaultArgumentTwiceRefused", CONFORMANCE_REFUSED,
		"int f(int a = 1);\n"
		"int f(int a = 1) { return a; }\n"
		"int main() { return f(); }\n"
	},
	//8.3.6 [dcl.fct.default] p7: not a local variable
	{
		"DclDefaultArgumentLocalRefused", CONFORMANCE_REFUSED,
		"int main() { int local = 1; struct L { static int f(int a = local) { return a; } }; return L::f(); }\n"
	},
	//8.3.6 [dcl.fct.default] p9: not another parameter
	{
		"DclDefaultArgumentParameterRefused", CONFORMANCE_REFUSED,
		"int f(int a, int b = a) { return a + b; }\n"
		"int main() { return f(1); }\n"
	},
	//8.3.6 [dcl.fct.default] p8: not this
	{
		"DclDefaultArgumentThisRefused", CONFORMANCE_REFUSED,
		"struct S { int m; int f(int a = m) { return a; } };\n"
		"int main() { S s; s.m = 1; return s.f(); }\n"
	},
	//8.3.6 [dcl.fct.default] p5: the default is converted as an argument is
	{
		"DclDefaultArgumentWrongTypeRefused", CONFORMANCE_REFUSED,
		"int f(int* p = 5) { return p ? 1 : 2; }\n"
		"int main() { return f(); }\n"
	},
	//8.4 [dcl.fct.def]: defaulted and deleted functions
	{
		"DclDefaultedAndDeleted", 63,
		"struct D\n"
		"{\n"
		"	int v;\n"
		"	D() = default;\n"
		"	D(int x) : v(x) { }\n"
		"	D(const D&) = default;\n"
		"	D& operator=(const D&) = default;\n"
		"	~D() = default;\n"
		"};\n"
		"struct OutOfClass { int v; OutOfClass(); OutOfClass(const OutOfClass& o); };\n"
		"OutOfClass::OutOfClass() = default;\n"
		"OutOfClass::OutOfClass(const OutOfClass& o) = default;\n"
		"struct NoCopy { int v; NoCopy(int x) : v(x) { } NoCopy(const NoCopy&) = delete; NoCopy& operator=(const NoCopy&) = delete; };\n"
		"int Only(int v) { return v; }\n"
		"int Only(double) = delete;\n"
		"template<class T> int Exact(T) = delete;\n"
		"int Exact(int v) { return v + 1; }\n"
		"const char* Name() { return __func__; }\n"
		"int main()\n"
		"{\n"
		"	D a(3);\n"
		"	D b = a;\n"
		"	D c;\n"
		"	c = b;\n"
		"	D zero = D();\n"
		"	OutOfClass o = OutOfClass();\n"
		"	OutOfClass p = o;\n"
		"	NoCopy n(4);\n"
		"	NoCopy& rn = n;\n"
		"	int r = 0;\n"
		"	if (b.v == 3 && c.v == 3) r += 1;\n"
		"	if (zero.v == 0 && o.v == 0 && p.v == 0) r += 2;\n"
		"	if (rn.v == 4 && Only(5) == 5 && Only('a') == 'a') r += 4;\n"
		"	if (Exact(1) == 2) r += 8;\n"
		"	if (Name()[0] == 'N' && Name()[3] == 'e' && Name()[4] == 0) r += 16;\n"
		"	static_assert(noexcept(D()) && noexcept(D(a)), \"defaulted functions don't throw\");\n"
		"	if (sizeof(D) == sizeof(int)) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//8.4.3 [dcl.fct.def.delete] p2: a deleted function is not used
	{
		"DclDeletedConversionCalledRefused", CONFORMANCE_REFUSED,
		"int Only(int v) { return v; }\n"
		"int Only(double) = delete;\n"
		"int main() { return Only(1.5); }\n"
	},
	{
		"DclDeletedTemplateInstanceRefused", CONFORMANCE_REFUSED,
		"template<class T> int Exact(T) = delete;\n"
		"int Exact(int v) { return v + 1; }\n"
		"int main() { return Exact(1L); }\n"
	},
	//8.4.3 [dcl.fct.def.delete] p4: deleted at its first declaration
	{
		"DclDeletedAfterDeclaredRefused", CONFORMANCE_REFUSED,
		"int f(int v);\n"
		"int f(int v) = delete;\n"
		"int main() { return 1; }\n"
	},
	//8.4.2 [dcl.fct.def.default] p1: only a special member function is defaulted
	{
		"DclDefaultedNotSpecialRefused", CONFORMANCE_REFUSED,
		"struct S { int f() = default; };\n"
		"int main() { return 1; }\n"
	},
	//8.4.2 [dcl.fct.def.default] p1: a defaulted function has no default arguments
	{
		"DclDefaultedWithDefaultArgumentRefused", CONFORMANCE_REFUSED,
		"struct S { S(int v = 1) = default; };\n"
		"int main() { return 1; }\n"
	},
	//8.4.1 [dcl.fct.def.general], 3.2 [basic.def.odr]: one definition
	{
		"DclFunctionDefinedTwiceRefused", CONFORMANCE_REFUSED,
		"int f() { return 1; }\n"
		"int f() { return 2; }\n"
		"int main() { return f(); }\n"
	},
	//8.5 [dcl.init] p6-p8: zero, default and value initialisation
	{
		"DclZeroAndDefaultInitialisation", 63,
		"struct P { int a; int* p; double d; char c[3]; };\n"
		"struct C { C() : v(5) { } int v; };\n"
		"struct M { int a = 3; int b; };\n"
		"struct Outer { P p; C c; int tail; };\n"
		"int gi;\n"
		"int* gp;\n"
		"double gd;\n"
		"P gs;\n"
		"int ga[3];\n"
		"static long sl;\n"
		"int Statics() { static int s; static P sp; return s + sp.a + (sp.p ? 1 : 0); }\n"
		"int main()\n"
		"{\n"
		"	C c;\n"
		"	C ca[2];\n"
		"	int r = 0;\n"
		"	if (gi == 0 && gp == 0 && gd == 0.0 && sl == 0 && ga[2] == 0) r += 1;\n"
		"	if (gs.a == 0 && gs.p == 0 && gs.d == 0.0 && gs.c[2] == 0 && Statics() == 0) r += 2;\n"
		"	if (c.v == 5 && ca[0].v == 5 && ca[1].v == 5) r += 4;\n"
		"	int vi = int();\n"
		"	int* vp = &vi;\n"
		"	P vs = P();\n"
		"	Outer vo = Outer();\n"
		"	if (vi == 0 && *vp == 0 && vs.a == 0 && vs.p == 0 && vs.c[1] == 0) r += 8;\n"
		"	if (vo.p.a == 0 && vo.p.d == 0.0 && vo.c.v == 5 && vo.tail == 0) r += 16;\n"
		"	M m = M();\n"
		"	int braced{};\n"
		"	P bracedObject{};\n"
		"	int bracedArray[3]{};\n"
		"	if (m.a == 3 && m.b == 0 && braced == 0 && bracedObject.d == 0.0 && bracedArray[2] == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5 [dcl.init] p7: a const object is initialised
	{
		"DclConstNeedsInitialiserRefused", CONFORMANCE_REFUSED,
		"int main() { const int x; return 1; }\n"
	},
	{
		"DclConstClassNeedsConstructorRefused", CONFORMANCE_REFUSED,
		"struct P { int a; };\n"
		"int main() { const P p; return 1; }\n"
	},
	//8.5 [dcl.init] p7: a const object of a class with a default constructor
	{
		"DclConstClassWithConstructor", 5,
		"struct C { C() : v(5) { } int v; };\n"
		"int main() { const C c; return c.v; }\n"
	},
	//8.5 [dcl.init] p15-p17: direct and copy initialisation
	{
		"DclDirectAndCopyInitialisation", 63,
		"struct FromInt { FromInt(int x) : v(x) { } int v; };\n"
		"struct Explicit { explicit Explicit(int x) : v(x) { } int v; };\n"
		"struct ToInt { operator int() const { return 7; } };\n"
		"struct Wrapper { Wrapper(FromInt f) : v(f.v + 100) { } int v; };\n"
		"int Take(FromInt f) { return f.v; }\n"
		"FromInt Give() { return 9; }\n"
		"int main()\n"
		"{\n"
		"	FromInt a = 1;\n"
		"	FromInt b(2);\n"
		"	FromInt c = FromInt(3);\n"
		"	FromInt d{ 4 };\n"
		"	FromInt e = { 5 };\n"
		"	Explicit x(6);\n"
		"	Explicit y{ 7 };\n"
		"	Explicit z = Explicit(8);\n"
		"	ToInt t;\n"
		"	int fromObject = t;\n"
		"	int direct(t);\n"
		"	long widened = t;\n"
		"	Wrapper w = FromInt(1);\n"
		"	Wrapper w2(2);\n"
		"	int r = 0;\n"
		"	if (a.v == 1 && b.v == 2 && c.v == 3 && d.v == 4 && e.v == 5) r += 1;\n"
		"	if (x.v == 6 && y.v == 7 && z.v == 8) r += 2;\n"
		"	if (fromObject == 7 && direct == 7 && widened == 7) r += 4;\n"
		"	if (Take(10) == 10 && Take(a) == 1 && Give().v == 9) r += 8;\n"
		"	if (w.v == 101 && w2.v == 102) r += 16;\n"
		"	int i(3.9);\n"
		"	int j = 3.9;\n"
		"	char ch = 65;\n"
		"	double dd = 1;\n"
		"	bool flag = 5;\n"
		"	if (i == 3 && j == 3 && ch == 'A' && dd == 1.0 && flag == true) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5 [dcl.init] p17: an explicit constructor is not used to copy initialise
	{
		"DclCopyInitialisationExplicitRefused", CONFORMANCE_REFUSED,
		"struct Explicit { explicit Explicit(int x) : v(x) { } int v; };\n"
		"int main() { Explicit e = 1; return e.v; }\n"
	},
	//8.5 [dcl.init] p17: one user-defined conversion
	{
		"DclCopyInitialisationTwoConversionsRefused", CONFORMANCE_REFUSED,
		"struct FromInt { FromInt(int x) : v(x) { } int v; };\n"
		"struct Wrapper { Wrapper(FromInt f) : v(f.v) { } int v; };\n"
		"int main() { Wrapper w = 1; return w.v; }\n"
	},
	//8.5 [dcl.init] p11: empty parentheses declare a function
	{
		"DclFunctionNotObject", 3,
		"struct C { C() : v(5) { } int v; };\n"
		"int main()\n"
		"{\n"
		"	C notAnObject();\n"
		"	C object;\n"
		"	C braced{};\n"
		"	int r = 0;\n"
		"	if (sizeof(notAnObject()) == sizeof(C)) r += 1;\n"
		"	if (object.v == 5 && braced.v == 5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.1 [dcl.init.aggr]: aggregates
	{
		"DclAggregates", 255,
		"struct P { int a; int b; };\n"
		"struct Nested { P p; int c; P q[2]; };\n"
		"struct WithDefaults { int a = 1; int b = a + 1; int c; };\n"
		"struct WithStatic { int a; static int s; int b; };\n"
		"int WithStatic::s = 9;\n"
		"union U { int i; char c; };\n"
		"struct WithReference { int& r; int v; };\n"
		"struct Empty { };\n"
		"struct HoldsEmpty { Empty e; int v; };\n"
		"int main()\n"
		"{\n"
		"	int gi = 5;\n"
		"	P full = { 1, 2 };\n"
		"	P partial = { 1 };\n"
		"	P none = { };\n"
		"	int a[5] = { 1, 2 };\n"
		"	Nested n = { { 1, 2 }, 3, { { 4, 5 }, { 6, 7 } } };\n"
		"	Nested elided = { 1, 2, 3, 4, 5, 6, 7 };\n"
		"	Nested part = { { 1 }, 3 };\n"
		"	P pa[] = { { 1, 2 }, { 3, 4 }, { 5 } };\n"
		"	P flat[] = { 1, 2, 3, 4 };\n"
		"	WithDefaults d = { };\n"
		"	WithDefaults d2 = { 10 };\n"
		"	WithDefaults d3 = { 10, 20, 30 };\n"
		"	WithStatic ws = { 1, 2 };\n"
		"	U u = { 65 };\n"
		"	WithReference wr = { gi, 2 };\n"
		"	HoldsEmpty he = { { }, 4 };\n"
		"	int r = 0;\n"
		"	if (full.a == 1 && full.b == 2 && partial.a == 1 && partial.b == 0 && none.a == 0 && none.b == 0) r += 1;\n"
		"	if (a[1] == 2 && a[2] == 0 && a[4] == 0) r += 2;\n"
		"	if (n.p.b == 2 && n.c == 3 && n.q[0].a == 4 && n.q[1].b == 7) r += 4;\n"
		"	if (elided.p.b == 2 && elided.c == 3 && elided.q[1].a == 6 && part.p.b == 0 && part.c == 3 && part.q[1].b == 0) r += 8;\n"
		"	if (sizeof(pa) == 3 * sizeof(P) && pa[2].a == 5 && pa[2].b == 0 && sizeof(flat) == 2 * sizeof(P) && flat[1].a == 3) r += 16;\n"
		"	if (d.a == 1 && d.b == 2 && d.c == 0 && d2.a == 10 && d2.b == 11 && d3.b == 20 && d3.c == 30) r += 32;\n"
		"	if (ws.a == 1 && ws.b == 2 && WithStatic::s == 9 && u.i == 65 && u.c == 'A') r += 64;\n"
		"	wr.r = 8;\n"
		"	if (gi == 8 && wr.v == 2 && he.v == 4) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.1 [dcl.init.aggr] p6: no more initialisers than members
	{
		"DclAggregateTooManyRefused", CONFORMANCE_REFUSED,
		"struct P { int a; int b; };\n"
		"int main() { P p = { 1, 2, 3 }; return p.a; }\n"
	},
	//8.5.1 [dcl.init.aggr] p6: no more initialisers than elements
	{
		"DclArrayTooManyRefused", CONFORMANCE_REFUSED,
		"int main() { int a[2] = { 1, 2, 3 }; return a[0]; }\n"
	},
	//8.5.1 [dcl.init.aggr] p1: a class with a base is not an aggregate
	{
		"DclAggregateWithBaseRefused", CONFORMANCE_REFUSED,
		"struct B { int b; };\n"
		"struct D : B { int d; };\n"
		"int main() { D d = { 1, 2 }; return d.d; }\n"
	},
	//8.5.1 [dcl.init.aggr] p1: a class with a constructor is not an aggregate
	{
		"DclAggregateWithConstructorRefused", CONFORMANCE_REFUSED,
		"struct C { C() : a(0), b(0) { } int a; int b; };\n"
		"int main() { C c = { 1, 2 }; return c.a; }\n"
	},
	//8.5.1 [dcl.init.aggr] p1: a class with private data is not an aggregate
	{
		"DclAggregatePrivateMemberRefused", CONFORMANCE_REFUSED,
		"class C { int a; public: int b; };\n"
		"int main() { C c = { 1, 2 }; return c.b; }\n"
	},
	//8.5.1 [dcl.init.aggr] p9: a reference member is given something to refer to
	{
		"DclAggregateReferenceMissingRefused", CONFORMANCE_REFUSED,
		"struct W { int v; int& r; };\n"
		"int main() { W w = { 1 }; return w.v; }\n"
	},
	//8.5.2 [dcl.init.string]: character arrays
	{
		"DclCharacterArrays", 63,
		"int main()\n"
		"{\n"
		"	char a[] = \"abc\";\n"
		"	char b[6] = \"ab\";\n"
		"	char c[] = { \"xyz\" };\n"
		"	char d[4] = \"abc\";\n"
		"	wchar_t w[] = L\"hi\";\n"
		"	char16_t u[] = u\"ok\";\n"
		"	char32_t U[3] = U\"\";\n"
		"	const char e[] = \"a\" \"b\" \"c\";\n"
		"	unsigned char uc[] = \"ab\";\n"
		"	signed char sc[3] = \"x\";\n"
		"	int r = 0;\n"
		"	if (sizeof(a) == 4 && a[2] == 'c' && a[3] == 0) r += 1;\n"
		"	if (sizeof(b) == 6 && b[1] == 'b' && b[2] == 0 && b[5] == 0) r += 2;\n"
		"	if (sizeof(c) == 4 && c[0] == 'x' && sizeof(d) == 4 && d[3] == 0) r += 4;\n"
		"	if (sizeof(w) == 3 * sizeof(wchar_t) && w[1] == L'i' && w[2] == 0 && sizeof(u) == 3 * sizeof(char16_t) && u[0] == u'o') r += 8;\n"
		"	if (sizeof(U) == 3 * sizeof(char32_t) && U[0] == 0 && U[2] == 0 && sizeof(e) == 4 && e[1] == 'b') r += 16;\n"
		"	a[0] = 'z';\n"
		"	if (a[0] == 'z' && sizeof(uc) == 3 && uc[1] == 'b' && sc[0] == 'x' && sc[2] == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.2 [dcl.init.string] p2: no more characters than elements
	{
		"DclCharacterArrayTooLongRefused", CONFORMANCE_REFUSED,
		"int main() { char a[3] = \"abc\"; return a[0]; }\n"
	},
	//8.5.2 [dcl.init.string] p1: the string's kind is the array's
	{
		"DclCharacterArrayWideMismatchRefused", CONFORMANCE_REFUSED,
		"int main() { char a[] = L\"abc\"; return a[0]; }\n"
	},
	{
		"DclWideArrayNarrowStringRefused", CONFORMANCE_REFUSED,
		"int main() { wchar_t a[] = \"abc\"; return a[0]; }\n"
	},
	//8.5.2 [dcl.init.string] p1: an array of a character type
	{
		"DclIntArrayFromStringRefused", CONFORMANCE_REFUSED,
		"int main() { int a[] = \"abc\"; return a[0]; }\n"
	},
	//8.5.3 [dcl.init.ref]: references
	{
		"DclReferenceBinding", 255,
		"struct B { int b; };\n"
		"struct D : B { int d; };\n"
		"struct ToRef { int v; operator int&() { return v; } };\n"
		"struct ToValue { operator long() const { return 9; } };\n"
		"int gCount = 0;\n"
		"struct T { T(int x) : v(x) { gCount += 10; } ~T() { gCount += 1; } int v; };\n"
		"int Value() { return 6; }\n"
		"int main()\n"
		"{\n"
		"	int i = 1;\n"
		"	D d;\n"
		"	d.b = 2; d.d = 3;\n"
		"	ToRef tr = { 4 };\n"
		"	ToValue tv;\n"
		"	int r = 0;\n"
		"	int& ri = i;\n"
		"	B& rb = d;\n"
		"	const B& crb = d;\n"
		"	int& fromConversion = tr;\n"
		"	if (&ri == &i && &rb == static_cast<B*>(&d) && crb.b == 2) r += 1;\n"
		"	fromConversion = 40;\n"
		"	if (tr.v == 40) r += 2;\n"
		"	const int& toTemporary = 5;\n"
		"	const long& widened = i;\n"
		"	const int& fromCall = Value();\n"
		"	const long& viaOperator = tv;\n"
		"	i = 100;\n"
		"	if (toTemporary == 5 && widened == 1 && fromCall == 6 && viaOperator == 9) r += 4;\n"
		"	int&& rvalue = 7;\n"
		"	int&& fromSum = i + 1;\n"
		"	int&& moved = static_cast<int&&>(i);\n"
		"	rvalue++;\n"
		"	moved = 50;\n"
		"	if (rvalue == 8 && fromSum == 101 && i == 50 && &moved == &i) r += 8;\n"
		"	{\n"
		"		const T& kept = T(3);\n"
		"		T&& alsoKept = T(4);\n"
		"		if (gCount == 20 && kept.v == 3 && alsoKept.v == 4) r += 16;\n"
		"	}\n"
		"	if (gCount == 22) r += 32;\n"
		"	const int& chain = ri;\n"
		"	const volatile int& cv = i;\n"
		"	int a[2] = { 1, 2 };\n"
		"	int (&ra)[2] = a;\n"
		"	const int (&cra)[2] = a;\n"
		"	int (&rf)() = Value;\n"
		"	if (&chain == &i && &cv == &i && &ra == &a && cra[1] == 2 && rf() == 6) r += 64;\n"
		"	B&& fromDerived = D();\n"
		"	const B& slicedTemporary = D();\n"
		"	(void)fromDerived; (void)slicedTemporary;\n"
		"	const D& cd = d;\n"
		"	const B& base = cd;\n"
		"	if (base.b == 2 && &base == static_cast<const B*>(&d)) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.3 [dcl.init.ref] p5: an lvalue reference to non-const binds to an lvalue
	{
		"DclReferenceToRvalueRefused", CONFORMANCE_REFUSED,
		"int main() { int& r = 5; return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: an lvalue reference to non-const binds to its own type
	{
		"DclReferenceToOtherTypeRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; long& r = i; return (int)r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: the reference is at least as qualified
	{
		"DclReferenceDropsConstRefused", CONFORMANCE_REFUSED,
		"int main() { const int i = 1; int& r = i; return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: an rvalue reference binds to an rvalue
	{
		"DclRvalueReferenceToLvalueRefused", CONFORMANCE_REFUSED,
		"int main() { int i = 1; int&& r = i; return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: a non-const reference does not bind to a bit-field
	{
		"DclReferenceToBitFieldRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 3; };\n"
		"int main() { S s; s.b = 1; int& r = s.b; return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: reference-compatible types
	{
		"DclReferenceToBaseFromUnrelatedRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B { int b; };\n"
		"int main() { A a; B& r = a; return 1; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: an lvalue reference to non-const binds to an lvalue
	{
		"DclReferenceToCallValueRefused", CONFORMANCE_REFUSED,
		"int Value() { return 6; }\n"
		"int main() { int& r = Value(); return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: a const reference to a bit-field is to a copy
	{
		"DclConstReferenceToBitField", 3,
		"struct S { int b : 4; };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.b = 3;\n"
		"	const int& r = s.b;\n"
		"	s.b = 5;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.4 [dcl.init.list]: list initialisation
	{
		"DclListInitialisation", 255,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct P { int a; int b; };\n"
		"struct C { C() : v(1) { } C(int a) : v(a) { } C(int a, int b) : v(a * 10 + b) { } int v; };\n"
		"P MakeP() { return { 3, 4 }; }\n"
		"C MakeC() { return { 5, 6 }; }\n"
		"int SumP(P p) { return p.a + p.b; }\n"
		"int ValueOf(C c) { return c.v; }\n"
		"int main()\n"
		"{\n"
		"	int a{ 5 };\n"
		"	int b = { 6 };\n"
		"	int c{ };\n"
		"	double d{ 1.5 };\n"
		"	char ch{ 'x' };\n"
		"	long widened{ a };\n"
		"	int* null{ };\n"
		"	P p{ 1, 2 };\n"
		"	P q = { 3 };\n"
		"	C none{ };\n"
		"	C one{ 7 };\n"
		"	C two{ 8, 9 };\n"
		"	C copy = { 2, 3 };\n"
		"	int arr[]{ 1, 2, 3 };\n"
		"	P parr[2]{ { 1, 2 }, { 3, 4 } };\n"
		"	const int k = 100;\n"
		"	char fits{ k };\n"
		"	unsigned char fromConstant{ 200 };\n"
		"	float fromInt{ 16 };\n"
		"	int r = 0;\n"
		"	if (a == 5 && b == 6 && c == 0 && d == 1.5 && ch == 'x' && widened == 5 && null == 0) r += 1;\n"
		"	if (p.b == 2 && q.a == 3 && q.b == 0) r += 2;\n"
		"	if (none.v == 1 && one.v == 7 && two.v == 89 && copy.v == 23) r += 4;\n"
		"	if (sizeof(arr) == 3 * sizeof(int) && arr[2] == 3 && parr[1].a == 3) r += 8;\n"
		"	if (MakeP().b == 4 && MakeC().v == 56 && SumP({ 1, 2 }) == 3 && ValueOf({ 4, 5 }) == 45 && ValueOf({ }) == 1) r += 16;\n"
		"	if (fits == 100 && fromConstant == 200 && fromInt == 16.0f) r += 32;\n"
		"	a = { 9 };\n"
		"	p = { 7, 8 };\n"
		"	two = { 1, 1 };\n"
		"	if (a == 9 && p.a == 7 && two.v == 11 && int{ 3 } == 3 && P{ 5, 6 }.b == 6 && C{ 1, 2 }.v == 12) r += 64;\n"
		"	int* fresh = new int{ 12 };\n"
		"	P* freshP = new P{ 13, 14 };\n"
		"	if (*fresh == 12 && freshP->b == 14) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.4 [dcl.init.list] p7: no narrowing
	{
		"DclListNarrowingDoubleRefused", CONFORMANCE_REFUSED,
		"int main() { int x{ 2.5 }; return x; }\n"
	},
	{
		"DclListNarrowingVariableRefused", CONFORMANCE_REFUSED,
		"int main() { long long l = 5; int x{ l }; return x; }\n"
	},
	{
		"DclListNarrowingConstantRefused", CONFORMANCE_REFUSED,
		"int main() { unsigned char c{ 300 }; return c; }\n"
	},
	{
		"DclListNarrowingNegativeRefused", CONFORMANCE_REFUSED,
		"int main() { unsigned int u{ -1 }; return u ? 1 : 2; }\n"
	},
	{
		"DclListNarrowingToFloatRefused", CONFORMANCE_REFUSED,
		"int main() { double d = 1.5; float f{ d }; return f > 1 ? 1 : 2; }\n"
	},
	{
		"DclListNarrowingIntToFloatRefused", CONFORMANCE_REFUSED,
		"int main() { long long big = 1; float f{ big }; return f > 0 ? 1 : 2; }\n"
	},
	{
		"DclListNarrowingInAggregateRefused", CONFORMANCE_REFUSED,
		"struct P { char a; int b; };\n"
		"int main() { int i = 300; P p = { i, 2 }; return p.b; }\n"
	},
	{
		"DclListNarrowingArgumentRefused", CONFORMANCE_REFUSED,
		"int Take(int v) { return v; }\n"
		"int main() { return Take({ 2.5 }); }\n"
	},
	//8.5.4 [dcl.init.list] p3: a scalar from a list of one
	{
		"DclListTooManyForScalarRefused", CONFORMANCE_REFUSED,
		"int main() { int x{ 1, 2 }; return x; }\n"
	},
	//8.5.4 [dcl.init.list] p3, 13.3.1.7: copy list initialisation does not use an explicit constructor
	{
		"DclListExplicitCopyRefused", CONFORMANCE_REFUSED,
		"struct E { explicit E(int a, int b) : v(a + b) { } int v; };\n"
		"int main() { E e = { 1, 2 }; return e.v; }\n"
	},
	//8.5.4 [dcl.init.list] p3: a constructor takes the list's elements
	{
		"DclListNoConstructorRefused", CONFORMANCE_REFUSED,
		"struct C { C(int a) : v(a) { } int v; };\n"
		"int main() { C c{ 1, 2, 3 }; return c.v; }\n"
	},
	//8.5.4 [dcl.init.list] p3: an empty list is value initialisation
	{
		"DclListEmptyNoDefaultRefused", CONFORMANCE_REFUSED,
		"struct C { C(int a) : v(a) { } int v; };\n"
		"int main() { C c{ }; return c.v; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: an object whose conversion function gives the lvalue
	{
		"DclReferenceConversionFunction", 3,
		"struct A { int a; };\n"
		"struct W { int v; A o; operator int&() { return v; } operator A&() { return o; } };\n"
		"int main()\n"
		"{\n"
		"	W w;\n"
		"	w.v = 1; w.o.a = 2;\n"
		"	int& ri = w;\n"
		"	A& ra = w;\n"
		"	ri = 5; ra.a = 6;\n"
		"	int r = 0;\n"
		"	if (w.v == 5) r += 1;\n"
		"	if (w.o.a == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.3 [dcl.init.ref] p4: reference-compatible with a base it has once
	{
		"DclReferenceToRepeatedBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A& r = d; return 1; }\n"
	},
	//8.5.3 [dcl.init.ref] p4, 11.2: the base is accessible
	{
		"DclReferenceToPrivateBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct P : private A { };\n"
		"int main() { P p; const A& r = p; return 1; }\n"
	},
	//8.5.3 [dcl.init.ref] p4: reference-compatible with a base it has once
	{
		"DclReferenceArgumentRepeatedBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int Take(A& r) { return 1; }\n"
		"int main() { D d; return Take(d); }\n"
	},
	//8.5.3 [dcl.init.ref] p5: an int& does not bind to an enum
	{
		"DclReferenceToEnumRefused", CONFORMANCE_REFUSED,
		"enum E { X, Y };\n"
		"int main() { E e = Y; int& r = e; return r; }\n"
	},
	//8.5.3 [dcl.init.ref] p5: char and signed char are different types
	{
		"DclReferenceToOtherCharRefused", CONFORMANCE_REFUSED,
		"int main() { char c = 1; signed char& r = c; return r; }\n"
	},
	//8.5 [dcl.init] p17: an object from one of a derived class, by the base it has once
	{
		"DclCopyFromRepeatedBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A a = d; return 1; }\n"
	},
	//8.5 [dcl.init] p17, 11.2: the base is accessible
	{
		"DclCopyFromPrivateBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct P : private A { };\n"
		"int main() { P p; A a(p); return 1; }\n"
	},
	//8.5 [dcl.init] p17: an object from one of a derived class, by the base it has once
	{
		"DclReturnCopyRepeatedBaseRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"A Give(D& d) { return d; }\n"
		"int main() { D d; return Give(d).a; }\n"
	},
	//8.5.3 [dcl.init.ref] p5, 13.3.1.6 [over.match.ref]: references and parameters bound through conversion functions
	{
		"DclReferenceConversionResults", 255,
		"struct A { int a; };\n"
		"struct B : A { int b; };\n"
		"struct W\n"
		"{\n"
		"	int pad; int v; B o; long l;\n"
		"	operator int&() { return v; }\n"
		"	operator B&() { return o; }\n"
		"	operator const long&() const { return l; }\n"
		"};\n"
		"struct N { int n; operator int() const { return n; } };\n"
		"int TakeI(int& r) { r += 1; return r; }\n"
		"int TakeA(const A& r) { return r.a; }\n"
		"int TakeB(B& r) { r.b += 1; return r.b; }\n"
		"int TakeC(const int& r) { return r; }\n"
		"int TakeR(int&& r) { return r; }\n"
		"int& GiveI(W& w) { return w; }\n"
		"int main()\n"
		"{\n"
		"	W w;\n"
		"	N n;\n"
		"	w.pad = 100; w.v = 1; w.o.a = 2; w.o.b = 3; w.l = 7; n.n = 9;\n"
		"	const W& cw = w;\n"
		"	const long& rl = cw;\n"
		"	A& ra = w;\n"
		"	const int& rc = n;\n"
		"	int&& rr = n;\n"
		"	long x = cw;\n"
		"	short s = 5;\n"
		"	int&& rs = s;\n"
		"	int r = 0;\n"
		"	if (TakeI(w) == 2 && w.v == 2 && w.pad == 100) r += 1;\n"
		"	if (TakeA(w) == 2 && TakeB(w) == 4 && w.o.b == 4) r += 2;\n"
		"	if (rl == 7 && x == 7) r += 4;\n"
		"	if (&ra == &w.o && ra.a == 2) r += 8;\n"
		"	GiveI(w) = 4;\n"
		"	if (w.v == 4) r += 16;\n"
		"	if (rc == 9 && rr == 9 && TakeC(n) == 9 && TakeR(n) == 9) r += 32;\n"
		"	if (TakeC(w) == 4) r += 64;\n"
		"	rs = 6;\n"
		"	if (s == 5 && rs == 6) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.5 [dcl.fct] p6: only a member function is volatile
	{
		"DclVolatileFreeFunctionRefused", CONFORMANCE_REFUSED,
		"int f() volatile { return 1; }\n"
		"int main() { return f(); }\n"
	},
	//8.2 [dcl.ambig.res] p1: an initialiser in brackets that begins as a parameter does, and a declarator whose name is in brackets
	{
		"DclAmbiguityInitialisers", 3,
		"struct S { S(int x) : v(x) { } S(int x, int y) : v(x + y) { } int v; S Plus(int n) const { return S(v + n); } };\n"
		"typedef int Int;\n"
		"int ga = 4;\n"
		"int Twice(int v) { return v * 2; }\n"
		"int main()\n"
		"{\n"
		"	int a = 3;\n"
		"	S p(int(a) * 2);\n"
		"	S q(Int(a) + ga, int(ga));\n"
		"	S t(S(a).Plus(1));\n"
		"	S u(Twice(Int(a)));\n"
		"	S (w)(7);\n"
		"	Int (k)(a + 1);\n"
		"	Int f(Int(a));\n"
		"	int r = 0;\n"
		"	if (p.v == 6 && q.v == 11 && t.v == 4 && u.v == 6) r += 1;\n"
		"	if (w.v == 7 && k == 4 && sizeof(f(1)) == sizeof(int)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.5 [dcl.fct] p10: functions, member functions and a parameter declared by the name of a function type
	{
		"DclFunctionsByTypedef", 7,
		"using Unary = int(int);\n"
		"typedef Unary Other;\n"
		"Unary Neg;\n"
		"Other Twice;\n"
		"extern Unary Declared;\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	typedef int Getter() const;\n"
		"	typedef int Setter(int);\n"
		"	Getter Get;\n"
		"	Setter Set;\n"
		"	static Unary Half;\n"
		"	virtual Getter Virtual;\n"
		"};\n"
		"int S::Get() const { return v; }\n"
		"int S::Set(int x) { v = x; return v; }\n"
		"int S::Half(int x) { return x / 2; }\n"
		"int S::Virtual() const { return v + 1; }\n"
		"int Neg(int v) { return -v; }\n"
		"int Twice(int v) { return v * 2; }\n"
		"int Declared(int v) { return v + 100; }\n"
		"int Apply(Unary f, int v) { return f(v); }\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.v = 0;\n"
		"	Unary* p = Neg;\n"
		"	int r = 0;\n"
		"	if (Neg(3) == -3 && Twice(4) == 8 && p(5) == -5 && Declared(1) == 101) r += 1;\n"
		"	if (s.Set(6) == 6 && s.Get() == 6 && S::Half(8) == 4 && s.Virtual() == 7) r += 2;\n"
		"	if (Apply(Twice, 5) == 10 && Apply(S::Half, 6) == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.6 [dcl.fct.default] p7: a default argument names a global, and a local where it is not evaluated
	{
		"DclDefaultArgumentUnevaluatedLocal", 1,
		"int g = 4;\n"
		"int main() { const int c = 3; struct L { static int f(int a = g, int n = sizeof(c)) { return a + n; } }; return L::f() == 4 + (int)sizeof(int); }\n"
	},
	//8.3.6 [dcl.fct.default] p7: not a static local variable either
	{
		"DclDefaultArgumentStaticLocalRefused", CONFORMANCE_REFUSED,
		"int main() { static int s = 2; struct L { static int f(int a = s) { return a; } }; return L::f(); }\n"
	},
	//8.4.2 [dcl.fct.def.default] p5, 12.8 [class.copy]: a copy constructor and a copy assignment defaulted after their class are functions, called by what was written before
	{
		"DclDefaultedOutsideClass", 7,
		"int gCopies = 0;\n"
		"struct M { int v; M() : v(1) { } M(const M& o) : v(o.v) { gCopies++; } M& operator=(const M& o) { v = o.v; gCopies += 10; return *this; } };\n"
		"struct S { M m; int n; S(); S(const S&); S& operator=(const S&); virtual int f() { return 1; } };\n"
		"int CopyEarly(const S& s) { S t = s; return t.m.v + t.n; }\n"
		"S::S() : n(2) { }\n"
		"S::S(const S&) = default;\n"
		"S& S::operator=(const S&) = default;\n"
		"int main()\n"
		"{\n"
		"	S a;\n"
		"	a.m.v = 5;\n"
		"	int r = 0;\n"
		"	if (CopyEarly(a) == 7 && gCopies == 1) r += 1;\n"
		"	S b;\n"
		"	S& same = (b = a);\n"
		"	if (b.m.v == 5 && b.n == 2 && gCopies == 11 && b.f() == 1 && &same == &b) r += 2;\n"
		"	S c = b;\n"
		"	if (c.m.v == 5 && c.n == 2 && gCopies == 12 && c.f() == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.1 [dcl.init.aggr] p7, p12, p16: braces left out of nested aggregates, default member initialisers that name earlier members, a union's first member
	{
		"DclAggregateElision", 127,
		"struct P { int a; int b; };\n"
		"struct Nested { P p; int c; P q[2]; };\n"
		"struct Deep { Nested n; char name[4]; P last; };\n"
		"struct WithDefaults { int a = 1; int b = a + 1; int c = b * 2; };\n"
		"union U { int i; char c; };\n"
		"struct HasUnion { U u; int v; };\n"
		"P MakeP() { P p = { 8, 9 }; return p; }\n"
		"Nested gn = { 1, 2, 3, 4, 5, 6, 7 };\n"
		"P gflat[] = { 1, 2, 3, 4, 5 };\n"
		"WithDefaults gd = { 10 };\n"
		"int gm[2][3] = { 1, 2, 3, 4 };\n"
		"int main()\n"
		"{\n"
		"	P p = { 3, 4 };\n"
		"	Deep d = { 1, 2, 3, 4, 5, 6, 7, \"abc\", p };\n"
		"	Deep e = { { p, 3, MakeP(), 6, 7 }, { 'x', 'y' }, 8 };\n"
		"	Nested arr[2] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };\n"
		"	int m[2][3] = { 1, 2, 3, 4 };\n"
		"	WithDefaults w = { 5 };\n"
		"	WithDefaults w2 = { 5, 7 };\n"
		"	HasUnion hu = { 65, 2 };\n"
		"	int r = 0;\n"
		"	if (gn.p.b == 2 && gn.c == 3 && gn.q[1].a == 6 && sizeof(gflat) == 3 * sizeof(P) && gflat[2].a == 5 && gflat[2].b == 0) r += 1;\n"
		"	if (d.n.q[1].b == 7 && d.name[2] == 'c' && d.name[3] == 0 && d.last.b == 4) r += 2;\n"
		"	if (e.n.p.a == 3 && e.n.c == 3 && e.n.q[0].b == 9 && e.n.q[1].a == 6 && e.name[1] == 'y' && e.name[2] == 0 && e.last.a == 8 && e.last.b == 0) r += 4;\n"
		"	if (arr[0].q[1].b == 7 && arr[1].p.a == 8 && arr[1].p.b == 9 && arr[1].c == 0) r += 8;\n"
		"	if (m[0][2] == 3 && m[1][0] == 4 && m[1][2] == 0 && gm[1][0] == 4 && gm[1][1] == 0) r += 16;\n"
		"	if (w.b == 6 && w.c == 12 && w2.b == 7 && w2.c == 14 && gd.a == 10 && gd.b == 11 && gd.c == 22) r += 32;\n"
		"	if (hu.u.i == 65 && hu.v == 2) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.1 [dcl.init.aggr] p6, p16: more values than the elements take, braces left out
	{
		"DclAggregateTooManyElidedRefused", CONFORMANCE_REFUSED,
		"struct P { int a; int b; };\n"
		"int main() { P a[1] = { 1, 2, 3 }; return a[0].a; }\n"
	},
	//8.5.1 [dcl.init.aggr] p6, p16: more values than the members take, braces left out
	{
		"DclAggregateTooManyNestedRefused", CONFORMANCE_REFUSED,
		"struct P { int a; int b; };\n"
		"struct N { P p; int c; };\n"
		"int main() { N n = { 1, 2, 3, 4 }; return n.c; }\n"
	},
	//8.5.1 [dcl.init.aggr] p6, p16: a union is given one value, its first member's
	{
		"DclUnionTwoValuesRefused", CONFORMANCE_REFUSED,
		"union U { int i; char c; };\n"
		"int main() { U u = { 1, 2 }; return u.i; }\n"
	},
};


//Clause 14, templates [temp].
static const SConformanceCase gasConformanceClause14[] =
{
	//14.5.5.1 [temp.class.spec.match], 14.8.2.5 [temp.deduct.type] p8: a partial specialisation's arguments are matched as types of the forms T(T...), T(*)(T...), T T::*, T (T::*)(T...)
	{
		"TempSpecialisationFunctionTypes", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { };\n"
		"template<class T> struct Kind { enum { V = 0 }; };\n"
		"template<class R, class... A> struct Kind<R(A...)> { enum { V = 1, N = sizeof...(A) }; };\n"
		"template<class R, class... A> struct Kind<R(A..., ...)> { enum { V = 2 }; };\n"
		"template<class R, class... A> struct Kind<R (*)(A...)> { enum { V = 3 }; };\n"
		"template<class R, class C, class... A> struct Kind<R (C::*)(A...)> { enum { V = 4 }; };\n"
		"template<class R, class C, class... A> struct Kind<R (C::*)(A...) const> { enum { V = 5 }; };\n"
		"template<class M, class C> struct Kind<M C::*> { enum { V = 6 }; };\n"
		"template<class T> struct Second;\n"
		"template<class R, class A> struct Second<R(int, A)> { typedef A Type; typedef R Result; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Kind<int(int)>::V == 1 && Kind<void(int, char)>::N == 2 && Kind<void()>::N == 0) r += 1;\n"
		"	if (Kind<int(int, ...)>::V == 2) r += 2;\n"
		"	if (Kind<int (*)(long)>::V == 3) r += 4;\n"
		"	if (Kind<int (S::*)(int)>::V == 4) r += 8;\n"
		"	if (Kind<int (S::*)() const>::V == 5) r += 16;\n"
		"	if (Kind<int S::*>::V == 6) r += 32;\n"
		"	if (Kind<int>::V == 0 && Kind<int*>::V == 0 && Kind<S>::V == 0) r += 64;\n"
		"	if (Same<Second<long(int, char*)>::Type, char*>::V && Same<Second<long(int, char*)>::Result, long>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.5.1 [temp.class.spec.match] p2, 14.8.2.5 [temp.deduct.type] p5: an argument its parameters can't be deduced from is the type they give it, and a specialisation they give no type isn't chosen
	{
		"TempSpecialisationNonDeduced", 127,
		"template<class...> using Void = void;\n"
		"template<class T, class = void> struct HasType { enum { V = 0 }; };\n"
		"template<class T> struct HasType<T, Void<typename T::Type>> { enum { V = 1 }; };\n"
		"template<bool B, class T = void> struct EnableIf { };\n"
		"template<class T> struct EnableIf<true, T> { typedef T Type; };\n"
		"template<class T, class = void> struct IsSmall { enum { V = 0 }; };\n"
		"template<class T> struct IsSmall<T, typename EnableIf<(sizeof(T) == 1)>::Type> { enum { V = 1 }; };\n"
		"template<class T> struct Identity { typedef T Type; };\n"
		"template<class T, class U> struct Pair { enum { V = 0 }; };\n"
		"template<class T> struct Pair<T, typename Identity<T>::Type*> { enum { V = 1 }; };\n"
		"struct A { typedef int Type; long l; };\n"
		"struct B { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (HasType<A>::V == 1) r += 1;\n"
		"	if (HasType<B>::V == 0) r += 2;\n"
		"	if (HasType<int>::V == 0) r += 4;\n"
		"	if (IsSmall<char>::V == 1 && IsSmall<A>::V == 0) r += 8;\n"
		"	if (IsSmall<long>::V == 0) r += 16;\n"
		"	if (Pair<int, int*>::V == 1 && Pair<A, A*>::V == 1) r += 32;\n"
		"	if (Pair<int, char*>::V == 0 && Pair<int, int>::V == 0) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.5 [temp.class.spec], 14 [temp] p1: a variable template is partially specialised as a class template is
	{
		"TempVariablePartialSpecialisation", 63,
		"template<class T> int gKind = 1;\n"
		"template<class T> int gKind<T*> = 2;\n"
		"template<class T, int N> int gKind<T[N]> = N;\n"
		"template<> int gKind<char> = 16;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (gKind<int> == 1) r += 1;\n"
		"	if (gKind<int*> == 2) r += 2;\n"
		"	if (gKind<long[8]> == 8) r += 4;\n"
		"	if (gKind<char> == 16) r += 8;\n"
		"	if (gKind<char*> == 2) r += 16;\n"
		"	gKind<int*> = 7;\n"
		"	if (gKind<int*> == 7 && gKind<char*> == 2 && gKind<int> == 1) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.3 [temp.expl.spec] p1, p13: a static data member of a class template is explicitly specialised for an instance
	{
		"TempStaticMemberExplicitSpecialisation", 63,
		"template<class T> struct S { static int k; static const int c; int Get() { return k; } };\n"
		"template<class T> int S<T>::k = 1;\n"
		"template<class T> const int S<T>::c = 3;\n"
		"template<> int S<int>::k = 5;\n"
		"template<> const int S<char>::c = 9;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S<int> s;\n"
		"	if (S<int>::k == 5) r += 1;\n"
		"	if (S<char>::k == 1) r += 2;\n"
		"	if (S<long>::k == 1) r += 4;\n"
		"	if (s.Get() == 5) r += 8;\n"
		"	if (S<char>::c == 9 && S<int>::c == 3) r += 16;\n"
		"	S<int>::k = 6;\n"
		"	if (S<char>::k == 1 && S<int>::k == 6) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p5, 12.1 [class.ctor], 12.8 [class.copy] p6: a constructor template's instance constructs and converts, and never copies
	{
		"TempConstructorTemplate", 1023,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[1024];\n"
		"size_t guiHeap = 0;\n"
		"size_t guiLastSize = 0;\n"
		"int giNews = 0;\n"
		"int giDeletes = 0;\n"
		"void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }\n"
		"void* operator new(size_t n) { return Allocate(n); }\n"
		"void* operator new[](size_t n) { return Allocate(n); }\n"
		"void operator delete(void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p) noexcept { if (p) giDeletes++; }\n"
		"void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }\n"
		"struct S { int v; template<class T> S(T t) : v((int)t + 1) { } };\n"
		"struct P { int a; int b; P() : a(0), b(0) { } template<class T, class U> P(T t, U u); };\n"
		"template<class T, class U> P::P(T t, U u) : a((int)t), b((int)sizeof(U)) { }\n"
		"template<class T> struct Box { T v; template<class U> Box(U u) : v((T)u * 2) { } Box(const Box& o) : v(o.v + 100) { } };\n"
		"struct Exact { int v; Exact(int i) : v(i) { } template<class T> Exact(T t) : v(-1) { } };\n"
		"int Take(S s) { return s.v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s(3L);\n"
		"	if (s.v == 4) r += 1;\n"
		"	S t = 'a';\n"
		"	if (t.v == 'a' + 1) r += 2;\n"
		"	S u(s);\n"
		"	if (u.v == 4) r += 4;\n"
		"	P p(7, 'c');\n"
		"	if (p.a == 7 && p.b == 1) r += 8;\n"
		"	P q;\n"
		"	if (q.a == 0) r += 16;\n"
		"	Box<long> b(21);\n"
		"	if (b.v == 42) r += 32;\n"
		"	Box<long> c(b);\n"
		"	if (c.v == 142) r += 64;\n"
		"	if (Take(5) == 6) r += 128;\n"
		"	S* h = new S(9);\n"
		"	if (h->v == 10) r += 256;\n"
		"	Exact e(4);\n"
		"	Exact f(4L);\n"
		"	if (e.v == 4 && f.v == -1) r += 512;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem], 8.5.1 [dcl.init.aggr] p1: a class that declares a constructor template is no aggregate, and has no default constructor
	{
		"TempConstructorTemplateNoAggregateRefused", CONFORMANCE_REFUSED,
		"struct S { int v; template<class T> S(T t, T u) : v((int)t) { } };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	return s.v;\n"
		"}\n"
	},
	//14.5.7 [temp.alias] p2, 14.8.2.5 [temp.deduct.type]: an alias template's name with arguments is the type it names, which template arguments are deduced from
	{
		"TempDeduceThroughAlias", 7,
		"template<class T> struct V { T v; };\n"
		"template<class T> using A = V<T>;\n"
		"template<class T> using Ptr = T*;\n"
		"template<class T, class U> using Second = V<U>;\n"
		"template<class T> T Get(A<T>& a) { return a.v; }\n"
		"template<class T> T Read(Ptr<T> p) { return *p; }\n"
		"template<class U> U Other(Second<int, U>* p) { return p->v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	V<int> v;\n"
		"	V<char> w;\n"
		"	long l = 7;\n"
		"	v.v = 5;\n"
		"	w.v = 'x';\n"
		"	if (Get(v) == 5) r += 1;\n"
		"	if (Read(&l) == 7 && sizeof(Read(&l)) == sizeof(long)) r += 2;\n"
		"	if (Other(&w) == 'x' && sizeof(Other(&w)) == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2.5 [temp.deduct.type] p8: a template template parameter is deduced from TT<T>
	{
		"TempDeduceTemplateTemplate", 7,
		"template<class T> struct V { T v; };\n"
		"template<class T, class U> struct Two { T a; U b; };\n"
		"template<template<class> class C, class T> T Get(C<T>& c) { return c.v; }\n"
		"template<template<class, class> class C, class T, class U> int Sizes(C<T, U>* p) { return (int)(sizeof(T) * 10 + sizeof(U)); }\n"
		"template<template<class> class C, class T> C<T*> Rebind(C<T>& c) { C<T*> r; r.v = &c.v; return r; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	V<int> v;\n"
		"	Two<char, char[3]> p;\n"
		"	v.v = 6;\n"
		"	if (Get(v) == 6) r += 1;\n"
		"	if (Sizes(&p) == 13) r += 2;\n"
		"	V<int*> w = Rebind(v);\n"
		"	if (*w.v == 6) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
};
