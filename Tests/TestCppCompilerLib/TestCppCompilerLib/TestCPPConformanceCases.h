//The conformance cases (TestCPPConformance.cpp): small programs, one a feature of the language, in the stages of
//Part B of CppCompilerLib/CppParserTodo.txt (section 6).  A case's value is what its main returns, and was found by
//compiling and running it with the Microsoft compiler (Conformance/hostcheck.py does it again for cases
//exported with TEST_CPP_CONFORMANCE_EXPORT); CONFORMANCE_REFUSED is a program a compiler must refuse.  Values are
//1 to 32767, so that they are an int's on every target and a program that returns 0 by accident isn't right.


//What the compiler did before Part B was started, and must go on doing.
static const SConformanceCase gasConformanceWorks[] =
{
	{
		"NamespaceBasic", 3,
		"namespace N { int f(int x) { return x + 1; } int v = 2; }\n"
		"int main() { return N::f(N::v); }\n"
	},
	{
		"NamespaceUsing", 8,
		"namespace N { int f(int x) { return x + 1; } int g(int x) { return x * 3; } }\n"
		"using namespace N;\n"
		"using N::g;\n"
		"int main() { return f(1) + g(2); }\n"
	},
	{
		"NamespaceNestedAlias", 8,
		"namespace A { namespace B { int v = 4; } }\n"
		"namespace C = A::B;\n"
		"int main() { return C::v + A::B::v; }\n"
	},
	{
		"NamespaceAnonymous", 8,
		"namespace { int v = 4; int f() { return v; } }\n"
		"int main() { return f() + v; }\n"
	},
	{
		"NamespaceADL", 3,
		"namespace N { struct S { int v; }; int get(S s) { return s.v; } }\n"
		"int main() { N::S s; s.v = 3; return get(s); }\n"
	},
	{
		"NamespaceClassOutside", 5,
		"namespace N { struct S { int f(); }; }\n"
		"int N::S::f() { return 5; }\n"
		"int main() { N::S s; return s.f(); }\n"
	},
	{
		"NamespaceInline", 12,
		"namespace N { inline namespace V1 { int f() { return 6; } } }\n"
		"int main() { return N::f() + N::V1::f(); }\n"
	},
	{
		"ConstMemberOverload", 12,
		"struct S { int f() { return 1; } int f() const { return 2; } };\n"
		"int main() { S s; const S c = S(); return s.f() * 10 + c.f(); }\n"
	},
	{
		"ConstPointerOverload", 12,
		"int f(int* p) { return 1; }\n"
		"int f(const int* p) { return 2; }\n"
		"int main() { int a = 0; const int b = 0; return f(&a) * 10 + f(&b); }\n"
	},
	{
		"ConstSpecialisation", 21,
		"template<class T> struct IsConst { enum { V = 1 }; };\n"
		"template<class T> struct IsConst<const T> { enum { V = 2 }; };\n"
		"int main() { return IsConst<const int>::V * 10 + IsConst<int>::V; }\n"
	},
	{
		"ConstAssigned", CONFORMANCE_REFUSED,
		"int main() { const int c = 1; c = 2; return c; }\n"
	},
	{
		"ConstPointerWrite", CONFORMANCE_REFUSED,
		"int main() { int a = 0; const int* p = &a; *p = 1; return a; }\n"
	},
	{
		"ConstTakenAway", CONFORMANCE_REFUSED,
		"int main() { const int a = 0; int* p = &a; return *p; }\n"
	},
	{
		"ConstObjectChanged", CONFORMANCE_REFUSED,
		"struct S { int v; void set(int a) { v = a; } };\n"
		"int main() { const S s = S(); s.set(1); return 0; }\n"
	},
	{
		"MutableMember", 2,
		"struct S { mutable int v; explicit S(int a) : v(a) {} int f() const { v++; return v; } };\n"
		"int main() { const S s(1); return s.f(); }\n"
	},
	{
		"ExplicitConstructorImplicit", CONFORMANCE_REFUSED,
		"struct S { explicit S(int) {} };\n"
		"int f(S) { return 1; }\n"
		"int main() { return f(3); }\n"
	},
	{
		"ReferenceToValue", CONFORMANCE_REFUSED,
		"int f(int& r) { return r; }\n"
		"int main() { return f(3); }\n"
	},
	{
		"UndeclaredName", CONFORMANCE_REFUSED,
		"int main() { return nothere; }\n"
	},
	{
		"AmbiguousCall", CONFORMANCE_REFUSED,
		"int f(unsigned char) { return 1; }\n"
		"int f(char) { return 2; }\n"
		"int main() { return f(1); }\n"
	},
	{
		"ConstructorTemplate", 3,
		"struct S { int v; template<class T> S(T t) : v((int)t) {} };\n"
		"int main() { S s(3L); return s.v; }\n"
	},
	{
		"MemberFunctionTemplate", 6,
		"struct S { template<class T> T twice(T t) { return t + t; } };\n"
		"int main() { S s; return s.twice(2) + (int)s.twice<long>(1); }\n"
	},
	{
		"DependentTypename", 3,
		"template<class T> struct A { typedef T Type; };\n"
		"template<class T> typename A<T>::Type f(T t) { typename A<T>::Type u = t; return u; }\n"
		"int main() { return f(3); }\n"
	},
	{
		"FunctionTemplateDefaults", 6,
		"template<class T = int> T f() { return 4; }\n"
		"template<class T, class U = T> U g(T t) { return (U)t; }\n"
		"int main() { return f() + g(2); }\n"
	},
	{
		"FunctionSpecialisation", 12,
		"template<class T> int f(T) { return 1; }\n"
		"template<> int f<char>(char) { return 2; }\n"
		"int main() { return f(1) * 10 + f('c'); }\n"
	},
	{
		"TemplateFriend", 2,
		"template<class T> class B { T v; template<class U> friend U get(const B<U>& b); public: B() : v(2) {} };\n"
		"template<class U> U get(const B<U>& b) { return b.v; }\n"
		"int main() { B<int> b; return get(b); }\n"
	},
	{
		"TemplateDefaultArgument", 4,
		"template<class T, int N = 4> struct A { T a[N]; };\n"
		"int main() { A<char> a; return sizeof(a); }\n"
	},
	{
		"TemplatePointerSpecialisation", 12,
		"template<class T> struct A { enum { V = 1 }; };\n"
		"template<class T> struct A<T*> { enum { V = 2 }; };\n"
		"int main() { return A<int>::V * 10 + A<int*>::V; }\n"
	},
	{
		"TemplateLateLookup", 2,
		"template<class T> int f(T t) { return g(t); }\n"
		"struct S {};\n"
		"int g(S) { return 2; }\n"
		"int main() { S s; return f(s); }\n"
	},
	{
		"TemplateRightAngles", 1,
		"template<class T> struct A { T v; };\n"
		"int main() { A<A<int>> a; a.v.v = 1; return a.v.v; }\n"
	},
	{
		"ExternTemplate", 1,
		"template<class T> struct S { T f() { return 1; } };\n"
		"extern template struct S<int>;\n"
		"template struct S<int>;\n"
		"int main() { S<int> s; return s.f(); }\n"
	},
	{
		"LocalTypeArgument", 2,
		"template<class T> int sz(T) { return sizeof(T); }\n"
		"int main() { struct L { char a; char b; }; L l; return sz(l); }\n"
	},
	{
		"VariadicCall", 14,
		"int g(int a, int b, int c) { return a + b * 2 + c * 3; }\n"
		"template<class... A> int f(A... a) { return g(a...); }\n"
		"int main() { return f(1, 2, 3); }\n"
	},
	{
		"VariadicRecursion", 102,
		"int sum() { return 0; }\n"
		"template<class T, class... R> int sum(T t, R... r) { return (int)t + sum(r...); }\n"
		"int main() { return sum(1, 2L, 'c'); }\n"
	},
	{
		"MemberPointerData", 3,
		"struct S { int a; int b; };\n"
		"int main() { int S::*p = &S::b; S s; s.a = 1; s.b = 3; return s.*p; }\n"
	},
	{
		"MemberPointerFunction", 12,
		"struct S { int v; int get() { return v; } virtual int vget() { return v + 1; } };\n"
		"int main() { int (S::*p)() = &S::get; int (S::*q)() = &S::vget; S s; s.v = 1; S* ps = &s; return (s.*p)() * 10 + (ps->*q)(); }\n"
	},
	{
		"OverloadedFunctionAddress", 2,
		"int f(char) { return 1; }\n"
		"int f(long) { return 2; }\n"
		"int main() { int (*p)(long) = f; return p(0); }\n"
	},
	{
		"AnonymousUnion", 14,
		"struct S { union { long i; char c[4]; }; };\n"
		"int main() { S s; s.i = 0x01020304; return s.c[0] + s.c[3] * 10; }\n"
	},
	{
		"LocalClass", 2,
		"int main() { struct L { int v; int get() { return v; } }; L l; l.v = 2; return l.get(); }\n"
	},
	{
		"BitFields", 23,
		"struct S { unsigned a : 3; unsigned b : 5; };\n"
		"int main() { S s; s.a = 5; s.b = 17; s.a++; return s.a + s.b; }\n"
	},
	{
		"WideString", 98,
		"int main() { const wchar_t* w = L\"ab\"; return w[1]; }\n"
	},
	{
		"MultipleInheritanceVirtual", 34,
		"struct A { virtual int f() { return 1; } };\n"
		"struct B { virtual int g() { return 2; } };\n"
		"struct C : A, B { int f() override { return 3; } int g() override { return 4; } };\n"
		"int main() { C c; B* b = &c; A* a = &c; return a->f() * 10 + b->g(); }\n"
	},
	{
		"Friend", 2,
		"class S { int v; friend int get(const S& s); public: S() : v(2) {} };\n"
		"int get(const S& s) { return s.v; }\n"
		"int main() { S s; return get(s); }\n"
	},
	{
		"UnionWithConstructor", 2,
		"struct M { int v; M() : v(1) {} M(int a) : v(a) {} };\n"
		"union U { M m; int i; U() : m(2) {} ~U() {} };\n"
		"int main() { U u; return u.m.v; }\n"
	},
	{
		"BraceInitialisation", 10,
		"struct S { int a; int b; };\n"
		"int main() { S s{1, 2}; S t = {3, 4}; int x{5}; return s.a + t.b + x; }\n"
	},
	{
		"BraceConstructor", 16,
		"struct S { int v; S(int a, int b) : v(a + b * 2) {} };\n"
		"S make() { return {1, 2}; }\n"
		"int main() { S s{3, 4}; return s.v + make().v; }\n"
	},
	{
		"MemberInitialisers", 23,
		"struct S { int a = 3; int b{4}; int c; S() : c(5) {} S(int x) : a(x), c(6) {} };\n"
		"int main() { S s; S t(1); return s.a + s.b + s.c + t.a + t.b + t.c; }\n"
	},
	{
		"AliasDeclaration", 2,
		"using Int = int;\n"
		"using Fn = int (*)(int);\n"
		"int f(Int x) { return x + 1; }\n"
		"int main() { Fn p = f; return p(1); }\n"
	},
	{
		"EnumClass", 6,
		"enum class E : unsigned char { A, B = 5 };\n"
		"int main() { E e = E::B; return (int)e + sizeof(E); }\n"
	},
	{
		"EnumClassUnscopedName", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { return (int)B; }\n"
	},
	{
		"EnumFixedType", 3,
		"enum E : short;\n"
		"enum E : short { A = 1 };\n"
		"int main() { return sizeof(E) + A; }\n"
	},
	{
		"OverrideFinal", 2,
		"struct A { virtual int f() { return 1; } };\n"
		"struct B final : A { int f() final { return 2; } };\n"
		"int main() { B b; A* a = &b; return a->f(); }\n"
	},
	{
		"Alignment", 88,
		"struct alignas(8) S { char c; };\n"
		"int main() { return alignof(S) * 10 + sizeof(S); }\n"
	},
	{
		"ExplicitConversionOperator", 21,
		"struct S { int v; explicit operator bool() const { return v != 0; } };\n"
		"int main() { S s; s.v = 1; S t; t.v = 0; return (s ? 1 : 0) + (t ? 10 : 20); }\n"
	},
	{
		"InheritedConstructor", 3,
		"struct A { int v; A(int a) : v(a) {} };\n"
		"struct B : A { using A::A; };\n"
		"int main() { B b(3); return b.v; }\n"
	},
	{
		"DelegatingConstructor", 4,
		"struct S { int v; S(int a) : v(a) {} S() : S(4) {} };\n"
		"int main() { S s; return s.v; }\n"
	},
	{
		"IntegerTypes", 196,
		"int main() { long long a = 1LL << 40; char16_t c = u'a'; char32_t d = U'b'; return (int)(a >> 40) + c + d; }\n"
	},
	{
		"NullPointer", 221,
		"struct S {};\n"
		"S* f() { return nullptr; }\n"
		"int main() { int* p = nullptr; S* q = f(); return (p == nullptr) + (q != nullptr ? 10 : 20) + (q ? 100 : 200); }\n"
	},
	{
		"Operators", 47,
		"struct V { int x; V operator+(const V& o) const { V r; r.x = x + o.x; return r; } V& operator+=(const V& o) { x += o.x; return *this; } bool operator==(const V& o) const { return x == o.x; } int operator[](int i) const { return x + i; } V operator-() const { V r; r.x = -x; return r; } };\n"
		"V operator*(const V& a, int k) { V r; r.x = a.x * k; return r; }\n"
		"int main() { V a; a.x = 1; V b = a + a; b += a; V c = -(b * 2); return (a == a) + b[1] * 10 - c.x; }\n"
	},
	{
		"OperatorChain", 6,
		"struct S { int v; S& operator=(int a) { v = a; return *this; } S& operator<<(int a) { v += a; return *this; } };\n"
		"int main() { S s; s = 1; s << 2 << 3; return s.v; }\n"
	},
	{
		"ReturnedObjects", 6,
		"struct S { int a[4]; S() { a[0] = 1; a[3] = 5; } };\n"
		"S make() { S s; return s; }\n"
		"int main() { S s = make(); S t; t = make(); return s.a[0] + t.a[3]; }\n"
	},
	{
		"TemporariesDestroyed", 22,
		"int g = 0;\n"
		"struct T { ~T() { g++; } int v() const { return 10; } };\n"
		"T make() { return T(); }\n"
		"int main() { int r = make().v() + T().v(); return r + g; }\n"
	},
	{
		"VirtualThroughReferenceName", 1 + 2 + 4 + 8 + 16,	//10.3/7: a reference's name isn't an object's
		"struct B { virtual int f() const { return 1; } int operator+(int i) const { return g() + i; } virtual int g() const { return 10; } virtual ~B() {} };\n"
		"struct D : B { int f() const override { return 17; } int g() const override { return 20; } };\n"
		"struct H { const B& r; B o; H(const B& b) : r(b), o(b) {} int viaRef() const { return r.f(); } int viaObject() const { return o.f(); } };\n"
		"int byRef(const B& b) { return b.f(); }\n"
		"int byValue(B b) { return b.f(); }\n"
		"int main()\n"
		"{\n"
		"	D d; const B& rb = d; H h(d);\n"
		"	int r = 0;\n"
		"	r += (byRef(d) == 17) * 1;\n"
		"	r += (rb.f() == 17) * 2;\n"
		"	r += (byValue(d) == 1 && d.f() == 17) * 4;\n"
		"	r += (h.viaRef() == 17 && h.viaObject() == 1) * 8;\n"
		"	r += (rb + 1 == 21) * 16;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"DynamicCastAccess", 1 + 2 + 4 + 8 + 16 + 32,	//5.2.7/8: public bases only
		"struct A { virtual ~A() {} int a; };\n"
		"struct D : A { int d; };\n"
		"struct P : private D { int p; A* base() { return this; } D* middle() { return this; } };\n"
		"struct R { virtual ~R() {} int r; };\n"
		"struct M : D, R { int m; };\n"
		"struct N : D, private R { int n; };\n"
		"int main()\n"
		"{\n"
		"	D d; P p; M m; N n;\n"
		"	A* ad = &d; A* ap = p.base(); A* am = &m; A* an = &n;\n"
		"	int r = 0;\n"
		"	r += (dynamic_cast<D*>(ad) == &d) * 1;\n"				//a public base of a D
		"	r += (dynamic_cast<D*>(ap) == p.middle()) * 2;\n"		//still a public base of a D, which is a private part of a P
		"	r += (dynamic_cast<P*>(ap) == 0) * 4;\n"					//not a public base of the P
		"	r += (dynamic_cast<R*>(am) == (R*)&m) * 8;\n"			//across, both public
		"	r += (dynamic_cast<R*>(an) == 0) * 16;\n"				//across to a private base
		"	r += (dynamic_cast<void*>(ap) == (void*)&p) * 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"TypeidOfOtherTypes", 1 + 2 + 4 + 8 + 16 + 32,	//5.2.8: a pointer's, an enum's and nullptr's type are types like any
		"#ifdef _MSC_VER\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"enum E { e1 }; enum class F { f1 };\n"
		"struct S { int m; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0; int* p = 0; const int* q = 0; E e = e1;\n"
		"	r += (typeid(p) == typeid(int*)) * 1;\n"
		"	r += (typeid(q) != typeid(p)) * 2;\n"
		"	r += (typeid(e) == typeid(E) && typeid(E) != typeid(F) && typeid(E) != typeid(int)) * 4;\n"
		"	r += (typeid(nullptr) == typeid(decltype(nullptr)) && typeid(nullptr) != typeid(void*)) * 8;\n"
		"	r += (typeid(&S::m) == typeid(int S::*) && typeid(int S::*) != typeid(const int S::*)) * 16;\n"
		"	r += (typeid(int**) != typeid(int*) && typeid(int[3]) != typeid(int[4])) * 32;\n"
		"	return r;\n"
		"}\n"
	},
	{
		"TemporariesReverseOrder", 3,	//12.2/3: destroyed in the reverse of the order their construction finished
		"int g = 0;\n"
		"struct T { int n; T(int i) : n(i) {} ~T() { g = g * 10 + n; } };\n"
		"struct S { int n; S(const T& t) : n(t.n + 1) {} ~S() { g = g * 10 + n; } };\n"
		"int f(const S& s) { return s.n; }\n"
		"int main() { int r = f(S(T(1))); return (g == 21) + 2 * (r == 2); }\n"
	},
	{
		"ConditionalTemporaryOrder", 3,	//12.2/3, in an operand that isn't always worked out
		"int g = 0;\n"
		"struct T { int n; T(int i) : n(i) {} ~T() { g = g * 10 + n; } };\n"
		"struct S { int n; S(const T& t) : n(t.n + 1) {} ~S() { g = g * 10 + n; } };\n"
		"int f(const S& s) { return s.n; }\n"
		"int main() { int r = 0; for (int i = 0; i < 2; i++) { r += (i == 1 && f(S(T(3))) == 4) ? 5 : 1; } return (g == 43) + 2 * (r == 6); }\n"
	},
	{
		"ArraysOfObjects", 33,
		"int g = 0;\n"
		"struct T { int v; T() : v(++g) {} ~T() { g--; } };\n"
		"int f() { T a[3]; return a[2].v * 10 + g; }\n"
		"int main() { return f() + g; }\n"
	},
	{
		"GlobalObjects", 8,
		"struct T { int v; T() : v(4) {} };\n"
		"T gt;\n"
		"T ga[2];\n"
		"int main() { return gt.v + ga[1].v; }\n"
	},
	{
		"SwitchLongLong", 7,
		"int main() { long long v = 1LL << 33; switch (v >> 33) { case 1: return 7; default: return 0; } }\n"
	},
	{
		"LongLongArithmetic", 315,
		"int main() { unsigned long long a = 0xFFFFFFFFFFULL; a >>= 4; a *= 3; a /= 7; a %= 1000; return (int)a; }\n"
	},
	{
		"StaticMembers", 44,
		"struct S { static const int N = 4; static int f() { return N; } char a[N]; };\n"
		"int main() { return S::f() * 10 + sizeof(S); }\n"
	},
	{
		"BaseInitialisationOrder", 13,
		"int g = 0;\n"
		"struct A { int a; A(int x) : a(x + g) { g = 10; } };\n"
		"struct B { int b; B(int x) : b(x + g) { g = 20; } };\n"
		"struct C : A, B { int c; C() : B(2), A(1), c(a + b) {} };\n"
		"int main() { C c; return c.c; }\n"
	},
	{
		"ExplicitDestructor", 9,
		"#ifdef _MSC_VER\n"
		"#include <new>\n"
		"#endif\n"
		"int g = 0;\n"
		"struct S { int v; ~S() { g = v; } };\n"
		"int main() { char buf[8]; S* p = new (buf) S; p->v = 9; p->~S(); return g; }\n"
	},
	{
		"FunctionPointerArray", 23,
		"typedef int (*Fn)(int);\n"
		"int a(int x) { return x; }\n"
		"int b(int x) { return x + 1; }\n"
		"int main() { Fn t[2] = { a, b }; return t[1](1) * 10 + t[0](3); }\n"
	},
	{
		"Loops", 8,
		"int main() { int i = 0; int s = 0; do { i++; if (i == 2) { continue; } s += i; } while (i < 4); for (;;) { break; } switch (i) { case 4: goto done; default: break; } s = 99; done: return s; }\n"
	},
	{
		"AttributesIgnored", 33,
		"[[noreturn]] void die();\n"
		"[[nodiscard]] int f() { return 1; }\n"
		"struct [[deprecated]] S { int v; };\n"
		"[[deprecated(\"old\")]] int g() { return 2; }\n"
		"int main() { [[maybe_unused]] int u = 0; int i = 1; switch (i) { case 1: i++; [[fallthrough]]; case 2: i++; } return i * 10 + f() + g(); }\n"
	},
	{
		"SizedDelete", 1,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacArena[256];\n"
		"size_t guiArena = 0;\n"
		"void* Take(size_t n) { void* p = &gacArena[guiArena]; guiArena += (n + 7) & ~(size_t)7; return p; }\n"
		"int gSize = 0;\n"
		"void* operator new(size_t n) { return Take(n); }\n"
		"void operator delete(void* p) noexcept {}\n"
		"void operator delete(void* p, size_t n) noexcept { gSize = (int)n; }\n"
		"struct S { char c[6]; };\n"
		"int main() { S* p = new S; delete p; return gSize == 6 || gSize == 0; }\n"
	},
	{
		"NoexceptSpecification", 3,
		"int f() noexcept { return 1; }\n"
		"int g() throw() { return 2; }\n"
		"int main() { return f() + g(); }\n"
	},
	{
		"VolatileObjects", 6,
		"volatile int gv = 3;\n"
		"int f(volatile int* p) { return *p + *p; }\n"
		"int main() { return f(&gv); }\n"
	},
};


