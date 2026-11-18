//typeid's names are the types as the program writes them (CppParserTodo.txt question 8): a bit for each that is.
//std::type_info is declared here, as TestCPPConformance's cases do, so nothing else is needed.
namespace std { class type_info { public: bool operator==(const type_info& o) const noexcept; bool operator!=(const type_info& o) const noexcept; const char* name() const noexcept; }; }
namespace N
{
	struct A { int x; };
	template<class T> struct Box { T v; };
	enum E { One };
	struct B { virtual ~B() {} };
	struct D : B { };
}
static int Same(const char* a, const char* b)
{
	while (*a && (*a == *b))
	{
		a++;
		b++;
	}
	return *a == *b;
}
int main()
{
	N::D d;
	N::B& rb = d;
	int r = 0;
	r |= Same(typeid(int).name(), "int") << 0;
	r |= Same(typeid(unsigned char).name(), "unsigned char") << 1;
	r |= Same(typeid(double).name(), "double") << 2;
	r |= Same(typeid(N::A).name(), "N::A") << 3;
	r |= Same(typeid(N::Box<double>).name(), "N::Box<double>") << 4;
	r |= Same(typeid(N::E).name(), "N::E") << 5;
	r |= Same(typeid(const char*).name(), "const char*") << 6;
	r |= Same(typeid(int (*)[3]).name(), "int (*)[3]") << 7;
	r |= Same(typeid(void (*)(int)).name(), "void (*)(int)") << 8;
	r |= Same(typeid(int N::A::*).name(), "int N::A::*") << 9;
	r |= Same(typeid(rb).name(), "N::D") << 10;
	r |= Same(typeid(char* const*).name(), "char* const*") << 11;
	r |= Same(typeid(bool).name(), "bool") << 12;
	return r;
}
