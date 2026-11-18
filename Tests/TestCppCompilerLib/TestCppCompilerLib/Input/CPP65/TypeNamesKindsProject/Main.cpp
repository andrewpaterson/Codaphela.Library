//typeid's names of integers are the types as declared, each its own however wide (CppParserTodo.txt question 13): a bit
//for each that is.
namespace std { class type_info { public: const char* name() const noexcept; }; }
static int Same(const char* a, const char* b) { while (*a && (*a == *b)) { a++; b++; } return *a == *b; }
template<class T> struct Box {};
int main() { return Same(typeid(long).name(), "long") + Same(typeid(signed char).name(), "signed char") * 2 + Same(typeid(char16_t).name(), "char16_t") * 4 + Same(typeid(unsigned short).name(), "unsigned short") * 8 + Same(typeid(Box<unsigned long>).name(), "Box<unsigned long>") * 16 + Same(typeid(wchar_t*).name(), "wchar_t*") * 32; }