//Stage 1: what was accepted and wrong, and small things refused.
static const SConformanceCase gasConformanceStage1[] =
{
	{
		"StaticAssertTrue", 7,
		"static_assert(sizeof(long) == 4, \"long\");\n"
		"int main() { static_assert(1 == 1, \"one\"); return 7; }\n"
	},
	{
		"StaticAssertFalse", CONFORMANCE_REFUSED,
		"static_assert(1 == 2, \"no\");\n"
		"int main() { return 0; }\n"
	},
	{
		"StaticAssertFalseInFunction", CONFORMANCE_REFUSED,
		"int main() { static_assert(sizeof(char) == 2, \"no\"); return 0; }\n"
	},
	{
		"StaticAssertFalseInClass", CONFORMANCE_REFUSED,
		"struct S { static_assert(sizeof(char) == 2, \"no\"); };\n"
		"int main() { return 0; }\n"
	},
	{
		"DeletedFunctionCalled", CONFORMANCE_REFUSED,
		"void f(int) = delete;\n"
		"int main() { f(1); return 0; }\n"
	},
	{
		"DeletedCopy", CONFORMANCE_REFUSED,
		"struct S { S() {} S(const S&) = delete; };\n"
		"int main() { S a; S b(a); return 0; }\n"
	},
	{
		"DeletedAssignment", CONFORMANCE_REFUSED,
		"struct S { S() {} S& operator=(const S&) = delete; };\n"
		"int main() { S a; S b; b = a; return 0; }\n"
	},
	{
		"DeletedOverloadNotChosen", 3,
		"void f(char) = delete;\n"
		"int f(long) { return 3; }\n"
		"int main() { return f(1L); }\n"
	},
	{
		"AbstractObject", CONFORMANCE_REFUSED,
		"struct A { virtual int f() = 0; };\n"
		"int main() { A a; return 0; }\n"
	},
	{
		"AbstractObjectInherited", CONFORMANCE_REFUSED,
		"struct A { virtual int f() = 0; };\n"
		"struct B : A { int g() { return 1; } };\n"
		"int main() { B b; return 0; }\n"
	},
	{
		"AbstractImplemented", 4,
		"struct A { virtual int f() = 0; };\n"
		"struct B : A { int f() { return 4; } };\n"
		"int main() { B b; A* a = &b; return a->f(); }\n"
	},
	{
		"PrivateMember", CONFORMANCE_REFUSED,
		"class S { int v; };\n"
		"int main() { S s; s.v = 1; return 0; }\n"
	},
	{
		"PrivateFunction", CONFORMANCE_REFUSED,
		"class S { int f() { return 1; } };\n"
		"int main() { S s; return s.f(); }\n"
	},
	{
		"ProtectedMember", CONFORMANCE_REFUSED,
		"class S { protected: int v; };\n"
		"int main() { S s; s.v = 1; return 0; }\n"
	},
	{
		"PrivateFromDerived", CONFORMANCE_REFUSED,
		"class A { int v; };\n"
		"class B : public A { public: int f() { return v; } };\n"
		"int main() { B b; return b.f(); }\n"
	},
	{
		"PrivateConstructor", CONFORMANCE_REFUSED,
		"class S { S() {} };\n"
		"int main() { S s; return 0; }\n"
	},
	{
		"PrivateBaseMember", CONFORMANCE_REFUSED,
		"struct A { int v; };\n"
		"class B : A { };\n"
		"int main() { B b; b.v = 1; return 0; }\n"
	},
	{
		"PrivateStatic", CONFORMANCE_REFUSED,
		"class S { static int v; };\n"
		"int S::v = 1;\n"
		"int main() { return S::v; }\n"
	},
	{
		"PrivateNestedType", CONFORMANCE_REFUSED,
		"class S { struct I { int v; }; };\n"
		"int main() { S::I i; i.v = 1; return i.v; }\n"
	},
	{
		"AccessAllowed", 3121,
		"class A { int a; protected: int b; public: A() : a(1), b(2) {} int geta() { return a; } friend int peek(A& x); friend class F; };\n"
		"class B : public A { public: int getb() { return b; } };\n"
		"class F { public: int get(A& x) { return x.a + x.b; } };\n"
		"int peek(A& x) { return x.a; }\n"
		"int main() { B b; F f; return b.geta() + b.getb() * 10 + peek(b) * 100 + f.get(b) * 1000; }\n"
	},
	{
		"AccessStaticAndNested", 7,
		"class S { static int v; struct I { int w; }; public: static int get() { I i; i.w = 2; return v + i.w; } };\n"
		"int S::v = 5;\n"
		"int main() { return S::get(); }\n"
	},
	{
		"EnumClassToInt", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { int i = E::B; return i; }\n"
	},
	{
		"EnumClassComparedWithInt", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { E e = E::B; return e == 1; }\n"
	},
	{
		"EnumClassArithmetic", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { E e = E::B; return e + 1; }\n"
	},
	{
		"IntToEnumClass", CONFORMANCE_REFUSED,
		"enum class E { A, B };\n"
		"int main() { E e = 1; return 0; }\n"
	},
	{
		"EnumClassCast", 152,
		"enum class E { A, B = 5 };\n"
		"int main() { E e = E::B; E f = (E)2; return (int)e * 10 + static_cast<int>(f) + (e == E::B) * 100 + (e < f) * 1000; }\n"
	},
	{
		"NarrowingVariable", CONFORMANCE_REFUSED,
		"int main() { long l = 70000; char c{l}; return c; }\n"
	},
	{
		"NarrowingFloating", CONFORMANCE_REFUSED,
		"int main() { int i{2.5}; return i; }\n"
	},
	{
		"NotNarrowing", 196,
		"struct S { char c; long l; };\n"
		"int main() { char c{65}; S s{66, 3}; long l{c}; return c + s.c + (int)l; }\n"
	},
	{
		"NullPointerArgument", 11,
		"int f(int* p) { return p == 0; }\n"
		"struct S {};\n"
		"int g(S* p) { return p == 0; }\n"
		"int main() { return f(nullptr) + g(nullptr) * 10; }\n"
	},
	{
		"NullPointerOverload", 12,
		"int f(int* p) { return 1; }\n"
		"int f(int i) { return 2; }\n"
		"int main() { return f(nullptr) * 10 + f(0); }\n"
	},
	{
		"AttributeOnParameter", 5,
		"int f([[maybe_unused]] int x, [[maybe_unused]] int y) { return 5; }\n"
		"int main() { return f(0, 1); }\n"
	},
	{
		"SizeofMember", 43,
		"struct S { char a; long b; char c[3]; };\n"
		"int main() { return sizeof(S::b) * 10 + sizeof(S::c); }\n"
	},
	{
		"BracedTemporaryArgument", 61,
		"struct S { int a; int b; };\n"
		"int f(S s) { return s.a + s.b * 2; }\n"
		"int main() { return f(S{1, 2}) * 10 + f(S{3, 4}); }\n"
	},
	{
		"CallOperator", 12,
		"struct V { int x; int operator()(int a, int b) const { return a + b + x; } };\n"
		"int main() { V a; a.x = 1; return a(5, 6); }\n"
	},
	{
		"CallOperatorOnCopy", 24,
		"struct V { int x; int operator()(int a, int b) const { return a + b + x; } V operator-() const { V r; r.x = -x; return r; } };\n"
		"V operator*(const V& a, int k) { V r; r.x = a.x * k; return r; }\n"
		"int main() { V b; b.x = 3; V c = -(b * 2); return c(10, 20); }\n"
	},
	{
		"DefaultedFunctions", 2,
		"struct S { int v; S() = default; S(const S&) = default; S& operator=(const S&) = default; ~S() = default; };\n"
		"int main() { S a; a.v = 1; S b(a); S c; c = b; return b.v + c.v; }\n"
	},
	{
		"AggregateMemberInitialisers", 52,
		"struct S { int a = 1; int b = 2; };\n"
		"int main() { S s{5}; return s.a * 10 + s.b; }\n"
	},
};


//Stage 2: rvalue references.
static const SConformanceCase gasConformanceStage2[] =
{
	{
		"RvalueOverload", 12,
		"int f(int& x) { return 1; }\n"
		"int f(int&& x) { return 2; }\n"
		"int main() { int a = 0; return f(a) * 10 + f(3); }\n"
	},
	{
		"ConstReferenceAndRvalueOverload", 12,
		"int f(const int& x) { return 1; }\n"
		"int f(int&& x) { return 2; }\n"
		"int main() { int a = 0; return f(a) * 10 + f(3); }\n"
	},
	{
		"RvalueBindsVariable", CONFORMANCE_REFUSED,
		"int main() { int a = 0; int&& r = a; return r; }\n"
	},
	{
		"RvalueLocal", 4,
		"int main() { int&& r = 3; r++; return r; }\n"
	},
	{
		"RvalueDistinctType", 1,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main() { return Same<int&, int&&>::V * 10 + Same<int&&, int&&>::V; }\n"
	},
	{
		"RvalueSpecialisations", 123,
		"template<class T> struct Kind { enum { V = 1 }; };\n"
		"template<class T> struct Kind<T&> { enum { V = 2 }; };\n"
		"template<class T> struct Kind<T&&> { enum { V = 3 }; };\n"
		"int main() { return Kind<int>::V * 100 + Kind<int&>::V * 10 + Kind<int&&>::V; }\n"
	},
	{
		"ReferenceCollapsing", 1111,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class T> struct AddRR { typedef T&& Type; };\n"
		"template<class T> struct AddLR { typedef T& Type; };\n"
		"int main() { return Same<AddRR<int&>::Type, int&>::V + Same<AddRR<int>::Type, int&&>::V * 10 + Same<AddLR<int&&>::Type, int&>::V * 100 + Same<AddRR<int&&>::Type, int&&>::V * 1000; }\n"
	},
	{
		"ForwardingDeduction", 212,
		"template<class T> struct IsLRef { enum { V = 1 }; };\n"
		"template<class T> struct IsLRef<T&> { enum { V = 2 }; };\n"
		"template<class T> int f(T&& t) { return IsLRef<T>::V; }\n"
		"int main() { int a = 0; const int c = 0; return f(a) * 100 + f(3) * 10 + f(c); }\n"
	},
	{
		"ForwardingCopies", 92,
		"template<class T> int F(T&& t) { T x = t; x = 9; return t; }\n"
		"int main() { int i = 1; return F(i) * 10 + F(2); }\n"
	},
	{
		"MoveConstructor", 1021,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct S { int v; S() : v(1) {} S(const S& o) : v(o.v + 10) {} S(S&& o) : v(o.v + 100) { o.v = 0; } };\n"
		"int main() { S a; S b(a); S c(Move(a)); return b.v + c.v * 10 + a.v * 1000; }\n"
	},
	{
		"MoveAssignment", 213,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct S { int v; S() : v(1) {} S& operator=(const S& o) { v = o.v + 10; return *this; } S& operator=(S&& o) { v = o.v + 100; return *this; } };\n"
		"S make() { return S(); }\n"
		"int main() { S a; S b; S c; b = a; c = Move(a); a = make(); return b.v + c.v + a.v; }\n"
	},
	{
		"ImplicitMove", 3501,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct M { int v; M() : v(0) {} M(M&& o) : v(o.v + 1) {} M(const M& o) : v(o.v + 50) {} M& operator=(M&& o) { v = o.v + 3; return *this; } M& operator=(const M& o) { v = o.v + 70; return *this; } };\n"
		"struct W { M m; };\n"
		"int main() { W a; W b(Move(a)); W c(a); W d; d = Move(a); return b.m.v + c.m.v * 10 + d.m.v * 1000; }\n"
	},
	{
		"MoveOnlyReturned", 240,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct U { int v; U(int a) : v(a) {} U(U&& o) : v(o.v) { o.v = 0; } U(const U&) = delete; };\n"
		"U make(int a) { U u(a); return u; }\n"
		"int take(U u) { return u.v; }\n"
		"int main() { U u = make(4); return take(Move(u)) * 10 + u.v + take(make(2)) * 100; }\n"
	},
	{
		"MoveOnlyCopied", CONFORMANCE_REFUSED,
		"struct U { int v; U(int a) : v(a) {} U(U&& o) : v(o.v) {} };\n"
		"int main() { U u(1); U w(u); return w.v; }\n"
	},
	{
		"PerfectForwarding", 123,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"int g(int& x) { return 1; }\n"
		"int g(const int& x) { return 2; }\n"
		"int g(int&& x) { return 3; }\n"
		"template<class T> int h(T&& t) { return g(Forward<T>(t)); }\n"
		"int main() { int a = 0; const int c = 0; return h(a) * 100 + h(c) * 10 + h(3); }\n"
	},
	{
		"RefQualifiedMembers", 123,
		"struct S { int f() & { return 1; } int f() && { return 2; } int g() const & { return 3; } };\n"
		"int main() { S s; return s.f() * 100 + S().f() * 10 + S().g(); }\n"
	},
	{
		"RvalueMember", 122,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct S { int v; };\n"
		"int f(int& x) { return 1; }\n"
		"int f(int&& x) { return 2; }\n"
		"S make() { S s; s.v = 0; return s; }\n"
		"int main() { S s; s.v = 0; return f(s.v) * 100 + f(Move(s).v) * 10 + f(make().v); }\n"
	},
	{
		"RvalueReturn", 8,
		"int g = 5;\n"
		"int&& f() { return static_cast<int&&>(g); }\n"
		"int main() { int&& r = f(); r = 8; return g; }\n"
	},
	{
		"NamedRvalueIsLvalue", 1,
		"int f(int& x) { return 1; }\n"
		"int f(int&& x) { return 2; }\n"
		"int h(int&& x) { return f(x); }\n"
		"int main() { return h(3); }\n"
	},
	{
		"TemporaryToConstReference", 542,
		"int f(const int& x) { return x + 1; }\n"
		"struct S { int v; S(int a) : v(a) {} };\n"
		"int g(const S& s) { return s.v; }\n"
		"int main() { return f(3) * 10 + g(S(2)) + g(5) * 100; }\n"
	},
};


//Stage 3: auto, decltype, range-for, alias templates, nullptr_t.
static const SConformanceCase gasConformanceStage3[] =
{
	{
		"AutoVariables", 409,
		"int main() { auto x = 3; auto* p = &x; const auto& r = x; auto y = x; y++; return x + *p + r + y * 100; }\n"
	},
	{
		"AutoDropsConstAndReference", 61,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main() { const int c = 5; int i = 1; int& ri = i; auto a = c; auto b = ri; a++; b = 7; return a * 10 + i; }\n"
	},
	{
		"AutoTypes", 31,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main() { auto a = 1L; auto b = 'c'; auto c = 1.5f; auto d = \"x\"; auto e = &a; return Same<decltype(a), long>::V + Same<decltype(b), char>::V * 2 + Same<decltype(c), float>::V * 4 + Same<decltype(d), const char*>::V * 8 + Same<decltype(e), long*>::V * 16; }\n"
	},
	{
		"AutoInFor", 20,
		"int main() { int a[4] = { 1, 2, 3, 4 }; int s = 0; for (auto i = 0; i < 4; i++) { s += a[i]; } for (auto* p = a; p != a + 4; ++p) { s += *p; } return s; }\n"
	},
	{
		"AutoFunctionPointer", 9,
		"int f(int x) { return x * 3; }\n"
		"int main() { auto p = f; auto q = &f; return p(2) + q(1); }\n"
	},
	{
		"AutoNoInitialiser", CONFORMANCE_REFUSED,
		"int main() { auto x; x = 1; return x; }\n"
	},
	{
		"AutoConflicting", CONFORMANCE_REFUSED,
		"int main() { auto a = 1, b = 2.5; return a; }\n"
	},
	{
		"TrailingReturn", 22,
		"auto f(int x) -> int { return x + 1; }\n"
		"struct S { auto g() const -> long { return 2; } };\n"
		"int main() { S s; return f(1) * 10 + (int)s.g(); }\n"
	},
	{
		"Decltype", 52,
		"int main() { int a = 1; decltype(a) b = 2; decltype((a)) c = a; c = 5; return a * 10 + b; }\n"
	},
	{
		"DecltypeTypes", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { long m; };\n"
		"int f();\n"
		"int a;\n"
		"int main() { S s; return Same<decltype(a), int>::V + Same<decltype((a)), int&>::V * 2 + Same<decltype(a + 1L), long>::V * 4 + Same<decltype(s.m), long>::V * 8 + Same<decltype(f()), int>::V * 16 + Same<decltype(nullptr), decltype(nullptr)>::V * 32; }\n"
	},
	{
		"DecltypeTrailing", 13,
		"template<class A, class B> auto add(A a, B b) -> decltype(a + b) { return a + b; }\n"
		"int main() { return (int)add(1, 2L) + (sizeof(add(1, 2L)) == sizeof(long)) * 10; }\n"
	},
	{
		"RangeForArray", 18,
		"int main() { int a[3] = { 1, 2, 3 }; int s = 0; for (int x : a) { s += x; } for (int& x : a) { x *= 2; } for (auto x : a) { s += x; } return s; }\n"
	},
	{
		"RangeForClass", 6,
		"struct R { int a[3]; int* begin() { return a; } int* end() { return a + 3; } };\n"
		"int main() { R r; r.a[0] = 1; r.a[1] = 2; r.a[2] = 3; int s = 0; for (int x : r) { s += x; } return s; }\n"
	},
	{
		"RangeForFreeBegin", 3,
		"namespace N { struct R { int a[2]; }; int* begin(R& r) { return r.a; } int* end(R& r) { return r.a + 2; } }\n"
		"int main() { N::R r; r.a[0] = 1; r.a[1] = 2; int s = 0; for (int x : r) { s += x; } return s; }\n"
	},
	{
		"RangeForIterator", 12,
		"struct It { int i; bool operator!=(const It& o) const { return i != o.i; } It& operator++() { i++; return *this; } int operator*() const { return i * 2; } };\n"
		"struct R { It begin() const { It t; t.i = 1; return t; } It end() const { It t; t.i = 4; return t; } };\n"
		"int main() { R r; int s = 0; for (auto x : r) { s += x; } return s; }\n"
	},
	{
		"RangeForBreakContinue", 8,
		"int main() { int a[5] = { 1, 2, 3, 4, 5 }; int s = 0; for (int x : a) { if (x == 2) { continue; } if (x == 5) { break; } s += x; } return s; }\n"
	},
	{
		"RangeForObjects", 430,
		"int g = 0;\n"
		"struct It { int* p; It(int* a) : p(a) {} ~It() { g++; } bool operator!=(const It& o) const { return p != o.p; } It& operator++() { p++; return *this; } int& operator*() const { return *p; } };\n"
		"struct V { int a[4]; It begin() { return It(a); } It end() { return It(a + 4); } };\n"
		"struct W { int v; W(int a) : v(a) {} ~W() { g += 100; } };\n"
		"template<class C> int Sum(C& c) { int s = 0; for (auto& x : c) { s += x; } return s; }\n"
		"int main() { V v; int i = 0; for (int& x : v) { x = ++i; } int s = 0; for (const auto& x : v) { if (x == 2) continue; if (x == 4) break; W w(x); s += w.v; } int a[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } }; for (auto& row : a) { for (int x : row) { s += x * 10; } } int t = Sum(v); return s + t + g; }\n"
	},
	{
		"DereferenceObject", 474,
		"struct It { int i; int operator*() const { return i * 2; } };\n"
		"struct Ref { int v; int& operator*() { return v; } };\n"
		"struct P { int x; };\n"
		"struct Obj { P p; P& operator*() { return p; } };\n"
		"int main() { It b; b.i = 2; int y = *b; Ref r; r.v = 1; *r = 5; int& q = *r; q += 2; Obj o; o.p.x = 3; (*o).x += 1; P c = *o; return y + *r * 10 + c.x * 100; }\n"
	},
	{
		"AliasTemplate", 61,
		"template<class T> struct Box { T v; };\n"
		"template<class T> using B = Box<T>;\n"
		"template<class T> using Ptr = T*;\n"
		"template<class T, int N> using Arr = T[N];\n"
		"int main() { B<int> b; b.v = 1; Ptr<int> p = &b.v; Arr<char, 6> a; return *p + sizeof(a) * 10; }\n"
	},
	{
		"NullPointerType", 121,
		"typedef decltype(nullptr) nullptr_t;\n"
		"int f(int*) { return 1; }\n"
		"int f(nullptr_t) { return 2; }\n"
		"int main() { nullptr_t n = nullptr; int* p = n; return f(nullptr) * 10 + f(p) + (sizeof(nullptr_t) == sizeof(void*)) * 100; }\n"
	},
	{
		"AutoReturn", 122,
		"auto f(int x) { return x + 1; }\n"
		"auto g(int x) { if (x > 0) { return 1L; } return 2L; }\n"
		"int main() { return f(1) * 10 + (int)g(0) + (sizeof(g(0)) == sizeof(long)) * 100; }\n"
	},
	{
		"DecltypeAuto", 3,
		"int g = 1;\n"
		"decltype(auto) f() { return (g); }\n"
		"decltype(auto) h() { return g; }\n"
		"int main() { f() = 3; int x = h(); x = 9; return g; }\n"
	},
};


//Stage 4: constexpr (ConstexprPlan.txt).
static const SConformanceCase gasConformanceStage4[] =
{
	{
		"ConstexprVariable", 12,
		"constexpr int N = 3 * 4;\n"
		"char a[N];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprFunction", 25169,
		"constexpr int sq(int x) { return x * x; }\n"
		"char a[sq(3)];\n"
		"enum { E = sq(4) };\n"
		"int main() { int v = 5; return sizeof(a) + E * 10 + sq(v) * 1000; }\n"
	},
	{
		"ConstexprRecursion", 120,
		"constexpr int fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }\n"
		"char a[fact(5)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprLoop", 21,
		"constexpr int sum(int n) { int r = 0; for (int i = 1; i <= n; i++) { r += i; } return r; }\n"
		"char a[sum(6)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprStatements", 10,
		"constexpr int f(int n) { int r = 1; int i = 0; while (i < n) { if (i % 2) { r += 3; } else { r *= 2; } i++; } switch (r) { case 13: return r + 100; default: break; } return r; }\n"
		"char a[f(3)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprCaseAndTemplateArgument", 310,
		"constexpr int sq(int x) { return x * x; }\n"
		"template<int N> struct A { enum { V = N + 1 }; };\n"
		"int main() { int x = 4; int r = 0; switch (x) { case sq(2): r = 3; break; default: r = 9; } return r * 100 + A<sq(3)>::V; }\n"
	},
	{
		"ConstexprStaticAssert", 9,
		"constexpr int sq(int x) { return x * x; }\n"
		"static_assert(sq(3) == 9, \"sq\");\n"
		"int main() { return 9; }\n"
	},
	{
		"ConstexprStaticAssertFalse", CONFORMANCE_REFUSED,
		"constexpr int sq(int x) { return x * x; }\n"
		"static_assert(sq(3) == 8, \"sq\");\n"
		"int main() { return 0; }\n"
	},
	{
		"ConstexprClass", 75,
		"struct P { int x; int y; constexpr P(int a, int b) : x(a), y(b) {} constexpr int sum() const { return x + y; } };\n"
		"constexpr P gp(5, 2);\n"
		"char a[gp.sum()];\n"
		"char b[gp.x];\n"
		"int main() { return sizeof(a) * 10 + sizeof(b); }\n"
	},
	{
		"ConstexprAggregate", 23,
		"struct P { int x; int y; };\n"
		"constexpr P gp = { 2, 3 };\n"
		"char a[gp.y];\n"
		"int main() { return sizeof(a) + gp.x * 10; }\n"
	},
	{
		"ConstexprArray", 53,
		"constexpr int ga[] = { 1, 2, 3 };\n"
		"char a[ga[1] + ga[2]];\n"
		"constexpr int at(const int* p, int i) { return p[i]; }\n"
		"char b[at(ga, 2)];\n"
		"int main() { return sizeof(a) * 10 + sizeof(b); }\n"
	},
	{
		"ConstexprString", 52,
		"constexpr int len(const char* s) { int n = 0; while (s[n]) { n++; } return n; }\n"
		"char a[len(\"hello\")];\n"
		"char b[\"abc\"[1] - 'a' + 1];\n"
		"int main() { return sizeof(a) * 10 + sizeof(b); }\n"
	},
	{
		"ConstexprFloating", 35,
		"constexpr double d = 1.5 * 2;\n"
		"constexpr double half(double x) { return x / 2; }\n"
		"char a[(int)d];\n"
		"char b[(int)(half(9.0) + 0.5)];\n"
		"int main() { return sizeof(a) * 10 + sizeof(b); }\n"
	},
	{
		"ConstexprFloatAgrees", 127,
		"constexpr double cThird = 1.0 / 3.0;\n"
		"constexpr double cSum = 0.1 + 0.2;\n"
		"constexpr float cFloat = 1.0f / 3.0f;\n"
		"constexpr float cNarrowed = (float)(2.0 / 3.0);\n"
		"constexpr double cBig = 9007199254740993.0 * 3.0;\n"
		"constexpr double cTiny = 1e-300 * 1e-10;\n"
		"constexpr int cTruncated = (int)(-7.9);\n"
		"int main()\n"
		"{\n"
		"\tvolatile double one = 1.0, three = 3.0, p1 = 0.1, p2 = 0.2, two = 2.0, big = 9007199254740993.0, small = 1e-300, smaller = 1e-10, neg = -7.9;\n"
		"\tvolatile float fone = 1.0f, fthree = 3.0f;\n"
		"\tint r = 0;\n"
		"\tr += (cThird == one / three);\n"
		"\tr += (cSum == p1 + p2) * 2;\n"
		"\tr += (cFloat == fone / fthree) * 4;\n"
		"\tr += (cNarrowed == (float)(two / three)) * 8;\n"
		"\tr += (cBig == big * three) * 16;\n"
		"\tr += (cTiny == small * smaller) * 32;\n"
		"\tr += (cTruncated == (int)neg) * 64;\n"
		"\treturn r;\n"
		"}\n"
	},
	{
		"ConstexprReferencesAndPointers", 75,
		"constexpr int gv = 7;\n"
		"constexpr const int& gr = gv;\n"
		"constexpr const int* gq = &gv;\n"
		"char a[gr];\n"
		"char b[*gq - 2];\n"
		"int main() { return sizeof(a) * 10 + sizeof(b); }\n"
	},
	{
		"ConstexprLocalObjects", 9,
		"struct V { int x; constexpr V(int a) : x(a) {} constexpr V add(V o) const { return V(x + o.x); } };\n"
		"constexpr int f() { V a(2); V b = a.add(V(3)); int t[3] = { 1, 2, 3 }; t[1] = b.x; return t[0] + t[1] + t[2]; }\n"
		"char a[f()];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprNotConstant", CONFORMANCE_REFUSED,
		"int g = 3;\n"
		"constexpr int v = g;\n"
		"int main() { return v; }\n"
	},
	{
		"ConstexprCallsOrdinary", CONFORMANCE_REFUSED,
		"int f(int x) { return x; }\n"
		"constexpr int g(int x) { return f(x); }\n"
		"char a[g(3)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprDivisionByZero", CONFORMANCE_REFUSED,
		"constexpr int f(int x) { return 10 / x; }\n"
		"char a[f(0) + 1];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprOutOfBounds", CONFORMANCE_REFUSED,
		"constexpr int ga[] = { 1, 2, 3 };\n"
		"constexpr int f(int i) { return ga[i]; }\n"
		"char a[f(5) + 1];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprVirtual", CONFORMANCE_REFUSED,
		"struct B { constexpr virtual int f() const { return 1; } };\n"
		"int main() { B b; return b.f(); }\n"
	},
	{
		"ConstexprGoto", CONFORMANCE_REFUSED,
		"constexpr int f(int n) { if (n) { goto out; } n = 2; out: return n; }\n"
		"char a[f(1)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprStaticLocal", CONFORMANCE_REFUSED,
		"constexpr int f(int n) { static int s = 1; return n + s; }\n"
		"char a[f(1)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprUninitialised", CONFORMANCE_REFUSED,
		"constexpr int f(int n) { int r; r = n; return r; }\n"
		"char a[f(1)];\n"
		"int main() { return sizeof(a); }\n"
	},
	{
		"ConstexprConstructorMissesMember", CONFORMANCE_REFUSED,
		"struct P { int x; int y; constexpr P(int a) : x(a) {} };\n"
		"constexpr P gp(1);\n"
		"int main() { return gp.x; }\n"
	},
	{
		"ConstexprNonLiteralParameter", CONFORMANCE_REFUSED,
		"struct N { int v; ~N() {} };\n"
		"constexpr int f(N n) { return 1; }\n"
		"int main() { N n; n.v = 1; return f(n); }\n"
	},
	{
		"ConstexprInitialisedFirst", 5,
		"struct P { int x; constexpr P(int a) : x(a) {} };\n"
		"int read();\n"
		"int g = read();\n"
		"constexpr P gp(5);\n"
		"int read() { return gp.x; }\n"
		"int main() { return g; }\n"
	},
	{
		"ConstexprPointerData", 447,
		"int gv = 3;\n"
		"constexpr int* gq = &gv;\n"
		"constexpr const char* gs = \"hello\";\n"
		"struct L { const char* s; int* p; int n; };\n"
		"constexpr L gl = { \"ab\", &gv, 7 };\n"
		"int main() { *gq = 4; return gv * 100 + gs[1] - 'e' + gl.s[1] - 'b' + *gl.p * 10 + gl.n; }\n"
	},
	{
		"ConstexprRunTime", 93,
		"constexpr int sq(int x) { return x * x; }\n"
		"struct P { int x; constexpr P(int a) : x(a) {} constexpr int get() const { return x; } };\n"
		"int main() { int v = 3; P p(v); return sq(v) * 10 + p.get(); }\n"
	},
	{
		//Signed overflow is undefined, so an expression that overflows isn't a constant; unsigned arithmetic wraps
		//([expr] 4, [expr.const] 2; CppParserTodo.txt question 3).  A left shift into the sign bit is C++14's
		//[expr.shift] 2.  int is 16 bits on the W65C816, so M is its largest from sizeof.  MSVC only warns of most
		//of these (C4307), which hostcheck.py makes an error.
		"ConstantUnsignedWraps", 7,
		"constexpr unsigned U = 0u - 1u;\n"
		"enum { E = (U == ~0u) ? 7 : 1 };\n"
		"int main() { return E; }\n"
	},
	{
		"ConstantShiftIntoSign", 9,
		"enum { S = 1 << (sizeof(int) * 8 - 1) };\n"
		"int main() { return (S < 0) ? 9 : 1; }\n"
	},
	{
		"ConstantSignedSumOverflows", CONFORMANCE_REFUSED,
		"constexpr int M = sizeof(int) == 2 ? 32767 : 2147483647;\n"
		"enum { E = M + 1 };\n"
		"int main() { return 1; }\n"
	},
	{
		"ConstantSignedProductOverflows", CONFORMANCE_REFUSED,
		"constexpr int M = sizeof(int) == 2 ? 32767 : 2147483647;\n"
		"char a[(M * 2) / 2 > 0 ? 1 : 2];\n"
		"int main() { return 1; }\n"
	},
	{
		"ConstantLeastDividedByMinusOne", CONFORMANCE_REFUSED,
		"constexpr int M = sizeof(int) == 2 ? 32767 : 2147483647;\n"
		"constexpr int L = -M - 1;\n"
		"constexpr int Q = L / -1;\n"
		"int main() { return 1; }\n"
	},
	{
		"ConstexprCallOverflows", CONFORMANCE_REFUSED,
		"constexpr int M = sizeof(int) == 2 ? 32767 : 2147483647;\n"
		"constexpr int Add(int a, int b) { return a + b; }\n"
		"enum { E = Add(M, 1) };\n"
		"int main() { return 1; }\n"
	},
	{
		"ConstexprLoopOverflows", CONFORMANCE_REFUSED,
		"constexpr int Grow(int n) { int s = 1; for (int i = 0; i < n; i++) { s *= 3; } return s; }\n"
		"enum { E = Grow(40) };\n"
		"int main() { return 1; }\n"
	},
	{
		//Where a constant isn't needed it is the program's to be right: it compiles.
		"SignedOverflowAtRunTimeCompiles", 5,
		"constexpr int M = sizeof(int) == 2 ? 32767 : 2147483647;\n"
		"int F(int x) { return x - M; }\n"
		"int main() { volatile int m = M; return F(m) + 5; }\n"
	},
};


//Stage 5: lambdas.
static const SConformanceCase gasConformanceStage5[] =
{
	{
		"LambdaPlain", 23,
		"int main() { auto f = [](int x) { return x + 1; }; return f(1) * 10 + [](int a, int b) { return a - b; }(5, 2); }\n"
	},
	{
		"LambdaCaptures", 443,
		"int main() { int a = 1, b = 2; auto f = [a, &b](int x) { b += x; return a + b; }; int r = f(1); a = 50; return r * 10 + b + f(0) * 100; }\n"
	},
	{
		"LambdaDefaultCaptures", 543,
		"int main() { int a = 1, b = 2, c = 3; auto f = [=]() { return a + b; }; auto g = [&]() { c = a + b; }; auto h = [=, &c]() { c += a; }; auto k = [&, a]() { b = a * 5; }; g(); h(); k(); return f() + c * 10 + b * 100; }\n"
	},
	{
		"LambdaMutable", 51,
		"int main() { int a = 1; auto f = [a]() mutable { a += 2; return a; }; f(); return f() * 10 + a; }\n"
	},
	{
		"LambdaToFunctionPointer", 45,
		"int apply(int (*p)(int), int x) { return p(x); }\n"
		"int main() { int (*p)(int) = [](int x) { return x * 2; }; return p(2) * 10 + apply([](int x) { return x + 1; }, 4); }\n"
	},
	{
		"LambdaThis", 34,
		"struct S { int v; int f() { auto g = [this]() { v++; return v + 1; }; return g(); } int h() { auto g = [=]() { return v * 2; }; return g(); } };\n"
		"int main() { S s; s.v = 1; return s.f() * 10 + s.h(); }\n"
	},
	{
		"LambdaAsTemplateArgument", 69,
		"template<class F> int apply(F f, int x) { return f(x); }\n"
		"template<class F> int twice(F&& f) { return f() + f(); }\n"
		"int main() { int k = 3; return apply([k](int x) { return x * k; }, 2) * 10 + twice([&k]() { return ++k; }); }\n"
	},
	{
		"LambdaReturnType", 15,
		"int main() { auto f = [](int x) -> long { return x; }; auto g = [](bool b) -> double { if (b) { return 1; } return 2.5; }; return (sizeof(f(1)) == sizeof(long)) * 10 + (int)(g(false) * 2); }\n"
	},
	{
		"LambdaNested", 7,
		"int main() { int a = 2; auto f = [a](int x) { auto g = [a, x](int y) { return a * x + y; }; return g(1); }; return f(3); }\n"
	},
	{
		"LambdaCopiedAndStored", 156,
		"struct H { int k; };\n"
		"int main() { int n = 4; auto f = [n](int x) { return x + n; }; auto g = f; n = 100; return f(1) * 10 + g(2) + (sizeof(f) == sizeof(int)) * 100; }\n"
	},
	{
		"LambdaCapturesObject", 22,
		"int gd = 0;\n"
		"struct T { int v; T(int a) : v(a) {} T(const T& o) : v(o.v + 1) {} ~T() { gd++; } };\n"
		"int f() { T t(1); auto g = [t]() { return t.v; }; return g(); }\n"
		"int main() { int r = f(); return r * 10 + gd; }\n"
	},
	{
		"LambdaGeneric", 231,
		"int main() { auto f = [](auto x) { return x + 1; }; auto g = [](auto a, auto b) { return sizeof(a) + sizeof(b); }; return f(1) * 100 + (int)f(2L) * 10 + (g('c', 'd') == 2); }\n"
	},
	{
		"LambdaInitCapture", 12,
		"int main() { int a = 1; auto f = [b = a + 1, &c = a]() { c = 10; return b; }; return f() + a; }\n"
	},
	{
		"LambdaCombined", 127,
		"int gd = 0;\n"
		"struct T { int v; T(int a) : v(a) {} T(const T& o) : v(o.v) {} ~T() { gd++; } };\n"
		"struct C { int k; int Run(int x) { auto outer = [this, x](int y) { auto inner = [&](int z) { return k * 100 + x * 10 + y + z; }; return inner(1); }; return outer(2); } };\n"
		"template<class F> int Twice(F f, int a) { return f(f(a)); }\n"
		"template<class T> int Sum(T a, T b) { auto add = [](T p, T q) { return p + q; }; return add(a, b); }\n"
		"int Apply(int (*p)(int), int x) { return p(x); }\n"
		"int main()\n"
		"{\n"
		"\tC c; c.k = 3;\n"
		"\tint n = 5; auto inc = [n](int x) { return x + n; };\n"
		"\tint got = 0;\n"
		"\t{ T t(7); auto get = [t]() { return t.v; }; auto copy = get; got = copy(); }\n"
		"\tauto generic = [](auto a, auto& b) { b = b + 1; return a + b; };\n"
		"\tint m = 1; int g = generic(2, m); long gl = generic(10L, m);\n"
		"\treturn (c.Run(4) == 343) + (Twice(inc, 1) == 11) * 2 + (Sum(2, 3) == 5) * 4 + (Apply([](int x) { return x * 3; }, 2) == 6) * 8 + (got == 7) * 16 + ((g == 4) && (gl == 13) && (m == 3)) * 32 + (gd == 3) * 64;\n"
		"}\n"
	},
	{
		//A constant whose value is used at once isn't odr-used, so a lambda may use it without capturing it ([basic.def.odr] 3,
		//[expr.prim.lambda] 12; CppParserTodo.txt question 10).  MSVC needs /Zc:lambda for it (hostcheck.py passes it).
		"LambdaUncapturedConstant", 150,
		"int main() { const int n = 5; constexpr long m = 3; const bool b = true; auto f = [](int x) { return x * n + (int)m + b; }; auto g = [=]() { return n * 2; }; return f(2) * 10 + g(); }\n"
	},
	{
		"LambdaUncapturedConstantNested", 3020,
		"template<class T> int Scale(T t) { const int k = 3; auto f = [](T x) { auto g = [](T y) { return y * k; }; return g(x) + k; }; return f(t); }\n"
		"int main() { const int n = 7; int a[n]; auto f = [&a](int i) { a[i % n] = i; return a[i % n] + n; }; return Scale(1000) + f(10); }\n"
	},
	{
		"LambdaUsesUncaptured", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = []() { return a; }; return f(); }\n"
	},
	{
		//Its address, or a reference parameter bound to it, odr-uses it.
		"LambdaAddressOfUncapturedConstant", CONFORMANCE_REFUSED,
		"int main() { const int n = 5; auto f = []() { return *&n; }; return f(); }\n"
	},
	{
		"LambdaUncapturedConstArgumentByReference", CONFORMANCE_REFUSED,
		"int Get(const int& r) { return r; }\n"
		"int main() { const int n = 5; auto f = []() { return Get(n); }; return f(); }\n"
	},
	{
		//A const initialised with what isn't a constant is a variable like any other.
		"LambdaUncapturedConstNotConstant", CONFORMANCE_REFUSED,
		"int F(int p) { const int n = p; auto g = []() { return n; }; return g(); }\n"
		"int main() { return F(3); }\n"
	},
	{
		"LambdaChangesCopy", CONFORMANCE_REFUSED,
		"int main() { int a = 1; auto f = [a]() { a = 2; return a; }; return f(); }\n"
	},
};


//Stage 6: templates.
static const SConformanceCase gasConformanceStage6[] =
{
	{
		"Sfinae", 12,
		"template<bool B, class T = void> struct EnableIf {};\n"
		"template<class T> struct EnableIf<true, T> { typedef T Type; };\n"
		"template<class T> typename EnableIf<sizeof(T) == 1, int>::Type f(T) { return 1; }\n"
		"template<class T> typename EnableIf<sizeof(T) != 1, int>::Type f(T) { return 2; }\n"
		"int main() { return f('a') * 10 + f(1L); }\n"
	},
	{
		"SfinaeMemberDetection", 11,
		"template<class T> struct HasSize { template<class U> static char test(int U::*); template<class U> static long test(...); enum { V = sizeof(test<T>(0)) == 1 }; };\n"
		"struct S {};\n"
		"int main() { return HasSize<S>::V * 10 + HasSize<int>::V + 1; }\n"
	},
	{
		"SfinaeDefaultArgument", 12,
		"template<bool B, class T = void> struct EnableIf {};\n"
		"template<class T> struct EnableIf<true, T> { typedef T Type; };\n"
		"template<class T, class = typename EnableIf<(sizeof(T) > 1)>::Type> int f(T) { return 1; }\n"
		"int f(...) { return 2; }\n"
		"int main() { return f(1L) * 10 + f('c'); }\n"
	},
	{
		"TemplateTemplateParameter", 42,
		"template<class T> struct Box { T v; };\n"
		"template<template<class> class C, class T> struct Use { C<T> c; };\n"
		"int main() { Use<Box, long> u; u.c.v = 2; return (int)u.c.v + sizeof(u.c.v) * 10; }\n"
	},
	{
		"MemberClassTemplate", 1442,
		"struct O { template<class T> struct I { T v; int size() { return sizeof(T); } }; };\n"
		"template<class A> struct P { template<class B> struct Q { enum { V = sizeof(A) * 10 + sizeof(B) }; }; };\n"
		"int main() { O::I<long> i; i.v = 2; return (int)i.v + i.size() * 10 + P<char>::Q<long>::V * 100; }\n"
	},
	{
		"ArrayBoundDeduced", 531,
		"template<class T, unsigned long N> int len(T (&)[N]) { return (int)N; }\n"
		"template<class T, unsigned long N> int size(T (&a)[N]) { return sizeof(a); }\n"
		"int main() { char a[5]; long b[3]; return len(a) * 100 + len(b) * 10 + (size(b) == 12); }\n"
	},
	{
		"ArraySpecialisation", 173,
		"template<class T> struct A { enum { V = 1 }; };\n"
		"template<class T, int N> struct A<T[N]> { enum { V = N }; };\n"
		"template<class T> struct A<T[]> { enum { V = 3 }; };\n"
		"int main() { return A<int>::V * 100 + A<int[7]>::V * 10 + A<int[]>::V; }\n"
	},
	{
		"PointerTemplateArgument", 328,
		"int g = 3;\n"
		"int f(int x) { return x + 1; }\n"
		"template<int* P> struct A { int get() { return *P; } };\n"
		"template<int (*F)(int)> struct B { int call(int x) { return F(x); } };\n"
		"template<int& R> struct C { void set() { R = 8; } };\n"
		"int main() { A<&g> a; B<f> b; C<g> c; int r = a.get() * 100 + b.call(1) * 10; c.set(); return r + g; }\n"
	},
	{
		"PackSizeof", 206,
		"template<class... T> struct Count { enum { V = sizeof...(T) }; };\n"
		"template<class... A> int f(A... a) { return sizeof...(A) + sizeof...(a); }\n"
		"int main() { return Count<int, char>::V * 100 + Count<>::V * 10 + f(1, 2, 3); }\n"
	},
	{
		"ForwardingPack", 114,
		"template<class T> struct RemoveRef { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&> { typedef T Type; };\n"
		"template<class T> struct RemoveRef<T&&> { typedef T Type; };\n"
		"template<class T> typename RemoveRef<T>::Type&& Move(T&& t) { return static_cast<typename RemoveRef<T>::Type&&>(t); }\n"
		"template<class T> T&& Forward(typename RemoveRef<T>::Type& t) { return static_cast<T&&>(t); }\n"
		"struct P { int v; P(int& a, int&& b, const int& c) : v(100 + a + b * 2 + c * 3) {} };\n"
		"template<class... A> P make(A&&... a) { return P(Forward<A>(a)...); }\n"
		"int main() { int a = 1; const int c = 3; return make(a, 2, c).v; }\n"
	},
	{
		"PackExpandedExpression", 654,
		"int g(int a, int b, int c) { return a + b * 10 + c * 100; }\n"
		"int twice(int x) { return x * 2; }\n"
		"template<class... A> int f(A... a) { return g((a + 1)...); }\n"
		"template<class... A> int h(A... a) { return g(twice(a)...); }\n"
		"int main() { return f(1, 2, 3) + h(1, 1, 1); }\n"
	},
	{
		"PackInInitialiser", 4131,
		"template<class... A> int f(A... a) { int v[] = { a... }; return v[0] + v[sizeof...(a) - 1] * 10; }\n"
		"template<class... A> int count(A... a) { int s = 0; int v[] = { (s += a, 0)... }; return s + sizeof(v) / sizeof(v[0]) * 100; }\n"
		"int main() { return f(1, 2, 3) + count(1, 2, 3, 4) * 10; }\n"
	},
	{
		"PackBases", 12,
		"struct A { int a; A() : a(1) {} };\n"
		"struct B { int b; B() : b(2) {} };\n"
		"template<class... T> struct D : T... { D() : T()... {} };\n"
		"int main() { D<A, B> d; return d.a * 10 + d.b; }\n"
	},
	{
		"PackTemplateArguments", 23,
		"template<class... T> struct List { enum { N = sizeof...(T) }; };\n"
		"template<class... T> struct Wrap { typedef List<T*...> Type; typedef List<int, T...> More; };\n"
		"int main() { return Wrap<int, char>::Type::N * 10 + Wrap<int, char>::More::N; }\n"
	},
	{
		"PackNonType", 10,
		"template<int... N> struct Sum;\n"
		"template<> struct Sum<> { enum { V = 0 }; };\n"
		"template<int H, int... T> struct Sum<H, T...> { enum { V = H + Sum<T...>::V }; };\n"
		"int main() { return Sum<1, 2, 3, 4>::V; }\n"
	},
	{
		"PackMembers", 12,
		"template<class... T> struct Tuple;\n"
		"template<> struct Tuple<> {};\n"
		"template<class H, class... T> struct Tuple<H, T...> : Tuple<T...> { H h; Tuple(H a, T... t) : Tuple<T...>(t...), h(a) {} };\n"
		"int main() { Tuple<int, long, char> t(1, 2L, 'c'); Tuple<long, char>& u = t; return t.h * 10 + (int)u.h; }\n"
	},
	{
		"VariableTemplate", 349,
		"template<class T> constexpr T pi = T(3);\n"
		"template<class T> constexpr int size = sizeof(T);\n"
		"template<> constexpr int size<char> = 9;\n"
		"int main() { return pi<int> * 100 + size<long> * 10 + size<char>; }\n"
	},
	{
		"TypeTraitsClasses", 1023,
		"struct B {};\n"
		"struct D : B {};\n"
		"union U { int i; };\n"
		"enum E { X };\n"
		"struct V { virtual void f() {} };\n"
		"struct A { virtual void f() = 0; };\n"
		"struct F final {};\n"
		"int main() { return __is_class(B) + __is_union(U) * 2 + __is_enum(E) * 4 + __is_polymorphic(V) * 8 + __is_abstract(A) * 16 + __is_empty(B) * 32 + __is_final(F) * 64 + !__is_class(U) * 128 + !__is_polymorphic(B) * 256 + !__is_empty(V) * 512; }\n"
	},
	{
		"TypeTraitsRelations", 255,
		"struct B {};\n"
		"struct D : B {};\n"
		"struct N { N(int) {} };\n"
		"int main() { return __is_base_of(B, D) + !__is_base_of(D, B) * 2 + __is_convertible_to(D*, B*) * 4 + !__is_convertible_to(B*, D*) * 8 + __is_constructible(N, int) * 16 + !__is_constructible(N) * 32 + __is_assignable(B&, B) * 64 + __is_base_of(B, B) * 128; }\n"
	},
	{
		"TypeTraitsTrivial", 1023,
		"struct P { int a; };\n"
		"struct C { C() {} C(const C&) {} ~C() {} };\n"
		"int main() { return __is_pod(P) + !__is_pod(C) * 2 + __is_trivially_copyable(P) * 4 + !__is_trivially_copyable(C) * 8 + __is_trivially_constructible(P) * 16 + __has_trivial_destructor(P) * 32 + !__has_trivial_destructor(C) * 64 + __is_standard_layout(P) * 128 + __is_trivial(P) * 256 + __is_literal_type(P) * 512; }\n"
	},
	{
		"UnderlyingType", 11,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E : short { X };\n"
		"enum class F : unsigned char { Y };\n"
		"int main() { return Same<__underlying_type(E), short>::V * 10 + Same<__underlying_type(F), unsigned char>::V; }\n"
	},
	{
		"DependentTemplateName", 44,
		"struct S { template<class T> T conv(int x) { return (T)x; } template<class T> struct In { T v; }; };\n"
		"template<class C> int f(C c) { typename C::template In<long> i; i.v = c.template conv<long>(4); return (int)i.v + sizeof(i.v) * 10; }\n"
		"int main() { S s; return f(s); }\n"
	},
	{
		"TemplateConversionOperator", 1411,
		"struct S { int v; template<class T> operator T() const { return (T)(v + sizeof(T)); } };\n"
		"int main() { S s; s.v = 10; long l = s; char c = s; return (int)l * 100 + c; }\n"
	},
	{
		"DefaultTemplateTemplate", 7,
		"template<class T> struct Less { bool operator()(T a, T b) const { return a < b; } };\n"
		"template<class T, class C = Less<T>> struct Max { T get(T a, T b) { C c; return c(a, b) ? b : a; } };\n"
		"int main() { Max<int> m; return m.get(3, 7); }\n"
	},
	{
		"PartialOrdering", 12,
		"template<class T> int f(T) { return 1; }\n"
		"template<class T> int f(T*) { return 2; }\n"
		"int main() { int a = 0; return f(a) * 10 + f(&a); }\n"
	},
	{
		"MemberSpecialisation", 12,
		"template<class T> struct S { int f() { return 1; } };\n"
		"template<> int S<char>::f() { return 2; }\n"
		"int main() { S<int> a; S<char> b; return a.f() * 10 + b.f(); }\n"
	},
	{
		"TemplateStaticMember", 43,
		"template<class T> struct S { static int count; };\n"
		"template<class T> int S<T>::count = 3;\n"
		"int main() { S<int>::count++; return S<int>::count * 10 + S<char>::count; }\n"
	},
	{
		"MemberClassTemplateTwoOuters", 1444,
		"template<class A> struct P { template<class B> struct Q { int f() { return sizeof(A) * 10 + sizeof(B); } }; };\n"
		"int main() { P<char>::Q<long> a; P<long>::Q<long> b; return a.f() * 100 + b.f(); }\n"
	},
	{
		"TemplateDeclaredBeforeDefined", 5,
		"template<class T> struct A;\n"
		"A<int>* gp;\n"
		"template<class T> struct A { T x; };\n"
		"int main() { A<int> a; a.x = 4; gp = &a; return gp->x + sizeof(A<char>); }\n"
	},
	{
		"TypedefOfInstanceQualifier", 67,
		"struct X { enum { V = 6 }; };\n"
		"template<class T> struct L { enum { N = T::V }; };\n"
		"template<class T> struct W { typedef L<T> Type; struct In { enum { M = 7 }; }; };\n"
		"int main() { return W<X>::Type::N * 10 + W<X>::In::M; }\n"
	},
	{
		//A qualifier of three components is walked from the first: Type is W<C>'s typedef, whatever W's T is.
		"TypedefOfInstanceCall", 3,
		"template<class T> struct W { typedef T Type; };\n"
		"struct C { static int F() { return 3; } };\n"
		"int main() { return W<C>::Type::F(); }\n"
	},
	{
		//A member class named through an instance is that instance's own.  [temp.mem.class]
		"InstanceMemberClassCall", 4,
		"template<class T> struct Outer { struct Inner { static int Count() { return sizeof(T); } }; };\n"
		"int main() { return Outer<long>::Inner::Count(); }\n"
	},
	{
		"VariadicClass", 3,
		"template<class... T> struct Tuple;\n"
		"template<> struct Tuple<> { enum { N = 0 }; };\n"
		"template<class H, class... T> struct Tuple<H, T...> : Tuple<T...> { enum { N = 1 + Tuple<T...>::N }; H h; };\n"
		"int main() { return Tuple<int, char, long>::N; }\n"
	},
};


//Stage 7: objects.
static const SConformanceCase gasConformanceStage7[] =
{
	{
		"StaticLocalInitialisedOnce", 101,
		"int gCalls = 0;\n"
		"int g() { gCalls++; return 7; }\n"
		"int f() { static int v = g(); v++; return v; }\n"
		"int main() { f(); f(); return f() * 10 + gCalls; }\n"
	},
	{
		"StaticLocalObject", 521,
		"int gMade = 0;\n"
		"struct S { int v; S() : v(3) { gMade++; } };\n"
		"int f() { static S s; s.v++; return s.v; }\n"
		"S& single() { static S s; return s; }\n"
		"int main() { f(); single().v = 9; return f() * 100 + gMade * 10 + (single().v == 9); }\n"
	},
	{
		"StaticLocalNotReached", 1,
		"int gMade = 0;\n"
		"struct S { S() { gMade++; } };\n"
		"void f(bool b) { if (b) { static S s; } }\n"
		"int main() { f(false); int a = gMade; f(true); f(true); return a * 10 + gMade; }\n"
	},
	{
		"PostfixOnObjects", 11121,
		"struct S { int v; S operator++(int) { S t = *this; v++; return t; } S& operator++() { v += 10; return *this; } S operator--(int) { S t = *this; v--; return t; } };\n"
		"int main() { S s; s.v = 1; S t = s++; ++s; S u = s--; return t.v + u.v * 10 + s.v * 1000; }\n"
	},
	{
		"ArrowReturnsObject", 7,
		"struct I { int v; };\n"
		"struct P { I* p; I* operator->() { return p; } };\n"
		"struct Q { P q; P operator->() { return q; } };\n"
		"int main() { I i; i.v = 6; Q q; q.q.p = &i; q->v++; return q->v; }\n"
	},
	{
		"ConversionFunctions", 1356,
		"struct A { int v; };\n"
		"struct B { int w; operator A() const { A a; a.v = w + 1; return a; } operator int&() { return w; } operator const char*() const { return \"xyz\"; } };\n"
		"int take(A a) { return a.v; }\n"
		"int main() { B b; b.w = 2; A a = b; int& r = b; r = 5; const char* s = b; return a.v * 100 + b.w * 10 + take(b) + (s[1] == 'y') * 1000; }\n"
	},
	{
		"ConvertingConstructors", 5641,
		"struct A { int v; A(int a) : v(a) {} A(const char* s) : v(s[0]) {} };\n"
		"struct B { int w; B(const A& a) : w(a.v * 2) {} };\n"
		"int take(A a) { return a.v; }\n"
		"A make() { return 4; }\n"
		"int main() { A a = 3; B b = a; A c = \"A\"; return take(5) * 1000 + b.w * 100 + make().v * 10 + (c.v == 65); }\n"
	},
	{
		"PointerToOverloadedMember", 214,
		"struct S { int f(char) { return 1; } int f(long) { return 2; } static int g(char) { return 3; } static int g(long) { return 4; } };\n"
		"int main() { int (S::*p)(long) = &S::f; int (S::*q)(char) = &S::f; int (*r)(long) = &S::g; S s; return (s.*p)(0) * 100 + (s.*q)(0) * 10 + r(0); }\n"
	},
	{
		"ReferenceBoundToTemporary", 1341,
		"int gDead = 0;\n"
		"struct S { int v; S(int a) : v(a) {} ~S() { gDead++; } };\n"
		"S make() { return S(3); }\n"
		"int f() { const S& r = make(); const int& i = 4; int alive = (gDead == 0); return r.v * 100 + i * 10 + alive; }\n"
		"int main() { int r = f(); return r + gDead * 1000; }\n"
	},
	{
		"ReferenceMemberAndGlobal", 357,
		"struct H { const int& r; H(const int& a) : r(a) {} };\n"
		"int g = 5;\n"
		"const int& gr = g;\n"
		"const long& gt = 7;\n"
		"int main() { int a = 2; H h(a); a = 3; return h.r * 100 + gr * 10 + (int)gt; }\n"
	},
	{
		"ReferenceTemporaryScopes", 110,
		"long gOrder = 0;\n"
		"struct S { int v; S(int a) : v(a) {} ~S() { gOrder = gOrder * 10 + v; } };\n"
		"struct D : S { D() : S(5) {} };\n"
		"int f(int n) { S a(1); const S& r = S(2); S&& q = S(3); const S& d = D(); for (int i = 0; i < n; i++) { const S& w = S(7); if (i == 1) break; } return r.v + q.v + d.v; }\n"
		"int main() { int x = f(3); return (gOrder == 775321) * 100 + x; }\n"
	},
	{
		"GlobalReferenceTemporaries", 3467,
		"int gDead = 0;\n"
		"struct S { int v; S(int a) : v(a) {} ~S() { gDead++; } };\n"
		"struct C { int c; C() : c(8) {} };\n"
		"struct D : C, S { D() : S(6) {} };\n"
		"S make() { return S(3); }\n"
		"const S& g1 = make();\n"
		"const S& g2 = 4;\n"
		"const S& g3 = D();\n"
		"const long& g4 = 7;\n"
		"int main() { return g1.v * 1000 + g2.v * 100 + g3.v * 10 + (int)g4 + gDead * 10000; }\n"
	},
	{
		"RangeForTemporary", 1231,
		"int gDead = 0;\n"
		"struct L { int a[3]; L() { a[0] = 1; a[1] = 2; a[2] = 3; } ~L() { gDead++; } int* begin() { return a; } int* end() { return a + 3; } };\n"
		"L make() { return L(); }\n"
		"int main() { int t = 0; for (int x : make()) { t = t * 10 + x + gDead * 5; } return t * 10 + gDead; }\n"
	},
	{
		"NewArrayForms", 3457,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacArena[256];\n"
		"size_t guiArena = 0;\n"
		"void* Take(size_t n) { void* p = &gacArena[guiArena]; guiArena += (n + 7) & ~(size_t)7; return p; }\n"
		"void* operator new[](size_t n) { return Take(n); }\n"
		"void operator delete[](void* p) noexcept {}\n"
		"struct S { int v; S() : v(7) {} };\n"
		"int main() { int* p = new int[3]{ 1, 2, 3 }; int* q = new int[3](); q[1] += 4; int (*m)[4] = new int[3][4]; m[2][3] = 5; S* s = new S[2]; int r = p[2] * 1000 + q[1] * 100 + m[2][3] * 10 + s[1].v; delete[] p; delete[] q; delete[] m; delete[] s; return r; }\n"
	},
	{
		"PlacementNewArguments", 442,
		"typedef decltype(sizeof(0)) size_t;\n"
		"struct Tag { int k; };\n"
		"void* operator new(size_t n, const Tag& t, void* p) { return (char*)p + t.k; }\n"
		"void* operator new(size_t n, void* p) noexcept { return p; }\n"
		"struct S { int v; S(int a) : v(a) {} };\n"
		"int main() { char buf[16]; Tag t; t.k = 4; S* p = new (t, buf) S(4); S* q = new (buf) S(2); return p->v * 10 + q->v + ((char*)p - buf) * 100; }\n"
	},
	{
		"ConditionalOnClasses", 120,
		"struct A { int v; };\n"
		"struct B : A {};\n"
		"int main() { A a; B b; a.v = 1; b.v = 2; A r = (a.v ? a : b); A s = (a.v ? b : a); return r.v * 100 + s.v * 10; }\n"
	},
	{
		"ExternLocal", 132,
		"int g = 3;\n"
		"int h();\n"
		"int main() { extern int g; extern int k; int h(); return g * 10 + k + h(); }\n"
		"int k = 2;\n"
		"int h() { return 100; }\n"
	},
	{
		"ThreadLocal", 33,
		"thread_local int gt = 2;\n"
		"int f() { static thread_local int lt = 1; lt++; return lt; }\n"
		"int main() { gt++; f(); return gt * 10 + f(); }\n"
	},
	{
		"ImplicitCopyAndAssign", 2115,
		"struct M { int v; M() : v(1) {} M(const M& o) : v(o.v + 1) {} M& operator=(const M& o) { v = o.v + 10; return *this; } };\n"
		"struct W { int a; M m; char c[3]; };\n"
		"struct X : W { long l; };\n"
		"int main() { X a; a.a = 5; a.c[2] = 3; a.l = 9; X b(a); X c; c = a; return b.m.v * 1000 + c.m.v * 10 + b.a + b.c[2] + (int)c.l - 17 + 5; }\n"
	},
	{
		"ImplicitDestructorOrder", 321,
		"int g = 0;\n"
		"struct A { ~A() { g = g * 10 + 1; } };\n"
		"struct B { ~B() { g = g * 10 + 2; } };\n"
		"struct C : A { B b; ~C() { g = g * 10 + 3; } };\n"
		"int main() { { C c; } return g; }\n"
	},
	{
		"ArrayMemberCopy", 25,
		"struct E { int v; E() : v(1) {} E(const E& o) : v(o.v * 2) {} };\n"
		"struct S { E e[3]; int n[2]; };\n"
		"int main() { S a; a.n[1] = 5; S b = a; return b.e[2].v * 10 + b.n[1]; }\n"
	},
	{
		"CommaAndLogicalOverloads", 212,
		"struct S { int v; S operator,(const S& o) const { S r; r.v = v * 10 + o.v; return r; } bool operator!() const { return v == 0; } S operator~() const { S r; r.v = ~v; return r; } };\n"
		"int main() { S a; a.v = 1; S b; b.v = 2; S c = (a, b); return c.v + (!a ? 100 : 200) + (~b).v + 3; }\n"
	},
	{
		"ClassOperatorNew", 5,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacArena[256];\n"
		"size_t guiArena = 0;\n"
		"void* Take(size_t n) { void* p = &gacArena[guiArena]; guiArena += (n + 7) & ~(size_t)7; return p; }\n"
		"int gDeleted = 0;\n"
		"struct S { int v; void* operator new(size_t n) { return Take(n); } void operator delete(void* p) { gDeleted++; } };\n"
		"int main() { S* p = new S; p->v = 4; int r = p->v; delete p; return r + gDeleted; }\n"
	},
	{
		"DeleteThroughSecondaryBase", 3,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacArena[256];\n"
		"size_t guiArena = 0;\n"
		"void* gpLast = 0;\n"
		"int gRight = 0;\n"
		"void* operator new(size_t n) { gpLast = &gacArena[guiArena]; guiArena += (n + 7) & ~(size_t)7; return gpLast; }\n"
		"void operator delete(void* p) noexcept { gRight += p == gpLast; }\n"
		"void operator delete(void* p, size_t n) noexcept { gRight += p == gpLast; }\n"
		"struct X { int x; virtual ~X() {} };\n"
		"struct Y { int y; virtual ~Y() {} };\n"
		"struct Z : X, Y { };\n"
		"int main() { Y* p = new Z; delete p; X* q = new Z; delete q; Z* r = new Z; delete r; return gRight; }\n"
	},
	{
		"CopyKeepsDerivedVtable", 2,
		"struct Y { int y; virtual int f() { return 1; } Y() : y(0) {} Y(const Y& o) : y(o.y) {} };\n"
		"struct C : Y { int c; int f() override { return 2; } };\n"
		"int main() { C c; C d(c); Y* volatile p = &d; return p->f(); }\n"
	},
	{
		"AssignToBaseSubobject", 23,
		"struct Y { int y; virtual int g() { return 10; } };\n"
		"struct X { int x; virtual int f() { return 1; } };\n"
		"struct D : X, Y { int g() override { return 20; } };\n"
		"int main() { D d; Y y; y.y = 3; Y* volatile p = &d; *p = y; return p->g() + p->y; }\n"
	},
	{
		"AssignMembersToBaseSubobject", 21,
		"struct S { int s; S() : s(0) {} S& operator=(const S& o) { s = o.s + 1; return *this; } };\n"
		"struct X { int x; virtual int f() { return 1; } };\n"
		"struct Y { S m; virtual int g() { return 10; } };\n"
		"struct D : X, Y { int g() override { return 20; } };\n"
		"int main() { D d; Y y; Y* volatile p = &d; *p = y; return p->g() + p->m.s; }\n"
	},
	{
		"StaticCastToDerived", 57,
		"struct X { int x; };\n"
		"struct Y { int y; };\n"
		"struct Z : X, Y { int z; };\n"
		"int main() { Z z; z.z = 5; Y* py = &z; Z* pz = static_cast<Z*>(py); Y& ry = z; Z& rz = static_cast<Z&>(ry); Y* pn = 0;\n"
		"             return (pz == &z) + (&rz == &z) * 2 + (static_cast<Z*>(pn) == 0) * 4 + pz->z * 10; }\n"
	},
};


//Stage 8: initializer lists and literal operators.
static const SConformanceCase gasConformanceStage8[] =
{
	{
		"InitializerList", 6390,
		"#ifdef _MSC_VER\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"struct V { int s; int n; V(std::initializer_list<int> l) : s(0), n((int)l.size()) { for (const int* i = l.begin(); i != l.end(); ++i) { s += *i; } } };\n"
		"int sum(std::initializer_list<long> l) { int s = 0; for (const long* i = l.begin(); i != l.end(); ++i) { s += (int)*i; } return s; }\n"
		"int main() { V v{ 1, 2, 3 }; V w = { 4, 5 }; return v.s * 1000 + v.n * 100 + w.s * 10 + sum({ 1, 2 }) - 3; }\n"
	},
	{
		"InitializerListPreferred", 1232,
		"#ifdef _MSC_VER\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"struct V { int k; V(int a, int b) : k(1) {} V(std::initializer_list<int> l) : k(2) {} };\n"
		"struct W { int k; W(int a, int b) : k(3) {} };\n"
		"int main() { V a(1, 2); V b{ 1, 2 }; W c{ 1, 2 }; V d{}; return a.k * 1000 + b.k * 100 + c.k * 10 + d.k; }\n"
	},
	{
		"InitializerListRangeFor", 15,
		"#ifdef _MSC_VER\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"int main() { int s = 0; for (int x : { 1, 2, 3 }) { s += x; } auto l = { 4, 5 }; for (int x : l) { s += x; } return s; }\n"
	},
	{
		"InitializerListForms", 5760,
		"#ifdef _MSC_VER\n"
		"#include <initializer_list>\n"
		"#else\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; const T* mpLast; public: constexpr initializer_list() noexcept : mpFirst(0), mpLast(0) {} constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpLast; } }; }\n"
		"#endif\n"
		"struct D { int k; D() : k(7) {} D(std::initializer_list<int> l) : k(0) { for (int x : l) { k += x; } } };\n"
		"int total(std::initializer_list<double> l) { double t = 0; for (double d : l) { t += d; } return (int)(t * 2); }\n"
		"int main() { auto x{5}; D a{}; D b{1, 2, 3}; return x * 1000 + a.k * 100 + b.k * 10 + total({1.5, 2.0}) - 7; }\n"
	},
	{
		"LiteralOperators", 2542,
		"typedef decltype(sizeof(0)) size_t;\n"
		"long long operator\"\" _k(unsigned long long v) { return (long long)v * 1000; }\n"
		"long double operator\"\" _half(long double v) { return v / 2; }\n"
		"int operator\"\" _len(const char* s, size_t n) { return (int)n; }\n"
		"int operator\"\" _c(char c) { return c - 'a'; }\n"
		"int main() { return (int)(2_k / 1000) * 1000 + (int)(5.0_half * 2) * 100 + \"abcd\"_len * 10 + 'c'_c; }\n"
	},
	{
		"LiteralOperatorRaw", 354,
		"int operator\"\" _digits(const char* s) { int n = 0; while (s[n]) { n++; } return n; }\n"
		"template<char... C> int operator\"\" _count() { return sizeof...(C); }\n"
		"int main() { return 12345_digits * 10 + 0x1F_digits + 123_count * 100; }\n"
	},
	{
		"LiteralOperatorForms", 8241,
		"typedef decltype(sizeof(0)) size_t;\n"
		"struct M { long v; };\n"
		"M operator\"\" _m(unsigned long long v) { M m; m.v = (long)v * 2; return m; }\n"
		"int operator\"\" _len(const char* s, size_t n) { return (int)n; }\n"
		"int operator\"\" _wlen(const wchar_t* s, size_t n) { return (int)n * 10; }\n"
		"int operator\"\" _raw(const char* s) { int n = 0; while (s[n]) { n++; } return n; }\n"
		"int main() { M m = 21_m; return (int)m.v * 100 + \"ab\" \"cd\"_len * 10 + (L\"xyz\"_wlen == 30) + 1.25_raw * 1000; }\n"
	},
	{
		"LiteralOperatorMissing", CONFORMANCE_REFUSED,
		"int main() { return 12_nothere; }\n"
	},
	{
		"BracedListArgument", 1117,
		"struct S { int a; int b; };\n"
		"int f(S s) { return s.a + s.b * 2; }\n"
		"S g() { return { 5, 6 }; }\n"
		"int main() { return f({3, 4}) * 100 + f(g()); }\n"
	},
	{
		"NoexceptOperator", 15,
		"int f() noexcept;\n"
		"int g();\n"
		"struct S { ~S() {} void h() noexcept {} };\n"
		"int main() { S s; return noexcept(f()) + !noexcept(g()) * 2 + noexcept(1 + 2) * 4 + noexcept(s.h()) * 8; }\n"
	},
	{
		"BracedListOverloads", 12130,
		"struct S { int a; int b; };\n"
		"struct P { int x; P(int a, int b) : x(a * b) {} };\n"
		"int f(const S& s) { return s.a - s.b; }\n"
		"int g(int x) { return 1; }\n"
		"int g(S s) { return 2; }\n"
		"int h(long x) { return 3; }\n"
		"int h(const char* p) { return 4; }\n"
		"int k(P p) { return p.x; }\n"
		"int m(double d) { return (int)d; }\n"
		"int main() { return (f({9, 4}) == 5) * 10000 + g({1, 2}) * 1000 + g({7}) * 100 + h({5}) * 10 + k({2, 3}) - 6 + m({}); }\n"
	},
	{
		"NoexceptForms", 4095,
		"int f() noexcept;\n"
		"int g();\n"
		"int k() throw();\n"
		"int m() noexcept(false);\n"
		"struct S { ~S() {} void h() noexcept {} void j() {} S operator+(const S&) const noexcept { return *this; } S operator-(const S&) const { return *this; } };\n"
		"int main() { S s; int x = 0; return noexcept(f()) + !noexcept(g()) * 2 + noexcept(1 + 2) * 4 + noexcept(s.h()) * 8 + !noexcept(s.j()) * 16 + noexcept(k()) * 32 + !noexcept(m()) * 64 + noexcept(s + s) * 128 + !noexcept(s - s) * 256 + noexcept(x = 3) * 512 + !noexcept(f() + g()) * 1024 + !noexcept(new int) * 2048 + x * 4096; }\n"
	},
};


//Stage 9: type information and virtual inheritance.
static const SConformanceCase gasConformanceStage9[] =
{
	{
		"Typeid", 127,
		"#ifdef _MSC_VER\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"struct B { virtual ~B() {} };\n"
		"struct D : B {};\n"
		"struct N {};\n"
		"int main() { D d; B b; B& r = d; B* p = &d; return (typeid(r) == typeid(D)) + (typeid(b) == typeid(B)) * 2 + (typeid(*p) != typeid(B)) * 4 + (typeid(int) == typeid(int)) * 8 + (typeid(N) != typeid(D)) * 16 + (typeid(p) == typeid(B*)) * 32 + (typeid(const int) == typeid(int)) * 64; }\n"
	},
	{
		"TypeidForms", 63,
		"#ifdef _MSC_VER\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"struct A { virtual ~A() {} int a; };\n"
		"struct B { virtual ~B() {} int b; };\n"
		"struct C : A, B { int c; };\n"
		"template<class T> bool same(const T& x, const std::type_info& t) { return typeid(x) == t; }\n"
		"int main() { C c; B* pb = &c; A* pa = &c; const B& rb = c; return (typeid(*pb) == typeid(C)) + (typeid(*pa) == typeid(*pb)) * 2 + (typeid(rb) == typeid(C)) * 4 + same(c, typeid(C)) * 8 + (typeid(pb) != typeid(pa)) * 16 + (typeid(char*) != typeid(const char*)) * 32; }\n"
	},
	{
		"DynamicCastReference", 44,
		"struct B { virtual ~B() {} };\n"
		"struct D : B { int v; };\n"
		"int main() { D d; d.v = 4; B& b = d; D& r = dynamic_cast<D&>(b); const B& cb = d; const D& cr = dynamic_cast<const D&>(cb); return r.v * 10 + cr.v; }\n"
	},
	{
		"DynamicCastPointers", 15,
		"struct A { virtual ~A() {} int a; };\n"
		"struct B { virtual ~B() {} int b; };\n"
		"struct C : A, B { int c; };\n"
		"struct E : A {};\n"
		"int main() { C c; A* a = &c; B* b = dynamic_cast<B*>(a); E* e = dynamic_cast<E*>(a); void* v = dynamic_cast<void*>(b); return (b == (B*)&c) + (e == 0) * 2 + (v == (void*)&c) * 4 + (dynamic_cast<C*>(b) == &c) * 8; }\n"
	},
	{
		"VirtualDiamond", 73,
		"struct A { int a; };\n"
		"struct B : virtual A { int b; };\n"
		"struct C : virtual A { int c; };\n"
		"struct D : B, C { int d; };\n"
		"int main() { D d; d.a = 1; B* b = &d; C* c = &d; b->a += 2; c->a += 4; A* a = &d; return d.a * 10 + (a == (A*)b) + (a == (A*)c) * 2; }\n"
	},
	{
		"VirtualBaseConstructedOnce", 973,
		"int g = 0;\n"
		"struct A { int a; A(int x) : a(x) { g = g * 10 + 1; } A() : a(9) { g = g * 10 + 5; } };\n"
		"struct B : virtual A { B() : A(2) { g = g * 10 + 2; } };\n"
		"struct C : virtual A { C() : A(3) { g = g * 10 + 3; } };\n"
		"struct D : B, C { D() : A(7) { g = g * 10 + 4; } };\n"
		"struct E : B, C { E() { g = g * 10 + 6; } };\n"
		"int main() { D d; int first = g; g = 0; E e; return (first == 1234) + (g == 5236) * 2 + d.a * 10 + e.a * 100; }\n"
	},
	{
		"VirtualBaseDestroyedOnce", 4321,
		"int g = 0;\n"
		"struct A { ~A() { g = g * 10 + 1; } };\n"
		"struct B : virtual A { ~B() { g = g * 10 + 2; } };\n"
		"struct C : virtual A { ~C() { g = g * 10 + 3; } };\n"
		"struct D : B, C { ~D() { g = g * 10 + 4; } };\n"
		"int main() { { D d; } return g; }\n"
	},
	{
		"VirtualFunctionsThroughVirtualBase", 15032,
		"struct A { virtual int f() { return 1; } virtual int g() { return 10; } int call() { return f() + g(); } };\n"
		"struct B : virtual A { int f() override { return 2; } };\n"
		"struct C : virtual A { int g() override { return 30; } };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A* a = &d; B* b = &d; C* c = &d; return a->f() + a->g() + b->g() * 100 + c->f() * 1000 + (a->call() == 32) * 10000; }\n"
	},
	{
		"VirtualBaseCasts", 515,
		"struct A { virtual ~A() {} int a; };\n"
		"struct B : virtual A { int b; };\n"
		"struct C : virtual A { int c; };\n"
		"struct D : B, C { int d; };\n"
		"int main() { D d; d.a = 5; A* a = &d; B* b = &d; D* pd = dynamic_cast<D*>(a); C* pc = dynamic_cast<C*>(b); B* pb = dynamic_cast<B*>(a); A* up = pc; return (pd == &d) + (pc == (C*)&d) * 2 + (pb == b) * 4 + (up == a) * 8 + up->a * 100; }\n"
	},
	{
		"VirtualBaseInConstructor", 2123,
		"struct A { int a; A() : a(1) {} virtual int f() { return 1; } };\n"
		"struct B : virtual A { int seen; B() : seen(f() * 10 + a) { a = 2; } int f() override { return 2; } };\n"
		"struct C : virtual A { int seen; C() : seen(a) { a = 3; } };\n"
		"struct D : B, C { int f() override { return 4; } };\n"
		"int main() { D d; return d.B::seen * 100 + d.C::seen * 10 + d.a; }\n"
	},
	{
		"VirtualBaseMemberPointers", 43,
		"struct A { int a; int fa() { return a; } };\n"
		"struct B : virtual A { int b; };\n"
		"struct D : B { int d; };\n"
		"int main() { D d; d.a = 3; d.b = 4; int B::*pb = &B::b; int (A::*pf)() = &A::fa; A* a = &d; return d.*pb * 10 + (a->*pf)(); }\n"
	},
	{
		"VirtualBaseMixed", 237,
		"struct A { int a; A() : a(1) {} };\n"
		"struct B : virtual A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"struct E : virtual A, virtual B { };\n"
		"int main() { D d; d.B::a = 2; d.C::a = 3; E e; e.a = 7; B* b = &e; return d.B::a * 100 + d.C::a * 10 + b->a; }\n"
	},
	{
		"VirtualBaseAssignmentAndCopy", 1413,
		"struct A { int a; };\n"
		"struct B : virtual A { int b; };\n"
		"struct C : virtual A { int c; };\n"
		"struct D : B, C { int d; };\n"
		"int main() { D x; x.a = 1; x.b = 2; x.c = 3; x.d = 4; D y(x); D z; z = x; x.a = 9; return y.a * 1000 + y.d * 100 + z.a * 10 + z.c; }\n"
	},
	{
		"VirtualBaseEmptyAndDeep", 233,
		"struct A { virtual int f() { return 1; } };\n"
		"struct B : virtual A { };\n"
		"struct C : virtual B { int f() override { return 3; } };\n"
		"struct D : virtual C, virtual A { int v; };\n"
		"int main() { D d; d.v = 2; A* a = &d; B* b = &d; return a->f() * 10 + b->f() + d.v * 100; }\n"
	},
	{
		"VirtualBaseNestedConstruction", 3034,
		"struct V { int v; V() : v(10) {} virtual int id() { return 1; } };\n"
		"struct X : virtual V { int x; X() : x(id() * 10 + v) {} int id() override { return 2; } };\n"
		"struct B : X { int b; B() : b(id()) {} int id() override { return 3; } };\n"
		"struct D : B { int id() override { return 4; } };\n"
		"int main() { D d; return d.x * 100 + d.b * 10 + d.id(); }\n"
	},
	{
		"VirtualBaseDestructorCalls", 4121,
		"int g = 0;\n"
		"struct A { virtual int f() { return 1; } virtual ~A() { g = g * 10 + f(); } };\n"
		"struct B : virtual A { int f() override { return 2; } ~B() { g = g * 10 + f(); } };\n"
		"struct C : virtual A { ~C() { g = g * 10 + f(); } };\n"
		"struct D : B, C { int f() override { return 4; } ~D() { g = g * 10 + f(); } };\n"
		"int main() { { D d; } return g; }\n"
	},
	{
		"VirtualBaseTypeidInConstruction", 33,
		"#ifdef _MSC_VER\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"struct A { int a; virtual ~A() {} };\n"
		"struct Z { int z; virtual ~Z() {} };\n"
		"struct D;\n"
		"struct B : virtual A { int t; B(); };\n"
		"struct D : Z, B { int u; D() { A* a = this; u = (typeid(*this) == typeid(D)) + (dynamic_cast<D*>(a) == this) * 2; } };\n"
		"B::B() { A* a = this; t = (typeid(*a) == typeid(B)) + (dynamic_cast<D*>(a) == 0) * 2; }\n"
		"int main() { D d; return d.t * 10 + d.u; }\n"
	},
	{
		"VirtualBaseDelegating", 6565,
		"struct A { int a; A() : a(1) {} };\n"
		"struct B : virtual A { int b; B(int x) : b(x) { a += x; } B() : B(5) {} };\n"
		"struct D : B { D() : B() {} };\n"
		"int main() { D d; B b; return d.a * 1000 + d.b * 100 + b.a * 10 + b.b; }\n"
	},
	{
		"VirtualBaseDeleteThrough", 22333,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacArena[512];\n"
		"size_t guiArena = 0;\n"
		"void* gpLast = 0;\n"
		"int gRight = 0;\n"
		"void* operator new(size_t n) { gpLast = &gacArena[guiArena]; guiArena += (n + 7) & ~(size_t)7; return gpLast; }\n"
		"void operator delete(void* p) noexcept { gRight += p == gpLast; }\n"
		"void operator delete(void* p, size_t n) noexcept { gRight += p == gpLast; }\n"
		"int n = 0;\n"
		"struct A { int a; virtual ~A() { n += 1; } };\n"
		"struct B : virtual A { int b; ~B() { n += 10; } };\n"
		"struct C : virtual A { int c; ~C() { n += 100; } };\n"
		"struct D : B, C { int d; ~D() { n += 1000; } };\n"
		"int main() { A* p = new B; delete p; C* q = new D; delete q; A* r = new D; delete r; return n * 10 + gRight; }\n"
	},
	{
		"VirtualBaseArraysAndMembers", 5964,
		"struct A { int a; A() : a(3) {} };\n"
		"struct B : virtual A { int b; B() : b(a * 2) {} };\n"
		"struct C : virtual A { int c; C() : c(a + 1) { a = 5; } };\n"
		"struct D : B, C { };\n"
		"struct E { int e; D ds[2]; E() : e(1) { ds[1].a = 9; } };\n"
		"int main() { E x; D* p = &x.ds[0]; return x.ds[0].a * 1000 + x.ds[1].a * 100 + p->b * 10 + p->c; }\n"
	},
	{
		"VirtualBaseCopyConstructors", 15705,
		"struct A { int a; A() : a(1) {} A(const A& o) : a(o.a + 10) {} A& operator=(const A& o) { a = o.a + 100; return *this; } };\n"
		"struct B : virtual A { int b; B() : b(2) {} };\n"
		"struct C : virtual A { int c; C() : c(3) {} };\n"
		"struct D : B, C { };\n"
		"int main() { D d; d.a = 5; d.b = 6; D e(d); D f; f = d; return e.a * 1000 + e.b * 100 + f.a; }\n"
	},
	{
		"VirtualBaseCopyFromSubobject", 78,
		"struct A { int a; A() : a(1) {} };\n"
		"struct B : virtual A { int b; B() : b(2) {} };\n"
		"struct E : B { int pad[3]; int e; };\n"
		"int main() { E x; x.a = 7; x.b = 8; B& r = x; B y(r); B* volatile p = &y; return p->a * 10 + p->b; }\n"
	},
	{
		"VirtualBaseBaseCopyConstructor", 1143,
		"int g = 0;\n"
		"struct A { int a; A() : a(1) {} A(const A& o) : a(o.a) { g += 1; } };\n"
		"struct B : virtual A { int b; B() : b(2) {} B(const B& o) : A(o), b(o.b + 1) { g += 10; } };\n"
		"struct C : virtual A { int c; C() : c(3) {} };\n"
		"struct D : B, C { };\n"
		"int main() { D d; d.a = 4; D e(d); return g * 100 + e.a * 10 + e.b; }\n"
	},
	{
		"VirtualBaseAssignment", 15679,
		"struct A { int a; A() : a(1) {} };\n"
		"struct B : virtual A { int b; };\n"
		"struct C : virtual A { int c; };\n"
		"struct D : B, C { int d; };\n"
		"struct E : B { int pad[2]; };\n"
		"int main() { D x, y; x.a = 5; x.b = 6; x.c = 7; y = x; E z; B& rb = z; B s; s.a = 9; s.b = 4; rb = s;\n"
		"             return y.a * 1000 + y.b * 100 + y.c * 10 + z.a + (z.b == 4) * 10000; }\n"
	},
	{
		"VirtualBaseStaticCastRefused", -1,
		"struct A { int a; virtual ~A() {} };\n"
		"struct B : virtual A { int b; };\n"
		"int main() { B b; A* a = &b; B* p = static_cast<B*>(a); return p != 0; }\n"
	},
};

//Stage 10: namespaces, and the lookup they change, as C++14 has them (N4140 [basic.namespace], [namespace.udecl],
//[namespace.udir], [basic.lookup]): each case is a paragraph, its number over it, and was checked with the Microsoft
//compiler (C++14, /permissive-).
static const SConformanceCase gasConformanceStage10[] =
{
	//7.3.1/2
	{
		"ExtendNamespace", 3,
		"namespace N { int a = 1; }\n"
		"namespace N { int b = 2; }\n"
		"int main() { return N::a + N::b; }\n"
	},
	//7.3.1.2/2
	{
		"MemberDefinedOutside", 4,
		"namespace N { int f(); }\n"
		"int N::f() { return 4; }\n"
		"int main() { return N::f(); }\n"
	},
	//7.3.1.2/2
	{
		"NestedMemberDefinedInEnclosing", 5,
		"namespace A { namespace B { int g(); } int B::g() { return 5; } }\n"
		"int main() { return A::B::g(); }\n"
	},
	//7.3.1.2/2
	{
		"DefinedInNonEnclosing", CONFORMANCE_REFUSED,
		"namespace A { int f(); }\n"
		"namespace C { int A::f() { return 1; } }\n"
		"int main() { return A::f(); }\n"
	},
	//7.3.1/4
	{
		"NamespaceInBlockScope", CONFORMANCE_REFUSED,
		"int main() { namespace X { int v = 1; } return 0; }\n"
	},
	//7.3.1
	{
		"SameNameDifferentNamespaces", 12,
		"namespace A { int v = 1; }\n"
		"namespace B { int v = 2; }\n"
		"int main() { return A::v * 10 + B::v; }\n"
	},
	//7.3.1/2
	{
		"OverloadsAcrossReopen", 12,
		"namespace N { int f(int) { return 1; } }\n"
		"namespace N { int f(char) { return 2; } }\n"
		"int main() { return N::f(1) * 10 + N::f('a'); }\n"
	},
	//3.4.1/6
	{
		"MemberDefinitionLooksInNamespace", 3,
		"namespace N { int k = 3; int f(); }\n"
		"int N::f() { return k; }\n"
		"int main() { return N::f(); }\n"
	},
	//3.4.1/8
	{
		"MemberDefinitionParameterType", 5,
		"namespace N { struct T { int v; }; struct S { int f(T t); }; }\n"
		"int N::S::f(T t) { return t.v; }\n"
		"int main() { N::S s; N::T t; t.v = 5; return s.f(t); }\n"
	},
	//8.3.6
	{
		"DefaultArgumentInNamespace", 4,
		"namespace N { int d = 4; int f(int x = d) { return x; } }\n"
		"int main() { return N::f(); }\n"
	},
	//7.3.1
	{
		"TypedefInNamespace", 3,
		"namespace N { typedef int I; }\n"
		"int main() { N::I x = 3; return x; }\n"
	},
	//3.3.10
	{
		"InnerHidesOuter", 2,
		"namespace A { int f() { return 1; } namespace B { int f(int) { return 2; } int g() { return f(5); } } }\n"
		"int main() { return A::B::g(); }\n"
	},
	//3.3.10
	{
		"InnerHidesOuterRefused", CONFORMANCE_REFUSED,
		"namespace A { int f() { return 1; } namespace B { int f(int) { return 2; } int g() { return f(); } } }\n"
		"int main() { return A::B::g(); }\n"
	},
	//9.4.2
	{
		"StaticMemberDefinedOutside", 5,
		"namespace N { struct S { static int s; }; }\n"
		"int N::S::s = 5;\n"
		"int main() { return N::S::s; }\n"
	},
	//14.7.3
	{
		"TemplateSpecialisedInNamespace", 12,
		"namespace N { template<class T> struct X { enum { v = 1 }; }; template<> struct X<int> { enum { v = 2 }; }; }\n"
		"int main() { return N::X<char>::v * 10 + N::X<int>::v; }\n"
	},
	//7.2
	{
		"EnumClassInNamespace", 2,
		"namespace N { enum class E { a = 1, b = 2 }; }\n"
		"int main() { return (int)N::E::b; }\n"
	},
	//7.3.1 (C++17 only)
	{
		"NestedNamespaceDefinitionCpp17", CONFORMANCE_REFUSED,
		"namespace A::B { int v = 1; }\n"
		"int main() { return A::B::v; }\n"
	},
	//7.3.1.1
	{
		"UnnamedInNamed", 7,
		"namespace N { namespace { int h() { return 7; } } }\n"
		"int main() { return N::h(); }\n"
	},
	//7.3.1.1/3.4.1
	{
		"UnnamedAmbiguousWithGlobal", CONFORMANCE_REFUSED,
		"int x = 1;\n"
		"namespace { int x = 2; }\n"
		"int main() { return x; }\n"
	},
	//7.3.1.1
	{
		"UnnamedReopened", 3,
		"namespace { int a = 1; }\n"
		"namespace { int b = 2; }\n"
		"int main() { return a + b; }\n"
	},
	//7.3.1.1
	{
		"UnnamedQualifiedGlobal", 2,
		"int x = 1;\n"
		"namespace { int y = 2; }\n"
		"int main() { return ::y; }\n"
	},
	//7.3.1.2/3
	{
		"FriendFoundByADL", 9,
		"namespace N { struct S { friend int fr(S) { return 9; } }; }\n"
		"int main() { N::S s; return fr(s); }\n"
	},
	//7.3.1.2/3
	{
		"FriendNotFoundByQualifiedLookup", CONFORMANCE_REFUSED,
		"namespace N { struct S { friend int fr2() { return 1; } }; }\n"
		"int main() { return N::fr2(); }\n"
	},
	//7.3.1.2/3
	{
		"FriendThenDeclared", 3,
		"namespace N { struct S { friend int fr3(); }; int fr3(); }\n"
		"int N::fr3() { return 3; }\n"
		"int main() { return N::fr3(); }\n"
	},
	//7.3.1/7
	{
		"InlineReopened", 3,
		"namespace N { inline namespace V { int a = 1; } }\n"
		"namespace N { inline namespace V { int b = 2; } }\n"
		"int main() { return N::a + N::b; }\n"
	},
	//7.3.1/8
	{
		"InlineSpecialisedFromEnclosing", 2,
		"namespace N { inline namespace V { template<class T> int f(T) { return 1; } } template<> int f<int>(int) { return 2; } }\n"
		"int main() { return N::f(0); }\n"
	},
	//3.4.2/2
	{
		"InlineADL", 4,
		"namespace N { inline namespace V { struct S {}; } int g(S) { return 4; } }\n"
		"int main() { return g(N::S()); }\n"
	},
	//7.3.1/8
	{
		"InlineNested", 5,
		"namespace N { inline namespace A { inline namespace B { int z = 5; } } }\n"
		"int main() { return N::z; }\n"
	},
	//7.3.1/8
	{
		"InlineUsingDirective", 6,
		"namespace N { inline namespace V { int w = 6; } }\n"
		"using namespace N;\n"
		"int main() { return w; }\n"
	},
	//7.3.2
	{
		"AliasOfAlias", 6,
		"namespace A { int v = 6; }\n"
		"namespace B = A;\n"
		"namespace C = B;\n"
		"int main() { return C::v; }\n"
	},
	//7.3.2
	{
		"AliasInBlock", 6,
		"namespace A { int v = 6; }\n"
		"int main() { namespace X = A; return X::v; }\n"
	},
	//7.3.2/3
	{
		"AliasRedefinedSame", 1,
		"namespace A { int v = 1; }\n"
		"namespace B = A;\n"
		"namespace B = A;\n"
		"int main() { return B::v; }\n"
	},
	//7.3.2/3
	{
		"AliasRedefinedDifferent", CONFORMANCE_REFUSED,
		"namespace A { int v = 1; }\n"
		"namespace D { int v = 2; }\n"
		"namespace B = A;\n"
		"namespace B = D;\n"
		"int main() { return B::v; }\n"
	},
	//7.3.2
	{
		"AliasQualifiedTarget", 4,
		"namespace A { namespace B { int v = 4; } }\n"
		"namespace Z = ::A::B;\n"
		"int main() { return Z::v; }\n"
	},
	//7.3.3
	{
		"UsingDeclarationBlock", 2,
		"namespace N { int f() { return 2; } }\n"
		"int main() { using N::f; return f(); }\n"
	},
	//7.3.3/11
	{
		"UsingDeclarationOnlyEarlierOverloads", 1,
		"namespace N { int f(int) { return 1; } }\n"
		"using N::f;\n"
		"namespace N { int f(char) { return 2; } }\n"
		"int main() { return f('a'); }\n"
	},
	//7.3.3/15
	{
		"UsingDeclarationInClass", 12,
		"struct B { int f(int) { return 1; } };\n"
		"struct D : B { using B::f; int f(char) { return 2; } };\n"
		"int main() { D d; return d.f(1) * 10 + d.f('a'); }\n"
	},
	//7.3.3
	{
		"UsingDeclarationIntoNamespace", 3,
		"namespace A { int x = 3; }\n"
		"namespace B { using A::x; }\n"
		"int main() { return B::x; }\n"
	},
	//7.3.3
	{
		"UsingDeclarationClassType", 4,
		"namespace A { struct S { int v; }; }\n"
		"using A::S;\n"
		"int main() { S s; s.v = 4; return s.v; }\n"
	},
	//7.3.3/6
	{
		"UsingDeclarationOfNamespace", CONFORMANCE_REFUSED,
		"namespace A { namespace B { int v = 1; } }\n"
		"using A::B;\n"
		"int main() { return 0; }\n"
	},
	//7.3.3/14
	{
		"UsingDeclarationConflicts", CONFORMANCE_REFUSED,
		"namespace A { int x = 1; }\n"
		"int x = 2;\n"
		"using A::x;\n"
		"int main() { return x; }\n"
	},
	//7.3.3
	{
		"UsingDeclarationEnumerator", 7,
		"namespace A { enum E { e1 = 7 }; }\n"
		"using A::e1;\n"
		"int main() { return e1; }\n"
	},
	//7.3.3/13
	{
		"UsingDeclarationWithLocalOverload", 12,
		"namespace A { int f(int) { return 1; } }\n"
		"int f(char) { return 2; }\n"
		"using A::f;\n"
		"int main() { return f(1) * 10 + f('c'); }\n"
	},
	//7.3.3/19
	{
		"UsingDeclarationAccess", 5,
		"struct B { protected: int p() { return 5; } };\n"
		"struct D : B { public: using B::p; };\n"
		"int main() { D d; return d.p(); }\n"
	},
	//12.9
	{
		"InheritingConstructors", 6,
		"struct B { int v; B(int x) : v(x) {} };\n"
		"struct D : B { using B::B; };\n"
		"int main() { D d(6); return d.v; }\n"
	},
	//7.3.3
	{
		"UsingDeclarationTemplate", 6,
		"namespace N { template<class T> T id(T x) { return x; } }\n"
		"using N::id;\n"
		"int main() { return id(6); }\n"
	},
	//7.3.4/4
	{
		"UsingDirectiveTransitive", 3,
		"namespace A { int a = 1; }\n"
		"namespace B { using namespace A; int b = 2; }\n"
		"using namespace B;\n"
		"int main() { return a + b; }\n"
	},
	//7.3.4
	{
		"UsingDirectiveBlock", 1,
		"namespace A { int a = 1; }\n"
		"int main() { using namespace A; return a; }\n"
	},
	//7.3.4/6
	{
		"UsingDirectiveAmbiguityUnused", 3,
		"namespace A { int x = 1; }\n"
		"namespace B { int x = 2; }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return A::x + B::x; }\n"
	},
	//7.3.4/6
	{
		"UsingDirectiveAmbiguityUsed", CONFORMANCE_REFUSED,
		"namespace A { int x = 1; }\n"
		"namespace B { int x = 2; }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return x; }\n"
	},
	//7.3.4/2
	{
		"UsingDirectiveNearestEnclosing", 5,
		"namespace A { int i = 4; }\n"
		"namespace B { int i = 5; int g() { using namespace A; return i; } }\n"
		"int main() { return B::g(); }\n"
	},
	//7.3.4/6
	{
		"UsingDirectiveOverloadsMerge", 12,
		"namespace A { int f(int) { return 1; } }\n"
		"namespace B { int f(char) { return 2; } }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return f(1) * 10 + f('c'); }\n"
	},
	//7.3.4/2
	{
		"UsingDirectiveLaterMembers", 8,
		"namespace A { int a = 1; }\n"
		"using namespace A;\n"
		"namespace A { int later = 8; }\n"
		"int main() { return later; }\n"
	},
	//7.3.4
	{
		"UsingDirectiveTemplateExplicitArgs", 6,
		"namespace N { template<class T> T id(T x) { return x; } }\n"
		"using namespace N;\n"
		"int main() { return id<int>(6); }\n"
	},
	//3.4.3.2/2
	{
		"QualifiedThroughDirective", 7,
		"namespace A { int a = 7; }\n"
		"namespace B { using namespace A; }\n"
		"int main() { return B::a; }\n"
	},
	//3.4.3.2/2
	{
		"QualifiedOwnHidesNominated", 2,
		"namespace A { int x = 1; }\n"
		"namespace B { using namespace A; int x = 2; }\n"
		"int main() { return B::x; }\n"
	},
	//3.4.3.2/2
	{
		"QualifiedAmbiguous", CONFORMANCE_REFUSED,
		"namespace A { int x = 1; }\n"
		"namespace C { int x = 2; }\n"
		"namespace B { using namespace A; using namespace C; }\n"
		"int main() { return B::x; }\n"
	},
	//3.4.3.2/2
	{
		"QualifiedThroughDirectiveNested", 8,
		"namespace A { int a = 8; }\n"
		"namespace C { using namespace A; }\n"
		"namespace B { using namespace C; }\n"
		"int main() { return B::a; }\n"
	},
	//3.4.3/4
	{
		"GlobalQualifier", 12,
		"int v = 1;\n"
		"namespace N { int v = 2; int f() { return ::v * 10 + v; } }\n"
		"int main() { return N::f(); }\n"
	},
	//3.4.3
	{
		"GlobalQualifiedNamespace", 3,
		"namespace N { int f() { return 3; } }\n"
		"int main() { return ::N::f(); }\n"
	},
	//3.4.2
	{
		"ADLOperator", 7,
		"namespace N { struct V { int v; }; int operator+(V a, V b) { return a.v + b.v; } }\n"
		"int main() { N::V a; N::V b; a.v = 3; b.v = 4; return a + b; }\n"
	},
	//3.4.2/2
	{
		"ADLTemplateArgument", 8,
		"template<class T> struct W {};\n"
		"namespace N { struct S {}; int k(W<S>) { return 8; } }\n"
		"int main() { return k(W<N::S>()); }\n"
	},
	//3.4.2/2
	{
		"ADLBaseClass", 9,
		"namespace N { struct B {}; int m(const B&) { return 9; } }\n"
		"struct D : N::B {};\n"
		"int main() { D d; return m(d); }\n"
	},
	//3.4.2/1
	{
		"ADLSuppressedByParentheses", CONFORMANCE_REFUSED,
		"namespace N { struct S {}; int p(S) { return 1; } }\n"
		"int main() { N::S s; return (p)(s); }\n"
	},
	//3.4.2/2
	{
		"ADLEnum", 3,
		"namespace N { enum E { a }; int q(E) { return 3; } }\n"
		"int main() { return q(N::a); }\n"
	},
	//3.4.2/2
	{
		"ADLPointer", 4,
		"namespace N { struct S {}; int r(S*) { return 4; } }\n"
		"int main() { N::S s; return r(&s); }\n"
	},
	//3.4.2/3
	{
		"ADLBlockDeclarationStops", 2,
		"namespace N { struct S {}; int t(S) { return 1; } }\n"
		"int t(N::S, int i = 0) { return 2; }\n"
		"int main() { int t(N::S, int); N::S s; return t(s, 0); }\n"
	},
	//3.4.2/3
	{
		"ADLMemberFunctionStops", 3,
		"namespace N { struct S {}; int u(S) { return 1; } }\n"
		"struct C { int u(N::S) { return 3; } int go() { N::S s; return u(s); } };\n"
		"int main() { C c; return c.go(); }\n"
	},
	//3.4.2/2
	{
		"ADLFunctionArgument", 5,
		"namespace N { struct S {}; int z(void (*)(S)) { return 5; } void cb(S) {} }\n"
		"int main() { return z(N::cb); }\n"
	},
	//7.5/6
	{
		"ExternCSameFunction", 6,
		"namespace A { extern \"C\" int cf(); }\n"
		"namespace B { extern \"C\" int cf(); }\n"
		"extern \"C\" int cf() { return 6; }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return cf(); }\n"
	},
	//Each fundamental type is a type of its own however wide (CppParserTodo.txt question 13).
	//3.9.1/2, 13.3.3.1.1
	{
		"DistinctIntLong", 12,
		"int f(int) { return 1; }\n"
		"int f(long) { return 2; }\n"
		"int main() { return f(1) * 10 + f(1L); }\n"
	},
	//3.9.1/2
	{
		"DistinctShortInt", 12,
		"int f(short) { return 1; }\n"
		"int f(int) { return 2; }\n"
		"int main() { short s = 1; return f(s) * 10 + f(1); }\n"
	},
	//3.9.1/1
	{
		"DistinctChars", 123,
		"int f(char) { return 1; }\n"
		"int f(signed char) { return 2; }\n"
		"int f(unsigned char) { return 3; }\n"
		"int main() { return f((char)1) * 100 + f((signed char)1) * 10 + f((unsigned char)1); }\n"
	},
	//3.9.1/5
	{
		"DistinctWideChars", 123,
		"int f(wchar_t) { return 1; }\n"
		"int f(char16_t) { return 2; }\n"
		"int f(char32_t) { return 3; }\n"
		"int main() { return f(L'a') * 100 + f(u'a') * 10 + f(U'a'); }\n"
	},
	//14.4
	{
		"DistinctSpecialisation", 1234,
		"template<class T> struct S { enum { V = 1 }; };\n"
		"template<> struct S<long> { enum { V = 2 }; };\n"
		"template<> struct S<short> { enum { V = 3 }; };\n"
		"template<> struct S<signed char> { enum { V = 4 }; };\n"
		"int main() { return S<int>::V * 1000 + S<long>::V * 100 + S<short>::V * 10 + S<signed char>::V; }\n"
	},
	//4.5/1, 13.3.3.1.1/3
	{
		"DistinctPromotionRank", 21,
		"int f(int) { return 1; }\n"
		"int f(long) { return 2; }\n"
		"int g(long) { return 2; }\n"
		"int g(double) { return 3; }\n"
		"int main() { short s = 1; long l = 1; return f(l) * 10 + f(s) + g(l) * 0; }\n"
	},
	//2.14.3, 2.14.5
	{
		"LiteralTypes", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main() { auto b = u\"ab\"; auto c = U\"ab\"; auto d = L\"ab\";\n"
		" return Same<decltype('a'), char>::V + Same<decltype(L'a'), wchar_t>::V * 2 + Same<decltype(u'a'), char16_t>::V * 4 + Same<decltype(U'a'), char32_t>::V * 8 + Same<decltype(1L), long>::V * 16 + Same<decltype(1UL), unsigned long>::V * 32 + (Same<decltype(b), const char16_t*>::V + Same<decltype(c), const char32_t*>::V + Same<decltype(d), const wchar_t*>::V == 3) * 64; }\n"
	},
	//4.13, 5/10
	{
		"ArithmeticTypes", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum E : short { X };\n"
		"enum class F : long { Y };\n"
		"int main() { short s = 1; signed char a = 1; char c = 1; unsigned char h = 1; long l = 1; long long x = 1; E e = X;\n"
		" return Same<decltype(+s), int>::V + Same<decltype(a + c), int>::V * 2 + Same<decltype(h + h), int>::V * 4 + Same<decltype(l + l), long>::V * 8 + Same<decltype(x + 1), long long>::V * 16 + Same<decltype(l + s), long>::V * 32 + Same<decltype(+e), int>::V * 64 + Same<__underlying_type(F), long>::V * 128; }\n"
	},
	//7.2/5
	{
		"EnumUnderlyingKinds", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum A : long { P };\n"
		"enum B : signed char { Q };\n"
		"enum C : char { R };\n"
		"enum D : unsigned short { S };\n"
		"int main() { return Same<__underlying_type(A), long>::V + Same<__underlying_type(B), signed char>::V * 2 + Same<__underlying_type(C), char>::V * 4 + Same<__underlying_type(D), unsigned short>::V * 8; }\n"
	},
	//18.2/6, /7, 5.3.3/6, 5.7/6: sizeof's and offsetof's type is size_t, a pointer difference's ptrdiff_t (on the W65C816
	//size_t is 16 bits and ptrdiff_t 32).
	{
		"SizeTypes", 15,
		"#include <stddef.h>\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int a; int b; };\n"
		"int main() { int ai[4]; int* p = &ai[3]; int* q = &ai[0];\n"
		" return Same<decltype(sizeof(int)), size_t>::V + Same<decltype(p - q), ptrdiff_t>::V * 2 + Same<decltype(offsetof(S, b)), size_t>::V * 4 + (p - q == 3) * 8; }\n"
	},
	//A new expression's size is a std::size_t ([expr.new] 10): a program's own operator new and new[] are given it.
	{
		"NewSizeIsSizeT", 7,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gac[64];\n"
		"size_t gn;\n"
		"void* operator new(size_t n) { gn = n; return gac; }\n"
		"void operator delete(void* p) noexcept {}\n"
		"void* operator new[](size_t n) { gn = n; return gac; }\n"
		"void operator delete[](void* p) noexcept {}\n"
		"struct S { char c[10]; };\n"
		"int main() { S* p = new S; int r = (gn == 10); delete p; char* q = new char[20]; r += (gn == 20) * 2; delete[] q; return r + 4; }\n"
	},
	//A function that isn't a template's is called rather than a template's instance that takes the arguments no better;
	//the instance when it converts one better and none worse ([over.match.best] 1).  The first f is the function's though
	//its pointer is converted, as the instance's is.  (BaseLib's memcpy and malloc overloads on the W65C816 lean on it.)
	{
		"TemplateOrFunction", 19121,
		"int f(const void* p, unsigned n) { return 1; }\n"
		"template<class T> int f(const void* p, T n) { return 2; }\n"
		"template<class T> int g(T a, long b) { return 3; }\n"
		"int g(int a, int b) { return 4; }\n"
		"char ga[4];\n"
		"int main() { unsigned u = 3; int i = 3; return f(ga, u) + f(ga, i) * 10 + f((const void*)ga, u) * 100 + g(1, 2) * 1000 + g('a', 2L) * 5000; }\n"
	},
	//std::nullptr_t is a type of its own ([basic.fundamental] 10, [lex.nullptr], [conv.ptr] 1, [conv.mem] 1, [conv.bool]):
	//nullptr's and decltype(nullptr)'s, not void*; it converts to any pointer and pointer to member, and to bool only in
	//a direct initialisation or a condition; nothing but a null pointer constant converts to it, and it has no arithmetic.
	{
		"NullPointerOwnType", 255,
		"typedef decltype(nullptr) nullptr_t;\n"
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int f(void*) { return 1; }\n"
		"int f(nullptr_t) { return 2; }\n"
		"template<class T> int h(T) { return Same<T, nullptr_t>::V; }\n"
		"int main() { void* pv = 0; nullptr_t n = nullptr; return (Same<nullptr_t, void*>::V == 0) + (f(pv) == 1) * 2 + (f(nullptr) == 2) * 4 + (f(n) == 2) * 8 + (h(nullptr) == 1) * 16 + (h(pv) == 0) * 32 + (sizeof(nullptr_t) == sizeof(void*)) * 64 + Same<decltype(nullptr), nullptr_t>::V * 128; }\n"
	},
	{
		"NullPointerConversions", 4095,
		"struct S { int m; int f() { return 1; } };\n"
		"typedef decltype(nullptr) nullptr_t;\n"
		"nullptr_t gn;\n"
		"int* id(int* p) { return p; }\n"
		"nullptr_t pass(nullptr_t n) { return n; }\n"
		"int main() { int i = 1; nullptr_t n = nullptr; nullptr_t z = 0; int* p = n; int S::*pm = n; int (S::*pf)() = nullptr; bool b(n); int* q = &i;\n"
		" return (p == 0) + (pm == nullptr) * 2 + (pf == nullptr) * 4 + (b == false) * 8 + (n == nullptr) * 16 + !(n != z) * 32 + (nullptr == p) * 64 + ((i ? q : nullptr) == q) * 128 + (id(nullptr) == 0) * 256 + (pass(gn) == nullptr) * 512 + (q != nullptr) * 1024 + (n ? 0 : 2048); }\n"
	},
	{
		"NullPointerNotBool", CONFORMANCE_REFUSED,
		"int main() { bool b = nullptr; return b + 1; }\n"
	},
	{
		"NullPointerFromPointer", CONFORMANCE_REFUSED,
		"typedef decltype(nullptr) nullptr_t;\n"
		"int main() { int* p = 0; nullptr_t n = p; return 1; }\n"
	},
	{
		"NullPointerArithmetic", CONFORMANCE_REFUSED,
		"typedef decltype(nullptr) nullptr_t;\n"
		"int main() { nullptr_t n = nullptr; return (n + 1) == n; }\n"
	},
	{
		"NullPointerTypeid", 7,
		"#ifdef _MSC_VER\n"
		"#include <typeinfo>\n"
		"#else\n"
		"namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }\n"
		"#endif\n"
		"typedef decltype(nullptr) nullptr_t;\n"
		"int main() { nullptr_t n = nullptr; return (typeid(nullptr) == typeid(nullptr_t)) + (typeid(n) != typeid(void*)) * 2 + (typeid(nullptr_t) != typeid(int*)) * 4; }\n"
	},
	{
		"NullPointerNotInteger", CONFORMANCE_REFUSED,
		"typedef decltype(nullptr) nullptr_t;\n"
		"int main() { nullptr_t n = nullptr; int i = n; return i + 1; }\n"
	},
	{
		"NullPointerCasts", 7,
		"typedef decltype(nullptr) nullptr_t;\n"
		"int main() { nullptr_t n = nullptr; long l = (long)n; long m = reinterpret_cast<long>(nullptr); void* p = static_cast<void*>(n); return (l == 0) + (m == 0) * 2 + (p == 0) * 4; }\n"
	},
	//Found 2026-11-19 by a probe of library style code (CppParserTodo.txt): both had been refused.  The value of a call
	//that returns a reference to a number or a pointer is what it refers to ([conv.lval]).  A static data member's
	//address is taken through its class's name, a reference is bound to it, and a class template's constexpr one
	//defined outside the class has the initialiser the class gave it, with the instance's arguments ([class.static.data]).
	{
		"ReferenceResultRead", 30317,
		"int gi = 3;\n"
		"int& same() { return gi; }\n"
		"const long& pick(const long& a) { return a; }\n"
		"struct V { int a[4]; int& at(int i) { return a[i]; } char c; char& ch() { return c; } };\n"
		"int take(int v) { return v; }\n"
		"int* gp = &gi;\n"
		"int*& ptr() { return gp; }\n"
		"int main() { V v; v.at(1) = 7; v.ch() = 'A'; long l = 40; int x = v.at(1); int r = same() + 1; if (same() == 3) { r += 10; } long m = same(); return r + take(same()) * 100 + x * 1000 + (int)pick(l) * 100 + (v.ch() == 'A') * 10000 + *ptr() * 3000 + (int)m; }\n"
	},
	{
		"StaticMemberAddress", 32375,
		"template<class T, T v> struct ic { static constexpr T value = v; };\n"
		"template<class T, T v> constexpr T ic<T, v>::value;\n"
		"template<class T> struct S { static constexpr int size = sizeof(T); static const int twice = sizeof(T) * 2; static int count; };\n"
		"template<class T> constexpr int S<T>::size;\n"
		"template<class T> const int S<T>::twice;\n"
		"template<class T> int S<T>::count = 3;\n"
		"struct N { static constexpr int k = 5; static int n; };\n"
		"constexpr int N::k;\n"
		"int N::n = 6;\n"
		"const int& ref(const int& r) { return r; }\n"
		"int main() { const int* p = &ic<int, 5>::value; const int* q = &N::k; int* c = &S<char>::count; int* n = &N::n; *c += 1; *n += 1;\n"
		" return *p + ref(ic<int, 7>::value) * 10 + ref(S<char>::size) * 100 + ref(S<char>::twice) * 200 + *q * 1000 + S<char>::count * 2000 + N::n * 400 + (p == &ic<int, 5>::value) * 16000; }\n"
	},
};


//Exceptions (CppCompilerLib/ExceptionsPlan.txt): throw, try and catch, what an exception destroys on its way, function-try-
//blocks, std::terminate and std::unexpected, exception specifications, and what the language and its support library
//throw.  They are compiled with exceptions on (CCPPCompileOptions::mbExceptions).  Each case has the paragraph of N4140 it
//is from.  Values are 1 to 255, so that a host whose programs return a byte can say them too: they were found with the
//Microsoft compiler (Conformance/hostcheck.py) and with g++ -std=c++14 (Conformance/gcccheck.sh), and a case that sees
//std::terminate or std::unexpected called gives them a handler that ends the program with _Exit(value).  Every case is
//what the standard says and what at least one of the two does; where one differs:
//  - the Microsoft compiler: a handler for a pointer doesn't catch nullptr (CatchNullPointer); copying the exception to a
//    handler's parameter may throw (CatchCopyThrowsTerminates); a handler for an rvalue reference is accepted
//    (CatchRvalueReferenceRefused); as C++17 it has no std::unexpected (the five cases that set its handler).
//  - g++: a class's operator delete with no specification isn't noexcept in a delete expression (DeallocationNoexcept); a
//    redeclaration with another specification and try in a constexpr function are only warned about
//    (RedeclaredDifferentRefused, ConstexprTryRefused).
static const SConformanceCase gasConformanceExceptions[] =
{
	{
		"ThrowInt", 5,	//15.1/1
		"int main() { try { throw 5; } catch (int e) { return e; } return 1; }\n"
	},
	{
		"ThrowThroughCalls", 7,	//15.1/1
		"int h(int x) { if (x > 2) throw x + 4; return x; }\n"
		"int k(int x) { return h(x) + 1; }\n"
		"int f(int x) { return k(x) + 1; }\n"
		"int main() { try { return f(3) + 100; } catch (int e) { return e; } }\n"
	},
	{
		"NoThrowSkipsHandler", 9,	//15/1
		"int f(int x) { if (x > 5) throw 1; return x; }\n"
		"int main() { int r = 0; try { r = f(4); r += 5; } catch (int) { r = 100; } return r; }\n"
	},
	{
		"CatchByReferenceChanges", 12,	//15.3/16
		"int main() { try { try { throw 5; } catch (int& e) { e += 7; throw; } } catch (int e) { return e; } return 1; }\n"
	},
	{
		"CatchByValueIsACopy", 5,	//15.3/16
		"struct T { int v; };\n"
		"int main() { try { try { T t; t.v = 5; throw t; } catch (T e) { e.v = 9; throw; } } catch (T& e) { return e.v; } return 1; }\n"
	},
	{
		"CatchEllipsis", 3,	//15.3/5
		"int main() { try { throw 2.5; } catch (...) { return 3; } return 1; }\n"
	},
	{
		"HandlersInOrder", 4,	//15.3/4
		"struct B {}; struct D : B {};\n"
		"int main() { int r = 0; try { throw D(); } catch (B&) { r = 4; } catch (D&) { r = 8; } return r; }\n"
	},
	{
		"NoMatchPropagates", 16,	//15.3/6
		"int main() { try { try { throw 6; } catch (char) { return 1; } catch (long) { return 2; } catch (unsigned) { return 3; } } catch (int e) { return e + 10; } return 4; }\n"
	},
	{
		"CodeAfterHandler", 15,	//15/1
		"int main() { int r = 1; try { throw 2; } catch (int e) { r += e; } r *= 5; return r; }\n"
	},
	{
		"ThrowInHandler", 65,	//15.1/1
		"int main() { try { try { throw 1; } catch (int e) { throw (char)(e + 64); } } catch (char c) { return c; } return 1; }\n"
	},
	{
		"RethrowKeepsDynamicType", 22,	//15.1/8
		"struct B { virtual int f() { return 1; } virtual ~B() {} }; struct D : B { int f() override { return 22; } };\n"
		"int main() { try { try { throw D(); } catch (B&) { throw; } } catch (D& d) { return d.f(); } catch (...) { return 2; } return 3; }\n"
	},
	{
		"ThrowStaticType", 34,	//15.1/3
		"struct B { int v; B() : v(3) {} }; struct D : B { D() { v = 4; } };\n"
		"int main() { D d; B& b = d; try { throw b; } catch (D&) { return 1; } catch (B& e) { return e.v + 30; } return 2; }\n"
	},
	{
		"CatchDerivedAsBaseReference", 17,	//15.3/3
		"struct B { virtual int f() const { return 1; } virtual ~B() {} }; struct D : B { int f() const override { return 17; } };\n"
		"int main() { try { throw D(); } catch (const B& b) { return b.f(); } return 2; }\n"
	},
	{
		"CatchDerivedAsBaseValue", 41,	//15.3/16
		"struct B { virtual int f() const { return 1; } virtual ~B() {} }; struct D : B { int f() const override { return 17; } };\n"
		"int main() { try { throw D(); } catch (B b) { return b.f() + 40; } return 2; }\n"
	},
	{
		"CatchPointerToBase", 6,	//15.3/3
		"struct B { int b; }; struct D : B { int d; };\n"
		"int main() { D d; d.b = 6; d.d = 7; try { throw &d; } catch (B* p) { return p->b; } return 1; }\n"
	},
	{
		"CatchSecondBase", 9,	//15.3/3
		"struct A { int a; }; struct B { int b; }; struct D : A, B { };\n"
		"int main() { try { D d; d.a = 2; d.b = 9; throw d; } catch (B& b) { return b.b; } return 1; }\n"
	},
	{
		"CatchPointerToSecondBase", 10,	//15.3/3
		"struct A { int a; }; struct B { int b; }; struct D : A, B { };\n"
		"int main() { D d; d.a = 2; d.b = 9; try { throw &d; } catch (B* p) { return p->b + (p == static_cast<B*>(&d)); } return 1; }\n"
	},
	{
		"CatchVoidPointer", 9,	//15.3/3
		"int main() { int x = 8; try { throw &x; } catch (void* p) { return *(int*)p + 1; } return 1; }\n"
	},
	{
		"CatchConstPointer", 10,	//15.3/3
		"int main() { int x = 8; try { throw &x; } catch (const int* p) { return *p + 2; } return 1; }\n"
	},
	{
		"PointerKeepsConst", 11,	//15.3/3
		"int main() { int x = 8; const int* q = &x; try { throw q; } catch (int*) { return 1; } catch (void*) { return 2; } catch (const void* p) { return *(const int*)p + 3; } return 4; }\n"
	},
	{
		"NoArithmeticConversion", 13,	//15.3/3
		"int main() { try { throw 1; } catch (long) { return 2; } catch (unsigned) { return 3; } catch (char) { return 4; } catch (const int&) { return 13; } return 5; }\n"
	},
	{
		"StringLiteralDecays", 98,	//15.1/3
		"int main() { try { throw \"ab\"; } catch (const char* s) { return s[1]; } return 1; }\n"
	},
	{
		"ArrayDecays", 23,	//15.1/3
		"int main() { int a[3] = { 1, 2, 3 }; try { throw a; } catch (int* p) { return p[2] + 20; } return 1; }\n"
	},
	{
		"FunctionDecays", 24,	//15.1/3
		"int f(int x) { return x + 1; }\n"
		"int main() { try { throw f; } catch (int (*p)(int)) { return p(23); } return 1; }\n"
	},
	{
		"CatchNullPointer", 25,	//15.3/3
		"int main() { try { throw nullptr; } catch (int* p) { return p == 0 ? 25 : 1; } catch (...) { return 2; } return 3; }\n"
	},
	{
		"HandlerConstIgnored", 26,	//15.3/3
		"int main() { try { throw 3; } catch (const int e) { return e + 23; } return 1; }\n"
	},
	{
		"ThrownConstDropped", 27,	//15.1/3
		"int main() { const int c = 4; try { throw c; } catch (int& e) { e += 23; return e; } return 1; }\n"
	},
	{
		"EnumIsNotInt", 28,	//15.3/3
		"enum E { A = 3 };\n"
		"int main() { try { throw A; } catch (int) { return 1; } catch (E e) { return e + 25; } return 2; }\n"
	},
	{
		"PrivateBaseNotCaught", 29,	//15.3/3
		"struct B {}; struct D : private B {};\n"
		"int main() { try { throw D(); } catch (B&) { return 1; } catch (...) { return 29; } return 2; }\n"
	},
	{
		"AmbiguousBaseNotCaught", 30,	//15.3/3
		"struct B {}; struct L : B {}; struct R : B {}; struct D : L, R {};\n"
		"int main() { try { throw D(); } catch (B&) { return 1; } catch (L&) { return 30; } return 2; }\n"
	},
	{
		"VirtualBaseCaught", 31,	//15.3/3
		"struct B { int v; B() : v(31) {} }; struct L : virtual B {}; struct R : virtual B {}; struct D : L, R {};\n"
		"int main() { try { throw D(); } catch (B& b) { return b.v; } return 2; }\n"
	},
	{
		"CatchReferenceToPointer", 32,	//15.3/3
		"int main() { int x = 32; int* p = &x; try { throw p; } catch (int*& r) { return *r; } return 1; }\n"
	},
	{
		"PointerIsNotReferenceToClass", 98,	//15.3/3
		"struct B {}; struct D : B {};\n"
		"int main() { D d; try { throw &d; } catch (B&) { return 1; } catch (D*) { return 98; } return 2; }\n"
	},
	{
		"ThrowDouble", 33,	//15.3/3
		"int main() { try { throw 2.5; } catch (float) { return 1; } catch (double d) { return (int)(d * 2) + 28; } return 2; }\n"
	},
	{
		"ThrowLongLong", 35,	//15.1/3
		"int main() { try { throw 0x100000022LL; } catch (long long v) { return (int)(v & 0xFF) + (int)(v >> 32); } return 2; }\n"
	},
	{
		"ThrowLargeObject", 36,	//15.1/3
		"struct Big { int a[20]; };\n"
		"Big make() { Big b; for (int i = 0; i < 20; i++) b.a[i] = i; return b; }\n"
		"int main() { try { throw make(); } catch (const Big& b) { int s = 0; for (int i = 0; i < 20; i++) s += b.a[i]; return s - 154; } return 1; }\n"
	},
	{
		"ThrowInConditional", 37,	//5.16/2
		"int f(int x) { return x > 0 ? x : throw 37; }\n"
		"int main() { try { return f(1) + f(0); } catch (int e) { return e; } }\n"
	},
	{
		"HandlerParameterShadows", 95,	//15.3/16
		"int main() { int e = 1; try { throw 94; } catch (int e) { e += 1; return e; } return e; }\n"
	},
	{
		"ExceptionObjectIsACopy", 96,	//15.1/3
		"struct T { int v; }; T gt;\n"
		"int main() { gt.v = 96; try { throw gt; } catch (T& e) { e.v = 1; } return gt.v; }\n"
	},
	{
		"ThrowClassWithCopyConstructor", 97,	//15.1/5
		"struct T { int v; T(int a) : v(a) {} T(const T& o) : v(o.v) {} };\n"
		"int main() { try { T t(97); throw t; } catch (T& e) { return e.v; } return 1; }\n"
	},
	{
		"NestedHandledExceptions", 93,	//15.1/8
		"int main() { try { try { throw 90; } catch (int) { try { throw 'a'; } catch (char) { } throw; } } catch (int e) { return e + 3; } catch (char) { return 1; } return 2; }\n"
	},
	{
		"RethrowInCalledFunction", 94,	//15.1/8
		"void re() { throw; }\n"
		"int main() { try { try { throw 94; } catch (...) { re(); } } catch (int e) { return e; } return 1; }\n"
	},
	{
		"RecursionCatchAndThrow", 92,	//15.1/1
		"int f(int n) { try { if (n == 0) throw 0; return f(n - 1); } catch (int e) { throw e + 1; } }\n"
		"int main() { try { f(91); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"ExceptionObjectLifetime", 38,	//15.1/4
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int main() { try { throw T(); } catch (T&) { g = live; } return g * 10 + live + 28; }\n"
	},
	{
		"CatchByValueCopies", 39,	//15.3/16
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int main() { try { throw T(); } catch (T e) { g = live; } return g * 10 + live + 19; }\n"
	},
	{
		"ExceptionLivesThroughRethrow", 40,	//15.1/4
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int main() { int a = 0; int b = 0; try { try { throw T(); } catch (T&) { a = live; throw; } } catch (T&) { b = live; } return a + b * 2 + live * 4 + 37; }\n"
	},
	{
		"ExceptionDestroyedOnReturn", 41,	//15.1/4
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int f() { try { throw T(); } catch (T&) { return live; } return 5; }\n"
		"int main() { int r = f(); return r * 10 + live + 31; }\n"
	},
	{
		"ExceptionDestroyedWhenHandlerThrows", 42,	//15.1/4
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int main() { g = 9; try { try { throw T(); } catch (T&) { throw 1; } } catch (int) { g = live; } return g + 42; }\n"
	},
	{
		"HandlerBreakContinue", 123,	//15/1
		"int main() { int s = 0; for (int i = 0; i < 5; i++) { try { if (i == 1) continue; if (i == 3) break; throw i; } catch (int e) { s += e + 10; if (e == 2) continue; s += 1; } s += 100; } return s; }\n"
	},
	{
		"TryInLoop", 43,	//15/1
		"int main() { int s = 0; for (int i = 0; i < 10; i++) { try { if (i & 1) throw i; s += 1; } catch (int e) { s += e; } } return s + 13; }\n"
	},
	{
		"LocalsSeenInHandler", 44,	//15/1
		"void f() { throw 1; }\n"
		"int main() { int x = 1; try { x = 2; f(); x = 3; } catch (...) { x += 40; } return x + 2; }\n"
	},
	{
		"LocalsAfterLoopThrow", 45,	//15/1
		"void f(int i) { if (i == 3) throw i; }\n"
		"int main() { int s = 0; int i = 0; try { for (i = 0; i < 10; i++) { s += i; f(i); } } catch (int) { } return s * 5 + i + 12; }\n"
	},
	{
		"ManyLocalsAcrossThrow", 46,	//15/1
		"int thrower(int a, int b, int c) { if (a + b + c > 0) throw a + b + c; return 0; }\n"
		"int main() { int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6; int r = 0; try { r = thrower(a, b, c); } catch (int v) { r = v; } return a + b + c + d + e + f + r + 19; }\n"
	},
	{
		"GotoOutOfTry", 148,	//15/2
		"int main() { int r = 140; try { r += 8; goto done; } catch (...) { r = 1; } r = 2; done: return r; }\n"
	},
	{
		"SwitchInsideTry", 149,	//15/2
		"int f(int x) { try { switch (x) { case 1: throw 100; case 2: throw 'c'; default: return 3; } } catch (int e) { return e + 49; } catch (char) { return 5; } }\n"
		"int main() { return f(1); }\n"
	},
	{
		"ReturnFromTryDestroys", 52,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"int f() { try { T a(1); return 5; } catch (...) { return 6; } }\n"
		"int main() { int r = f(); return g == 1 ? r + 47 : 1; }\n"
	},
	{
		"UnwindLocalsInReverse", 47,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"void f() { T a(1); T b(2); throw 0; }\n"
		"int main() { try { T c(3); f(); } catch (int) { t(4); } return g == 2134 ? 47 : 1; }\n"
	},
	{
		"UnwindOnlyConstructed", 48,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"void f(int n) { T a(1); if (n) throw 0; T b(2); }\n"
		"int main() { try { f(1); } catch (int) { t(3); } return g == 13 ? 48 : 1; }\n"
	},
	{
		"UnwindNestedScopes", 49,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"void f() { T a(1); { T b(2); { T c(3); throw 0; } } }\n"
		"int main() { try { f(); } catch (int) { t(4); } return g == 3214 ? 49 : 1; }\n"
	},
	{
		"UnwindThroughFrames", 50,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"void h() { T a(1); throw 0; }\n"
		"void k() { T b(2); h(); t(9); }\n"
		"void f() { T c(3); k(); t(9); }\n"
		"int main() { try { f(); } catch (int) { t(4); } return g == 1234 ? 50 : 1; }\n"
	},
	{
		"UnwindTemporaries", 51,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"int thrower() { throw 0; }\n"
		"int use(const T& a, int b) { return a.k + b; }\n"
		"int main() { try { (use(T(1), 0), thrower()); } catch (int) { t(2); } return g == 12 ? 51 : 1; }\n"
	},
	{
		"DeepUnwind", 53,	//15.2/1
		"int live; struct C { C() { live++; } ~C() { live--; } };\n"
		"int rec(int n) { C c; if (n == 0) throw live; return rec(n - 1) + 1; }\n"
		"int main() { try { rec(40); } catch (int e) { return e + live + 12; } return 1; }\n"
	},
	{
		"DestructorCatchesWhileUnwinding", 54,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct I { ~I() { try { throw 5; } catch (int e) { t(e); } } };\n"
		"void f() { I i; throw 1; }\n"
		"int main() { try { f(); } catch (int e) { t(e); } return g == 51 ? 54 : 1; }\n"
	},
	{
		"UncaughtException", 55,	//15.5.3/1
		"#include <exception>\n"
		"int g;\n"
		"struct U { ~U() { g = g * 2 + (std::uncaught_exception() ? 1 : 0); } };\n"
		"int main() { { U u; } try { U u; throw 1; } catch (int) { g = g * 2 + (std::uncaught_exception() ? 1 : 0); } return g + 53; }\n"
	},
	{
		"ParameterDestroyed", 70,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} T(const T& o) : k(o.k + 1) {} ~T() { t(k); } };\n"
		"void f(T a) { T b(2); throw 0; }\n"
		"int main() { T x(0); try { f(x); } catch (int) { t(3); } return g == 213 ? 70 : 1; }\n"
	},
	{
		"ConditionalTemporaryDestroyed", 71,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"int thrower() { throw 0; }\n"
		"int pick(bool c) { return c ? (T(1).k, thrower()) : 5; }\n"
		"int main() { try { pick(true); } catch (int) { t(2); } int r = pick(false); return g == 12 ? r + 66 : 1; }\n"
	},
	{
		"ExtendedTemporaryDestroyed", 72,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct T { int k; T(int a) : k(a) {} ~T() { t(k); } };\n"
		"void f() { const T& r = T(1); T b(2); throw r.k; }\n"
		"int main() { try { f(); } catch (int e) { t(e + 2); } return g == 213 ? 72 : 1; }\n"
	},
	{
		"RangeForUnwinds", 73,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct R { int a[3]; R() { a[0] = 1; a[1] = 2; a[2] = 3; } ~R() { t(7); } int* begin() { return a; } int* end() { return a + 3; } };\n"
		"void f() { for (int v : R()) { t(v); if (v == 2) throw 0; } }\n"
		"int main() { try { f(); } catch (int) { t(4); } return g == 1274 ? 73 : 1; }\n"
	},
	{
		"OperatorThrows", 78,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct V { int v; V(int a) : v(a) {} ~V() { t(v); } V operator+(const V& o) const { if (o.v == 2) throw 0; return V(v + o.v); } };\n"
		"int main() { try { V a(1); V b(2); V c = a + b; } catch (int) { t(3); } return g == 213 ? 78 : 1; }\n"
	},
	{
		"HiddenResultThrows", 81,	//15.2/1
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct O { int v; O() : v(1) { t(1); } O(const O& o) : v(o.v) { t(2); } ~O() { t(3); } };\n"
		"O make(int x) { if (x) throw 0; return O(); }\n"
		"int main() { try { O o = make(1); (void)o; } catch (int) { t(4); } return g == 4 ? 81 : 1; }\n"
	},
	{
		"ConstructorBodyThrows", 56,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct M { int k; M(int a) : k(a) { t(k); } ~M() { t(k + 5); } };\n"
		"struct X { M a; M b; X() : a(1), b(2) { throw 0; } ~X() { t(9); } };\n"
		"int main() { try { X x; } catch (int) { t(3); } return g == 12763 ? 56 : 1; }\n"
	},
	{
		"MemberConstructorThrows", 57,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct N { N(int a) { if (a == 2) throw 0; t(a); } ~N() { t(7); } };\n"
		"struct X { N a; N b; N c; X() : a(1), b(2), c(3) {} };\n"
		"int main() { try { X x; } catch (int) { t(4); } return g == 174 ? 57 : 1; }\n"
	},
	{
		"BaseDestroyedWhenMemberThrows", 58,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct B { B() { t(1); } ~B() { t(2); } }; struct Th { Th() { throw 0; } };\n"
		"struct D : B { Th m; D() {} };\n"
		"int main() { try { D d; } catch (int) { t(3); } return g == 123 ? 58 : 1; }\n"
	},
	{
		"ArrayPartlyConstructed", 59,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"int n; struct E { int k; E() : k(++n) { if (k == 3) throw 0; } ~E() { t(k); } };\n"
		"int main() { try { E a[4]; } catch (int) { t(5); } return g == 215 ? 59 : 1; }\n"
	},
	{
		"NewConstructorThrowsFrees", 60,	//5.3.4/20
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"struct F { static int news, deletes; F() { throw 0; } static void* operator new(size_t s) { news++; return malloc(s); } static void operator delete(void* p) { deletes++; free(p); } };\n"
		"int F::news; int F::deletes;\n"
		"int main() { try { F* p = new F; (void)p; } catch (int) { } return F::news * 10 + F::deletes * 5 + 45; }\n"
	},
	{
		"NewArrayConstructorThrows", 61,	//5.3.4/20
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"int n; struct E { int k; E() : k(++n) { if (k == 3) throw 0; } ~E() { t(k); } static void* operator new[](size_t s) { t(8); return malloc(s); } static void operator delete[](void* p) { t(9); free(p); } };\n"
		"int main() { try { E* p = new E[4]; (void)p; } catch (int) { } return g == 8219 ? 61 : 1; }\n"
	},
	{
		"PlacementDeleteCalled", 62,	//5.3.4/22
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct P { P() { throw 0; } static void* operator new(size_t s, int tag) { t(tag); return malloc(s); } static void operator delete(void* p, int tag) { t(tag + 1); free(p); } static void operator delete(void* p) { t(9); free(p); } };\n"
		"int main() { try { new (3) P; } catch (int) { } return g == 34 ? 62 : 1; }\n"
	},
	{
		"NoPlacementDeleteNoCall", 63,	//5.3.4/20
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct Q { Q() { throw 0; } static void* operator new(size_t s, int tag) { static char b[16]; t(tag); return b; } static void operator delete(void*) { t(9); } };\n"
		"int main() { try { new (3) Q; } catch (int) { t(5); } return g == 35 ? 63 : 1; }\n"
	},
	{
		"DelegatingBodyThrows", 64,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct G { G(int) { t(1); } G() : G(0) { t(2); throw 0; } ~G() { t(3); } };\n"
		"int main() { try { G x; } catch (int) { t(4); } return g == 1234 ? 64 : 1; }\n"
	},
	{
		"DelegatedConstructorThrows", 65,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct M { int k; M(int a) : k(a) { t(k); } ~M() { t(k + 5); } };\n"
		"struct G { M m; G(int) : m(1) { throw 0; } G() : G(0) { t(2); } ~G() { t(3); } };\n"
		"int main() { try { G x; } catch (int) { t(4); } return g == 164 ? 65 : 1; }\n"
	},
	{
		"VirtualBaseDestroyed", 66,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct V { V() { t(1); } ~V() { t(2); } }; struct L : virtual V { L() { t(3); } ~L() { t(4); } }; struct D : L { D() { throw 0; } };\n"
		"int main() { try { D d; } catch (int) { t(5); } return g == 13425 ? 66 : 1; }\n"
	},
	{
		"DestructorThrows", 67,	//15.2/3
		"struct W { ~W() noexcept(false) { throw 3; } };\n"
		"int main() { try { W w; } catch (int e) { return e + 64; } return 1; }\n"
	},
	{
		"MembersDestroyedWhenDestructorThrows", 68,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct M2 { ~M2() { t(2); } }; struct W { M2 m; ~W() noexcept(false) { t(1); throw 0; } };\n"
		"int main() { try { W w; } catch (int) { t(3); } return g == 123 ? 68 : 1; }\n"
	},
	{
		"StaticLocalRetried", 69,	//6.7/4
		"int n;\n"
		"int init() { if (++n < 3) throw n; return n; }\n"
		"int f() { static int s = init(); return s; }\n"
		"int main() { int r = 0; for (int i = 0; i < 5; i++) { try { r += f(); } catch (int e) { r += e * 10; } } return r + 30; }\n"
	},
	{
		"InitializerListObjectsDestroyed", 76,	//8.5.4/6, 15.2/2
		"#ifdef _MSC_VER\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"long g; void t(int k) { g = g * 10 + k; }\n"
		"struct E { int k; E(int a) : k(a) { if (a == 3) throw 0; } E(const E& o) : k(o.k) {} ~E() { t(k); } };\n"
		"int Sum(const E* b, const E* e) { int s = 0; for (; b != e; ++b) { s += b->k; } return s; }\n"
		"int main() { int s; { std::initializer_list<E> l = { 1, 2 }; s = Sum(l.begin(), l.end()); t(5); }\n"
		"try { std::initializer_list<E> m = { 1, 3 }; (void)m; } catch (int) { t(6); }\n"
		"return (s == 3) && (g == 52116) ? 76 : 1; }\n"
	},
	{
		"LambdaCapturesDestroyed", 75,	//5.1.2/21, 15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct A { A() {} A(const A&) { t(1); } ~A() { t(2); } }; struct B { B() {} B(const B&) { throw 0; } };\n"
		"int main() { A a; B b; try { auto l = [a, b] { }; (void)l; } catch (int) { t(3); } return g == 123 ? 75 : 1; }\n"
	},
	{
		"ThrownCopyThrows", 72,	//15.1/3, 15.2/2
		"struct C { C() {} C(const C&) { throw 2; } };\n"
		"int main() { try { C c; throw c; } catch (int e) { return e + 70; } catch (...) { return 1; } return 3; }\n"
	},
	{
		"ImplicitCopyPartsDestroyed", 73,	//12.8/15, 15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct A { A() {} A(const A&) { t(1); } ~A() { t(2); } }; struct B { B() {} B(const B&) { throw 0; } };\n"
		"struct X { A a; B b; };\n"
		"int main() { X x; try { X y(x); } catch (int) { t(3); } return g == 123 ? 73 : 1; }\n"
	},
	{
		"ArrayOfArraysPartlyConstructed", 74,	//15.2/2
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"int n; struct E { int k; E() : k(++n) { if (k == 4) throw 0; } ~E() { t(k); } };\n"
		"int main() { try { E a[2][3]; } catch (int) { t(5); } return g == 3215 ? 74 : 1; }\n"
	},
	{
		"TryInConstructorBody", 99,	//15/1
		"struct S { int v; S() { try { throw 99; } catch (int e) { v = e; } } };\n"
		"int main() { S s; return s.v; }\n"
	},
	{
		"TryInDestructor", 100,	//15/1
		"int g;\n"
		"struct S { ~S() { try { throw 5; } catch (int e) { g = e; } } };\n"
		"int main() { { S s; } return g + 95; }\n"
	},
	{
		"ThrowInGlobalConstructorCaught", 91,	//15/1
		"struct G { int v; G() { try { throw 91; } catch (int e) { v = e; } } };\n"
		"G gg;\n"
		"int main() { return gg.v; }\n"
	},
	{
		"DefaultArgumentThrows", 79,	//15.1/1
		"int th() { throw 79; }\n"
		"int f(int a = th()) { return a; }\n"
		"int main() { try { return f(); } catch (int e) { return e; } }\n"
	},
	{
		"MemberInitialiserThrows", 80,	//15.2/2
		"int th() { throw 80; }\n"
		"struct S { int v; S() : v(th()) {} };\n"
		"int main() { try { S s; return s.v; } catch (int e) { return e; } }\n"
	},
	{
		"LambdaThrows", 74,	//15.1/1
		"int main() { int k = 70; auto l = [k](int x) -> int { if (x) throw k + x; return 0; }; try { l(4); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"VirtualCallThrows", 75,	//15.1/1
		"struct B { virtual void f() = 0; virtual ~B() {} }; struct D : B { void f() override { throw 75; } };\n"
		"int main() { D d; B* b = &d; try { b->f(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"FunctionPointerThrows", 76,	//15.1/1
		"void f() { throw 76; }\n"
		"int main() { void (*p)() = f; try { p(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"TemplateThrows", 77,	//15.1/1
		"template<class T> void th(T v) { throw v; }\n"
		"int main() { try { th<short>(77); } catch (int) { return 1; } catch (short s) { return s; } return 2; }\n"
	},
	{
		"RegisterArgumentsThrow", 82,	//15.1/1
		"int f(int a, int b, int c) { if (c) throw a * 64 + b * 8 + c; return 0; }\n"
		"int main() { try { f(1, 2, 3); } catch (int e) { return e - 1; } return 1; }\n"
	},
	{
		"VariadicThrows", 83,	//15.1/1
		"#include <stdarg.h>\n"
		"int sum(int n, ...) { va_list a; va_start(a, n); int s = 0; for (int i = 0; i < n; i++) s += va_arg(a, int); va_end(a); if (s > 0) throw s; return 0; }\n"
		"int main() { try { sum(3, 20, 30, 33); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"MemberFunctionThrows", 84,	//15.1/1
		"struct S { int v; int f(int a) { if (a) throw v + a; return 0; } };\n"
		"int main() { S s; s.v = 80; try { s.f(4); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"BigFrameThrows", 85,	//15.1/1
		"int f(int n) { volatile char buf[600]; buf[0] = (char)n; buf[599] = 1; if (n) throw (int)buf[0] + buf[599]; return 0; }\n"
		"int main() { volatile char pad[300]; pad[0] = 1; try { f(84); } catch (int e) { return e + pad[0] - 1; } return 1; }\n"
	},
	{
		"LongLongFunctionThrows", 86,	//15.1/1
		"long long f(int x) { if (x) throw x; return 1; }\n"
		"int main() { try { long long v = f(86); return (int)v; } catch (int e) { return e; } }\n"
	},
	{
		"ForwarderThrows", 87,	//15.1/1
		"int f(int x) { if (x) throw x; return 0; }\n"
		"int w(int x) { return f(x); }\n"
		"int w2(int x) { return w(x); }\n"
		"int main() { try { return w2(87) + 1; } catch (int e) { return e; } }\n"
	},
	{
		"ThunkThrows", 88,	//15.1/1
		"struct A { virtual void fa() {} int a; virtual ~A() {} }; struct B { virtual void fb() = 0; int b; virtual ~B() {} }; struct D : A, B { int d; void fb() override { throw d; } };\n"
		"int main() { D x; x.d = 88; B* p = &x; try { p->fb(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"VirtualBaseThunkThrows", 89,	//15.1/1
		"struct V { virtual void f() = 0; virtual ~V() {} }; struct M : virtual V { int m; void f() override { throw m; } };\n"
		"int main() { M x; x.m = 89; V* p = &x; try { p->f(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"FloatFunctionThrows", 90,	//15.1/1
		"float f(float x) { if (x > 1.0f) throw (int)(x * 2); return x; }\n"
		"int main() { try { f(45.0f); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"FunctionTryBody", 101,	//15/4
		"int f(int x) try { if (x) throw x; return 1; } catch (int e) { return e + 1; }\n"
		"int main() { return f(100); }\n"
	},
	{
		"FunctionTryParameters", 102,	//15.3/13
		"int f(int a, int b) try { throw a; } catch (int e) { return e + b; }\n"
		"int main() { return f(100, 2); }\n"
	},
	{
		"FunctionTryFallsOff", 103,	//15.3/15
		"int g;\n"
		"void f() try { throw 3; } catch (int e) { g = e; }\n"
		"int main() { f(); return g + 100; }\n"
	},
	{
		"MainFunctionTry", 104,	//15/4
		"int main() try { throw 104; } catch (int e) { return e; }\n"
	},
	{
		"ConstructorFunctionTryRethrows", 105,	//15.3/15
		"int g;\n"
		"struct Th { Th(int) { throw 5; } };\n"
		"struct S { Th m; S() try : m(1) { } catch (int e) { g = e; } };\n"
		"int main() { try { S s; } catch (int e) { return g * 10 + e + 50; } return 1; }\n"
	},
	{
		"ConstructorFunctionTryCatchesBase", 106,	//15.3/13
		"int g;\n"
		"struct B { B() { throw 6; } };\n"
		"struct S : B { S() try { } catch (int e) { g = e; } };\n"
		"int main() { try { S s; } catch (int e) { return g * 10 + e + 40; } return 1; }\n"
	},
	{
		"ConstructorFunctionTryThrowsOther", 107,	//15.3/15
		"struct Th { Th(int) { throw 5; } };\n"
		"struct S { Th m; S() try : m(1) { } catch (int e) { throw (char)(e + 102); } };\n"
		"int main() { try { S s; } catch (char c) { return c; } return 1; }\n"
	},
	{
		"FunctionTryMembersDestroyedFirst", 108,	//15.3/11
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct Th { Th(int) { throw 5; } };\n"
		"struct M { M() { t(1); } ~M() { t(2); } };\n"
		"struct S { M a; Th b; S() try : b(1) { } catch (int) { t(3); } };\n"
		"int main() { try { S s; } catch (int) { t(4); } return g == 1234 ? 108 : 1; }\n"
	},
	{
		"DestructorFunctionTryRethrows", 109,	//15.3/15
		"int g;\n"
		"struct S { ~S() noexcept(false) try { throw 9; } catch (int e) { g = e; } };\n"
		"int main() { try { S s; } catch (int e) { return g * 10 + e + 10; } return 1; }\n"
	},
	{
		"ConstructorFunctionTryBodyThrows", 110,	//15.3/11
		"int g; void t(int k) { g = g * 10 + k; }\n"
		"struct M { M() { t(1); } ~M() { t(2); } };\n"
		"struct S { M a; S() try { throw 1; } catch (int) { t(3); } };\n"
		"int main() { try { S s; } catch (int) { t(4); } return g == 1234 ? 110 : 1; }\n"
	},
	{
		"UncaughtTerminates", 111,	//15.5.1/1
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(111); }\n"
		"int main() { std::set_terminate(bye); throw 1; }\n"
	},
	{
		"NoexceptTerminates", 112,	//15.4/9
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(112); }\n"
		"void f() noexcept { throw 1; }\n"
		"int main() { std::set_terminate(bye); try { f(); } catch (...) { return 1; } return 2; }\n"
	},
	{
		"NoexceptThroughCallTerminates", 113,	//15.4/9
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(113); }\n"
		"void th() { throw 1; }\n"
		"void f() noexcept { th(); }\n"
		"int main() { std::set_terminate(bye); try { f(); } catch (...) { return 1; } return 2; }\n"
	},
	{
		"ImplicitNoexceptDestructorTerminates", 114,	//12.4/3
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(114); }\n"
		"struct S { ~S() { throw 1; } };\n"
		"int main() { std::set_terminate(bye); try { S s; } catch (...) { return 1; } return 2; }\n"
	},
	{
		"ThrowWhileUnwindingTerminates", 115,	//15.5.1/1
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(115); }\n"
		"struct S { ~S() noexcept(false) { throw 2; } };\n"
		"int main() { std::set_terminate(bye); try { S s; throw 1; } catch (...) { return 1; } return 2; }\n"
	},
	{
		"RethrowNothingTerminates", 116,	//15.1/9
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(116); }\n"
		"int main() { std::set_terminate(bye); throw; }\n"
	},
	{
		"CatchCopyThrowsTerminates", 117,	//15.5.1/1
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(117); }\n"
		"int armed;\n"
		"struct T { T() {} T(const T&) { if (armed) throw 1; } };\n"
		"struct Arm { ~Arm() { armed = 1; } };\n"
		"int main() { std::set_terminate(bye); try { Arm a; throw T(); } catch (T) { return 1; } catch (...) { return 2; } return 3; }\n"
	},
	{
		"GlobalConstructorThrowsTerminates", 118,	//15.5.1/1
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(118); }\n"
		"struct G { G() { std::set_terminate(bye); throw 1; } };\n"
		"G gg;\n"
		"int main() { return &gg != 0; }\n"
	},
	{
		"SetTerminateReturnsPrevious", 119,	//18.8.3.2
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void a() { _Exit(1); }\n"
		"void b() { _Exit(2); }\n"
		"int main() { std::set_terminate(a); std::terminate_handler p = std::set_terminate(b); return (p == a) + (std::get_terminate() == b) * 2 + 116; }\n"
	},
	{
		"TerminateCallsHandler", 120,	//18.8.3.4
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(120); }\n"
		"int main() { std::set_terminate(bye); std::terminate(); }\n"
	},
	{
		"NoexceptOperatorThrow", 121,	//5.3.7/3
		"int main() { return noexcept(throw 1) ? 1 : 121; }\n"
	},
	{
		"NoexceptImplicitDestructor", 122,	//15.4/14
		"struct A { ~A() {} }; struct B { ~B() noexcept(false) {} }; struct C { B b; }; struct D2 { A a; };\n"
		"template<class T> T& ref() noexcept { return *(T*)0; }\n"
		"int main() { return noexcept(ref<A>().~A()) + !noexcept(ref<B>().~B()) * 2 + !noexcept(ref<C>().~C()) * 4 + noexcept(ref<D2>().~D2()) * 8 + 107; }\n"
	},
	{
		"NoexceptImplicitConstructors", 123,	//15.4/14
		"struct A { A() noexcept {} A(const A&) noexcept {} }; struct B { B() {} B(const B&) {} }; struct HA { A a; }; struct HB { B b; }; struct Tr { int x; };\n"
		"template<class T> T& ref() noexcept { return *(T*)0; }\n"
		"int main() { return noexcept(HA()) + !noexcept(HB()) * 2 + noexcept(HA(ref<HA>())) * 4 + !noexcept(HB(ref<HB>())) * 8 + noexcept(Tr()) * 16 + noexcept(Tr(ref<Tr>())) * 32 + 60; }\n"
	},
	{
		"NoexceptLambda", 124,	//5.1.2/5
		"int main() { auto a = []() noexcept { }; auto b = []() { }; return noexcept(a()) + !noexcept(b()) * 2 + 121; }\n"
	},
	{
		"NoexceptFunctionPointer", 125,	//15.4/2
		"void f() noexcept {}\n"
		"int main() { void (*p)() noexcept = f; void (*q)() = f; return noexcept(p()) + !noexcept(q()) * 2 + 122; }\n"
	},
	{
		"NoexceptDependent", 126,	//15.4/1
		"template<class T> void call(T& t) noexcept(noexcept(t.f())) { t.f(); }\n"
		"struct A { void f() noexcept {} }; struct B { void f() {} };\n"
		"int main() { A a; B b; return noexcept(call(a)) + !noexcept(call(b)) * 2 + 123; }\n"
	},
	{
		"NoexceptTemporaries", 127,	//5.3.7/3
		"struct X { X(int) {} ~X() {} }; struct Y { Y(int) noexcept {} };\n"
		"void take(const X&) noexcept; void takeY(const Y&) noexcept;\n"
		"int main() { return !noexcept(take(1)) + noexcept(takeY(1)) * 2 + 124; }\n"
	},
	{
		"DeallocationNoexcept", 128,	//15.4/15
		"struct S { static void operator delete(void*) {} };\n"
		"int main() { S* p = 0; return noexcept(delete p) ? 128 : 1; }\n"
	},
	{
		"NoexceptFalseThrows", 129,	//15.4/1
		"void f() noexcept(false) { throw 129; }\n"
		"int main() { try { f(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"OverrideLooserRefused", CONFORMANCE_REFUSED,	//15.4/5
		"struct B { virtual void f() noexcept; };\n"
		"struct D : B { void f() override; };\n"
		"int main() { D d; return 1; }\n"
	},
	{
		"PointerLooserRefused", CONFORMANCE_REFUSED,	//15.4/5
		"void f();\n"
		"void (*p)() noexcept = f;\n"
		"int main() { return 1; }\n"
	},
	{
		"RedeclaredDifferentRefused", CONFORMANCE_REFUSED,	//15.4/3
		"void f() noexcept;\n"
		"void f();\n"
		"int main() { return 1; }\n"
	},
	{
		"PointerAssignedLooserRefused", CONFORMANCE_REFUSED,	//15.4/5
		"void f();\n"
		"int main() { void (*p)() noexcept = 0; p = f; return 1; }\n"
	},
	{
		"ImplicitDestructorLooserRefused", CONFORMANCE_REFUSED,	//15.4/5, 12.4/3
		"struct M { ~M() noexcept(false) {} };\n"
		"struct B { virtual ~B() noexcept {} };\n"
		"struct D : B { M m; };\n"
		"int main() { D d; return 1; }\n"
	},
	{
		"PointerToPointerSpecification", 202,	//15.4/5
		"void f() noexcept {}\n"
		"int main() { void (*p)() noexcept = f; void (*q)() noexcept = p; void (*r)() = q; return noexcept(q()) + !noexcept(r()) * 2 + noexcept((*p)()) * 4 + 195; }\n"
	},
	{
		"CatchIncompleteRefused", CONFORMANCE_REFUSED,	//15.3/1
		"struct I;\n"
		"int main() { try { } catch (I) { } return 1; }\n"
	},
	{
		"CatchRvalueReferenceRefused", CONFORMANCE_REFUSED,	//15.3/1
		"int main() { try { } catch (int&&) { } return 1; }\n"
	},
	{
		"CatchAbstractRefused", CONFORMANCE_REFUSED,	//15.3/1
		"struct A { virtual void f() = 0; };\n"
		"int main() { try { } catch (A) { } return 1; }\n"
	},
	{
		"ThrowIncompleteRefused", CONFORMANCE_REFUSED,	//15.1/3
		"struct I;\n"
		"void f(I* p) { throw *p; }\n"
		"int main() { return 1; }\n"
	},
	{
		"ThrowAbstractRefused", CONFORMANCE_REFUSED,	//15.1/3
		"struct A { virtual void f() = 0; };\n"
		"struct D : A { void f() {} };\n"
		"void g(A& a) { throw a; }\n"
		"int main() { D d; try { g(d); } catch (...) { } return 1; }\n"
	},
	{
		"GotoIntoTryRefused", CONFORMANCE_REFUSED,	//15/2
		"int main() { goto L; try { L: ; } catch (...) { } return 1; }\n"
	},
	{
		"GotoIntoHandlerRefused", CONFORMANCE_REFUSED,	//15/2
		"int main() { goto L; try { } catch (...) { L: ; } return 1; }\n"
	},
	{
		"SwitchIntoTryRefused", CONFORMANCE_REFUSED,	//15/2
		"int main() { int x = 1; switch (x) { case 0: try { case 1: x = 2; } catch (...) { } } return x; }\n"
	},
	{
		"FunctionTryBlock", 6,
		"struct S { int v; S(int a) try : v(a) { } catch (...) { } };\n"
		"int main() { S s(6); return s.v; }\n"
	},
	{
		"ConstructorFunctionTryReturnRefused", CONFORMANCE_REFUSED,	//15.3/14
		"struct S { S() try { } catch (...) { return; } };\n"
		"int main() { S s; return 1; }\n"
	},
	{
		"EllipsisNotLastRefused", CONFORMANCE_REFUSED,	//15.3/5
		"int main() { try { } catch (...) { } catch (int) { } return 1; }\n"
	},
	{
		"ThrowPrivateCopyRefused", CONFORMANCE_REFUSED,	//15.1/5
		"struct N { N() {} private: N(const N&); };\n"
		"void f() { N n; throw n; }\n"
		"int main() { return 1; }\n"
	},
	{
		"TryWithoutHandlerRefused", CONFORMANCE_REFUSED,	//15/1
		"int main() { try { } return 1; }\n"
	},
	{
		"ConstexprThrowTakenRefused", CONFORMANCE_REFUSED,	//5.19/2
		"constexpr int f(int x) { return x < 0 ? throw 1 : x; }\n"
		"constexpr int k = f(-1);\n"
		"int main() { return k; }\n"
	},
	{
		"ConstexprTryRefused", CONFORMANCE_REFUSED,	//7.1.5/3
		"constexpr int f() { try { return 1; } catch (...) { return 2; } }\n"
		"int main() { return f(); }\n"
	},
	{
		"ConstexprThrowNotTaken", 147,	//5.19/2
		"constexpr int f(int x) { return x < 0 ? throw 1 : x; }\n"
		"int a[f(3)];\n"
		"int main() { try { return (int)(sizeof(a) / sizeof(int)) + f(-1); } catch (int e) { return e + 146; } }\n"
	},
	{
		"DynamicSpecAllows", 150,	//15.4/8
		"void f() throw(int) { throw 150; }\n"
		"int main() { try { f(); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"UnexpectedCalled", 151,	//15.5.2/1
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(151); }\n"
		"void f() throw(int) { throw 'a'; }\n"
		"int main() { std::set_unexpected(bye); try { f(); } catch (...) { return 1; } return 2; }\n"
	},
	{
		"UnexpectedThrowsAllowed", 152,	//15.5.2/2
		"#include <exception>\n"
		"void un() { throw 152; }\n"
		"void f() throw(int) { throw 'a'; }\n"
		"int main() { std::set_unexpected(un); try { f(); } catch (int e) { return e; } catch (...) { return 1; } return 2; }\n"
	},
	{
		"UnexpectedBadException", 153,	//15.5.2/3
		"#include <exception>\n"
		"void un() { throw 1.5; }\n"
		"void f() throw(int, std::bad_exception) { throw 'a'; }\n"
		"int main() { std::set_unexpected(un); try { f(); } catch (std::bad_exception&) { return 153; } catch (...) { return 1; } return 2; }\n"
	},
	{
		"UnexpectedTerminates", 154,	//15.5.2/3
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(154); }\n"
		"void un() { throw 1.5; }\n"
		"void f() throw(int) { throw 'a'; }\n"
		"int main() { std::set_terminate(bye); std::set_unexpected(un); try { f(); } catch (...) { return 1; } return 2; }\n"
	},
	{
		"ThrowNothingSpecUnexpected", 155,	//15.4/9
		"#include <exception>\n"
		"#include <stdlib.h>\n"
		"void bye() { _Exit(155); }\n"
		"void f() throw() { throw 1; }\n"
		"int main() { std::set_unexpected(bye); try { f(); } catch (...) { return 1; } return 2; }\n"
	},
	{
		"DynamicSpecAllowsDerived", 156,	//15.4/8
		"struct B {}; struct D : B {};\n"
		"void f() throw(B) { throw D(); }\n"
		"int main() { try { f(); } catch (D&) { return 156; } catch (...) { return 1; } return 2; }\n"
	},
	{
		"BadAllocFromReplacedNew", 157,	//18.6.2.1
		"#include <new>\n"
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"void* operator new(size_t s) { if (s > 100) throw std::bad_alloc(); return malloc(s); }\n"
		"void operator delete(void* p) noexcept { free(p); }\n"
		"void operator delete(void* p, size_t) noexcept { free(p); }\n"
		"struct Big { char b[200]; };\n"
		"int main() { try { Big* p = new Big; return p ? 1 : 2; } catch (std::bad_alloc& e) { return e.what() ? 157 : 3; } }\n"
	},
	{
		"NothrowNewNull", 158,	//5.3.4/13
		"#include <new>\n"
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"int g;\n"
		"void* operator new(size_t, const std::nothrow_t&) noexcept { return 0; }\n"
		"struct C { C() { g = 1; } };\n"
		"int main() { C* p = new (std::nothrow) C; return p == 0 && g == 0 ? 158 : 1; }\n"
	},
	{
		"ClassNewNoexceptNull", 159,	//5.3.4/13
		"#include <stdlib.h>\n"
		"#include <stddef.h>\n"
		"int g;\n"
		"struct C { C() { g = 1; } static void* operator new(size_t) noexcept { return 0; } static void operator delete(void*) {} };\n"
		"int main() { C* p = new C; return p == 0 && g == 0 ? 159 : 1; }\n"
	},
	{
		"SetNewHandlerReturnsPrevious", 160,	//18.6.2.4
		"#include <new>\n"
		"void a() {}\n"
		"void b() {}\n"
		"int main() { std::set_new_handler(a); std::new_handler p = std::set_new_handler(b); return (p == a) + (std::get_new_handler() == b) * 2 + 157; }\n"
	},
	{
		"BadArrayNewLength", 161,	//5.3.4/7
		"#include <new>\n"
		"int main() { volatile int n = -1; try { int* p = new int[n]; return p ? 1 : 2; } catch (std::bad_array_new_length&) { return 161; } catch (...) { return 3; } }\n"
	},
	{
		"BadCast", 162,	//5.2.7/9
		"#include <typeinfo>\n"
		"struct B { virtual ~B() {} }; struct D : B {}; struct E : B {};\n"
		"int main() { E e; B& b = e; try { D& d = dynamic_cast<D&>(b); (void)d; return 1; } catch (std::bad_cast& x) { return x.what() ? 162 : 2; } }\n"
	},
	{
		"BadTypeid", 163,	//5.2.8/2
		"#include <typeinfo>\n"
		"struct B { virtual ~B() {} };\n"
		"int main() { B* p = 0; try { return typeid(*p) == typeid(B) ? 1 : 2; } catch (std::bad_typeid&) { return 163; } }\n"
	},
	{
		"StandardExceptionsDerive", 164,	//18.8.1
		"#include <exception>\n"
		"#include <new>\n"
		"#include <typeinfo>\n"
		"int g;\n"
		"template<class T> void th() { try { throw T(); } catch (std::exception& e) { g += e.what() != 0; } }\n"
		"int main() { th<std::bad_alloc>(); th<std::bad_cast>(); th<std::bad_typeid>(); th<std::bad_exception>(); return g + 160; }\n"
	},
	{
		"UserExceptionWhat", 165,	//18.8.1
		"#include <exception>\n"
		"struct My : std::exception { const char* what() const noexcept override { return \"m\"; } };\n"
		"int main() { try { throw My(); } catch (const std::exception& e) { return e.what()[0] + 56; } return 1; }\n"
	},
	{
		"ExceptionPtr", 166,	//18.8.5
		"#include <exception>\n"
		"int main() { std::exception_ptr p; try { throw 160; } catch (...) { p = std::current_exception(); } try { std::rethrow_exception(p); } catch (int e) { return e + 6; } return 1; }\n"
	},
	{
		"ExceptionPtrNull", 167,	//18.8.5
		"#include <exception>\n"
		"int main() { std::exception_ptr p = std::current_exception(); std::exception_ptr q; return (p == nullptr) + (p == q) * 2 + !p * 4 + 160; }\n"
	},
	{
		"ExceptionPtrKeepsObject", 168,	//18.8.5/8
		"#include <exception>\n"
		"int live; int g;\n"
		"struct T { T() { live++; } T(const T&) { live++; } ~T() { live--; } };\n"
		"int main() { int a = 0; { std::exception_ptr p; try { throw T(); } catch (...) { p = std::current_exception(); } a = live > 0; } return a + (live == 0) * 2 + 165; }\n"
	},
	{
		"MakeExceptionPtr", 169,	//18.8.5
		"#include <exception>\n"
		"int main() { std::exception_ptr p = std::make_exception_ptr(169); try { std::rethrow_exception(p); } catch (int e) { return e; } return 1; }\n"
	},
	{
		"NestedException", 170,	//18.8.6
		"#include <exception>\n"
		"struct Outer { int v; Outer(int a) : v(a) {} virtual ~Outer() {} };\n"
		"int main() { try { try { throw 100; } catch (...) { std::throw_with_nested(Outer(60)); } } catch (Outer& o) { try { std::rethrow_if_nested(o); } catch (int e) { return e + o.v + 10; } } return 1; }\n"
	},
	{
		"ExceptionCopies", 171,	//18.8.1
		"#include <exception>\n"
		"int main() { std::exception a; std::exception b(a); b = a; std::bad_exception c; return (b.what() != 0) + (c.what() != 0) * 2 + 168; }\n"
	},
};
