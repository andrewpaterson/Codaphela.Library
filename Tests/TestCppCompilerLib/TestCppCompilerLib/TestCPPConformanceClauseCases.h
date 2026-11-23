//The conformance cases of the clause by clause sweep of the standard (TestCPPConformance.cpp;
//CppCompilerLib/Cpp14CompliancePlan.txt): each case is a rule of a clause of N4140, named in the comment above it, and
//a group is a clause.  The tables are written by Conformance/clauses.py from the cases in Conformance/Clauses: change
//a case there, not here.  They were written to measure the compiler, not as it was made to do them, so what a group's
//lists name is what the compiler has yet to do.  A case that runs returns a sum of powers of two, one for each
//thing it checks, so what it returned says which of them went wrong; what depends on the target (an int's size, what
//an unsigned short promotes to) is worked out from sizeof in the case, so that one value is right on every target.
//
//The values are what g++ 13 gives (-std=c++14 -pedantic-errors, -m32 and -m64) and a refused case is one it refuses:
//the Microsoft compiler has not been run on them yet (Conformance/hostcheck.py).  A case that includes <typeinfo>
//for that compiler declares std::type_info itself for this one, as the stages' cases do.


//Clause 2, lexical conventions [lex].
static const SConformanceCase gasConformanceClause2[] =
{
	//2.2 [lex.phases] p1.2: a backslash and a new-line are deleted, splicing the lines, in a name, a number, a string and a // comment
	{
		"LexLineSplices", 15,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int va\\\n"
		"lue = 1\\\n"
		"2;\n"
		"	const char* s = \"a\\\n"
		"b\";\n"
		"	if (value == 12) r += 1;\n"
		"	if (s[0] == 'a' && s[1] == 'b' && s[2] == 0) r += 2;\n"
		"	// a comment that goes on \\\n"
		"	r += 100;\n"
		"	r += 4;\n"
		"	in\\\n"
		"t x = 8;\n"
		"	r += x;\n"
		"	return r;\n"
		"}\n"
	},
	//2.2 [lex.phases] p1.6: adjacent string literals are joined after escapes are replaced
	{
		"LexPhasesStringConcatenation", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	const char* s = \"\\x41\" \"B\";\n"
		"	const char* t = \"a\" \"\" \"b\" \"c\";\n"
		"	if (s[0] == 'A' && s[1] == 'B' && s[2] == 0) r += 1;\n"
		"	if (sizeof(\"ab\" \"cd\") == 5) r += 2;\n"
		"	if (t[0] == 'a' && t[1] == 'b' && t[2] == 'c' && t[3] == 0) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.3 [lex.charset] p2: \u and \U name characters in literals; a UTF-16 literal holds one beyond 0xFFFF as two
	{
		"LexUniversalCharacterNames", 15,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (u'\\u00e9' == 0xE9 && U'\\U0001F600' == 0x1F600) r += 1;\n"
		"	if (sizeof(u\"\\U0001F600\") == 3 * sizeof(char16_t) && sizeof(U\"\\U0001F600\") == 2 * sizeof(char32_t)) r += 2;\n"
		"	if (sizeof(u8\"\\u00e9\") == 3 && (unsigned char)u8\"\\u00e9\"[0] == 0xC3 && (unsigned char)u8\"\\u00e9\"[1] == 0xA9) r += 4;\n"
		"	if (u\"\\U0001F600\"[0] == 0xD83D && u\"\\U0001F600\"[1] == 0xDE00 && U'\\u0041' == 'A') r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//2.3 [lex.charset] p2: a universal-character-name is not a surrogate's code point
	{
		"LexUniversalCharacterNameSurrogateRefused", CONFORMANCE_REFUSED,
		"int main() { return u'\\uD800' ? 1 : 2; }\n"
	},
	//2.3 [lex.charset] p2: \u has four hexadecimal digits
	{
		"LexUniversalCharacterNameShortRefused", CONFORMANCE_REFUSED,
		"int main() { return u'\\u41' ? 1 : 2; }\n"
	},
	//2.3 [lex.charset] p2: a universal-character-name is not a surrogate's code point, in a string too
	{
		"LexUniversalCharacterNameSurrogateInStringRefused", CONFORMANCE_REFUSED,
		"int main() { return u\"\\uDC00\"[0] ? 1 : 2; }\n"
	},
	//2.3 [lex.charset] p2: outside a literal a universal-character-name doesn't name a basic source character
	{
		"LexUniversalCharacterNameBasicInNameRefused", CONFORMANCE_REFUSED,
		"int main() { int \\u0041 = 1; return A; }\n"
	},
	//2.4 [lex.trigraph] p1: the nine trigraph sequences are replaced first, in strings too, and the backslash one splices a line
	{
		"LexTrigraphs", 63,
		"?\?=define ONE 1\n"
		"int main()\n"
		"?\?<\n"
		"	int a?\?(2?\?) = ?\?< 1, 2 ?\?>;\n"
		"	int r = 0;\n"
		"	if ((ONE ?\?' 3) == 2 && (4 ?\?! 1) == 5 && ?\?-0 == -1) r += 1;\n"
		"	if (a?\?(1?\?) == 2) r += 2;\n"
		"	const char* s = \"?\?(?\?)?\?<?\?>?\?=?\?/?\?/\";\n"
		"	if (s[0] == '[' && s[1] == ']' && s[2] == '{' && s[3] == '}' && s[4] == '#' && s[5] == '\\\\' && s[6] == 0) r += 4;\n"
		"	if (sizeof(\"?\?!\") == 2) r += 8;\n"
		"	int b = 1?\?/\n"
		"6;\n"
		"	if (b == 16) r += 16;\n"
		"	if (sizeof(\"?\" \"?=\") == 4 && sizeof(\"?\?\\?\") == 4) r += 32;\n"
		"	return r;\n"
		"?\?>\n"
	},
	//2.4 [lex.trigraph] p1, 2.2 [lex.phases] p1.2: a trigraph in a comment is replaced too: ??/ at a // comment's end goes on to the next line
	{
		"LexTrigraphsInComments", 7,
		"int main()\n"
		"{\n"
		"	int r = 1;\n"
		"	// the next line is this comment's ?\?/\n"
		"	r += 100;\n"
		"	/* ?\?> and ?\?< in a comment */ r += 2;\n"
		"	r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.5 [lex.pptoken] p3: the longest token that can be made is made: a+++b is a++ + b; <:: before a name is < ::
	{
		"LexMaximalMunch", 31,
		"template<class T> struct W { T v; };\n"
		"struct X { int i; };\n"
		"int a = 1;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int b = 2;\n"
		"	int c = a+++b;\n"
		"	if (c == 3 && a == 2) r += 1;\n"
		"	int d = a - -b;\n"
		"	if (d == 4) r += 2;\n"
		"	W<::X> w;\n"
		"	w.v.i = 5;\n"
		"	if (w.v.i == 5) r += 4;\n"
		"	int e = b--- -1;\n"
		"	if (e == 3 && b == 1) r += 8;\n"
		"	int f = 7 / 1/**/;\n"
		"	if (f == 7) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//2.5 [lex.pptoken] p3: x+++++y is x ++ ++ + y, which increments what isn't an object
	{
		"LexMaximalMunchRefused", CONFORMANCE_REFUSED,
		"int main() { int x = 1; int y = 2; return x+++++y; }\n"
	},
	//2.6 [lex.digraph]: the alternative tokens are the tokens they stand for
	{
		"LexAlternativeTokens", 63,
		"%:define TWO 2\n"
		"%:define JOIN(a, b) a %:%: b\n"
		"int main()\n"
		"<%\n"
		"	int a<:2:> = <% 1, TWO %>;\n"
		"	int r = 0;\n"
		"	if (a<:1:> == 2 and a<:0:> == 1) r += 1;\n"
		"	if ((not false) && (true or false)) r += 2;\n"
		"	if ((6 bitand 3) == 2 && (4 bitor 1) == 5 && (6 xor 3) == 5 && compl 0 == -1) r += 4;\n"
		"	int x = 1;\n"
		"	x and_eq 3; x or_eq 4; x xor_eq 1;\n"
		"	if (x == 4 && 1 not_eq 2) r += 8;\n"
		"	int JOIN(v, w) = 3;\n"
		"	if (vw == 3) r += 16;\n"
		"	int and q = 5;\n"
		"	int bitand p = x;\n"
		"	if (q == 5 && &p == &x) r += 32;\n"
		"	return r;\n"
		"%>\n"
	},
	//2.6 [lex.digraph] p2, 16.3.2 [cpp.stringize]: an alternative token is spelled as written when it is made a string
	{
		"LexAlternativeTokenSpelling", 7,
		"#define STR(x) #x\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(STR(%:)) == 3 && STR(%:)[0] == '%') r += 1;\n"
		"	if (sizeof(STR(<%)) == 3 && STR(<:)[1] == ':') r += 2;\n"
		"	if (sizeof(STR(and)) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.8 [lex.comment]: /* */ ends at the first */ and holds //; // holds /* and */; neither is in a string
	{
		"LexComments", 15,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	/* a comment // with // in it */ r += 1;\n"
		"	// a comment /* with */ in it\n"
		"	r += 2; // */\n"
		"	const char* s = \"/* not a comment */\";\n"
		"	if (s[0] == '/' && s[1] == '*') r += 4;\n"
		"	int x = 16 /* / 2 */ /2;\n"
		"	if (x == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//2.8 [lex.comment] p1: /* */ comments don't nest
	{
		"LexCommentsDontNestRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	/* outer /* inner */ still */\n"
		"	return 0;\n"
		"}\n"
	},
	//2.10 [lex.ppnumber]: 0xe+1 is one preprocessing number, and no literal
	{
		"LexPreprocessingNumberRefused", CONFORMANCE_REFUSED,
		"int main() { return 0xe+1; }\n"
	},
	//2.10 [lex.ppnumber]: 0xe + 1 is three tokens
	{
		"LexPreprocessingNumberSpaced", 15,
		"int main() { return 0xe + 1; }\n"
	},
	//2.11 [lex.name]: letters, digits and _, of any length and case; \u names in identifiers; override and final are names
	{
		"LexIdentifiers", 63,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int _ = 1, _a1 = 2, A_ = 4, a_ = 8;\n"
		"	int an_identifier_of_more_than_sixty_three_characters_which_is_still_one_name = 16;\n"
		"	int an_identifier_of_more_than_sixty_three_characters_which_is_still_one_nameX = 0;\n"
		"	int \\u00e9t\\u00e9 = 32;\n"
		"	int override = 0, final = 0;\n"
		"	r = _ + _a1 + A_ + a_ + an_identifier_of_more_than_sixty_three_characters_which_is_still_one_name + an_identifier_of_more_than_sixty_three_characters_which_is_still_one_nameX;\n"
		"	r += \\u00e9t\\u00e9 + override + final;\n"
		"	return r;\n"
		"}\n"
	},
	//2.11 [lex.name] p1: an identifier starts with a nondigit
	{
		"LexIdentifierStartsWithDigitRefused", CONFORMANCE_REFUSED,
		"int main() { int 2x = 1; return 2x; }\n"
	},
	//2.12 [lex.key]: a keyword is not an identifier
	{
		"LexKeywordAsNameRefused", CONFORMANCE_REFUSED,
		"int main() { int class = 1; return class; }\n"
	},
	//2.12 [lex.key]: nullptr is a keyword
	{
		"LexKeywordNullptrAsNameRefused", CONFORMANCE_REFUSED,
		"int main() { int nullptr = 1; return nullptr; }\n"
	},
	//2.12 [lex.key]: alignof is a keyword
	{
		"LexKeywordAlignofAsNameRefused", CONFORMANCE_REFUSED,
		"int main() { int alignof = 1; return alignof; }\n"
	},
	//2.12 [lex.key]: C++11's keywords: alignas, alignof, char16_t, char32_t, constexpr, decltype, noexcept, nullptr, static_assert, thread_local
	{
		"LexKeywords", 31,
		"constexpr int Four() noexcept { return 4; }\n"
		"static_assert(sizeof(char16_t) == 2, \"\");\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	alignas(4) char c = 1;\n"
		"	static thread_local int t = 2;\n"
		"	decltype(c) d = 3;\n"
		"	char32_t e = U'a';\n"
		"	int* p = nullptr;\n"
		"	if (alignof(int) >= 1 && c == 1) r += 1;\n"
		"	if (t == 2 && d == 3) r += 2;\n"
		"	if (Four() == 4 && noexcept(Four())) r += 4;\n"
		"	if (e == 'a' && !p) r += 8;\n"
		"	if (sizeof(char32_t) == 4) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.2 [lex.icon] p1: decimal, octal, hexadecimal and binary literals, with ' between digits
	{
		"LexIntegerLiterals", 63,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (10 == 012 && 012 == 0xA && 0xA == 0Xa && 0xa == 0b1010 && 0b1010 == 0B1010) r += 1;\n"
		"	if (0 == 00 && 0x0 == 0b0 && 07 == 7) r += 2;\n"
		"	if (1'000 == 1000 && 0x1'0 == 16 && 0b1'0000 == 16 && 01'0 == 8) r += 4;\n"
		"	if (0xFFFF == 65535 && 0XdeadBEEFu == 3735928559u) r += 8;\n"
		"	if (1'2'3 == 123 && 0'7 == 7) r += 16;\n"
		"	if (0b1111'1111 == 255u) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.2 [lex.icon] p2: a literal's type is the first of its list that holds it; u, l and ll choose the lists
	{
		"LexIntegerLiteralTypes", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<bool B, class T, class F> struct Pick { typedef T Type; };\n"
		"template<class T, class F> struct Pick<false, T, F> { typedef F Type; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(1), int>::V && Same<decltype(1u), unsigned>::V && Same<decltype(1U), unsigned>::V) r += 1;\n"
		"	if (Same<decltype(1l), long>::V && Same<decltype(1L), long>::V && Same<decltype(1ul), unsigned long>::V && Same<decltype(1LU), unsigned long>::V) r += 2;\n"
		"	if (Same<decltype(1ll), long long>::V && Same<decltype(1LL), long long>::V && Same<decltype(1ull), unsigned long long>::V && Same<decltype(1uLL), unsigned long long>::V) r += 4;\n"
		"	if (Same<decltype(32767), int>::V && Same<decltype(0x7FFF), int>::V) r += 8;\n"
		"	//A decimal literal stays signed; an octal or hexadecimal one may be unsigned.\n"
		"	if (Same<decltype(65535), Pick<(sizeof(int) > 2), int, long>::Type>::V) r += 16;\n"
		"	if (Same<decltype(0xFFFF), Pick<(sizeof(int) > 2), int, unsigned>::Type>::V) r += 32;\n"
		"	if (Same<decltype(2147483647), Pick<(sizeof(int) >= 4), int, long>::Type>::V) r += 64;\n"
		"	if (Same<decltype(0xFFFFFFFF), Pick<(sizeof(int) >= 4), unsigned, Pick<(sizeof(long) >= 4), unsigned long, long long>::Type>::Type>::V) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.2 [lex.icon] p2: long long holds what long doesn't
	{
		"LexIntegerLiteralLongLongTypes", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<bool B, class T, class F> struct Pick { typedef T Type; };\n"
		"template<class T, class F> struct Pick<false, T, F> { typedef F Type; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(4294967296), Pick<(sizeof(long) > 4), long, long long>::Type>::V) r += 1;\n"
		"	if (Same<decltype(0x7FFFFFFFFFFFFFFF), Pick<(sizeof(long) > 4), long, long long>::Type>::V) r += 2;\n"
		"	if (Same<decltype(0xFFFFFFFFFFFFFFFF), Pick<(sizeof(long) > 4), unsigned long, unsigned long long>::Type>::V) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.2 [lex.icon] p1: an octal literal has no 8 or 9
	{
		"LexOctalDigitRefused", CONFORMANCE_REFUSED,
		"int main() { return 08; }\n"
	},
	//2.14.2 [lex.icon] p1: a binary literal has only 0 and 1
	{
		"LexBinaryDigitRefused", CONFORMANCE_REFUSED,
		"int main() { return 0b12; }\n"
	},
	//2.14.2 [lex.icon] p1: a binary literal has a digit
	{
		"LexBinaryNoDigitsRefused", CONFORMANCE_REFUSED,
		"int main() { return 0b; }\n"
	},
	//2.14.2 [lex.icon] p1: a hexadecimal literal has a digit
	{
		"LexHexadecimalNoDigitsRefused", CONFORMANCE_REFUSED,
		"int main() { return 0x; }\n"
	},
	//2.14.2 [lex.icon] p1: ' is between digits, not after them
	{
		"LexDigitSeparatorAtEndRefused", CONFORMANCE_REFUSED,
		"int main() { return 1'; }\n"
	},
	//2.14.2 [lex.icon] p1: ' is between digits, one at a time
	{
		"LexDigitSeparatorDoubledRefused", CONFORMANCE_REFUSED,
		"int main() { return 1''0; }\n"
	},
	//2.14.2 [lex.icon] p1: ' is between digits, not after 0x
	{
		"LexDigitSeparatorAfterPrefixRefused", CONFORMANCE_REFUSED,
		"int main() { return 0x'1; }\n"
	},
	//2.14.2 [lex.icon] p1: ll and LL, not lL
	{
		"LexIntegerSuffixMixedCaseRefused", CONFORMANCE_REFUSED,
		"int main() { return (int)1lL; }\n"
	},
	//2.14.2 [lex.icon] p1: the suffixes are u, l and ll
	{
		"LexIntegerSuffixUnknownRefused", CONFORMANCE_REFUSED,
		"int main() { return (int)1uu; }\n"
	},
	//2.14.2 [lex.icon] p2: a literal no integer type holds
	{
		"LexIntegerLiteralTooLargeRefused", CONFORMANCE_REFUSED,
		"int main() { return 18446744073709551616ull == 0; }\n"
	},
	//2.14.3 [lex.ccon]: a character literal's type and value: char, int for more than one character, char16_t, char32_t, wchar_t
	{
		"LexCharacterLiterals", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype('a'), char>::V && 'a' == 97) r += 1;\n"
		"	if (Same<decltype(u'a'), char16_t>::V && Same<decltype(U'a'), char32_t>::V && Same<decltype(L'a'), wchar_t>::V) r += 2;\n"
		"	if (Same<decltype('ab'), int>::V) r += 4;\n"
		"	if (u'\\xFFFF' == 0xFFFF && U'\\U0010FFFF' == 0x10FFFF) r += 8;\n"
		"	if (L'a' == 97 && u'z' == 122 && U'\\0' == 0) r += 16;\n"
		"	if (sizeof('a') == 1) r += 32;\n"
		"	if ('\\x7F' == 127 && '\\177' == 127 && '\\0' == 0 && '\\60' == '0') r += 64;\n"
		"	if (u'\\u20ac' == 0x20AC) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.3 [lex.ccon] p3: the simple escapes, octal escapes of up to three digits and hexadecimal escapes of any length
	{
		"LexEscapeSequences", 127,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if ('\\n' == 10 && '\\t' == 9 && '\\v' == 11 && '\\b' == 8 && '\\r' == 13 && '\\f' == 12 && '\\a' == 7) r += 1;\n"
		"	if ('\\\\' == 92 && '\\'' == 39 && '\\\"' == 34 && '\\?' == 63 && '\"' == 34) r += 2;\n"
		"	if (sizeof(\"\\1234\") == 3 && \"\\1234\"[0] == 'S' && \"\\1234\"[1] == '4') r += 4;\n"
		"	if (sizeof(\"\\x41g\") == 3 && \"\\x41g\"[0] == 'A') r += 8;\n"
		"	if (sizeof(\"\\x000041\") == 2 && \"\\x000041\"[0] == 'A') r += 16;\n"
		"	if (sizeof(\"\\0\") == 2 && sizeof(\"a\\0b\") == 4) r += 32;\n"
		"	if (u\"\\x1234\"[0] == 0x1234 && sizeof(u\"\\x1234\") == 4) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.3 [lex.ccon] p1: a character literal has a character
	{
		"LexCharacterLiteralEmptyRefused", CONFORMANCE_REFUSED,
		"int main() { return ''; }\n"
	},
	//2.14.3 [lex.ccon] p2: a char16_t literal has one character
	{
		"LexChar16LiteralTwoCharactersRefused", CONFORMANCE_REFUSED,
		"int main() { return u'ab'; }\n"
	},
	//2.14.3 [lex.ccon] p2: a char16_t literal's character fits one UTF-16 code unit
	{
		"LexChar16LiteralBeyondRefused", CONFORMANCE_REFUSED,
		"int main() { return u'\\U0001F600' ? 1 : 2; }\n"
	},
	//2.14.3 [lex.ccon] p2: a char32_t literal has one character
	{
		"LexChar32LiteralTwoCharactersRefused", CONFORMANCE_REFUSED,
		"int main() { return U'ab' ? 1 : 2; }\n"
	},
	//2.14.4 [lex.fcon]: a fraction, an exponent or both; f and l; ' between digits
	{
		"LexFloatingLiterals", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (1.5 == 15e-1 && .5 == 0.5 && 5. == 5.0 && 1e2 == 100.0 && 1E+2 == 100.0) r += 1;\n"
		"	if (Same<decltype(1.0), double>::V && Same<decltype(1.0f), float>::V && Same<decltype(1.0F), float>::V) r += 2;\n"
		"	if (Same<decltype(1.0l), long double>::V && Same<decltype(1.0L), long double>::V) r += 4;\n"
		"	if (1'0.2'5 == 10.25 && 1e1'0 == 1e10) r += 8;\n"
		"	if (2.5e-1f == 0.25f && 1e0 == 1.0) r += 16;\n"
		"	if (Same<decltype(1e3), double>::V && 1e3 == 1000) r += 32;\n"
		"	if (0.1f != 0.1 && 0.5f == 0.5) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.4 [lex.fcon] p1: an exponent has digits
	{
		"LexFloatingExponentNoDigitsRefused", CONFORMANCE_REFUSED,
		"int main() { return 1.0e > 0; }\n"
	},
	//2.14.4 [lex.fcon] p1: the suffixes are f and l
	{
		"LexFloatingSuffixUnknownRefused", CONFORMANCE_REFUSED,
		"int main() { return 1.0u > 0; }\n"
	},
	//2.14.4 [lex.fcon] p1: one suffix
	{
		"LexFloatingSuffixDoubledRefused", CONFORMANCE_REFUSED,
		"int main() { return 1.0ff > 0; }\n"
	},
	//2.14.5 [lex.string]: a string literal is an array of const characters, with a zero after them; its prefix chooses its type
	{
		"LexStringLiterals", 255,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(\"ab\"), const char (&)[3]>::V) r += 1;\n"
		"	if (Same<decltype(u\"ab\"), const char16_t (&)[3]>::V && Same<decltype(U\"ab\"), const char32_t (&)[3]>::V && Same<decltype(L\"ab\"), const wchar_t (&)[3]>::V) r += 2;\n"
		"	if (Same<decltype(u8\"ab\"), const char (&)[3]>::V) r += 4;\n"
		"	if (sizeof(\"abc\") == 4 && sizeof(u\"abc\") == 4 * sizeof(char16_t) && sizeof(U\"abc\") == 4 * sizeof(char32_t) && sizeof(L\"abc\") == 4 * sizeof(wchar_t)) r += 8;\n"
		"	if (\"abc\"[3] == 0 && u\"abc\"[3] == 0 && U\"abc\"[2] == 'c') r += 16;\n"
		"	const char* p = \"x\";\n"
		"	const char16_t* q = u\"y\";\n"
		"	if (*p == 'x' && *q == 'y') r += 32;\n"
		"	if (sizeof(\"\") == 1 && sizeof(u\"\") == sizeof(char16_t)) r += 64;\n"
		"	if (L\"\\x41\"[0] == 'A') r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.5 [lex.string] p4, p5: a raw string has no escapes, holds new-lines and ends at )delimiter"
	{
		"LexRawStrings", 127,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	const char* a = R\"(a\\nb)\";\n"
		"	if (a[0] == 'a' && a[1] == '\\\\' && a[2] == 'n' && a[3] == 'b' && a[4] == 0) r += 1;\n"
		"	const char* b = R\"x(a)\"b)x\";\n"
		"	if (sizeof(R\"x(a)\"b)x\") == 5 && b[1] == ')' && b[2] == '\"') r += 2;\n"
		"	const char* c = R\"(line\n"
		"next)\";\n"
		"	if (c[4] == '\\n' && c[5] == 'n') r += 4;\n"
		"	if (sizeof(u8R\"(ab)\") == 3 && sizeof(uR\"(ab)\") == 3 * sizeof(char16_t) && sizeof(UR\"(ab)\") == 3 * sizeof(char32_t) && sizeof(LR\"(ab)\") == 3 * sizeof(wchar_t)) r += 8;\n"
		"	if (sizeof(R\"()\") == 1) r += 16;\n"
		"	if (sizeof(R\"a()a\" \"b\") == 2 && R\"a(?\?=)a\"[0] == '?') r += 32;\n"
		"	const char* d = R\"(\\\n"
		")\";\n"
		"	if (d[0] == '\\\\' && d[1] == '\\n') r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.5 [lex.string] p2: a raw string's delimiter is at most 16 characters
	{
		"LexRawStringDelimiterTooLongRefused", CONFORMANCE_REFUSED,
		"const char* s = R\"abcdefghijklmnopq(x)abcdefghijklmnopq\";\n"
		"int main() { return s[0]; }\n"
	},
	//2.14.5 [lex.string] p2: a raw string's delimiter has no space, \, ( or )
	{
		"LexRawStringDelimiterSpaceRefused", CONFORMANCE_REFUSED,
		"const char* s = R\"a b(x)a b\";\n"
		"int main() { return s[0]; }\n"
	},
	//2.14.5 [lex.string] p13: joined strings take the prefix one has; u8 joins with none
	{
		"LexStringConcatenation", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(u\"a\" \"b\"), const char16_t (&)[3]>::V && Same<decltype(\"a\" U\"b\"), const char32_t (&)[3]>::V) r += 1;\n"
		"	if (Same<decltype(L\"a\" \"b\" L\"c\"), const wchar_t (&)[4]>::V) r += 2;\n"
		"	if (Same<decltype(u8\"a\" \"b\"), const char (&)[3]>::V && sizeof(u8\"a\" \"b\") == 3) r += 4;\n"
		"	if (u\"a\" \"b\"[1] == 'b' && sizeof(U\"a\" \"bc\") == 4 * sizeof(char32_t)) r += 8;\n"
		"	if (sizeof(\"\\x4\" \"1\") == 3 && (\"\\x4\" \"1\")[0] == 4) r += 16;\n"
		"	if (sizeof(R\"(a)\" \"b\") == 3) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.5 [lex.string] p13: u and U strings aren't joined
	{
		"LexStringConcatenationMixedRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof(u\"a\" U\"b\"); }\n"
	},
	//2.14.5 [lex.string] p13: a UTF-8 string isn't joined with a wide one
	{
		"LexStringConcatenationUtf8WideRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof(u8\"a\" L\"b\"); }\n"
	},
	//2.14.5 [lex.string] p1: a string ends on its line
	{
		"LexStringUnterminatedRefused", CONFORMANCE_REFUSED,
		"int main() { const char* s = \"abc;\n"
		"return s[0]; }\n"
	},
	//2.14.6 [lex.bool]: true and false are bools, 1 and 0 as integers
	{
		"LexBooleanLiterals", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(true), bool>::V && Same<decltype(false), bool>::V) r += 1;\n"
		"	if (true + true == 2 && false * 5 == 0) r += 2;\n"
		"	if (sizeof(true) == sizeof(bool)) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.7 [lex.nullptr]: nullptr is a std::nullptr_t, a null pointer to any pointer or member pointer type
	{
		"LexPointerLiteral", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef decltype(nullptr) nullptr_t;\n"
		"struct S { int m; int F() { return 1; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (!Same<nullptr_t, int>::V && Same<decltype(nullptr), nullptr_t>::V) r += 1;\n"
		"	int* p = nullptr;\n"
		"	int S::* pm = nullptr;\n"
		"	int (S::* pf)() = nullptr;\n"
		"	if (!p && !pm && !pf) r += 2;\n"
		"	if (sizeof(nullptr) == sizeof(void*)) r += 4;\n"
		"	nullptr_t n = nullptr;\n"
		"	char* q = n;\n"
		"	if (!q && n == nullptr) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.8 [lex.ext]: integer, floating, string and character literals with a suffix call their literal operator
	{
		"LexUserDefinedLiterals", 255,
		"typedef decltype(sizeof(0)) size_t;\n"
		"unsigned long long operator\"\" _i(unsigned long long v) { return v + 1; }\n"
		"long double operator\"\" _f(long double v) { return v * 2; }\n"
		"size_t operator\"\" _s(const char* s, size_t n) { return n * 10 + (s[0] == 'a'); }\n"
		"size_t operator\"\" _s(const char16_t* s, size_t n) { return n * 100 + (s[0] == 'a'); }\n"
		"int operator\"\" _c(char c) { return c + 1; }\n"
		"int operator\"\" _c(char32_t c) { return (int)c + 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (12_i == 13 && 0x10_i == 17 && 0b11_i == 4 && 1'0_i == 11) r += 1;\n"
		"	if (1.5_f == 3.0L && 2e1_f == 40.0L) r += 2;\n"
		"	if (\"abc\"_s == 31) r += 4;\n"
		"	if (u\"ab\"_s == 201) r += 8;\n"
		"	if ('a'_c == 'b' && U'a'_c == 'a' + 2) r += 16;\n"
		"	if (\"ab\" \"cd\"_s == 41 && \"ab\"_s \"cd\" == 41) r += 32;\n"
		"	if (R\"(ab)\"_s == 21) r += 64;\n"
		"	if (12_i + 1 == 14) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.8 [lex.ext] p6: a character literal's operator is given the literal's own type: wchar_t, char16_t or char32_t
	{
		"LexUserDefinedWideCharacterLiterals", 7,
		"int operator\"\" _k(char c) { return 1; }\n"
		"int operator\"\" _k(wchar_t c) { return 2; }\n"
		"int operator\"\" _k(char16_t c) { return 3; }\n"
		"int operator\"\" _k(char32_t c) { return 4; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if ('a'_k == 1 && L'a'_k == 2) r += 1;\n"
		"	if (u'a'_k == 3) r += 2;\n"
		"	if (U'a'_k == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.8 [lex.ext] p3, p4: a raw literal operator gets the characters; a literal operator template gets them as its arguments
	{
		"LexUserDefinedRawAndTemplateLiterals", 31,
		"int operator\"\" _r(const char* s) { int n = 0; while (s[n]) n++; return n * 10 + (s[0] == '0'); }\n"
		"template<char... C> struct Count { enum { V = sizeof...(C) }; };\n"
		"template<char... C> int operator\"\" _t() { return Count<C...>::V; }\n"
		"template<char First, char... C> struct FirstOf { enum { V = First }; };\n"
		"template<char... C> int operator\"\" _first() { return FirstOf<C...>::V; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (0x12_r == 41) r += 1;\n"
		"	if (1.5e3_r == 50) r += 2;\n"
		"	if (123_t == 3 && 0x1F_t == 4) r += 4;\n"
		"	if (7_first == '7') r += 8;\n"
		"	if (1'2_t == 3) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//2.14.8 [lex.ext] p8: joined strings have one suffix
	{
		"LexUserDefinedLiteralSuffixesDifferRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"size_t operator\"\" _a(const char*, size_t n) { return n; }\n"
		"size_t operator\"\" _b(const char*, size_t n) { return n; }\n"
		"int main() { return (int)(\"x\"_a \"y\"_b); }\n"
	},
	//2.14.8 [lex.ext] p3: a literal with a suffix needs its literal operator
	{
		"LexUserDefinedLiteralNoOperatorRefused", CONFORMANCE_REFUSED,
		"int main() { return 12_none; }\n"
	},
	//13.5.8 [over.literal] p3, 2.14.8 [lex.ext]: a literal operator's parameters are of the listed kinds
	{
		"LexUserDefinedLiteralBadParametersRefused", CONFORMANCE_REFUSED,
		"int operator\"\" _x(int v) { return v; }\n"
		"int main() { return 1_x; }\n"
	},
};


//Clause 3, basic concepts [basic].
static const SConformanceCase gasConformanceClause3[] =
{
	//3.7.4.1 [basic.stc.dynamic.allocation] p1, 3.7.4.2 [basic.stc.dynamic.deallocation] p2: an allocation function returns void* and takes a std::size_t first, a deallocation function returns void and takes a void* first
	{
		"BasicAllocationFunctions", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[32];\n"
		"int giFreed = 0;\n"
		"void* operator new(size_t n) { return gacHeap; }\n"
		"void* operator new[](size_t n) { return gacHeap + 8; }\n"
		"void* operator new(size_t n, int k) { return gacHeap + k; }\n"
		"void operator delete(void* p) noexcept { giFreed += 1; }\n"
		"void operator delete[](void* p) noexcept { giFreed += 10; }\n"
		"void operator delete(void* p, int k) noexcept { giFreed += 100; }\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	static void* operator new(size_t n) { return gacHeap + 16; }\n"
		"	static void operator delete(void* p, size_t n) { giFreed += (int)n * 1000; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int* a = new int;\n"
		"	int* b = new int[2];\n"
		"	int* c = new (4) int;\n"
		"	S* s = new S;\n"
		"	if ((char*)a == gacHeap && (char*)b == gacHeap + 8) r += 1;\n"
		"	if ((char*)c == gacHeap + 4) r += 2;\n"
		"	if ((char*)s == gacHeap + 16) r += 4;\n"
		"	delete a;\n"
		"	delete[] b;\n"
		"	delete s;\n"
		"	if (giFreed == 11 + (int)sizeof(S) * 1000) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.7.4.1 [basic.stc.dynamic.allocation] p1: an allocation function's first parameter has type std::size_t
	{
		"BasicAllocationFirstParameterRefused", CONFORMANCE_REFUSED,
		"void* operator new(unsigned long long n);\n"
		"int main() { return 0; }\n"
	},
	//3.7.4.1 [basic.stc.dynamic.allocation] p1: an allocation function returns void*
	{
		"BasicAllocationReturnRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"int operator new(size_t n) { return 0; }\n"
		"int main() { return 0; }\n"
	},
	//3.7.4.2 [basic.stc.dynamic.deallocation] p2: a deallocation function's first parameter has type void*
	{
		"BasicDeallocationFirstParameterRefused", CONFORMANCE_REFUSED,
		"void operator delete(int* p) noexcept { }\n"
		"int main() { return 0; }\n"
	},
	//3.1 [basic.def] p2: what only declares (extern, a function's declaration, a class's name, a static member in its class, typedef, using, extern "C", an opaque enum) and what defines
	{
		"BasicDeclarationsAndDefinitions", 31,
		"extern int ga;\n"
		"int Twice(int);\n"
		"struct S;\n"
		"struct T { static int k; int v; };\n"
		"typedef int Int;\n"
		"using Long = long;\n"
		"extern \"C\" int gc;\n"
		"enum class E : int;\n"
		"int ga = 3;\n"
		"int gc = 4;\n"
		"int T::k = 5;\n"
		"struct S { int s; };\n"
		"int Twice(int x) { return x * 2; }\n"
		"enum class E : int { A = 7 };\n"
		"extern \"C\" { int gd = 6; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (ga == 3 && Twice(4) == 8) r += 1;\n"
		"	S s; s.s = 2; Int i = s.s; Long l = 3;\n"
		"	if (i == 2 && l == 3) r += 2;\n"
		"	if (T::k == 5 && gc == 4 && gd == 6) r += 4;\n"
		"	if ((int)E::A == 7) r += 8;\n"
		"	extern int ge;\n"
		"	if (ge == 9) r += 16;\n"
		"	return r;\n"
		"}\n"
		"int ge = 9;\n"
	},
	//3.1 [basic.def] p2, 3.2 [basic.def.odr] p1: a declaration inside extern "C" { } is a definition, so one more defines the variable twice
	{
		"BasicExternCBlockDefinesRefused", CONFORMANCE_REFUSED,
		"extern \"C\" { int gz; }\n"
		"int gz;\n"
		"int main() { return gz; }\n"
	},
	//3.1 [basic.def] p3, 12 [special] p1: the members a class gets implicitly, each doing its members' own
	{
		"BasicImplicitMembers", 31,
		"struct A { int v; A() : v(1) { } A(const A& a) : v(a.v + 10) { } A& operator=(const A& a) { v = a.v + 100; return *this; } };\n"
		"struct C { A a; int k; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (c.a.v == 1) r += 1;\n"
		"	C d(c);\n"
		"	if (d.a.v == 11) r += 2;\n"
		"	C e; e = c;\n"
		"	if (e.a.v == 101) r += 4;\n"
		"	C f(static_cast<C&&>(c));\n"
		"	if (f.a.v == 11) r += 8;\n"
		"	C g; g = static_cast<C&&>(d);\n"
		"	if (g.a.v == 111) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//3.2 [basic.def.odr] p1: a variable defined twice in a unit
	{
		"BasicVariableDefinedTwiceRefused", CONFORMANCE_REFUSED,
		"int g;\n"
		"int g;\n"
		"int main() { return g; }\n"
	},
	//3.2 [basic.def.odr] p1: a function defined twice in a unit
	{
		"BasicFunctionDefinedTwiceRefused", CONFORMANCE_REFUSED,
		"int F() { return 1; }\n"
		"int F() { return 2; }\n"
		"int main() { return F(); }\n"
	},
	//3.2 [basic.def.odr] p1: a class defined twice in a unit
	{
		"BasicClassDefinedTwiceRefused", CONFORMANCE_REFUSED,
		"struct S { int a; };\n"
		"struct S { int a; };\n"
		"int main() { S s; s.a = 0; return s.a; }\n"
	},
	//3.2 [basic.def.odr] p3, 9.8 [class.local] p1: a constant whose value is used at once isn't odr-used, so a local class may use its function's; and its static locals
	{
		"BasicOdrConstantNotOdrUsed", 7,
		"int main()\n"
		"{\n"
		"	const int k = 5;\n"
		"	enum { N = 2 };\n"
		"	struct L { int Get() { return k + N; } int Size() { return sizeof(int[k]) / sizeof(int); } };\n"
		"	L l;\n"
		"	int r = 0;\n"
		"	if (l.Get() == 7) r += 1;\n"
		"	if (l.Size() == 5) r += 2;\n"
		"	static int s = 3;\n"
		"	struct M { int Get() { return s; } };\n"
		"	M m;\n"
		"	if (m.Get() == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.2 [basic.def.odr] p3, 9.8 [class.local] p1: an automatic variable of the enclosing function can't be odr-used in a local class
	{
		"BasicOdrLocalClassUsesAutomaticRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	struct L { int Get() { return x; } };\n"
		"	L l;\n"
		"	return l.Get();\n"
		"}\n"
	},
	//3.2 [basic.def.odr] p3, 9.8 [class.local] p1: a local class may use the value of its function's constant, not its object
	{
		"BasicOdrLocalClassConstantAddressRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	const int k = 5;\n"
		"	struct L { const int* Get() { return &k; } };\n"
		"	L l;\n"
		"	return *l.Get();\n"
		"}\n"
	},
	//3.3.1 [basic.scope.declarative] p4: two enumerators of one name in one scope
	{
		"BasicEnumeratorRedeclaredRefused", CONFORMANCE_REFUSED,
		"enum E { A, B };\n"
		"enum F { C, A };\n"
		"int main() { return B; }\n"
	},
	//3.3.1 [basic.scope.declarative] p4: a function and a variable of one name in one scope
	{
		"BasicFunctionAndVariableSameNameRefused", CONFORMANCE_REFUSED,
		"int f;\n"
		"int f();\n"
		"int main() { return 0; }\n"
	},
	//3.3.1 [basic.scope.declarative] p4, 3.3.10 [basic.scope.hiding] p2: a class name is hidden by a variable or function of the name in its scope, and an elaborated type specifier still finds it
	{
		"BasicClassHiddenByVariable", 15,
		"struct S { int v; };\n"
		"int S = 4;\n"
		"struct T { int w; };\n"
		"int T(int a) { return a * 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (S == 4) r += 1;\n"
		"	struct S s; s.v = 3;\n"
		"	if (s.v == 3) r += 2;\n"
		"	if (T(5) == 10) r += 4;\n"
		"	struct T t; t.w = 6;\n"
		"	if (t.w == 6 && sizeof(struct S) == sizeof(int)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.10 [basic.scope.hiding] p2, 3.4.3 [basic.lookup.qual] p1, 3.4.4 [basic.lookup.elab] p3: a member of a namespace or class that hides a class of its name is what a qualified name finds; an elaborated type specifier and a name before :: find the class
	{
		"BasicClassHiddenInQualifiedLookup", 31,
		"namespace N { struct S { int v; static const int k = 16; }; int S(int x) { return x + 1; } }\n"
		"struct C { struct T { int w; }; static int T; };\n"
		"int C::T = 7;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (N::S(2) == 3) r += 1;\n"
		"	struct N::S s;\n"
		"	s.v = 4;\n"
		"	if (s.v == 4) r += 2;\n"
		"	if (C::T == 7) r += 4;\n"
		"	struct C::T t;\n"
		"	t.w = 5;\n"
		"	if (t.w == 5) r += 8;\n"
		"	if (N::S::k == 16) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.2 [basic.scope.pdecl] p1, p3: a name is declared after its complete declarator, before its initialiser; an enumerator after its definition
	{
		"BasicPointOfDeclaration", 7,
		"const int i = 2;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		int i[i];\n"
		"		if (sizeof(i) == 2 * sizeof(int)) r += 1;\n"
		"	}\n"
		"	{\n"
		"		enum { A = 3, B = A + 1 };\n"
		"		if (B == 4) r += 2;\n"
		"	}\n"
		"	{\n"
		"		int y = sizeof(y) == sizeof(int) ? 5 : 0;\n"
		"		if (y == 5) r += 4;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.2 [basic.scope.pdecl] p3: an enumerator is declared after its definition, so its initialiser names the constant outside
	{
		"BasicEnumeratorPointOfDeclaration", 1,
		"const int x = 12;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		enum { x = x };\n"
		"		if (x == 12) r += 1;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.2 [basic.scope.pdecl] p2, p7: a class is declared after its name in its head; an elaborated type specifier in a function's declaration declares the class in the namespace
	{
		"BasicPointOfDeclarationClass", 7,
		"struct Node { Node* next; int v; };\n"
		"struct Q* Make(struct Q* p) { return p; }\n"
		"struct Q { int q; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Node a, b; a.next = &b; b.v = 4;\n"
		"	if (a.next->v == 4) r += 1;\n"
		"	Q q; q.q = 5;\n"
		"	if (Make(&q)->q == 5) r += 2;\n"
		"	struct Node* p = &a;\n"
		"	if (p->next == &b) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.3 [basic.scope.block] p1, p4: names declared in a for, if, while or switch are the statement's
	{
		"BasicBlockScopes", 31,
		"int i = 100;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int n = 0;\n"
		"	for (int i = 0; i < 3; i++) n += i;\n"
		"	if (n == 3 && i == 100) r += 1;\n"
		"	if (int k = n * 2) { if (k == 6) r += 2; } else { r += 64; }\n"
		"	if (int z = 0) { r += 64; } else { if (z == 0) r += 4; }\n"
		"	int count = 0;\n"
		"	while (int w = 3 - count) { count++; if (w < 1) break; }\n"
		"	if (count == 3) r += 8;\n"
		"	switch (int s = n + 1) { case 4: if (s == 4) r += 16; break; default: break; }\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.3 [basic.scope.block] p4, 6.4 [stmt.select] p4: a switch's condition may declare a variable, of an integer or enum type, which is in scope in its body
	{
		"BasicSwitchDeclarationCondition", 7,
		"enum E { A, B, C };\n"
		"E Pick(int i) { return (E)i; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	switch (E e = Pick(1)) { case A: break; case B: if (e == B) r += 1; break; default: break; }\n"
		"	switch (char c = 'q') { case 'q': { int c2 = c + 1; if (c2 == 'r') r += 2; } break; }\n"
		"	for (int i = 0; i < 2; i++)\n"
		"	{\n"
		"		switch (int k = i * 3) { case 3: if (k == 3) r += 4; break; default: break; }\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: a switch's condition, a declared one too, is of an integer or enum type
	{
		"BasicSwitchFloatConditionRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	switch (double d = 1.5) { case 1: return 1; }\n"
		"	return 0;\n"
		"}\n"
	},
	//3.3.3 [basic.scope.block] p4: the outermost block of a for's body can't declare a name its init-statement does
	{
		"BasicForBodyRedeclaresRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	for (int i = 0; i < 2; i++) { int i = 5; (void)i; }\n"
		"	return 0;\n"
		"}\n"
	},
	//3.3.3 [basic.scope.block] p4: an if's else can't declare a name its condition does
	{
		"BasicConditionRedeclaredRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	if (int x = 1) { return x; } else { int x = 2; return x; }\n"
		"}\n"
	},
	//3.3.3 [basic.scope.block] p2: a function's outermost block can't declare a name of a parameter
	{
		"BasicParameterRedeclaredRefused", CONFORMANCE_REFUSED,
		"int F(int a) { int a = 2; return a; }\n"
		"int main() { return F(1); }\n"
	},
	//3.3.4 [basic.scope.proto]: a parameter's name ends with its declarator; the declarations of a function may name parameters differently
	{
		"BasicFunctionPrototypeScope", 3,
		"int F(int a, int b);\n"
		"int F(int b, int a);\n"
		"int G(int a, int (*)(int a));\n"
		"int F(int x, int y) { return x - y; }\n"
		"int Neg(int v) { return -v; }\n"
		"int G(int a, int (*f)(int)) { return f(a); }\n"
		"int a = 7;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(5, 2) == 3) r += 1;\n"
		"	if (G(a, Neg) == -7) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.5 [basic.funscope]: a label is in scope in the whole function, before it and in other blocks, and doesn't clash with other names
	{
		"BasicFunctionScopeLabels", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int L = 0;\n"
		"	goto L;\n"
		"	r += 64;\n"
		"L:\n"
		"	r += 1;\n"
		"	{\n"
		"		if (L == 0) { L = 1; goto Inner; }\n"
		"		r += 64;\n"
		"	}\n"
		"	{\n"
		"	Inner:\n"
		"		r += 2;\n"
		"	}\n"
		"	int count = 0;\n"
		"Again:\n"
		"	count++;\n"
		"	if (count < 3) goto Again;\n"
		"	if (count == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.5 [basic.funscope], 6.1 [stmt.label] p1: a label declared twice in a function, though in another block
	{
		"BasicLabelDeclaredTwiceRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	goto A;\n"
		"A:\n"
		"	{\n"
		"A:\n"
		"		;\n"
		"	}\n"
		"	return 0;\n"
		"}\n"
	},
	//3.3.1 [basic.scope.declarative] p4, 3.3.3 [basic.scope.block] p2, p4, 3.3.9 [basic.scope.temp], 6.1 [stmt.label] p1: names declared again where that is allowed: in a nested block, as overloads, an extern twice, labels of one name in two functions and a lambda, a template parameter's name in another template
	{
		"BasicRedeclarationsAllowed", 63,
		"int F(int a) { { int a = 2; return a; } }\n"
		"int F(int a, int b) { return a + b; }\n"
		"int G() { L: return 1; goto L; }\n"
		"int H() { auto f = []() { L: return 2; goto L; }; L: return f(); goto L; }\n"
		"template<class T> struct A { T v; };\n"
		"template<class T> struct B { T w; };\n"
		"enum E { X };\n"
		"namespace N { enum E2 { X }; }\n"
		"int gi = 4;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1) == 2 && F(1, 2) == 3) r += 1;\n"
		"	for (int i = 0; i < 1; i++) { { int i = 5; if (i == 5) r += 2; } }\n"
		"	{ extern int gi; extern int gi; if (gi == 4) r += 4; }\n"
		"	if (G() == 1 && H() == 2) r += 8;\n"
		"	A<int> a; B<char> b; a.v = 1; b.w = 2;\n"
		"	if (a.v + b.w == 3) r += 16;\n"
		"	if (X == 0 && N::X == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.7 [basic.scope.class] p1: a member is in scope in member function bodies, default arguments and mem-initialisers, though declared after them
	{
		"BasicClassScope", 31,
		"struct S\n"
		"{\n"
		"	int Get() { return v + Later(); }\n"
		"	int Default(int a = k) { return a; }\n"
		"	S() : w(k * 2) { v = 1; }\n"
		"	int v;\n"
		"	int w;\n"
		"	static const int k = 4;\n"
		"	int Later() { return 10; }\n"
		"	typedef int Int;\n"
		"	Int Typed() { Int i = 3; return i; }\n"
		"	struct In { int Twice() { return k * 2; } };\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	int r = 0;\n"
		"	if (s.Get() == 11) r += 1;\n"
		"	if (s.Default() == 4) r += 2;\n"
		"	if (s.w == 8) r += 4;\n"
		"	if (s.Typed() == 3) r += 8;\n"
		"	S::In in;\n"
		"	if (in.Twice() == 8) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.7 [basic.scope.class] p1, 9.2 [class.mem] p2: a default argument in a class names members declared after it, in a class template and a constructor too, and for a function defined outside the class
	{
		"BasicClassScopeDefaultArguments", 15,
		"template<class T> struct A { int Get(int a = n) { return a; } static const int n = sizeof(T) * 0 + 6; };\n"
		"struct B\n"
		"{\n"
		"	B(int a = Seed()) : v(a) { }\n"
		"	int Add(int a, int b = Inner::k);\n"
		"	static int Seed() { return 7; }\n"
		"	int v;\n"
		"	struct Inner { static const int k = 2; };\n"
		"};\n"
		"int B::Add(int a, int b) { return a + b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<char> x;\n"
		"	if (x.Get() == 6) r += 1;\n"
		"	B b;\n"
		"	if (b.v == 7) r += 2;\n"
		"	if (b.Add(1) == 3) r += 4;\n"
		"	if (A<int>().Get(1) == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.7 [basic.scope.class] p1, 3.4.1 [basic.lookup.unqual] p8: after a member's declarator-id outside its class, the class's names are in scope
	{
		"BasicClassScopeOutOfClassDefinition", 7,
		"struct S\n"
		"{\n"
		"	typedef int T;\n"
		"	static const int k = 3;\n"
		"	T Twice(T t);\n"
		"	static T Data;\n"
		"	enum Kind { One = 1, Two };\n"
		"	Kind Next(Kind k);\n"
		"};\n"
		"S::T S::Twice(T t) { return t * 2 + k; }\n"
		"S::T S::Data = k + 1;\n"
		"S::Kind S::Next(Kind e) { return e == One ? Two : One; }\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	int r = 0;\n"
		"	if (s.Twice(2) == 7) r += 1;\n"
		"	if (S::Data == 4) r += 2;\n"
		"	if (s.Next(S::One) == S::Two) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.8 [basic.scope.enum]: a scoped enumeration's enumerators are in its scope only; an unscoped one's are also its enclosing scope's, and may be qualified
	{
		"BasicEnumerationScope", 15,
		"enum class Colour { Red = 1, Green = 2 };\n"
		"enum Shape { Circle = 4, Square = 8 };\n"
		"int Red = 16;\n"
		"struct H { enum In { X = 32 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if ((int)Colour::Green == 2) r += 1;\n"
		"	if (Red == 16) r += 2;\n"
		"	if (Circle == 4 && Shape::Square == 8) r += 4;\n"
		"	if (H::X == 32 && H::In::X == 32) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.8 [basic.scope.enum] p1: a scoped enumerator isn't in the enclosing scope
	{
		"BasicScopedEnumeratorUnqualifiedRefused", CONFORMANCE_REFUSED,
		"enum class Colour { Red, Green };\n"
		"int main() { Colour c = Green; return (int)c; }\n"
	},
	//3.3.9 [basic.scope.temp]: a template parameter is in scope from its declaration to the end of its template, in later parameters too, and hides a name outside
	{
		"BasicTemplateParameterScope", 7,
		"typedef double T;\n"
		"const int N = 100;\n"
		"template<class T, T N = sizeof(T)> struct A { T a; static const T n = N; };\n"
		"template<class T> T Same(T t) { return t; }\n"
		"template<class U, class V = U*> struct P { V p; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<int, 3>::n == 3) r += 1;\n"
		"	if (A<char>::n == 1 && Same(5) == 5) r += 2;\n"
		"	int x = 4;\n"
		"	P<int> p; p.p = &x;\n"
		"	if (*p.p == 4 && sizeof(T) == sizeof(double)) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.9 [basic.scope.temp] p1, 14.1 [temp.param] p10: a non-type parameter's default uses the parameters before it, in a class template and a function template
	{
		"BasicTemplateParameterInLaterDefault", 7,
		"template<class T, int N = sizeof(T) + 1> struct A { static const int n = N; };\n"
		"template<class T, int N = sizeof(T) * 2> int Size() { return N; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<char>::n == 2) r += 1;\n"
		"	if (A<char, 7>::n == 7) r += 2;\n"
		"	if (Size<char>() == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.9 [basic.scope.temp], 14.6.1 [temp.local] p6: a template parameter's name can't be declared again in its scope
	{
		"BasicTemplateParameterRedeclaredRefused", CONFORMANCE_REFUSED,
		"template<class T> struct S { int T; };\n"
		"int main() { S<int> s; return 0; }\n"
	},
	//3.3.10 [basic.scope.hiding] p1: a name in a nested block hides the outer one, which :: still reaches
	{
		"BasicNameHiddenInBlock", 7,
		"int v = 1;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int v = 2;\n"
		"	{\n"
		"		int v = 3;\n"
		"		if (v == 3 && ::v == 1) r += 1;\n"
		"	}\n"
		"	if (v == 2) r += 2;\n"
		"	{\n"
		"		double v = 4.5;\n"
		"		if (v > 4 && v < 5) r += 4;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.10 [basic.scope.hiding] p3, 10.2 [class.member.lookup]: a derived class's member hides every base member of the name, overloads too, but for what a using-declaration brings in
	{
		"BasicDerivedHidesBase", 15,
		"struct B { int F(int a) { return a; } int F(double) { return 50; } int G() { return 1; } int x = 7; };\n"
		"struct D : B { int F() { return 2; } int x = 8; };\n"
		"struct U : B { using B::F; int F() { return 3; } };\n"
		"int main()\n"
		"{\n"
		"	D d; U u;\n"
		"	int r = 0;\n"
		"	if (d.F() == 2 && d.B::F(4) == 4) r += 1;\n"
		"	if (d.G() == 1 && d.x == 8 && d.B::x == 7) r += 2;\n"
		"	if (u.F() == 3 && u.F(5) == 5 && u.F(1.5) == 50) r += 4;\n"
		"	B& b = d;\n"
		"	if (b.F(6) == 6) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.3.10 [basic.scope.hiding] p3: the base's overload a derived member hides isn't a candidate
	{
		"BasicDerivedHidesBaseRefused", CONFORMANCE_REFUSED,
		"struct B { int F(int a) { return a; } };\n"
		"struct D : B { int F() { return 2; } };\n"
		"int main() { D d; return d.F(1); }\n"
	},
	//3.4.1 [basic.lookup.unqual] p4, p6: a name used in a function at namespace scope must be declared before its use
	{
		"BasicLookupDeclaredAfterRefused", CONFORMANCE_REFUSED,
		"int F() { return g; }\n"
		"int g = 1;\n"
		"int main() { return F(); }\n"
	},
	//3.4.1 [basic.lookup.unqual] p8: in a member function: the block, the class and its bases, the enclosing classes, then the namespaces
	{
		"BasicLookupInMemberFunction", 7,
		"int a = 1, b = 2, c = 3, d = 4;\n"
		"namespace N\n"
		"{\n"
		"	int d = 40;\n"
		"	struct Base { int b = 20; static const int e = 5; };\n"
		"	struct Outer\n"
		"	{\n"
		"		static const int c = 30;\n"
		"		typedef int OuterInt;\n"
		"		struct Inner : Base\n"
		"		{\n"
		"			int Get(int a) { return a + b + c + d + e; }\n"
		"			OuterInt Typed() { OuterInt v = 6; return v; }\n"
		"			int Later() { return f; }\n"
		"			int f = 9;\n"
		"		};\n"
		"	};\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	N::Outer::Inner in;\n"
		"	int r = 0;\n"
		"	int v = in.Get(100);\n"
		"	if (v == 100 + 20 + 30 + 40 + 5) r += 1;\n"
		"	if (in.Typed() == 6) r += 2;\n"
		"	if (in.Later() == 9) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p8, 9.8 [class.local] p1: a local class's member function sees its function's types, static locals and enumerators, then the namespace
	{
		"BasicLookupLocalClass", 7,
		"int g = 3;\n"
		"int main()\n"
		"{\n"
		"	typedef int Local;\n"
		"	static int s = 4;\n"
		"	enum { E = 5 };\n"
		"	struct L { Local Sum() { Local x = s + E + g; return x; } };\n"
		"	L l;\n"
		"	int r = 0;\n"
		"	if (l.Sum() == 12) r += 1;\n"
		"	s = 10;\n"
		"	if (l.Sum() == 18) r += 2;\n"
		"	struct M : L { int More() { return Sum() + 1; } };\n"
		"	M m;\n"
		"	if (m.More() == 19) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p9: a friend function defined in its class looks names up as a member function does
	{
		"BasicLookupFriendDefinedInClass", 3,
		"int k = 100;\n"
		"struct S\n"
		"{\n"
		"	static const int k = 4;\n"
		"	typedef int Int;\n"
		"	int v;\n"
		"	friend Int Get(const S& s) { return s.v + k; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S s; s.v = 3;\n"
		"	int r = 0;\n"
		"	if (Get(s) == 7) r += 1;\n"
		"	if (k == 100) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p10: in a friend declaration of another class's member function, a parameter's type is looked up first in that member's class
	{
		"BasicLookupFriendMemberDeclaration", 3,
		"struct A { typedef int AT; int F(AT a); int G(int a); };\n"
		"struct B { typedef char AT; typedef int BT; friend int A::F(AT); friend int A::G(BT); private: int p = 7; };\n"
		"int A::F(AT a) { B b; return b.p + a; }\n"
		"int A::G(int a) { B b; return b.p * a; }\n"
		"int main()\n"
		"{\n"
		"	A x;\n"
		"	int r = 0;\n"
		"	if (x.F(1) == 8) r += 1;\n"
		"	if (x.G(2) == 14) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p13: a static member's initialiser outside its class is looked up in the class
	{
		"BasicLookupStaticMemberDefinition", 7,
		"int k = 100;\n"
		"struct S { static int a; static const int k = 3; static int b; static int Twice(int x) { return x * 2; } };\n"
		"int S::a = k + 1;\n"
		"int S::b = Twice(k);\n"
		"int c = k;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (S::a == 4) r += 1;\n"
		"	if (S::b == 6) r += 2;\n"
		"	if (c == 100) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p14: a namespace member's initialiser outside its namespace is looked up in the namespace first
	{
		"BasicLookupNamespaceMemberDefinition", 3,
		"int y = 100;\n"
		"namespace N { extern int x; int y = 3; }\n"
		"int N::x = y;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (N::x == 3) r += 1;\n"
		"	if (y == 100) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.1 [basic.lookup.unqual] p8, 12.6.2 [class.base.init] p2, p15: a mem-initialiser's expression is in the constructor's scope, its parameters hiding members
	{
		"BasicLookupMemberInitialiser", 7,
		"struct S\n"
		"{\n"
		"	int v; int w;\n"
		"	S(int v) : v(v), w(v + this->v) { }\n"
		"	S(int a, int b) : v(a), w(v * b) { }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S s(3); S t(2, 5);\n"
		"	int r = 0;\n"
		"	if (s.v == 3) r += 1;\n"
		"	if (s.w == 6) r += 2;\n"
		"	if (t.w == 10) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.3 [basic.lookup.qual] p1: the name before :: is looked up among namespaces, types and templates only
	{
		"BasicQualifiedLookupIgnoresVariables", 3,
		"namespace N { int v = 3; }\n"
		"struct C { static const int k = 2; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int N = 1;\n"
		"	if (N::v + N == 4) r += 1;\n"
		"	int C = 5;\n"
		"	if (C::k + C == 7) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.3 [basic.lookup.qual] p6: a qualified destructor name, the second type-name looked up as the first
	{
		"BasicQualifiedDestructorName", 3,
		"void* operator new(decltype(sizeof(0)), void* p) noexcept { return p; }\n"
		"int giGone = 0;\n"
		"struct X { ~X() { giGone++; } };\n"
		"typedef X XT;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	X x;\n"
		"	x.X::~X();\n"
		"	if (giGone == 1) r += 1;\n"
		"	new (&x) X;\n"
		"	x.XT::~XT();\n"
		"	if (giGone == 2) r += 2;\n"
		"	new (&x) X;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.3.1 [class.qual] p2: A::A, where a function's name may be, names the constructor and not the class
	{
		"BasicLookupConstructorNameRefused", CONFORMANCE_REFUSED,
		"struct A { A() { } int v; };\n"
		"int main() { A::A a; return 0; }\n"
	},
	//3.4.3.1 [class.qual] p2: A::A is the class where it is followed by ::, and after struct
	{
		"BasicConstructorNameQualifierAllowed", 4,
		"struct A { A() { } int v; static const int k = 3; };\n"
		"int main()\n"
		"{\n"
		"	A a;\n"
		"	struct A::A b;\n"
		"	b.v = 1;\n"
		"	return A::A::k + b.v;\n"
		"}\n"
	},
	//3.4.3.1 [class.qual] p2, 9 [class] p2: the injected-class-name: struct A::A and A::A:: are the class, and a template's name in it is its instance
	{
		"BasicLookupInjectedClassName", 7,
		"struct A { int v; static int k; };\n"
		"int A::k = 6;\n"
		"template<class T> struct W { W* self; int Get() { W<T>* p = self; return p ? 1 : 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	struct A::A a; a.v = 3;\n"
		"	if (a.v == 3) r += 1;\n"
		"	if (A::A::k == 6) r += 2;\n"
		"	W<int> w; w.self = 0;\n"
		"	if (w.Get() == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.4 [basic.lookup.elab]: an elaborated type specifier finds the class or enumeration a variable hides; one that finds nothing declares the class
	{
		"BasicLookupElaborated", 15,
		"struct S { int v; };\n"
		"int S = 5;\n"
		"void Use(struct Fresh* p);\n"
		"struct Fresh { int f; };\n"
		"void Use(struct Fresh* p) { p->f = 8; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	struct S s; s.v = 2;\n"
		"	if (s.v == 2 && S == 5) r += 1;\n"
		"	Fresh f; Use(&f);\n"
		"	if (f.f == 8) r += 2;\n"
		"	{\n"
		"		struct Local* p = 0;\n"
		"		struct Local { int l; };\n"
		"		Local l; l.l = 4; p = &l;\n"
		"		if (p->l == 4) r += 4;\n"
		"	}\n"
		"	enum E { A = 9 };\n"
		"	int E = 1;\n"
		"	enum E e = A;\n"
		"	if (e == 9 && E == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.5 [basic.lookup.classref]: a name after . or -> is looked up in the object's class; a qualified one too; a template member through template; a pseudo-destructor's type-name in the context
	{
		"BasicLookupMemberAccess", 15,
		"struct B { int v = 1; int F() { return 10; } };\n"
		"struct D : B { int v = 2; typedef B Base; };\n"
		"template<class T> struct H { template<class U> int Size() { return sizeof(U); } };\n"
		"template<class T> int Call(T& t) { return t.template Size<char>(); }\n"
		"int main()\n"
		"{\n"
		"	D d; D* p = &d;\n"
		"	int r = 0;\n"
		"	if (d.v == 2 && d.B::v == 1 && p->B::v == 1) r += 1;\n"
		"	if (p->Base::v == 1 && d.D::Base::F() == 10) r += 2;\n"
		"	H<int> h;\n"
		"	if (Call(h) == 1) r += 4;\n"
		"	typedef int I;\n"
		"	I i = 5; i.~I();\n"
		"	int* q = &i; q->~I();\n"
		"	if (i == 5) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.5 [basic.lookup.classref] p7: the type in a conversion-function-id is looked up in the object's class, then in the context
	{
		"BasicLookupConversionFunctionId", 3,
		"struct S { typedef int I; operator I() const { return 7; } operator double() const { return 2.5; } };\n"
		"typedef double D;\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	int r = 0;\n"
		"	if (s.operator I() == 7) r += 1;\n"
		"	if (s.operator D() == 2.5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.5 [basic.lookup.classref] p7, 12.3.2 [class.conv.fct] p1: a conversion function called by name is the one to that type, a base's too, through a pointer and with a qualifier
	{
		"BasicConversionFunctionCalledByName", 7,
		"struct B { operator int() const { return 3; } };\n"
		"struct D : B { operator char() const { return 'x'; } };\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	const D* p = &d;\n"
		"	int r = 0;\n"
		"	if (p->operator int() == 3) r += 1;\n"
		"	if (d.operator char() == 'x') r += 2;\n"
		"	if (d.B::operator int() == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.4.6 [basic.lookup.udir]: a using-directive and a namespace alias consider only namespace names
	{
		"BasicNamespaceNameLookup", 3,
		"namespace A { int v = 4; int w = 8; }\n"
		"int main()\n"
		"{\n"
		"	int A = 1;\n"
		"	namespace B = A;\n"
		"	using namespace A;\n"
		"	int r = 0;\n"
		"	if (B::v == 4 && A == 1) r += 1;\n"
		"	if (w == 8) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.5 [basic.link] p3, p4, p6: static, unnamed-namespace and const names have internal linkage; a block scope extern names the static one
	{
		"BasicLinkageInternal", 15,
		"static int Twice(int x) { return x * 2; }\n"
		"static int gs = 3;\n"
		"namespace { int gu = 4; int Thrice(int x) { return x * 3; } }\n"
		"const int gc = 5;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Twice(gs) == 6) r += 1;\n"
		"	if (Thrice(gu) == 12) r += 2;\n"
		"	if (gc == 5) r += 4;\n"
		"	extern int gs;\n"
		"	if (gs == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.5 [basic.link] p6, p7: a block scope extern or function declaration names the entity of the innermost enclosing namespace
	{
		"BasicLinkageBlockScopeDeclarations", 7,
		"int F(int);\n"
		"int gv = 2;\n"
		"namespace N { int H() { extern int gw; return gw; } int gw = 6; }\n"
		"int G() { int F(int); return F(3); }\n"
		"int F(int x) { return x + 1; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (G() == 4) r += 1;\n"
		"	if (N::H() == 6) r += 2;\n"
		"	{\n"
		"		extern int gv;\n"
		"		if (gv == 2) r += 4;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//3.5 [basic.link] p10: every declaration of a variable has one type
	{
		"BasicLinkageConflictingTypesRefused", CONFORMANCE_REFUSED,
		"int gx;\n"
		"int main() { extern long gx; return (int)gx; }\n"
	},
	//3.5 [basic.link] p10: every declaration of a function has one type
	{
		"BasicLinkageConflictingReturnRefused", CONFORMANCE_REFUSED,
		"int F();\n"
		"long F();\n"
		"int main() { return 0; }\n"
	},
	//3.5 [basic.link] p4, 7.1.3 [dcl.typedef] p9: an unnamed class or enumeration with a typedef name for linkage purposes
	{
		"BasicLinkageTypedefNameForUnnamedClass", 3,
		"typedef struct { int a; int Twice() { return a * 2; } } T;\n"
		"typedef enum { X = 3, Y } E;\n"
		"int Use(T t) { return t.Twice(); }\n"
		"int main()\n"
		"{\n"
		"	T t; t.a = 4;\n"
		"	int r = 0;\n"
		"	if (Use(t) == 8) r += 1;\n"
		"	E e = Y;\n"
		"	if (e == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.6.2 [basic.start.init] p2: constant initialisation comes before any dynamic initialisation
	{
		"BasicStaticBeforeDynamicInitialisation", 3,
		"int ReadB();\n"
		"int a = ReadB();\n"
		"int b = 7;\n"
		"int Make() { return 9; }\n"
		"int d = Make();\n"
		"int ReadB() { return b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (a == 7) r += 1;\n"
		"	if (d == 9) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.6.2 [basic.start.init] p2: an object made by a constexpr constructor from constants, a reference to a static object and a pointer to one are initialised before any dynamic initialisation
	{
		"BasicConstantInitialisationOfObjects", 7,
		"struct P { constexpr P(int a) : v(a) { } int v; };\n"
		"int ReadP();\n"
		"int ReadR();\n"
		"int ReadQ();\n"
		"int x = ReadP();\n"
		"int y = ReadR();\n"
		"int z = ReadQ();\n"
		"P p(9);\n"
		"int g = 3;\n"
		"int& rg = g;\n"
		"const int* pg = &g;\n"
		"int ReadP() { return p.v; }\n"
		"int ReadR() { return rg; }\n"
		"int ReadQ() { return *pg; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (x == 9) r += 1;\n"
		"	if (y == 3) r += 2;\n"
		"	if (z == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.6.2 [basic.start.init] p2: a table of pointers to static objects, and an object with a destructor made by a constexpr constructor, are initialised before any dynamic initialisation
	{
		"BasicConstantInitialisationTables", 15,
		"struct D { constexpr D(int a) : v(a) { } ~D() { giDestroyed++; } int v; static int giDestroyed; };\n"
		"int D::giDestroyed = 0;\n"
		"int ReadTable();\n"
		"int ReadD();\n"
		"int ReadText();\n"
		"int x = ReadTable();\n"
		"int y = ReadD();\n"
		"int z = ReadText();\n"
		"int ga = 4;\n"
		"int gb = 5;\n"
		"int* gap[2] = { &ga, &gb };\n"
		"D d(6);\n"
		"const char* gsz = \"ab\";\n"
		"int ReadTable() { return *gap[0] + *gap[1]; }\n"
		"int ReadD() { return d.v; }\n"
		"int ReadText() { return gsz ? gsz[1] : 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (x == 9) r += 1;\n"
		"	if (y == 6) r += 2;\n"
		"	if (z == 'b') r += 4;\n"
		"	if (D::giDestroyed == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.6.2 [basic.start.init] p2, 5.19 [expr.const] p2: an object initialised as a constant but not declared constexpr has no value a constant expression can read
	{
		"BasicConstantInitialisedIsNoConstantRefused", CONFORMANCE_REFUSED,
		"struct P { constexpr P(int a) : v(a) { } int v; };\n"
		"P p(9);\n"
		"constexpr int k = p.v;\n"
		"int main() { return k; }\n"
	},
	//3.6.2 [basic.start.init] p2: the ordered dynamic initialisations of a unit are in the order of their definitions
	{
		"BasicDynamicInitialisationOrder", 7,
		"int giStep = 0;\n"
		"int Next(int k) { giStep = giStep * 10 + k; return giStep; }\n"
		"int a = Next(1);\n"
		"int b = Next(2);\n"
		"struct C { int v; C() : v(Next(3)) { } };\n"
		"C c;\n"
		"int d = Next(4);\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (a == 1 && b == 12) r += 1;\n"
		"	if (c.v == 123) r += 2;\n"
		"	if (d == 1234) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.7.3 [basic.stc.auto] p1, p3: each call has its own automatic objects, which last to the end of their block however it is left; one with side effects isn't left out though unused
	{
		"BasicAutomaticStorage", 15,
		"int giMade = 0, giGone = 0;\n"
		"struct C { C() { giMade++; } ~C() { giGone++; } };\n"
		"int Depth(int n, int* pOuter)\n"
		"{\n"
		"	int local = n;\n"
		"	if (pOuter && pOuter == &local) return -1;\n"
		"	if (n == 0) return 0;\n"
		"	int v = Depth(n - 1, &local);\n"
		"	return local == n ? v + 1 : -100;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Depth(4, 0) == 4) r += 1;\n"
		"	{ C unused; }\n"
		"	if (giMade == 1 && giGone == 1) r += 2;\n"
		"	for (int i = 0; i < 3; i++) { C each; }\n"
		"	if (giMade == 4 && giGone == 4) r += 4;\n"
		"	{\n"
		"		C a;\n"
		"		goto Out;\n"
		"	}\n"
		"Out:\n"
		"	if (giGone == 5) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.7.3 [basic.stc.auto] p1, 6.6 [stmt.jump] p2: a goto out of scopes destroys their objects, the last made first, forward and back
	{
		"BasicGotoDestroysWhatItLeaves", 7,
		"int giLog = 0;\n"
		"struct G { int i; G(int x) : i(x) { } ~G() { giLog = giLog * 10 + i; } };\n"
		"int Forward(int n)\n"
		"{\n"
		"	G a(1);\n"
		"	{\n"
		"		G b(2);\n"
		"		if (n) goto Out;\n"
		"		G c(3);\n"
		"	}\n"
		"Out:\n"
		"	return 0;\n"
		"}\n"
		"int Back()\n"
		"{\n"
		"	int tries = 0;\n"
		"Again:\n"
		"	{\n"
		"		G d(4);\n"
		"		tries++;\n"
		"		if (tries < 3) goto Again;\n"
		"	}\n"
		"	return tries;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Forward(1);\n"
		"	if (giLog == 21) r += 1;\n"
		"	giLog = 0;\n"
		"	if (Back() == 3 && giLog == 444) r += 2;\n"
		"	giLog = 0;\n"
		"	{\n"
		"		G e(5);\n"
		"		goto Done;\n"
		"	}\n"
		"Done:\n"
		"	if (giLog == 5) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p3: a goto may not jump past the declaration of an object with a destructor into its scope
	{
		"BasicGotoPastDeclarationRefused", CONFORMANCE_REFUSED,
		"struct G { ~G() { } };\n"
		"int main()\n"
		"{\n"
		"	goto A;\n"
		"	G g;\n"
		"A:\n"
		"	return 0;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p3: a goto may not jump into a block past the making of an object with a destructor
	{
		"BasicGotoIntoBlockRefused", CONFORMANCE_REFUSED,
		"struct G { ~G() { } };\n"
		"int main()\n"
		"{\n"
		"	int n = 0;\n"
		"	{\n"
		"		G a;\n"
		"	B:\n"
		"		n++;\n"
		"	}\n"
		"	if (n < 2) goto B;\n"
		"	return n;\n"
		"}\n"
	},
	//3.7.4 [basic.stc.dynamic] p1: an allocation function declared in a namespace other than the global one
	{
		"BasicAllocationInNamespaceRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace N { void* operator new(size_t n); }\n"
		"int main() { return 0; }\n"
	},
	//3.7.4 [basic.stc.dynamic] p1: an allocation function declared static at global scope
	{
		"BasicAllocationStaticRefused", CONFORMANCE_REFUSED,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[8];\n"
		"static void* operator new(size_t n) { return gacHeap; }\n"
		"int main() { return 0; }\n"
	},
	//3.7.4 [basic.stc.dynamic] p1: a deallocation function declared in a namespace other than the global one
	{
		"BasicDeallocationInNamespaceRefused", CONFORMANCE_REFUSED,
		"namespace N { void operator delete(void* p) noexcept; }\n"
		"int main() { return 0; }\n"
	},
	//3.7.4.1 [basic.stc.dynamic.allocation] p2: an allocation of no bytes that succeeds gives a pointer that isn't null and isn't one already given
	{
		"BasicNewZeroBytes", 3,
		"typedef decltype(sizeof(0)) size_t;\n"
		"void* operator new(size_t n);\n"
		"void operator delete(void* p) noexcept;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	void* a = ::operator new(0);\n"
		"	void* b = ::operator new(0);\n"
		"	if (a != 0 && b != 0 && a != b) r += 1;\n"
		"	::operator delete(a);\n"
		"	::operator delete(b);\n"
		"	int* c = new int[0];\n"
		"	int* d = new int[0];\n"
		"	if (c != 0 && d != 0 && c != d) r += 2;\n"
		"	delete[] c;\n"
		"	delete[] d;\n"
		"	return r;\n"
		"}\n"
	},
	//3.7.4.1 [basic.stc.dynamic.allocation] p2: what an allocation function gives is aligned for any object of fundamental alignment
	{
		"BasicAllocationAligned", 1,
		"struct Big { double d; long long l; int i; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Big* p = new Big; char* c = new char[3]; long long* q = new long long;\n"
		"	if ((unsigned long long)p % alignof(Big) == 0 && (unsigned long long)c % alignof(Big) == 0 && (unsigned long long)q % alignof(long long) == 0) r += 1;\n"
		"	delete p; delete[] c; delete q;\n"
		"	return r;\n"
		"}\n"
	},
	//3.7.4.2 [basic.stc.dynamic.deallocation] p2: a class's operator delete with one parameter is its usual one, not the one that also takes the size
	{
		"BasicClassDeleteOneParameterUsual", 1,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[16];\n"
		"int giOne = 0, giTwo = 0;\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	static void* operator new(size_t n) { return gacHeap; }\n"
		"	static void operator delete(void* p) { giOne++; }\n"
		"	static void operator delete(void* p, size_t n) { giTwo++; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	S* p = new S;\n"
		"	delete p;\n"
		"	int r = 0;\n"
		"	if (giOne == 1 && giTwo == 0) r += 1;\n"
		"	return r;\n"
		"}\n"
	},
	//3.8 [basic.life] p1, p4, p7: a destructor call ends an object's life; a new object made in its storage is reached through the old name
	{
		"BasicLifetimeReuse", 15,
		"void* operator new(decltype(sizeof(0)), void* p) noexcept { return p; }\n"
		"int giMade = 0, giGone = 0;\n"
		"struct C { int v; C(int a) : v(a) { giMade++; } ~C() { giGone++; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c(1);\n"
		"	c.~C();\n"
		"	if (giGone == 1) r += 1;\n"
		"	C* p = new (&c) C(5);\n"
		"	if (p == &c && c.v == 5 && giMade == 2) r += 2;\n"
		"	int i = 3;\n"
		"	int* q = new (&i) int(8);\n"
		"	if (i == 8 && q == &i) r += 4;\n"
		"	struct T { int a; } t; t.a = 1;\n"
		"	new (&t) T{9};\n"
		"	if (t.a == 9) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9 [basic.types] p2, p3: a trivially copyable object's bytes copied into another object of its type give it the same value
	{
		"BasicTriviallyCopyable", 7,
		"struct P { int a; char b; double d; };\n"
		"int main()\n"
		"{\n"
		"	P x = { 7, 'q', 2.5 }, y = { 0, 0, 0 };\n"
		"	unsigned char buf[sizeof(P)];\n"
		"	const unsigned char* s = (const unsigned char*)&x;\n"
		"	for (unsigned i = 0; i < sizeof(P); i++) buf[i] = s[i];\n"
		"	unsigned char* t = (unsigned char*)&y;\n"
		"	for (unsigned i = 0; i < sizeof(P); i++) t[i] = buf[i];\n"
		"	int r = 0;\n"
		"	if (y.a == 7 && y.b == 'q') r += 1;\n"
		"	if (y.d == 2.5) r += 2;\n"
		"	int n = 300, m = 0;\n"
		"	for (unsigned i = 0; i < sizeof(int); i++) ((unsigned char*)&m)[i] = ((unsigned char*)&n)[i];\n"
		"	if (m == 300) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9 [basic.types] p4: an object's representation is its sizeof unsigned chars
	{
		"BasicObjectRepresentation", 3,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int v = 1;\n"
		"	unsigned sum = 0;\n"
		"	for (unsigned i = 0; i < sizeof(v); i++) sum += ((unsigned char*)&v)[i];\n"
		"	if (sum == 1) r += 1;\n"
		"	unsigned u = ~0u;\n"
		"	unsigned all = 0;\n"
		"	for (unsigned i = 0; i < sizeof(u); i++) all += ((unsigned char*)&u)[i] == 255;\n"
		"	if (all == sizeof(unsigned)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9 [basic.types] p5, p6: a pointer to an incomplete class; a class and an array of unknown bound completed later
	{
		"BasicIncompleteTypes", 15,
		"struct X;\n"
		"extern X* xp;\n"
		"extern int arr[];\n"
		"struct X { int i; };\n"
		"X x = { 4 };\n"
		"X* xp = &x;\n"
		"int arr[10];\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (xp->i == 4) r += 1;\n"
		"	if (sizeof(arr) == 10 * sizeof(int)) r += 2;\n"
		"	arr[9] = 5;\n"
		"	if (arr[9] == 5) r += 4;\n"
		"	struct Y* yp = 0;\n"
		"	X* p = 0;\n"
		"	if (p == 0 && yp == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9 [basic.types] p5: an object of an incomplete class
	{
		"BasicIncompleteClassObjectRefused", CONFORMANCE_REFUSED,
		"struct X;\n"
		"X x;\n"
		"int main() { return 0; }\n"
	},
	//3.9 [basic.types] p6, 5.3.3 [expr.sizeof] p1: sizeof an array of unknown bound, before it is completed
	{
		"BasicIncompleteArraySizeofRefused", CONFORMANCE_REFUSED,
		"extern int arr[];\n"
		"int main() { return sizeof(arr); }\n"
		"int arr[3];\n"
	},
	//3.9 [basic.types] p5, 5.2.5 [expr.ref] p2: a member of a class used where the class is still incomplete
	{
		"BasicIncompleteClassMemberRefused", CONFORMANCE_REFUSED,
		"struct X;\n"
		"struct X* gp;\n"
		"int main() { return gp->i; }\n"
		"struct X { int i; };\n"
	},
	//3.9 [basic.types] p5, 9.2 [class.mem] p2: a class is complete after its definition, and in the member function bodies of a class it is defined in
	{
		"BasicClassCompleteWhereDefined", 7,
		"struct A\n"
		"{\n"
		"	struct B;\n"
		"	int F(B* b) { return b->x + G(); }\n"
		"	static int G() { return 1; }\n"
		"	struct B { int x; };\n"
		"};\n"
		"struct X;\n"
		"int Read(X* p);\n"
		"struct X { int i; };\n"
		"int Read(X* p) { return p->i; }\n"
		"int main() { A a; A::B b; b.x = 2; X x; x.i = 4; return a.F(&b) + Read(&x); }\n"
	},
	//3.9 [basic.types] p10: literal types: scalars, references, arrays of them, aggregates of them, and classes with a constexpr constructor and a trivial destructor
	{
		"BasicLiteralTypes", 7,
		"struct L { int a; constexpr L(int x) : a(x) { } constexpr int Get() const { return a * 2; } };\n"
		"struct Agg { int a; double d; };\n"
		"constexpr L gl(3);\n"
		"constexpr Agg ga = { 4, 1.5 };\n"
		"constexpr int garr[3] = { 1, 2, 3 };\n"
		"constexpr const int& gref = garr[1];\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	static_assert(gl.Get() == 6, \"\");\n"
		"	if (gl.Get() == 6) r += 1;\n"
		"	enum { E = ga.a + garr[2] };\n"
		"	if (E == 7) r += 2;\n"
		"	if (gref == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9 [basic.types] p10, 7.1.5 [dcl.constexpr] p9: a constexpr variable of a class with a destructor of its own isn't of a literal type
	{
		"BasicNonLiteralConstexprRefused", CONFORMANCE_REFUSED,
		"struct N { int a; constexpr N(int x) : a(x) { } ~N() { } };\n"
		"constexpr N n(1);\n"
		"int main() { return n.a; }\n"
	},
	//3.9.1 [basic.fundamental]: the three char types are one byte, alike aligned and distinct; the integer sizes in order; signed and unsigned alike in size; unsigned arithmetic modulo 2^n; bool; the character types distinct
	{
		"BasicFundamentalTypes", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(char) == 1 && sizeof(signed char) == 1 && sizeof(unsigned char) == 1 && alignof(char) == alignof(signed char) && alignof(char) == alignof(unsigned char)) r += 1;\n"
		"	if (sizeof(short) <= sizeof(int) && sizeof(int) <= sizeof(long) && sizeof(long) <= sizeof(long long) && sizeof(short) >= 2 && sizeof(long long) >= 8) r += 2;\n"
		"	unsigned u = 0; u = u - 1;\n"
		"	unsigned char uc = 255; uc = uc + 1;\n"
		"	unsigned long long ull = 0; ull--;\n"
		"	if (u == ~0u && uc == 0 && ull == ~0ull && u + 1 == 0) r += 4;\n"
		"	if (sizeof(unsigned short) == sizeof(short) && sizeof(unsigned) == sizeof(int) && sizeof(unsigned long) == sizeof(long) && sizeof(unsigned long long) == sizeof(long long)) r += 8;\n"
		"	bool b = true;\n"
		"	int i = b + b;\n"
		"	if (i == 2 && (bool)2 == true && !Same<char, signed char>::V && !Same<char, unsigned char>::V) r += 16;\n"
		"	if (sizeof(char16_t) >= 2 && sizeof(char32_t) >= 4 && !Same<wchar_t, int>::V && !Same<wchar_t, unsigned short>::V && !Same<char16_t, unsigned short>::V) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9.1 [basic.fundamental] p9: void expressions: a call of a void function, a cast to void, as an operand of , and ?: and in a return
	{
		"BasicVoid", 7,
		"int giCalls = 0;\n"
		"void Touch() { giCalls++; }\n"
		"void Forward() { return Touch(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Forward();\n"
		"	(void)giCalls;\n"
		"	if (giCalls == 1) r += 1;\n"
		"	int v = (Touch(), 5);\n"
		"	static_cast<void>(v);\n"
		"	if (v == 5 && giCalls == 2) r += 2;\n"
		"	bool b = true;\n"
		"	b ? Touch() : Forward();\n"
		"	if (giCalls == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9.1 [basic.fundamental] p9: no object has type void
	{
		"BasicVoidObjectRefused", CONFORMANCE_REFUSED,
		"int main() { void v; return 0; }\n"
	},
	//3.9.1 [basic.fundamental] p9, 5.3.3 [expr.sizeof] p1: void is incomplete, so has no size
	{
		"BasicSizeofVoidRefused", CONFORMANCE_REFUSED,
		"int main() { return sizeof(void); }\n"
	},
	//3.9.2 [basic.compound] p3, p4: a void* holds any object pointer and gives it back; void* is a char*'s size and alignment; a pointer to a cv-qualified type is a pointer's size
	{
		"BasicCompoundTypes", 15,
		"struct S { int a; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s; s.a = 6; int i = 4; double d = 1.5;\n"
		"	void* p = &s; void* q = &i; const void* c = &d;\n"
		"	if (static_cast<S*>(p)->a == 6 && *static_cast<int*>(q) == 4 && *static_cast<const double*>(c) == 1.5) r += 1;\n"
		"	if (sizeof(void*) == sizeof(char*) && alignof(void*) == alignof(char*)) r += 2;\n"
		"	const int* ci = &i; volatile int* vi = &i; const volatile int* cvi = ci;\n"
		"	if (sizeof(ci) == sizeof(int*) && ci == &i && cvi == vi) r += 4;\n"
		"	S* sp = &s; int* ap = &s.a;\n"
		"	if ((void*)sp == (void*)ap) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9.3 [basic.type.qualifier]: cv on an array type is its elements'; a const object's members are const but for mutable ones; adding cv at every level
	{
		"BasicCvQualifiers", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef int A[2];\n"
		"struct M { int v; mutable int m; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	const A a = { 1, 2 };\n"
		"	if (Same<decltype(a[0]), const int&>::V && Same<const A, const int[2]>::V && a[1] == 2) r += 1;\n"
		"	const M cm = { 3, 4 };\n"
		"	cm.m = 5;\n"
		"	if (Same<decltype((cm.v)), const int&>::V && Same<decltype((cm.m)), int&>::V && cm.m == 5) r += 2;\n"
		"	int i = 7;\n"
		"	int* p = &i;\n"
		"	const volatile int* cv = p;\n"
		"	const int* const* cpp = &p;\n"
		"	if (*cv == 7 && **cpp == 7) r += 4;\n"
		"	if (Same<const volatile int, volatile const int>::V && !Same<const int, int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.9.3 [basic.type.qualifier] p2: cv on an array type applies to its elements
	{
		"BasicConstArrayElementRefused", CONFORMANCE_REFUSED,
		"typedef int A[2];\n"
		"int main() { const A a = { 1, 2 }; a[0] = 3; return a[0]; }\n"
	},
	//3.10 [basic.lval] p1, p5: lvalues, xvalues and prvalues; a class prvalue's member function may change it
	{
		"BasicValueCategories", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gi = 1;\n"
		"int& L() { return gi; }\n"
		"int&& X() { return static_cast<int&&>(gi); }\n"
		"int P() { return gi; }\n"
		"struct S { int m; };\n"
		"S MakeS() { S s; s.m = 2; return s; }\n"
		"struct T { int v; T& Set(int a) { v = a; return *this; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype((L())), int&>::V && Same<decltype((X())), int&&>::V && Same<decltype((P())), int>::V) r += 1;\n"
		"	L() = 5;\n"
		"	if (gi == 5) r += 2;\n"
		"	if (Same<decltype((gi)), int&>::V && Same<decltype((\"ab\")), const char (&)[3]>::V) r += 4;\n"
		"	if (Same<decltype((MakeS().m)), int&&>::V) r += 8;\n"
		"	int a = 3;\n"
		"	if (Same<decltype((static_cast<int&&>(a))), int&&>::V && Same<decltype((a + 1)), int>::V && Same<decltype((++a)), int&>::V && Same<decltype((a++)), int>::V) r += 16;\n"
		"	MakeS() = MakeS();\n"
		"	if (T().Set(4).v == 4) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//3.10 [basic.lval] p1, 5.17 [expr.ass] p1: a prvalue of a scalar type isn't assigned to
	{
		"BasicAssignToPrvalueRefused", CONFORMANCE_REFUSED,
		"int P() { return 1; }\n"
		"int main() { P() = 2; return 0; }\n"
	},
	//3.10 [basic.lval] p1, 5.3.1 [expr.unary.op] p3: the address of a prvalue
	{
		"BasicAddressOfPrvalueRefused", CONFORMANCE_REFUSED,
		"int P() { return 1; }\n"
		"int main() { int* p = &P(); return *p; }\n"
	},
	//3.10 [basic.lval] p10: an object may be reached through its signed or unsigned counterpart, and through unsigned char
	{
		"BasicAliasing", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int x = -1;\n"
		"	unsigned* pu = (unsigned*)&x;\n"
		"	if (*pu == ~0u) r += 1;\n"
		"	*pu = 5;\n"
		"	if (x == 5) r += 2;\n"
		"	long long big = 0x0102;\n"
		"	unsigned char* c = (unsigned char*)&big;\n"
		"	unsigned sum = 0;\n"
		"	for (unsigned i = 0; i < sizeof(big); i++) sum += c[i];\n"
		"	if (sum == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//3.11 [basic.align] p1, p2, p5: alignof of arrays and classes; sizes and offsets that follow from it; an alignment given by alignas, of a class and of an array
	{
		"BasicAlignment", 15,
		"struct S { char c; int i; };\n"
		"struct alignas(8) A8 { char c; };\n"
		"struct Holder { char c; A8 a; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (alignof(char) == 1 && alignof(int[5]) == alignof(int) && alignof(S) == alignof(int)) r += 1;\n"
		"	S s[2];\n"
		"	if (sizeof(S) % alignof(S) == 0 && (unsigned)((char*)&s[1] - (char*)&s[0]) == sizeof(S)) r += 2;\n"
		"	if (alignof(A8) == 8 && sizeof(A8) == 8 && sizeof(Holder) == 16) r += 4;\n"
		"	alignas(8) char buf[3];\n"
		"	alignas(int) char ibuf[sizeof(int)];\n"
		"	if ((unsigned long long)buf % 8 == 0 && (unsigned long long)ibuf % alignof(int) == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//3.11 [basic.align] p1, 7.6.2 [dcl.align] p1: an object is placed as alignas asks, of the object or of its class or of a member, as a local, a static local and a global
	{
		"BasicAlignasObjects", 127,
		"struct alignas(8) A8 { char c; };\n"
		"struct Holder { char c; A8 a; };\n"
		"alignas(16) char gbuf[3];\n"
		"A8 ga;\n"
		"char gc;\n"
		"Holder gh;\n"
		"int F(int n)\n"
		"{\n"
		"	char c = (char)n;\n"
		"	alignas(8) char buf[3];\n"
		"	alignas(4) char b4[3];\n"
		"	alignas(int) char ibuf[sizeof(int)];\n"
		"	int r = 0;\n"
		"	buf[0] = c; b4[0] = c; ibuf[0] = c;\n"
		"	if ((unsigned long long)buf % 8 == 0) r += 1;\n"
		"	if ((unsigned long long)b4 % 4 == 0) r += 2;\n"
		"	if ((unsigned long long)ibuf % alignof(int) == 0) r += 4;\n"
		"	return r + (buf[0] + b4[0] + ibuf[0]) * 0;\n"
		"}\n"
		"int G(int n)\n"
		"{\n"
		"	char c;\n"
		"	A8 a;\n"
		"	Holder h;\n"
		"	static A8 s;\n"
		"	a.c = 1; c = (char)n; h.c = 2;\n"
		"	int r = 0;\n"
		"	if ((unsigned long long)&a % 8 == 0) r += 8;\n"
		"	if ((unsigned long long)&h.a % 8 == 0) r += 16;\n"
		"	if ((unsigned long long)&s % 8 == 0) r += 32;\n"
		"	return r + c * 0;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = F(1) + G(1) + F(2) * 0;\n"
		"	if ((unsigned long long)gbuf % 16 == 0 && (unsigned long long)&ga % 8 == 0 && (unsigned long long)&gh.a % 8 == 0) r += 64;\n"
		"	return r + gc;\n"
		"}\n"
	},
	//3.11 [basic.align] p1: an object's address is a multiple of its type's alignment wherever its function's frame is: locals of 2, 4 and 8 bytes and a class, at several depths of call and after an odd number of bytes of locals
	{
		"BasicAlignmentLocals", 31,
		"struct S { char c; long l; };\n"
		"int Aligned(int n)\n"
		"{\n"
		"	char c = (char)n;\n"
		"	int i = n;\n"
		"	long l = n;\n"
		"	double d = n;\n"
		"	long long ll = n;\n"
		"	S s;\n"
		"	s.l = n;\n"
		"	int r = 0;\n"
		"	if ((unsigned long long)&i % alignof(int) == 0) r += 1;\n"
		"	if ((unsigned long long)&l % alignof(long) == 0) r += 2;\n"
		"	if ((unsigned long long)&d % alignof(double) == 0) r += 4;\n"
		"	if ((unsigned long long)&ll % alignof(long long) == 0) r += 8;\n"
		"	if ((unsigned long long)&s % alignof(S) == 0 && (unsigned long long)&s.l % alignof(long) == 0) r += 16;\n"
		"	return (n > 0) ? (r & Aligned(n - 1)) : r + c * 0;\n"
		"}\n"
		"int Odd(char a, int n) { char b[3]; b[0] = a; return Aligned(n) + b[0] - a; }\n"
		"int main()\n"
		"{\n"
		"	char pad = 1;\n"
		"	return Aligned(3) & Odd(pad, 2);\n"
		"}\n"
	},
	//3.11 [basic.align] p4: an alignment is a power of two
	{
		"BasicAlignmentNotPowerOfTwoRefused", CONFORMANCE_REFUSED,
		"struct alignas(3) S { char c; };\n"
		"int main() { return sizeof(S); }\n"
	},
	//3.11 [basic.align] p4, 7.6.2 [dcl.align] p2: a local's alignment is a power of two
	{
		"BasicAlignmentLocalNotPowerOfTwoRefused", CONFORMANCE_REFUSED,
		"int main() { alignas(3) char c = 1; return c; }\n"
	},
};


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
	//5.1.1 [expr.prim.general] p8, 5.4 [expr.cast]: (decltype(f())::k) is k's value in parentheses where k is no type, and a cast where it is one
	{
		"PrimDecltypeQualifierOfCall", 15,
		"struct S { static const int k = 4; typedef int T; };\n"
		"S f();\n"
		"S f(int);\n"
		"S f() { return S(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	char c = 2;\n"
		"	if ((decltype(f())::k) + 1 == 5) r += 1;\n"
		"	if ((decltype(f())::T)c + 1 == 3) r += 2;\n"
		"	if ((decltype(f(1))::k) - 1 == 3) r += 4;\n"
		"	if ((decltype(f())::k) * 2 == 8) r += 8;\n"
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
	//5.2.1 [expr.sub], 13.5.5 [over.sub], 5.17 [expr.ass]: object[index] is its operator[]'s result, an object too, which is assigned to and called on
	{
		"SubscriptOperatorReturnsObject", 15,
		"struct Bits\n"
		"{\n"
		"	unsigned b;\n"
		"	struct Ref\n"
		"	{\n"
		"		Bits& o;\n"
		"		int i;\n"
		"		Ref& operator=(bool v) { if (v) o.b |= 1u << i; else o.b &= ~(1u << i); return *this; }\n"
		"		operator bool() const { return (o.b >> i) & 1; }\n"
		"		void Flip() { o.b ^= 1u << i; }\n"
		"	};\n"
		"	Ref operator[](int i) { return Ref{ *this, i }; }\n"
		"};\n"
		"struct Cell { int* p; Cell& operator+=(int v) { *p += v; return *this; } Cell& operator++() { ++*p; return *this; } int Get() const { return *p; } };\n"
		"struct Row { int a[3]; Cell operator[](int i) { Cell c = { &a[i] }; return c; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Bits x{ 0 };\n"
		"	x[2] = true;\n"
		"	x[0] = true;\n"
		"	if (x.b == 5 && x[2] && !x[1]) r += 1;\n"
		"	x[2] = false;\n"
		"	x[1].Flip();\n"
		"	if (x.b == 3) r += 2;\n"
		"	Row row = { { 1, 2, 3 } };\n"
		"	row[1] += 5;\n"
		"	++row[2];\n"
		"	if (row.a[1] == 7 && row.a[2] == 4 && row[1].Get() == 7) r += 4;\n"
		"	(x[3] = true) = false;\n"
		"	if (x.b == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const], 2.14.8 [lex.ext] p2: a user-defined literal is its literal operator's call, a constant expression when the operator is constexpr
	{
		"ConstantLiteralOperator", 15,
		"constexpr int operator\"\" _twice(unsigned long long v) { return (int)v * 2; }\n"
		"constexpr int operator\"\" _code(char c) { return c - 'a'; }\n"
		"constexpr unsigned operator\"\" _len(const char* s, decltype(sizeof(0)) n) { return (unsigned)n; }\n"
		"int operator\"\" _run(unsigned long long v) { return (int)v + 1; }\n"
		"int gaiRow[3_twice];\n"
		"enum { E = 4_twice + 'c'_code };\n"
		"template<int N> struct Count { enum { V = N }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(gaiRow) == 6 * sizeof(int)) r += 1;\n"
		"	if (E == 10) r += 2;\n"
		"	if (Count<\"four\"_len>::V == 4) r += 4;\n"
		"	switch (5)\n"
		"	{\n"
		"	case 2_twice + 1:\n"
		"		r += 8;\n"
		"		break;\n"
		"	}\n"
		"	return r + (9_run == 10 ? 0 : 16);\n"
		"}\n"
	},
	//5.19 [expr.const] p2: a call of a function that isn't constexpr is no constant expression, a literal operator's too
	{
		"ConstantLiteralOperatorNotConstexprRefused", CONFORMANCE_REFUSED,
		"int operator\"\" _run(unsigned long long v) { return (int)v + 1; }\n"
		"int gaiRow[3_run];\n"
		"int main() { return sizeof(gaiRow) / sizeof(int); }\n"
	},
	//5.1.2 [expr.prim.lambda] p5, 8.3.6 [dcl.fct.default]: a lambda's parameters have default arguments, as its function call operator's
	{
		"LambdaDefaultArguments", 15,
		"int gCalls = 0;\n"
		"int Next() { return ++gCalls; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int k = 2;\n"
		"	auto add = [](int a, int b = 4) { return a + b; };\n"
		"	auto all = [k](int a = 5, int b = 6) { return a + b + k; };\n"
		"	auto each = [](int n = Next()) { return n; };\n"
		"	auto generic = [](auto a, int b = 3) { return (int)sizeof(a) * 10 + b; };\n"
		"	if ([](int w = 3) { return w; }() == 3) r += 1;\n"
		"	if (add(1) == 5 && add(1, 2) == 3) r += 2;\n"
		"	if (all() == 13 && all(1) == 9 && all(1, 1) == 4) r += 4;\n"
		"	if (each() == 1 && each() == 2 && each(7) == 7 && gCalls == 2) r += 8;\n"
		"	return r + (generic('c') == 13 && generic('c', 1) == 11 ? 0 : 16);\n"
		"}\n"
	},
	//5.1.1 [expr.prim.general] p8, 7.1.6.2 [dcl.type.simple] p4, 5.3.3 [expr.sizeof]: decltype(e)::name names a member of e's class, a type or not, or an enumerator of e's enumeration
	{
		"DecltypeQualifiesMemberOrEnumerator", 63,
		"struct D { static char s; long m; typedef short T; enum { K = 3 }; };\n"
		"char D::s = 'x';\n"
		"enum class E { a = 1, b = 2 };\n"
		"enum U { u0, u1 = 5 };\n"
		"D d;\n"
		"E e;\n"
		"U u;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int a[(int)decltype(e)::b + decltype(d)::K];\n"
		"	if (sizeof(decltype(d)::s) == 1) r += 1;\n"
		"	if (sizeof(decltype(d)::m) == sizeof(long)) r += 2;\n"
		"	if (sizeof(decltype(d)::T) == sizeof(short)) r += 4;\n"
		"	if ((int)decltype(e)::b == 2 && decltype(e)::a != decltype(e)::b) r += 8;\n"
		"	if (decltype(u)::u1 == 5) r += 16;\n"
		"	if (sizeof(a) == 5 * sizeof(int) && decltype(d)::s == 'x') r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2, 5.2.9 [expr.static.cast]: a static_cast of a constant to an integer type is a constant expression
	{
		"ConstantNamedCast", 15,
		"int gaiRow[static_cast<int>(3 > 2) + 1];\n"
		"enum { E = static_cast<int>(7.9), F = static_cast<unsigned char>(300) };\n"
		"template<int N> struct Count { enum { V = N }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(gaiRow) == 2 * sizeof(int)) r += 1;\n"
		"	if (E == 7 && F == 44) r += 2;\n"
		"	if (Count<static_cast<int>(sizeof(char)) + static_cast<char>(4)>::V == 5) r += 4;\n"
		"	switch (2)\n"
		"	{\n"
		"	case static_cast<int>(2L):\n"
		"		r += 8;\n"
		"		break;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//5.19 [expr.const] p2, 14.5.1.3 [temp.static]: a class template's static const member defined after the class with a constant is a constant in each instance
	{
		"ConstantClassTemplateStaticMember", 15,
		"template<class T> struct W { static const int k; static const int big; enum { Twice = 2 * sizeof(T) }; };\n"
		"template<class T> const int W<T>::k = 3;\n"
		"template<class T> const int W<T>::big = sizeof(T) + 4;\n"
		"template<> const int W<char>::k = 7;\n"
		"int gaiRow[W<int>::k];\n"
		"int gaiBig[W<long>::big];\n"
		"int gaiChar[W<char>::k];\n"
		"template<int N> struct Count { enum { V = N }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(gaiRow) == 3 * sizeof(int)) r += 1;\n"
		"	if (sizeof(gaiBig) == (sizeof(long) + 4) * sizeof(int)) r += 2;\n"
		"	if (sizeof(gaiChar) == 7 * sizeof(int) && W<char>::k == 7 && W<long>::k == 3) r += 4;\n"
		"	if (Count<W<short>::big>::V == sizeof(short) + 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.9 [expr.static.cast] p4, 4.12 [conv.bool]: nullptr becomes a pointer or, initialised directly, a bool; not a number
	{
		"StaticCastNullPointerToNumberRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	return static_cast<int>(nullptr);\n"
		"}\n"
	},
	//5.2.9 [expr.static.cast] p4, 4.10 [conv.ptr], 4.12 [conv.bool]: static_cast of nullptr to a pointer, a pointer to a member, bool and std::nullptr_t
	{
		"StaticCastNullPointer", 7,
		"struct S { int m; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int* p = static_cast<int*>(nullptr);\n"
		"	int S::* q = static_cast<int S::*>(nullptr);\n"
		"	decltype(nullptr) n = static_cast<decltype(nullptr)>(nullptr);\n"
		"	if (p == 0 && q == 0) r += 1;\n"
		"	if (!static_cast<bool>(nullptr)) r += 2;\n"
		"	if (n == nullptr) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p10, 5.3.4 [expr.new] p11: a class's operator delete[] that takes a size is given its array's, for elements with no destructor too
	{
		"DeleteArraySizedNoDestructor", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"char gacHeap[128];\n"
		"size_t guiNew = 0;\n"
		"size_t guiDeleted = 0;\n"
		"void* gpDeleted = 0;\n"
		"int giGlobal = 0;\n"
		"void* operator new[](size_t n) { return gacHeap + 64; }\n"
		"void operator delete[](void* p) noexcept { giGlobal++; }\n"
		"struct P\n"
		"{\n"
		"	long a;\n"
		"	long b;\n"
		"	static void* operator new[](size_t n) { guiNew = n; return gacHeap; }\n"
		"	static void operator delete[](void* p, size_t n) { guiDeleted = n; gpDeleted = p; }\n"
		"};\n"
		"struct Q : P { long c; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	P* p = new P[3];\n"
		"	p[2].b = 7;\n"
		"	if (guiNew >= 3 * sizeof(P) && p[2].b == 7) r += 1;\n"
		"	delete[] p;\n"
		"	if (guiDeleted == guiNew && gpDeleted == gacHeap) r += 2;\n"
		"	Q* q = new Q[2];\n"
		"	delete[] q;\n"
		"	if (guiDeleted == guiNew && guiNew >= 2 * sizeof(Q)) r += 4;\n"
		"	P* g = ::new P[2];\n"
		"	::delete[] g;\n"
		"	if (giGlobal == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.5 [expr.ref] p1, 14.2 [temp.names] p3: object.name<type> and pointer->name<type> name a member variable template's instance (a type is compared with nothing, so the < begins template arguments)
	{
		"ExprMemberVariableTemplate", 7,
		"struct S\n"
		"{\n"
		"	template<class T> static const int size = sizeof(T);\n"
		"	template<class T> static T value;\n"
		"	int k;\n"
		"};\n"
		"template<class T> T S::value = T(5);\n"
		"template<class C> struct W { template<class T> static const int twice = sizeof(T) * 2; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s; S* p = &s;\n"
		"	if (s.size<char> == 1 && p->size<long> == sizeof(long)) r += 1;\n"
		"	s.value<int> = 7;\n"
		"	if (S::value<int> == 7 && p->value<int> == 7 && s.value<char> == 5) r += 2;\n"
		"	W<int> w;\n"
		"	if (w.twice<char> == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p6, 14.8.2.3 [temp.deduct.conv]: a generic lambda whose last parameter is a pack (auto... args) converts to a pointer to a function, the pack taking each of the function's parameters from its place on (none too)
	{
		"ExprLambdaGenericPackToPointer", 63,
		"int Sum() { return 0; }\n"
		"template<class T, class... R> int Sum(T t, R... r) { return (int)t + Sum(r...); }\n"
		"int Call2(int (*f)(int, int)) { return f(3, 4); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int (*p)(int, int) = [](auto... a) { return Sum(a...); };\n"
		"	if (p(1, 2) == 3) r += 1;\n"
		"	int (*q)(char, long, int) = [](auto... a) { return (int)sizeof...(a) * 100 + Sum(a...); };\n"
		"	if (q(1, 2, 3) == 306) r += 2;\n"
		"	int (*z)() = [](auto... a) { return (int)sizeof...(a) + 5; };\n"
		"	if (z() == 5) r += 4;\n"
		"	if (Call2([](auto... a) { return Sum(a...) * 2; }) == 14) r += 8;\n"
		"	long (*m)(int, long) = [](int first, auto... rest) { return (long)first + Sum(rest...); };\n"
		"	if (m(10, 20L) == 30) r += 16;\n"
		"	auto g = [](auto... a) { return Sum(a...); };\n"
		"	int (*h)(int, char) = g;\n"
		"	if (h(5, 6) == 11 && g(1, 2, 3) == 6) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//5.1.2 [expr.prim.lambda] p24, 14.5.3 [temp.variadic] p5: a generic lambda with a parameter pack of its own captures the pack of the function it is in and expands both, apart and together; a lambda in it captures the two
	{
		"ExprLambdaCapturesPackBesideOwn", 127,
		"int Sum() { return 0; }\n"
		"template<class T, class... R> int Sum(T t, R... r) { return (int)t + Sum(r...); }\n"
		"template<class... A> int Both(A... a) { auto f = [a...](auto... b) { return Sum(a...) * 100 + Sum(b...); }; return f(1, 2); }\n"
		"template<class... A> int Two(int k, A... a) { auto f = [a..., k]() { return Sum(a...) + k; }; return f(); }\n"
		"template<class... A> int Count(A... a) { auto f = [=](auto... b) { return (int)sizeof...(a) * 10 + (int)sizeof...(b) + Sum(a...) * 100 + Sum(b...) * 1000; }; return f(1, 1); }\n"
		"template<class... A> int Nest(A... a) { auto f = [a...](auto... b) { auto g = [a..., b...]() { return Sum(a...) * 10 + Sum(b...); }; return g(); }; return f(1, 2); }\n"
		"template<class... A> int Refs(A... a) { auto f = [&a...](auto&... b) { int n = Sum(a...) + Sum(b...); return n; }; int x = 5; return f(x); }\n"
		"template<class... A> int Zip(A... a) { auto f = [a...](auto... b) { return Sum((a * b)...); }; return f(10, 100); }\n"
		"template<class... A> int None(A... a) { auto f = [a...](auto... b) { return (int)sizeof...(a) + (int)sizeof...(b) * 10; }; return f() + f(1) * 100; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Both(3, 4) == 703) r += 1;\n"
		"	if (Two(5, 1, 2) == 8) r += 2;\n"
		"	if (Count(2, 3) == 2522) r += 4;\n"
		"	if (Nest(3, 4) == 73) r += 8;\n"
		"	if (Refs(1, 2) == 8) r += 16;\n"
		"	if (Zip(2, 3) == 320) r += 32;\n"
		"	if (None() == 1000 && None(7, 8) == 2 + 1200) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//5.4 [expr.cast] p2, 7.1.6.2 [dcl.type.simple] p4, 5.1.1 [expr.prim.general] p8: (decltype(e)::name) is a cast where the name is a type and the member's value in parentheses where it isn't, before + - * & and at the end of an expression
	{
		"ExprParenthesisedDecltypeMember", 31,
		"struct D { static const int k = 5; enum { e = 7 }; static int F() { return 3; } typedef int Type; int m; };\n"
		"const int D::k;\n"
		"int Use(D p) { return (decltype(p)::k) + 1; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d; int x = 2; int a[8]; a[5] = 4;\n"
		"	if ((decltype(d)::k) + 1 == 6 && (decltype(d)::e) + 1 == 8 && (decltype(d)::F()) + 1 == 4) r += 1;\n"
		"	if ((decltype(d)::k) - 1 == 4 && (decltype(d)::k) * x == 10 && -(decltype(d)::k) + 10 == 5) r += 2;\n"
		"	if ((decltype(d)::Type) +1.9 == 1 && (int)(decltype(d)::Type)1.5 + (decltype(d)::k) == 6) r += 4;\n"
		"	if (a[(decltype(d)::k)] == 4 && sizeof((decltype(d)::Type)0) == sizeof(int)) r += 8;\n"
		"	if (Use(d) == 6 && ((decltype(d)::k) & 4) == 4) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//5.3.5 [expr.delete] p10, 3.7.4.2 [basic.stc.dynamic.deallocation] p2, 12.4 [class.dtor] p13: the global operator delete that takes a size is given the size of the object's own class when an object with a virtual destructor is deleted through a base's pointer; a class's own operator delete is still the one called for it
	{
		"DeleteSizedGlobalPolymorphic", 15,
		"typedef decltype(sizeof(0)) size_t;\n"
		"static char gacHeap[256];\n"
		"static size_t guiUsed = 0;\n"
		"static size_t guiFreed = 0;\n"
		"static int giSized = 0;\n"
		"static int giPlain = 0;\n"
		"void* operator new(size_t n) { void* p = gacHeap + guiUsed; guiUsed += (n + 7) & ~(size_t)7; return p; }\n"
		"void operator delete(void* p) noexcept { giPlain++; }\n"
		"void operator delete(void* p, size_t n) noexcept { giSized++; guiFreed += n; }\n"
		"struct B { int b; virtual ~B() { } };\n"
		"struct D : B { long d[3]; };\n"
		"struct P { int p; };\n"
		"struct O { int o; virtual ~O() { } static int siOwn; void operator delete(void* p) { siOwn++; } };\n"
		"int O::siOwn = 0;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B* pb = new D;\n"
		"	delete pb;\n"
		"	if (giSized == 1 && giPlain == 0 && guiFreed == sizeof(D)) r += 1;\n"
		"	B* pb2 = new B;\n"
		"	delete pb2;\n"
		"	if (giSized == 2 && guiFreed == sizeof(D) + sizeof(B)) r += 2;\n"
		"	P* pp = new P;\n"
		"	delete pp;\n"
		"	if (giSized == 3 && giPlain == 0 && guiFreed == sizeof(D) + sizeof(B) + sizeof(P)) r += 4;\n"
		"	O* po = new O;\n"
		"	delete po;\n"
		"	if (O::siOwn == 1 && giSized == 3 && giPlain == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.1 [expr.sub] p1, 5.2.2 [expr.call] p11, 3.10 [basic.lval] p1: a[i] of an operator[] that returns a class by value is a prvalue, the object the call makes: it binds R&&, and f(R&&) is chosen over f(R&)
	{
		"ExprSubscriptByValueIsPrvalue", 2,
		"struct R { int v; };\n"
		"struct B { R operator[](int i) { R r = { i }; return r; } };\n"
		"int f(R&) { return 1; }\n"
		"int f(R&&) { return 2; }\n"
		"int main() { B b; return f(b[3]); }\n"
	},
	//5.16 [expr.cond] p4, 8.5 [dcl.init] p17: c ? a : b of two lvalues of a class is an lvalue, which initialises a variable of the class by its copy constructor, once
	{
		"ExprConditionalOfLvaluesCopiesOnce", 0,
		"int copies = 0;\n"
		"struct S { int v; S(int x) : v(x) {} S(const S& o) : v(o.v) { copies++; } };\n"
		"int main() { S a(1); S b(2); bool c = a.v == 1; S s = c ? a : b; a.v = 7; return (copies == 1 && s.v == 1) ? 0 : 10 + copies; }\n"
	},
	//5.16 [expr.cond] p4: c ? a : b of two lvalues of a class is an lvalue: it can be assigned to, and binds S&, so f(S&) is chosen over f(S&&)
	{
		"ExprConditionalOfLvaluesIsLvalue", 15,
		"struct S { int v; };\n"
		"int f(S&) { return 1; }\n"
		"int f(S&&) { return 2; }\n"
		"int main() { S a = { 1 }; S b = { 2 }; bool c = a.v == 1; (c ? a : b).v = 5; return f(c ? a : b) * 10 + a.v; }\n"
	},
};


//Clause 6, statements [stmt.stmt].
static const SConformanceCase gasConformanceClause6[] =
{
	//6.1 [stmt.label] p1: labels have their own names: a label and a variable of one name
	{
		"StmtLabelOwnNames", 3,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int L = 1;\n"
		"	goto L;\n"
		"	r += 8;\n"
		"L:\n"
		"	if (L == 1) r += 1;\n"
		"	{ int x = 2; if (x == 2) r += 2; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.1 [stmt.label] p2: a case label only in a switch
	{
		"StmtCaseOutsideSwitchRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	case 1: return 0;\n"
		"}\n"
	},
	//6.1 [stmt.label] p2: a default label only in a switch
	{
		"StmtDefaultOutsideSwitchRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	default: return 0;\n"
		"}\n"
	},
	//6.1 [stmt.label], 6.4.2 [stmt.switch] p2, 6.6.3 [stmt.return] p2, 6.7 [stmt.dcl] p3: what those refusals leave allowed
	{
		"StmtJumpRulesKept", 63,
		"struct P { P(int x) : v(x) { } int v; };\n"
		"struct Q { explicit Q(int x) : v(x) { } int v; };\n"
		"P MakeP() { return 2; }\n"
		"P MakePList() { return { 4 }; }\n"
		"Q MakeQ() { return Q(8); }\n"
		"void Nothing() { return Nothing(); }\n"
		"int Pick(long x)\n"
		"{\n"
		"	switch (x)\n"
		"	{\n"
		"	case 1: { int y = 16; return y; }\n"
		"	case -1: return 32;\n"
		"	case 0xFFFFL: return 0;\n"
		"	default: break;\n"
		"	}\n"
		"	return 0;\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{ goto L; r = 100; L: ; }\n"
		"	r += 1 + MakeP().v + MakePList().v + MakeQ().v + Pick(1) + Pick(-1);\n"
		"	return r;\n"
		"}\n"
	},
	//6.1 [stmt.label] p1: a label is followed by a statement
	{
		"StmtLabelAtBlockEndRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 0;\n"
		"	{ goto L; x = 1; L: }\n"
		"	return x;\n"
		"}\n"
	},
	//6.2 [stmt.expr]: an expression statement's value is thrown away; a null statement does nothing
	{
		"StmtExpressionStatement", 7,
		"int g = 0;\n"
		"int Inc() { return ++g; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Inc();\n"
		"	;\n"
		"	(void)Inc();\n"
		"	if (g == 2) r += 1;\n"
		"	for (;;) { break; }\n"
		"	if (true) ; else r += 8;\n"
		"	r += 2;\n"
		"	int a = 5;\n"
		"	if (a == 5) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.3 [stmt.block]: a compound statement is a block whose names end with it
	{
		"StmtCompoundScope", 7,
		"int x = 1;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		int x = 2;\n"
		"		if (x == 2) r += 1;\n"
		"		{ int x = 3; if (x == 3) r += 2; }\n"
		"		if (x == 2) r += 4;\n"
		"	}\n"
		"	return r + ((x == 1) ? 0 : 64);\n"
		"}\n"
	},
	//6.4 [stmt.select] p1, 6.5 [stmt.iter] p2: a substatement is a block even without braces
	{
		"StmtSubstatementIsBlock", 3,
		"int x = 7;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (x) int x = 2;\n"
		"	if (x == 7) r += 1;\n"
		"	for (int i = 0; i < 1; i++) int x = 3;\n"
		"	while (x == 7) { r += 2; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.4 [stmt.select] p1: what a substatement declares ends with it
	{
		"StmtSubstatementNameEndsRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	if (1) int y = 2;\n"
		"	return y;\n"
		"}\n"
	},
	//6.4 [stmt.select] p3: an if's condition variable is in scope in both its branches
	{
		"StmtIfConditionScope", 7,
		"int F(int a) { if (int x = a) return x; else return x + 10; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(3) == 3) r += 1;\n"
		"	if (F(0) == 10) r += 2;\n"
		"	if (int* p = 0) r += 8; else if (!p) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.4 [stmt.select] p4: a condition's object is converted to bool, by an explicit conversion function too
	{
		"StmtConditionContextual", 3,
		"struct B { int v; explicit operator bool() const { return v != 0; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (B b = B{1}) r += 1;\n"
		"	B z{0};\n"
		"	if (!z) r += 2;\n"
		"	while (B w = B{0}) { r += 8; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.4 [stmt.select] p2: a condition's declarator declares neither an array nor a function
	{
		"StmtConditionArrayRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	if (int a[2] = { 1, 2 }) return 1;\n"
		"	return 0;\n"
		"}\n"
	},
	//6.4 [stmt.select] p3, 6.5.1 [stmt.while] p2: a while's condition variable ends with the while
	{
		"StmtWhileConditionNameEndsRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	while (int x = 0) { }\n"
		"	return x;\n"
		"}\n"
	},
	//6.5.1 [stmt.while] p2: a while's condition variable is made and destroyed each time round
	{
		"StmtWhileConditionRemade", 3,
		"int giMade = 0;\n"
		"int giGone = 0;\n"
		"struct C { int v; C(int x) : v(x) { giMade++; } ~C() { giGone++; } explicit operator bool() const { return v > 0; } };\n"
		"int n = 3;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	while (C c{n--}) { if (giMade - giGone != 1) r += 8; }\n"
		"	if (giMade == 4) r += 1;\n"
		"	if (giGone == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: the condition is promoted and the case values converted to its type
	{
		"StmtSwitchPromotion", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	char c = 'b';\n"
		"	switch (c) { case 'a': break; case 98: r += 1; break; }\n"
		"	unsigned char u = 200;\n"
		"	switch (u) { case 200: r += 2; break; default: break; }\n"
		"	bool b = true;\n"
		"	switch (b) { case 1: r += 4; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: two case values in one switch are different
	{
		"StmtSwitchDuplicateCaseRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	switch (x) { case 1: break; case 1: break; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: different after conversion to the condition's type
	{
		"StmtSwitchDuplicateConvertedRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	long x = 1;\n"
		"	switch (x) { case 1: break; case 1u: break; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p3: one default label in a switch
	{
		"StmtSwitchTwoDefaultsRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	switch (x) { default: break; default: break; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: a case's value is a constant expression
	{
		"StmtSwitchNonConstantCaseRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	int y = 1;\n"
		"	switch (x) { case y: break; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2, 6.4 [stmt.select] p4: a switch's condition declares an object whose one conversion function gives an integer
	{
		"StmtSwitchDeclaredObjectCondition", 107,
		"struct C { int v; C(int x) : v(x) {} operator int() const { return v; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	switch (C c = 2) { case 1: r = 10; break; case 2: r = c.v + 5; break; default: r = 99; }\n"
		"	switch (C d{3}) { case 3: r += 100; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p5, p6: case labels anywhere in the switch's statement, falling through to the next (Duff's device)
	{
		"StmtSwitchNestedCases", 15,
		"void Copy(char* to, const char* from, int count)\n"
		"{\n"
		"	int n = (count + 3) / 4;\n"
		"	switch (count % 4)\n"
		"	{\n"
		"	case 0: do { *to++ = *from++;\n"
		"	case 3: *to++ = *from++;\n"
		"	case 2: *to++ = *from++;\n"
		"	case 1: *to++ = *from++;\n"
		"		} while (--n > 0);\n"
		"	}\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	char a[8] = \"abcdefg\";\n"
		"	char b[8] = { 0 };\n"
		"	Copy(b, a, 7);\n"
		"	if (b[0] == 'a' && b[6] == 'g' && b[7] == 0) r += 1;\n"
		"	int x = 2;\n"
		"	int t = 0;\n"
		"	switch (x) { case 1: t += 1; case 2: t += 2; case 3: t += 4; break; case 4: t += 8; }\n"
		"	if (t == 6) r += 2;\n"
		"	switch (x) { { case 2: r += 4; } }\n"
		"	switch (x) { case 9: r += 64; }\n"
		"	r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p5: the condition is worked out once; no matching case and no default skips the statement
	{
		"StmtSwitchConditionOnce", 3,
		"int g = 0;\n"
		"int Next() { return ++g; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	switch (Next()) { case 1: case 2: r += 1; }\n"
		"	switch (Next()) { case 7: r += 8; }\n"
		"	if (g == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: a scoped enum's value, and an enum with a value an int holds
	{
		"StmtSwitchScopedEnum", 3,
		"enum class Colour : char { Red, Green, Blue };\n"
		"enum E { A = 300, B };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Colour c = Colour::Blue;\n"
		"	switch (c) { case Colour::Red: break; case Colour::Blue: r += 1; break; default: break; }\n"
		"	E e = B;\n"
		"	switch (e) { case B: r += 2; break; default: break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.4.2 [stmt.switch] p2: a class converted to an integer by its conversion function, declared in the condition too
	{
		"StmtSwitchClassCondition", 3,
		"struct N { int v; operator int() const { return v; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	N n{2};\n"
		"	switch (n) { case 2: r += 1; break; }\n"
		"	switch (N m = N{3}) { case 3: r += 2; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p3, 6.4.2 [stmt.switch]: a case may not jump past an initialised declaration into its scope
	{
		"StmtSwitchJumpPastInitRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 2;\n"
		"	switch (x) { case 1: int y = 5; (void)y; break; case 2: break; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p3: past a scalar declared without an initialiser is allowed
	{
		"StmtSwitchJumpPastScalar", 2,
		"int main()\n"
		"{\n"
		"	int x = 2;\n"
		"	int r = 0;\n"
		"	switch (x) { case 1: int y; y = 5; r += y; break; case 2: y = 2; r += y; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.1 [stmt.while], 6.5.2 [stmt.do], 6.5.3 [stmt.for]: while tests first, do after, a for's parts in their order
	{
		"StmtWhileDoFor", 15,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int n = 0;\n"
		"	while (n < 0) n++;\n"
		"	if (n == 0) r += 1;\n"
		"	do n++; while (n < 0);\n"
		"	if (n == 1) r += 2;\n"
		"	int s = 0;\n"
		"	for (int i = 0; i < 4; i += 1) s += i;\n"
		"	if (s == 6) r += 4;\n"
		"	int k = 0;\n"
		"	for (;;) { if (++k == 3) break; }\n"
		"	if (k == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.3 [stmt.for] p1: a for's first part's names are the for's, and hide those outside
	{
		"StmtForInitScope", 7,
		"int i = 100;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	for (int i = 0; i < 2; i++) { if (i < 2) r |= 1; }\n"
		"	if (i == 100) r += 2;\n"
		"	for (int i = 0, j = 10; i < j; i++, j--) { if (i == 4) r += 4; }\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.3 [stmt.for] p1: a for's first part's names end with it
	{
		"StmtForInitNameEndsRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	for (int i = 0; i < 2; i++) { }\n"
		"	return i;\n"
		"}\n"
	},
	//6.5.3 [stmt.for] p1, 6.5.1 [stmt.while] p2: a for's condition declaration is made and destroyed each time round
	{
		"StmtForConditionObject", 3,
		"int giMade = 0;\n"
		"int giGone = 0;\n"
		"struct C { int v; C(int x) : v(x) { giMade++; } ~C() { giGone++; } explicit operator bool() const { return v > 0; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int n = 2;\n"
		"	for (; C c{n}; n--) { if (giMade - giGone != 1) r += 8; }\n"
		"	if (giMade == 3) r += 1;\n"
		"	if (giGone == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.4 [stmt.select] p3, 6.5.1 [stmt.while] p2, 6.5.3 [stmt.for] p1: a condition's object lives to the end of an if; a while's is destroyed each time round, by continue and break too; a for's is destroyed before its step (as g++ does)
	{
		"StmtConditionObjectLifetimes", 7,
		"int giMade = 0;\n"
		"int giGone = 0;\n"
		"struct C { int v; C(int x) : v(x) { giMade++; } ~C() { giGone++; } explicit operator bool() const { return v > 0; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		if (C c{1}) { }\n"
		"		if (giGone == 1) r += 1;\n"
		"	}\n"
		"	giMade = 0;\n"
		"	giGone = 0;\n"
		"	int n = 5;\n"
		"	while (C c{n--}) { if (n == 3) continue; if (n == 1) break; }\n"
		"	if (giMade == 4 && giGone == 4) r += 2;\n"
		"	giMade = 0;\n"
		"	giGone = 0;\n"
		"	int k = 2;\n"
		"	int seen = 0;\n"
		"	for (; C c{k}; seen += giMade - giGone, k--) { if (k == 2) continue; }\n"
		"	if (seen == 0 && giMade == 3 && giGone == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.6.2 [stmt.cont]: continue goes to the loop's end: a for's step runs, a do's condition and a while's are tested
	{
		"StmtContinue", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = 0;\n"
		"	for (int i = 0; i < 5; i++) { if (i % 2) continue; s += i; }\n"
		"	if (s == 6) r += 1;\n"
		"	int n = 0;\n"
		"	int t = 0;\n"
		"	do { n++; if (n < 3) continue; t++; } while (n < 5);\n"
		"	if (t == 3) r += 2;\n"
		"	int w = 0;\n"
		"	int u = 0;\n"
		"	while (w < 4) { w++; if (w == 2) continue; u++; }\n"
		"	if (u == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.6.1 [stmt.break]: break leaves the innermost loop or switch
	{
		"StmtBreakInnermost", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int c = 0;\n"
		"	for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) { if (j == 1) break; c++; }\n"
		"	if (c == 3) r += 1;\n"
		"	int x = 1;\n"
		"	int t = 0;\n"
		"	for (int k = 0; k < 2; k++) { switch (x) { case 1: t++; break; } t += 10; }\n"
		"	if (t == 22) r += 2;\n"
		"	while (true) { switch (x) { default: break; } break; }\n"
		"	r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.6.1 [stmt.break] p1: break only in a loop or a switch
	{
		"StmtBreakOutsideRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	break;\n"
		"	return 0;\n"
		"}\n"
	},
	//6.6.2 [stmt.cont] p1: continue only in a loop
	{
		"StmtContinueInSwitchRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	switch (x) { case 1: continue; }\n"
		"	return 0;\n"
		"}\n"
	},
	//6.6.3 [stmt.return] p2: a return's value is converted to the function's type by copy initialisation; a braced list too
	{
		"StmtReturnConversion", 15,
		"struct P { int a; int b; };\n"
		"struct Q { int v; Q(int x) : v(x * 2) { } };\n"
		"double Half() { return 1; }\n"
		"char Letter() { return 65; }\n"
		"P Pair() { return { 3, 4 }; }\n"
		"Q Make() { return 5; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Half() == 1.0) r += 1;\n"
		"	if (Letter() == 'A') r += 2;\n"
		"	P p = Pair();\n"
		"	if (p.a + p.b == 7) r += 4;\n"
		"	if (Make().v == 10) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//6.6.3 [stmt.return] p2: copy initialisation uses no explicit constructor
	{
		"StmtReturnExplicitRefused", CONFORMANCE_REFUSED,
		"struct Q { explicit Q(int x) : v(x) { } int v; };\n"
		"Q Make() { return 5; }\n"
		"int main() { return Make().v; }\n"
	},
	//6.6.3 [stmt.return] p2, p3: a void function may return a void expression, or return early with none
	{
		"StmtReturnVoidExpression", 3,
		"int g = 0;\n"
		"void Set() { g = 2; }\n"
		"void Call() { return Set(); }\n"
		"void Early(int x) { if (x) return; g += 1; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Call();\n"
		"	if (g == 2) r += 1;\n"
		"	Early(1);\n"
		"	Early(0);\n"
		"	if (g == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.6.3 [stmt.return] p2: a void function returns no value
	{
		"StmtReturnValueInVoidRefused", CONFORMANCE_REFUSED,
		"void F() { return 1; }\n"
		"int main() { F(); return 0; }\n"
	},
	//6.6.3 [stmt.return] p2: a function that returns a value returns one
	{
		"StmtReturnNoValueRefused", CONFORMANCE_REFUSED,
		"int F() { return; }\n"
		"int main() { return F(); }\n"
	},
	//6.6 [stmt.jump] p2, 6.6.3 [stmt.return]: a return's value is made before the block's objects are destroyed, the last made first
	{
		"StmtReturnBeforeDestruction", 7,
		"int giLog = 0;\n"
		"struct L { int i; L(int x) : i(x) { } ~L() { giLog = giLog * 10 + i; } };\n"
		"int Get()\n"
		"{\n"
		"	L a(1);\n"
		"	L b(2);\n"
		"	{\n"
		"		L c(3);\n"
		"		return giLog + c.i;\n"
		"	}\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int v = Get();\n"
		"	if (v == 3) r += 1;\n"
		"	if (giLog == 321) r += 2;\n"
		"	giLog = 0;\n"
		"	for (int i = 0; i < 2; i++) { L d(4); if (i == 1) break; }\n"
		"	if (giLog == 44) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p2: an automatic variable is initialised each time its declaration is reached
	{
		"StmtDeclarationEachTime", 3,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = 0;\n"
		"	for (int i = 0; i < 3; i++) { int k = 1; s += k; k = 10; }\n"
		"	if (s == 3) r += 1;\n"
		"	int n = 0;\n"
		"again:\n"
		"	int m = 5;\n"
		"	n += m;\n"
		"	m = 0;\n"
		"	if (n < 10) goto again;\n"
		"	if (n == 10) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p4: a static local is zero first, and initialised once, the first time its declaration is passed
	{
		"StmtStaticLocalOnce", 15,
		"int giCalls = 0;\n"
		"int Init() { giCalls++; return 7; }\n"
		"int Count() { static int n = Init(); return ++n; }\n"
		"int Zero() { static int z; return z++; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Count();\n"
		"	int c = Count();\n"
		"	if (c == 9) r += 1;\n"
		"	if (giCalls == 1) r += 2;\n"
		"	Zero();\n"
		"	if (Zero() == 1) r += 4;\n"
		"	static int s;\n"
		"	if (s == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p4: a static local object is made the first time control passes its declaration, and only then
	{
		"StmtStaticLocalObjectOnce", 3,
		"int giMade = 0;\n"
		"struct M { M() { giMade++; } };\n"
		"void F(bool b) { if (b) { static M m; } }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	F(false);\n"
		"	if (giMade == 0) r += 1;\n"
		"	F(true);\n"
		"	F(true);\n"
		"	if (giMade == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.8 [stmt.ambig] p1: a statement that could be either is a declaration: T(a); T(b)[3]; T(c) = 9;
	{
		"StmtAmbiguityDeclaration", 7,
		"struct T { int v; T() : v(1) { } T(int x) : v(x) { } };\n"
		"int a = 5;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	T(a);\n"
		"	if (a.v == 1) r += 1;\n"
		"	T(b)[3];\n"
		"	if (b[2].v == 1) r += 2;\n"
		"	T(c) = 9;\n"
		"	if (c.v == 9) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.8 [stmt.ambig] p1: one that can't be a declaration is an expression: T(a)->v = 7; T(a)++;
	{
		"StmtAmbiguityExpression", 3,
		"int g = 0;\n"
		"struct T { int v; T(int x) : v(x) { } T* operator->() { g += 1; return this; } T operator++(int) { g += 2; return *this; } };\n"
		"int a = 5;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	T(a)->v = 7;\n"
		"	T(a)++;\n"
		"	if (g == 3) r += 1;\n"
		"	if (a == 5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1: over an array; auto& changes the elements, a copy doesn't
	{
		"StmtRangeForArray", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int a[4] = { 1, 2, 3, 4 };\n"
		"	int s = 0;\n"
		"	for (int x : a) s += x;\n"
		"	if (s == 10) r += 1;\n"
		"	for (auto& x : a) x *= 2;\n"
		"	if (a[3] == 8) r += 2;\n"
		"	int t = 0;\n"
		"	for (const auto& x : a) t += x;\n"
		"	for (auto x : a) x = 0;\n"
		"	if (t == 20 && a[0] == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1: a class with begin and end members, const ones for a const object
	{
		"StmtRangeForMembers", 3,
		"struct Bag\n"
		"{\n"
		"	int a[3];\n"
		"	int* begin() { return a; }\n"
		"	int* end() { return a + 3; }\n"
		"	const int* begin() const { return a; }\n"
		"	const int* end() const { return a + 3; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Bag b = { { 1, 2, 3 } };\n"
		"	int s = 0;\n"
		"	for (int x : b) s += x;\n"
		"	if (s == 6) r += 1;\n"
		"	const Bag& c = b;\n"
		"	int t = 0;\n"
		"	for (int x : c) t += x * 2;\n"
		"	if (t == 12) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1: begin and end found by argument dependent lookup
	{
		"StmtRangeForFreeFunctions", 3,
		"namespace N\n"
		"{\n"
		"	struct Bag { int a[3]; };\n"
		"	int* begin(Bag& b) { return b.a; }\n"
		"	int* end(Bag& b) { return b.a + 3; }\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	N::Bag b = { { 4, 5, 6 } };\n"
		"	int s = 0;\n"
		"	for (int x : b) s += x;\n"
		"	if (s == 15) r += 1;\n"
		"	for (int& x : b) x = 1;\n"
		"	if (b.a[2] == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1: over a braced list, which is an initializer_list
	{
		"StmtRangeForInitializerList", 3,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = 0;\n"
		"	for (int x : { 1, 2, 3 }) s += x;\n"
		"	if (s == 6) r += 1;\n"
		"	std::initializer_list<int> l = { 4, 5 };\n"
		"	for (auto x : l) s += x;\n"
		"	if (s == 15) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1, 12.2 [class.temporary] p5: the range's temporary, or the temporary a member of which is the range, lives to the loop's end
	{
		"StmtRangeForTemporary", 3,
		"struct Bag { int a[2]; ~Bag() { a[0] = 0; a[1] = 0; } int* begin() { return a; } int* end() { return a + 2; } };\n"
		"Bag Make() { return Bag{ { 3, 4 } }; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = 0;\n"
		"	for (int x : Make()) s += x;\n"
		"	if (s == 7) r += 1;\n"
		"	int t = 0;\n"
		"	for (int x : Make().a) t += x;\n"
		"	if (t == 7) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//6.5.4 [stmt.ranged] p1: a class needs begin and end, members or found by argument dependent lookup
	{
		"StmtRangeForNoBeginRefused", CONFORMANCE_REFUSED,
		"struct Bag { int a[2]; };\n"
		"int main()\n"
		"{\n"
		"	Bag b = { { 1, 2 } };\n"
		"	int s = 0;\n"
		"	for (int x : b) s += x;\n"
		"	return s;\n"
		"}\n"
	},
	//6.7 [stmt.dcl] p4: a static local of a member function is initialised the first time the function comes to it, from the fields of the object it is then called on
	{
		"StmtStaticLocalFromField", 8,
		"struct S { int m; int f() { static int s = m + 1; return s; } };\n"
		"int main() { S a; a.m = 3; S b; b.m = 9; return a.f() + b.f(); }\n"
	},
};


//Clause 7, declarations [dcl.dcl].
static const SConformanceCase gasConformanceClause7[] =
{
	//7 [dcl.dcl] p4: static_assert at namespace, class and block scope
	{
		"DclStaticAssert", 3,
		"static_assert(sizeof(char) == 1, \"char is a byte\");\n"
		"struct S { static_assert(true, \"in a class\"); int v; };\n"
		"int main()\n"
		"{\n"
		"	static_assert(1 + 1 == 2, \"in a block\");\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.v = 3;\n"
		"	if (s.v == 3) r += 1;\n"
		"	r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7 [dcl.dcl] p4: a static_assert whose constant is false
	{
		"DclStaticAssertFalseRefused", CONFORMANCE_REFUSED,
		"static_assert(sizeof(char) == 0, \"never\");\n"
		"int main() { return 0; }\n"
	},
	//7 [dcl.dcl] p4: a static_assert's expression is a constant
	{
		"DclStaticAssertNotConstantRefused", CONFORMANCE_REFUSED,
		"int n = 1;\n"
		"static_assert(n == 1, \"not a constant\");\n"
		"int main() { return 0; }\n"
	},
	//7 [dcl.dcl] p3: a simple declaration declares something
	{
		"DclDeclaresNothingRefused", CONFORMANCE_REFUSED,
		"int;\n"
		"int main() { return 0; }\n"
	},
	//7 [dcl.dcl] p1: an empty declaration at namespace scope
	{
		"DclEmptyDeclaration", 1,
		";\n"
		"int F() { return 1; };\n"
		"int main() { return F(); }\n"
	},
	//7.1.1 [dcl.stc] p1: one storage class in a declaration
	{
		"DclTwoStorageClassesRefused", CONFORMANCE_REFUSED,
		"static extern int x;\n"
		"int main() { return 0; }\n"
	},
	//7.1.1 [dcl.stc] p5, 9.4.2 [class.static.data] p5: static isn't written on a static member's definition outside its class
	{
		"DclStaticMemberDefinitionRefused", CONFORMANCE_REFUSED,
		"struct S { static int k; };\n"
		"static int S::k = 1;\n"
		"int main() { return S::k; }\n"
	},
	//7.1.1 [dcl.stc] p7: a name declared static, then extern, keeps its internal linkage
	{
		"DclStaticThenExtern", 3,
		"static int g = 1;\n"
		"extern int g;\n"
		"static int F() { return 2; }\n"
		"extern int F();\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (g == 1) r += 1;\n"
		"	if (F() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.1 [dcl.stc] p11: a mutable member can be changed through a const object
	{
		"DclMutable", 3,
		"struct C { mutable int n; int v; void Touch() const { n++; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	const C c = { 0, 5 };\n"
		"	c.Touch();\n"
		"	c.Touch();\n"
		"	if (c.n == 2) r += 1;\n"
		"	if (c.v == 5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.1 [dcl.stc] p11: a mutable member isn't const
	{
		"DclMutableConstRefused", CONFORMANCE_REFUSED,
		"struct C { mutable const int n = 1; };\n"
		"int main() { C c; return c.n; }\n"
	},
	//7.1.1 [dcl.stc] p11: a mutable member isn't static
	{
		"DclMutableStaticRefused", CONFORMANCE_REFUSED,
		"struct C { mutable static int n; };\n"
		"int main() { return 0; }\n"
	},
	//7.1.1 [dcl.stc] p3: register is deprecated, and allowed in C++14
	{
		"DclRegisterAllowed", 1,
		"int main()\n"
		"{\n"
		"	register int x = 1;\n"
		"	return x;\n"
		"}\n"
	},
	//7.1.1 [dcl.stc] p4: thread_local at namespace and block scope, where it is static too
	{
		"DclThreadLocal", 3,
		"thread_local int t = 2;\n"
		"int Count() { thread_local int n = 0; return ++n; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Count();\n"
		"	if (Count() == 2) r += 1;\n"
		"	if (t == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.2 [dcl.fct.spec] p2, p4: an inline function, one declared inline after it is declared; its address is one address
	{
		"DclInlineFunction", 3,
		"inline int Twice(int x) { return x * 2; }\n"
		"int Thrice(int x);\n"
		"inline int Thrice(int x) { return x * 3; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Twice(2) == 4 && Thrice(2) == 6) r += 1;\n"
		"	int (*p)(int) = Twice;\n"
		"	int (*q)(int) = &Twice;\n"
		"	if (p == q) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.2 [dcl.fct.spec] p5: virtual only on a member function in its class
	{
		"DclVirtualOutsideClassRefused", CONFORMANCE_REFUSED,
		"virtual int F() { return 0; }\n"
		"int main() { return F(); }\n"
	},
	//7.1.2 [dcl.fct.spec] p6: explicit only in the class's declaration of a constructor
	{
		"DclExplicitOutsideClassRefused", CONFORMANCE_REFUSED,
		"struct S { S(int); int v; };\n"
		"explicit S::S(int x) : v(x) { }\n"
		"int main() { S s(1); return s.v; }\n"
	},
	//7.1.2 [dcl.fct.spec] p6: explicit only on a constructor or a conversion function
	{
		"DclExplicitNotConstructorRefused", CONFORMANCE_REFUSED,
		"struct S { explicit int F() { return 0; } };\n"
		"int main() { return 0; }\n"
	},
	//7.1.3 [dcl.typedef] p3: a typedef may be declared again for the same type
	{
		"DclTypedefRedeclared", 3,
		"typedef int I;\n"
		"typedef int I;\n"
		"typedef I J;\n"
		"struct S { typedef int K; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	I a = 1;\n"
		"	J b = 2;\n"
		"	if (a + b == 3) r += 1;\n"
		"	S::K k = 2;\n"
		"	if (k == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.3 [dcl.typedef] p3: not for another type
	{
		"DclTypedefRedeclaredOtherRefused", CONFORMANCE_REFUSED,
		"typedef int I;\n"
		"typedef long I;\n"
		"int main() { return 0; }\n"
	},
	//7.1.3 [dcl.typedef] p6: a typedef of a class's name in its scope is that class
	{
		"DclTypedefNamesClassOtherRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"typedef int S;\n"
		"int main() { return 0; }\n"
	},
	//7.1.3 [dcl.typedef] p6: typedef struct S S; names S
	{
		"DclTypedefSameClass", 1,
		"struct S { int v; };\n"
		"typedef struct S S;\n"
		"int main() { S s; s.v = 1; return s.v; }\n"
	},
	//7.1.3 [dcl.typedef] p9: a typedef of an unnamed class names it
	{
		"DclTypedefUnnamedClass", 3,
		"typedef struct { int a; int b; } Pair;\n"
		"Pair Make(int a) { Pair p = { a, a + 1 }; return p; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Pair p = Make(1);\n"
		"	if (p.b == 2) r += 1;\n"
		"	Pair q = p;\n"
		"	if (q.a == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.3 [dcl.typedef] p1, 8.3.5 [dcl.fct] p10: a typedef of a function type declares functions, and pointers to them
	{
		"DclTypedefFunctionType", 3,
		"typedef int F(int);\n"
		"F Twice;\n"
		"int Twice(int x) { return 2 * x; }\n"
		"F* gp = Twice;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Twice(3) == 6) r += 1;\n"
		"	if (gp(4) == 8) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.4 [dcl.friend] p1: friend only in a class
	{
		"DclFriendOutsideClassRefused", CONFORMANCE_REFUSED,
		"friend int F();\n"
		"int main() { return 0; }\n"
	},
	//7.1.5 [dcl.constexpr] p2, p9: a constexpr object is const, and a constexpr function's call a constant
	{
		"DclConstexprImplies", 3,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"constexpr int k = 5;\n"
		"constexpr int Sq(int x) { return x * x; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(k), const int>::V) r += 1;\n"
		"	int a[Sq(2)];\n"
		"	if (sizeof(a) == 4 * sizeof(int)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.5 [dcl.constexpr] p9: a constexpr object is const
	{
		"DclConstexprChangeRefused", CONFORMANCE_REFUSED,
		"constexpr int k = 5;\n"
		"int main() { k = 6; return k; }\n"
	},
	//7.1.5 [dcl.constexpr] p3: a constexpr function may have local variables, loops, if and assignments (C++14)
	{
		"DclConstexprRelaxed", 7,
		"constexpr int Sum(int n) { int s = 0; for (int i = 1; i <= n; i++) { if (i % 2) s += i; } return s; }\n"
		"constexpr int Fact(int n) { int f = 1; while (n > 1) f *= n--; return f; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	static_assert(Sum(5) == 9, \"\");\n"
		"	int a[Sum(3)];\n"
		"	if (sizeof(a) == 4 * sizeof(int)) r += 1;\n"
		"	constexpr int f = Fact(5);\n"
		"	if (f == 120) r += 2;\n"
		"	switch (5) { case Fact(3) - 1: r += 4; break; }\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.5 [dcl.constexpr] p3: a constexpr function has no static variable
	{
		"DclConstexprStaticLocalRefused", CONFORMANCE_REFUSED,
		"constexpr int F() { static int n = 1; return n; }\n"
		"int main() { return F(); }\n"
	},
	//7.1.5 [dcl.constexpr] p3: nor one left uninitialised
	{
		"DclConstexprUninitialisedRefused", CONFORMANCE_REFUSED,
		"constexpr int G() { int x; x = 1; return x; }\n"
		"int main() { return G(); }\n"
	},
	//7.1.5 [dcl.constexpr] p3: a constexpr function isn't virtual
	{
		"DclConstexprVirtualRefused", CONFORMANCE_REFUSED,
		"struct S { constexpr virtual int F() const { return 1; } };\n"
		"int main() { return 0; }\n"
	},
	//7.1.5 [dcl.constexpr] p4, p9: a constexpr constructor makes constant objects, and their constexpr members are constants
	{
		"DclConstexprConstructor", 3,
		"struct P { int x; int y; constexpr P(int a, int b) : x(a), y(b) { } constexpr int Sum() const { return x + y; } };\n"
		"constexpr P p(2, 3);\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	static_assert(p.Sum() == 5, \"\");\n"
		"	int a[p.x];\n"
		"	if (sizeof(a) == 2 * sizeof(int)) r += 1;\n"
		"	constexpr P q = P(1, 1);\n"
		"	if (q.Sum() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.5 [dcl.constexpr] p9, 9.4.2 [class.static.data] p3: a constexpr static member has its initialiser in its class
	{
		"DclConstexprStaticMemberNoInitRefused", CONFORMANCE_REFUSED,
		"struct S { static constexpr int k; };\n"
		"constexpr int S::k = 1;\n"
		"int main() { return S::k; }\n"
	},
	//7.1.6.1 [dcl.type.cv], 8.5 [dcl.init] p7: a const object of a type with no default constructor has an initialiser
	{
		"DclConstNeedsInitialiserRefused", CONFORMANCE_REFUSED,
		"const int k;\n"
		"int main() { return k; }\n"
	},
	//7.1.6.1 [dcl.type.cv] p4: a const object can't be changed
	{
		"DclConstChangeRefused", CONFORMANCE_REFUSED,
		"int main() { const int k = 1; k = 2; return k; }\n"
	},
	//7.1.6.1 [dcl.type.cv] p4: nor a member of one
	{
		"DclConstMemberChangeRefused", CONFORMANCE_REFUSED,
		"struct C { int v; };\n"
		"int main() { const C c = { 1 }; c.v = 2; return c.v; }\n"
	},
	//7.1.6.1 [dcl.type.cv] p6: a volatile object is read and written each time
	{
		"DclVolatile", 3,
		"volatile int gv = 1;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int a = gv;\n"
		"	gv = 2;\n"
		"	int b = gv;\n"
		"	if (a == 1 && b == 2) r += 1;\n"
		"	const volatile int cv = 3;\n"
		"	if (cv == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.2 [dcl.type.simple] p2, Table 10: the type specifiers' combinations, in any order
	{
		"DclSimpleTypeSpecifiers", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<unsigned, unsigned int>::V && Same<signed, int>::V && Same<short int, short>::V) r += 1;\n"
		"	if (Same<long unsigned int, unsigned long>::V && Same<int long long, long long>::V) r += 2;\n"
		"	if (!Same<char, signed char>::V && !Same<char, unsigned char>::V) r += 4;\n"
		"	if (Same<double long, long double>::V && Same<int const, const int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.2 [dcl.type.simple] p2: unsigned double isn't a type
	{
		"DclUnsignedDoubleRefused", CONFORMANCE_REFUSED,
		"unsigned double d = 1;\n"
		"int main() { return 0; }\n"
	},
	//7.1.6.2 [dcl.type.simple] p2: nor long long long
	{
		"DclLongLongLongRefused", CONFORMANCE_REFUSED,
		"long long long x;\n"
		"int main() { return 0; }\n"
	},
	//7.1.6.2 [dcl.type.simple] p2: nor short char
	{
		"DclShortCharRefused", CONFORMANCE_REFUSED,
		"short char c;\n"
		"int main() { return 0; }\n"
	},
	//7.1.6.2 [dcl.type.simple] p4: decltype of a name, a member, an lvalue in brackets, a call, a prvalue
	{
		"DclDecltype", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S { int m; };\n"
		"int f();\n"
		"int& g();\n"
		"int&& h();\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = 0;\n"
		"	const S s = { 1 };\n"
		"	if (Same<decltype(i), int>::V && Same<decltype((i)), int&>::V) r += 1;\n"
		"	if (Same<decltype(s.m), int>::V && Same<decltype((s.m)), const int&>::V) r += 2;\n"
		"	if (Same<decltype(f()), int>::V && Same<decltype(g()), int&>::V && Same<decltype(h()), int&&>::V) r += 4;\n"
		"	if (Same<decltype(i + 1), int>::V && Same<decltype(1), int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p7: decltype(auto) keeps what decltype would give, a reference too
	{
		"DclDecltypeAuto", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int gi = 1;\n"
		"int& Ref() { return gi; }\n"
		"decltype(auto) Forward() { return Ref(); }\n"
		"decltype(auto) Value() { return gi; }\n"
		"decltype(auto) Bracketed() { return (gi); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<decltype(Forward()), int&>::V) r += 1;\n"
		"	if (Same<decltype(Value()), int>::V) r += 2;\n"
		"	if (Same<decltype(Bracketed()), int&>::V) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p7: auto from an initialiser; auto&, const auto*, and a braced list's initializer_list
	{
		"DclAutoDeduction", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	auto a = 1;\n"
		"	auto& b = a;\n"
		"	const auto* c = &a;\n"
		"	if (Same<decltype(a), int>::V && Same<decltype(b), int&>::V && Same<decltype(c), const int*>::V) r += 1;\n"
		"	auto d = 'x';\n"
		"	auto e = 1.5;\n"
		"	if (Same<decltype(d), char>::V && Same<decltype(e), double>::V) r += 2;\n"
		"	auto l = { 1, 2 };\n"
		"	if (Same<decltype(l), std::initializer_list<int> >::V) r += 4;\n"
		"	auto x{3};\n"
		"	if (Same<decltype(x), std::initializer_list<int> >::V || Same<decltype(x), int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p7: auto from an array is a pointer; auto& is the array
	{
		"DclAutoArray", 3,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	auto p = a;\n"
		"	auto& q = a;\n"
		"	if (Same<decltype(p), int*>::V) r += 1;\n"
		"	if (Same<decltype(q), int(&)[3]>::V) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p8: auto with several declarators that deduce one type
	{
		"DclAutoSeveralDeclarators", 1,
		"int main()\n"
		"{\n"
		"	auto a = 1, *b = &a;\n"
		"	return *b;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p8: they deduce one type
	{
		"DclAutoDifferentTypesRefused", CONFORMANCE_REFUSED,
		"int main() { auto a = 1, b = 2.0; return a; }\n"
	},
	//7.1.6.4 [dcl.spec.auto] p3: auto needs an initialiser
	{
		"DclAutoNoInitialiserRefused", CONFORMANCE_REFUSED,
		"int main() { auto a; return 0; }\n"
	},
	//7.1.6.4 [dcl.spec.auto] p3: an auto variable isn't named in its own initialiser
	{
		"DclAutoSelfRefused", CONFORMANCE_REFUSED,
		"int main() { auto a = a + 1; return 0; }\n"
	},
	//7.1.6.4 [dcl.spec.auto] p2, p9, p11: a return type deduced from the returns; a recursive call after one; a member; a declaration then its definition
	{
		"DclAutoReturnType", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"auto Twice(int x) { return x * 2; }\n"
		"auto Fact(int n) { if (n <= 1) return 1; else return n * Fact(n - 1); }\n"
		"struct S { auto Get() const { return 'c'; } };\n"
		"auto Later();\n"
		"auto Later() { return 2.5; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Twice(3) == 6 && Same<decltype(Twice(1)), int>::V) r += 1;\n"
		"	if (Fact(5) == 120) r += 2;\n"
		"	S s;\n"
		"	if (Same<decltype(s.Get()), char>::V) r += 4;\n"
		"	if (Same<decltype(Later()), double>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p9: every return deduces one type
	{
		"DclAutoReturnConflictRefused", CONFORMANCE_REFUSED,
		"auto F(int x) { if (x) return 1; return 2.0; }\n"
		"int main() { return 0; }\n"
	},
	//7.1.6.4 [dcl.spec.auto] p11: a function isn't used before its return type is deduced
	{
		"DclAutoReturnBeforeDeducedRefused", CONFORMANCE_REFUSED,
		"auto F();\n"
		"int main() { return F(); }\n"
		"auto F() { return 1; }\n"
	},
	//7.1.6.4 [dcl.spec.auto] p9: a function with no return deduces void
	{
		"DclAutoReturnVoid", 1,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"int g = 0;\n"
		"auto Set() { g = 1; }\n"
		"int main()\n"
		"{\n"
		"	Set();\n"
		"	return (Same<decltype(Set()), void>::V && (g == 1)) ? 1 : 0;\n"
		"}\n"
	},
	//7.1.6.4 [dcl.spec.auto] p2, 8.3.5 [dcl.fct] p2: auto with a trailing return type, which can name the parameters
	{
		"DclAutoTrailingReturn", 3,
		"auto Add(int a, int b) -> int { return a + b; }\n"
		"template<class T> auto Twice(T t) -> decltype(t * 2) { return t * 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Add(1, 2) == 3) r += 1;\n"
		"	if (Twice(3) == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.3 [dcl.type.elab] p3: class and struct name the same class in an elaborated type specifier
	{
		"DclElaboratedClassStruct", 1,
		"struct S { int v; };\n"
		"class S* p;\n"
		"int main() { S s; s.v = 1; p = &s; return p->v; }\n"
	},
	//7.1.6.2 [dcl.type.simple] p1, 3.4.3 [basic.lookup.qual] p4: a type named with :: in a declaration at namespace scope, in a block, and after const
	{
		"DclGlobalQualifiedTypeName", 7,
		"struct A { int v = 1; };\n"
		"::A ga;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	::A a;\n"
		"	if (a.v == 1) r += 1;\n"
		"	const ::A* p = &ga;\n"
		"	if (p->v == 1) r += 2;\n"
		"	::A* q = &a;\n"
		"	if (q->v == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.1.6.3 [dcl.type.elab] p3: an enum isn't named with a class key
	{
		"DclElaboratedEnumAsClassRefused", CONFORMANCE_REFUSED,
		"enum E { X };\n"
		"struct E* p;\n"
		"int main() { return 0; }\n"
	},
	//7.1.6.3 [dcl.type.elab] p3: a union is named with union
	{
		"DclElaboratedUnionAsStructRefused", CONFORMANCE_REFUSED,
		"union U { int a; };\n"
		"struct U* p;\n"
		"int main() { return 0; }\n"
	},
	//7.1.5 [dcl.constexpr] p9: a constexpr object of a class is initialised by its constexpr default constructor, with no initialiser written
	{
		"DclConstexprObjectDefaultConstructed", 1,
		"struct P { constexpr P() : x(1) {} int x; };\n"
		"int main() { constexpr P p; return p.x; }\n"
	},
	//7.1.5 [dcl.constexpr] p9, 9.4.2 [class.static.data] p3: a constexpr static member with no initialiser in its class is an error where it is declared, with no definition outside to say it of
	{
		"DclConstexprStaticMemberNeverInitialisedRefused", CONFORMANCE_REFUSED,
		"struct S { static constexpr int a; };\n"
		"int main() { return S::a; }\n"
	},
	//7.1.5 [dcl.constexpr] p9, 5.19 [expr.const] p2: a constexpr variable's initialiser can't be the variable's own value, which it has not got yet
	{
		"DclConstexprOwnValueRefused", CONFORMANCE_REFUSED,
		"constexpr int a = a + 1;\n"
		"int main() { return a; }\n"
	},
	//7.1.5 [dcl.constexpr] p9, 14.7.1 [temp.inst] p1: an instance's constexpr static member is worked out from another instance's, through a constexpr function
	{
		"DclConstexprStaticOfInstances", 3,
		"constexpr int g(int x) { return x; }\n"
		"template<int N> struct F { static constexpr int v = g(F<N - 1>::v) + 1; };\n"
		"template<> struct F<0> { static constexpr int v = 1; };\n"
		"int main() { return F<2>::v; }\n"
	},
	//7.2 [dcl.enum] p5, p6, 4.5 [conv.prom] p4: an enum's underlying type, fixed or big enough for its values; a fixed one's values promoted
	{
		"DclEnumUnderlyingType", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"enum A : unsigned char { A1 };\n"
		"enum class B { B1 };\n"
		"enum class C : short { C1 };\n"
		"enum D { D1 = -1, D2 = 1 };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(A) == 1) r += 1;\n"
		"	if (sizeof(B) == sizeof(int) && sizeof(C) == sizeof(short)) r += 2;\n"
		"	if ((int)D1 == -1 && D2 > D1) r += 4;\n"
		"	A a = A1;\n"
		"	if (Same<decltype(+a), int>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//7.2 [dcl.enum] p10, p11: a scoped enum converts only by a cast; its enumerators are named through it
	{
		"DclEnumScopedConversion", 3,
		"enum class E { X = 2, Y };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	E e = E::Y;\n"
		"	if ((int)e == 3) r += 1;\n"
		"	if (e == E::Y && static_cast<int>(E::X) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.2 [dcl.enum] p10: a scoped enum's value doesn't convert to an int without a cast
	{
		"DclEnumScopedToIntRefused", CONFORMANCE_REFUSED,
		"enum class E { X };\n"
		"int main() { int i = E::X; return i; }\n"
	},
	//7.2 [dcl.enum] p11: a scoped enum's enumerator isn't in the scope around it
	{
		"DclEnumScopedUnqualifiedRefused", CONFORMANCE_REFUSED,
		"enum class E { X };\n"
		"int main() { E e = X; return 0; }\n"
	},
	//7.2 [dcl.enum] p10, p11: an unscoped enum's value converts to an integer; its enumerators are named either way
	{
		"DclEnumUnscopedConverts", 3,
		"enum E { X = 5 };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = X;\n"
		"	if (i == 5) r += 1;\n"
		"	if (E::X == X) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.2 [dcl.enum] p10, 4 [conv]: an integer doesn't convert to an enum without a cast
	{
		"DclEnumFromIntRefused", CONFORMANCE_REFUSED,
		"enum E { X };\n"
		"int main() { E e = 0; return e; }\n"
	},
	//7.2 [dcl.enum] p3: an opaque enum declaration, used before the enum's definition
	{
		"DclEnumOpaque", 3,
		"enum class E : int;\n"
		"enum F : char;\n"
		"int Get(E e) { return (int)e; }\n"
		"enum class E : int { X = 4 };\n"
		"enum F : char { Y = 'y' };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Get(E::X) == 4) r += 1;\n"
		"	if (Y == 'y') r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.2 [dcl.enum] p3: an unscoped enum's opaque declaration gives its underlying type
	{
		"DclEnumOpaqueNoBaseRefused", CONFORMANCE_REFUSED,
		"enum F;\n"
		"int main() { return 0; }\n"
	},
	//7.2 [dcl.enum] p5: an enum declared again has the same underlying type
	{
		"DclEnumOtherBaseRefused", CONFORMANCE_REFUSED,
		"enum class E : int;\n"
		"enum class E : short { X };\n"
		"int main() { return 0; }\n"
	},
	//7.2 [dcl.enum] p5: an enumerator's value fits a fixed underlying type
	{
		"DclEnumValueNotFitRefused", CONFORMANCE_REFUSED,
		"enum E : unsigned char { X = 256 };\n"
		"int main() { return 0; }\n"
	},
	//7.2 [dcl.enum] p2: an enumerator without a value is one more than the one before
	{
		"DclEnumValuesIncrease", 7,
		"enum E { A, B, C = 10, D, F = -3, G };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A == 0 && B == 1) r += 1;\n"
		"	if (D == 11) r += 2;\n"
		"	if (G == -2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.1 [namespace.def] p2: a namespace opened again adds to the same namespace
	{
		"DclNamespaceReopened", 3,
		"namespace N { int a = 1; }\n"
		"namespace N { int b = a + 1; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (N::b == 2) r += 1;\n"
		"	if (N::a == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.1 [namespace.def] p8: an inline namespace's members are found in the namespace around it
	{
		"DclInlineNamespace", 7,
		"namespace L\n"
		"{\n"
		"	inline namespace V2 { int Get() { return 2; } struct S { int v; }; }\n"
		"	namespace V1 { int Get() { return 1; } }\n"
		"}\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (L::Get() == 2) r += 1;\n"
		"	if (L::V1::Get() == 1 && L::V2::Get() == 2) r += 2;\n"
		"	L::S s;\n"
		"	s.v = 4;\n"
		"	if (s.v == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.1 [namespace.def] p8: a template in an inline namespace specialised in the namespace around it
	{
		"DclInlineNamespaceSpecialisation", 1,
		"namespace L\n"
		"{\n"
		"	inline namespace V { template<class T> struct W { enum { v = 0 }; }; }\n"
		"	template<> struct W<int> { enum { v = 1 }; };\n"
		"}\n"
		"int main() { return L::W<int>::v + L::W<char>::v; }\n"
	},
	//7.3.1.1 [namespace.unnamed] p1: an unnamed namespace's members are named without qualification, and through the namespace around it
	{
		"DclUnnamedNamespace", 3,
		"namespace { int h = 3; int Get() { return h; } }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Get() == 3) r += 1;\n"
		"	if (::h == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.1.2 [namespace.memdef] p2: a namespace's member defined outside it with a qualified name, its body looking in the namespace
	{
		"DclNamespaceMemberDefinedOutside", 3,
		"namespace N { int F(int); extern int v; namespace M { int G(); } }\n"
		"int N::F(int x) { return x + 1; }\n"
		"int N::v = 5;\n"
		"int N::M::G() { return F(1) + v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (N::F(1) == 2) r += 1;\n"
		"	if (N::M::G() == 7) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.1.2 [namespace.memdef] p2: a member is defined in a namespace that encloses its declaration
	{
		"DclNamespaceDefinitionNotEnclosingRefused", CONFORMANCE_REFUSED,
		"namespace A { int F(); }\n"
		"namespace B { int A::F() { return 1; } }\n"
		"int main() { return A::F(); }\n"
	},
	//7.3.1.2 [namespace.memdef] p2: a qualified definition is of a member declared before
	{
		"DclNamespaceMemberUndeclaredRefused", CONFORMANCE_REFUSED,
		"namespace N { }\n"
		"int N::F() { return 1; }\n"
		"int main() { return 0; }\n"
	},
	//7.3.2 [namespace.alias]: an alias of a nested namespace, and an alias of an alias
	{
		"DclNamespaceAlias", 3,
		"namespace Long { namespace Inner { int v = 3; } }\n"
		"namespace LI = Long::Inner;\n"
		"namespace L2 = LI;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (LI::v == 3) r += 1;\n"
		"	if (L2::v == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.3 [namespace.udecl] p1, p11: a using-declaration brings the functions declared before it, not those after
	{
		"DclUsingDeclarationOverloads", 7,
		"namespace N { int F(int) { return 1; } }\n"
		"using N::F;\n"
		"namespace N { int F(char) { return 2; } }\n"
		"int F(double) { return 3; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1) == 1) r += 1;\n"
		"	if (F('c') == 1) r += 2;\n"
		"	if (F(1.0) == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.3 [namespace.udecl] p14: a function of the same signature as one a using-declaration brings conflicts with it
	{
		"DclUsingDeclarationConflictRefused", CONFORMANCE_REFUSED,
		"namespace N { int F(int) { return 1; } }\n"
		"using N::F;\n"
		"int F(int) { return 2; }\n"
		"int main() { return F(1); }\n"
	},
	//7.3.3 [namespace.udecl] p10: a using-declaration of a name declared in the block already
	{
		"DclUsingDeclarationVariableConflictRefused", CONFORMANCE_REFUSED,
		"namespace N { int x = 1; }\n"
		"int main() { int x = 2; using N::x; return x; }\n"
	},
	//7.3.3 [namespace.udecl] p3, p15, p19: a class's using-declaration brings a base's overloads, with the declaration's access
	{
		"DclUsingDeclarationInClass", 7,
		"struct B { int F(int) { return 1; } protected: int G() { return 4; } };\n"
		"struct D : B { using B::F; int F(char) { return 2; } using B::G; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.F(1) == 1) r += 1;\n"
		"	if (d.F('c') == 2) r += 2;\n"
		"	if (d.G() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.3 [namespace.udecl] p15: a derived class's function hides a base's of the same parameters that a using-declaration brings
	{
		"DclUsingDeclarationHidden", 3,
		"struct B { int G(int) { return 1; } };\n"
		"struct D : B { using B::G; int G(int) { return 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.G(1) == 2) r += 1;\n"
		"	B& b = d;\n"
		"	if (b.G(1) == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.3 [namespace.udecl] p3: a class's using-declaration names a base's member
	{
		"DclUsingDeclarationNotBaseRefused", CONFORMANCE_REFUSED,
		"namespace N { int F(); }\n"
		"struct S { using N::F; };\n"
		"int main() { return 0; }\n"
	},
	//7.3.4 [namespace.udir] p4: a nominated namespace's own using-directives count too
	{
		"DclUsingDirectiveTransitive", 3,
		"namespace A { int a = 1; }\n"
		"namespace B { using namespace A; int b = 2; }\n"
		"using namespace B;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (a == 1) r += 1;\n"
		"	if (b == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.3.4 [namespace.udir] p6: two namespaces' names of one name are an error only where the name is used alone
	{
		"DclUsingDirectiveAmbiguityOnlyWhenUsed", 1,
		"namespace A { int v = 1; }\n"
		"namespace B { int v = 2; }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return A::v; }\n"
	},
	//7.3.4 [namespace.udir] p6: and where it is
	{
		"DclUsingDirectiveAmbiguousRefused", CONFORMANCE_REFUSED,
		"namespace A { int v = 1; }\n"
		"namespace B { int v = 2; }\n"
		"using namespace A;\n"
		"using namespace B;\n"
		"int main() { return v; }\n"
	},
	//7.3.4 [namespace.udir] p2, p3: a using-directive in a block lasts to its end; a local declaration hides the namespace's
	{
		"DclUsingDirectiveInBlock", 3,
		"namespace A { int v = 1; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		using namespace A;\n"
		"		if (v == 1) r += 1;\n"
		"		int v = 2;\n"
		"		if (v == 2) r += 2;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//7.4 [dcl.asm]: an asm declaration
	{
		"DclAsmDeclaration", 1,
		"int main()\n"
		"{\n"
		"	asm(\"nop\");\n"
		"	return 1;\n"
		"}\n"
	},
	//7.4 [dcl.asm]: asm statements of adjacent strings, and asm volatile, in two functions
	{
		"DclAsmStatements", 3,
		"int g = 0;\n"
		"void Two() { asm(\"nop\\n\" \"nop\"); asm volatile (\"nop\"); g = 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 1;\n"
		"	asm(\"nop\");\n"
		"	Two();\n"
		"	return r + g;\n"
		"}\n"
	},
	//7.5 [dcl.link] p1, p2: extern "C" on one declaration and on a block of them
	{
		"DclLinkageC", 3,
		"extern \"C\" int CAdd(int a, int b) { return a + b; }\n"
		"extern \"C\" { int gc = 3; int CGet() { return gc; } }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (CAdd(1, 2) == 3) r += 1;\n"
		"	if (CGet() == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//7.5 [dcl.link] p4: linkage specifications nest, the innermost deciding
	{
		"DclLinkageNested", 1,
		"extern \"C\" { extern \"C++\" { int Pp(int) { return 1; } int Pp(char) { return 2; } } }\n"
		"int main() { return Pp(1) + Pp('c') - 2; }\n"
	},
	//7.5 [dcl.link] p6: two extern "C" functions of one name are one function, which can't be overloaded
	{
		"DclLinkageCOverloadRefused", CONFORMANCE_REFUSED,
		"extern \"C\" int F(int) { return 1; }\n"
		"extern \"C\" int F(char) { return 2; }\n"
		"int main() { return F(1); }\n"
	},
	//7.5 [dcl.link] p6: extern "C" declarations of a function in two namespaces are one function
	{
		"DclLinkageCInNamespaces", 1,
		"namespace A { extern \"C\" int CF(); }\n"
		"namespace B { extern \"C\" int CF(); }\n"
		"extern \"C\" int CF() { return 1; }\n"
		"int main() { return (A::CF == B::CF) ? A::CF() : 0; }\n"
	},
	//7.6.3 [dcl.attr.noreturn]: [[noreturn]] on a function that doesn't return
	{
		"DclAttributeNoreturn", 1,
		"[[noreturn]] void Forever() { for (;;) { } }\n"
		"int main()\n"
		"{\n"
		"	int x = 1;\n"
		"	if (x == 0) Forever();\n"
		"	return x;\n"
		"}\n"
	},
	//7.6.5 [dcl.attr.deprecated]: [[deprecated]] makes a use a warning, not an error
	{
		"DclAttributeDeprecated", 1,
		"[[deprecated(\"use New\")]] int Old() { return 1; }\n"
		"int main() { return Old(); }\n"
	},
	//7.6.1 [dcl.attr.grammar] p5: an attribute the implementation doesn't know is ignored
	{
		"DclAttributeUnknownIgnored", 1,
		"[[vendor_thing]] int F() { return 1; }\n"
		"int main() { return F(); }\n"
	},
	//7.6.4 [dcl.attr.depend]: [[carries_dependency]] on a parameter is accepted
	{
		"DclAttributeCarriesDependency", 1,
		"int F([[carries_dependency]] int* p) { return *p; }\n"
		"int main() { int x = 1; return F(&x); }\n"
	},
	//7.3.3 [namespace.udecl] p15: a member hides a used one only with the same parameter types: const int* is not int*
	{
		"UsingMemberConstBelowNotHidden", 1,
		"struct B { int f(int* p) { return 1; } };\n"
		"struct D : B { using B::f; int f(const int* p) { return 2; } };\n"
		"int main() { int x; D d; return d.f(&x); }\n"
	},
	//7.3.3 [namespace.udecl] p15: a member hides a used one only with the same const: f(int) doesn't hide the base's f(int) const
	{
		"UsingMemberConstNotHidden", 1,
		"struct B { int f(int) const { return 1; } };\n"
		"struct D : B { using B::f; int f(int) { return 2; } };\n"
		"int main() { const D d = D(); return d.f(0); }\n"
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
	//8.4.1 [dcl.fct.def.general] p2: a function is defined in namespace or class scope only
	{
		"DclFunctionDefinedInBlockRefused", CONFORMANCE_REFUSED,
		"int main() { int f() { return 1; } return f(); }\n"
	},
	//8.4.1 [dcl.fct.def.general] p2, 9.8 [class.local] p2: a function declared in a block, and a local class's member defined in it
	{
		"DclFunctionDeclaredInBlock", 3,
		"int main() { int f(); struct L { int g() { return 2; } }; L l; return f() + l.g(); }\n"
		"int f() { return 1; }\n"
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
	//8.5 [dcl.init] p17.8: a scalar from one expression in parentheses
	{
		"DclParenthesesTooManyForScalarRefused", CONFORMANCE_REFUSED,
		"int main() { int x(1, 2); return x; }\n"
	},
	//8.5 [dcl.init] p17.8: a comma expression in its own parentheses is one value
	{
		"DclParenthesesCommaForScalar", 2,
		"int main() { int x((1, 2)); return x; }\n"
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
	//8.5.4 [dcl.init.list] p5, p6: a std::initializer_list's array of objects is made from the list's elements and destroyed with the temporary it is, or with the list it initialises
	{
		"DclInitListObjectsDestroyed", 63,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"int giMade = 0;\n"
		"int giCopied = 0;\n"
		"int giDestroyed = 0;\n"
		"struct D\n"
		"{\n"
		"	int v;\n"
		"	D(int a) : v(a) { giMade++; }\n"
		"	D(const D& o) : v(o.v) { giCopied++; }\n"
		"	~D() { giDestroyed++; }\n"
		"};\n"
		"int Sum(std::initializer_list<D> l) { int s = 0; for (const D& d : l) s += d.v; return s; }\n"
		"struct Vec { int n; Vec(std::initializer_list<D> l) : n((int)l.size()) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = Sum({ D(1), D(2) });\n"
		"	if (s == 3 && giMade == 2 && giDestroyed == giMade + giCopied) r += 1;\n"
		"	s = Sum({ 4, 5, 6 });\n"
		"	if (s == 15 && giMade == 5 && giDestroyed == giMade + giCopied) r += 2;\n"
		"	{\n"
		"		D a(7);\n"
		"		s = Sum({ a, a });\n"
		"		if (s == 14 && giCopied >= 2 && giDestroyed == giMade + giCopied - 1) r += 4;\n"
		"	}\n"
		"	if (giDestroyed == giMade + giCopied) r += 8;\n"
		"	{\n"
		"		std::initializer_list<D> l = { D(8), D(9) };\n"
		"		s = 0;\n"
		"		for (const D& d : l) s += d.v;\n"
		"		if (s == 17 && giDestroyed == giMade + giCopied - 2) r += 16;\n"
		"	}\n"
		"	Vec v = { D(1), D(2), D(3) };\n"
		"	if (v.n == 3 && giDestroyed == giMade + giCopied) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.4 [dcl.init.list] p3, 7.1.6.4 [dcl.spec.auto] p7: a reference initialised from a list of one element it can bind to is bound to that element, and to a temporary only where it can't be
	{
		"DclInitListReferenceBinds", 255,
		"int gx = 3;\n"
		"int& gr{gx};\n"
		"const int& gc{gx};\n"
		"struct B { int v; };\n"
		"struct D : B { int w; };\n"
		"struct S { int& m; const int& k; S(int& a) : m{a}, k{a} { } };\n"
		"int Take(const int& p) { gx = 9; return p; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int x = 3;\n"
		"	int& lr{x};\n"
		"	const int& lc{x};\n"
		"	const int& le = {x};\n"
		"	const long& ll{x};\n"
		"	lr = 7;\n"
		"	if (x == 7 && lc == 7 && le == 7 && ll == 3) r += 1;\n"
		"	auto& ar{x};\n"
		"	const auto& ac{x};\n"
		"	auto&& af{x};\n"
		"	ar = 8;\n"
		"	af += 1;\n"
		"	if (x == 9 && ac == 9) r += 2;\n"
		"	int&& lm{(int&&)x};\n"
		"	lm = 4;\n"
		"	if (x == 4) r += 4;\n"
		"	gr = 5;\n"
		"	if (gx == 5 && gc == 5 && &gr == &gx && &gc == &gx) r += 8;\n"
		"	S s(gx);\n"
		"	s.m = 6;\n"
		"	if (gx == 6 && s.k == 6) r += 16;\n"
		"	if (Take({gx}) == 9) r += 32;\n"
		"	D d;\n"
		"	d.v = 1;\n"
		"	d.w = 2;\n"
		"	B& b{d};\n"
		"	b.v = 3;\n"
		"	if (d.v == 3 && &b == &d) r += 64;\n"
		"	int a[3] = { 1, 2, 3 };\n"
		"	int (&ra)[3]{a};\n"
		"	ra[1] = 5;\n"
		"	if (a[1] == 5 && sizeof(ra) == 3 * sizeof(int)) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.4 [dcl.init.list] p3, p7: a list's value isn't narrowed to an array's element, a global array's either
	{
		"DclGlobalArrayNarrowingRefused", CONFORMANCE_REFUSED,
		"unsigned gauValues[] = { 1, -1, 2 };\n"
		"int main() { return (int)gauValues[0]; }\n"
	},
	//8.4.2 [dcl.fct.def.default] p5, 12.8 [class.copy] p15 and p28: a move constructor and a move assignment defaulted after their class move the members that have a move of their own, as the copies copy them
	{
		"DclDefaultedOutsideMoves", 15,
		"struct P { int k; P() : k(1) { } P(const P& o) : k(o.k + 10) { } P(P&& o) : k(o.k + 20) { } P& operator=(const P& o) { k = o.k + 100; return *this; } P& operator=(P&& o) { k = o.k + 200; return *this; } };\n"
		"struct S { P p; int n; S() : n(5) { } S(const S&); S(S&&); S& operator=(const S&); S& operator=(S&&); };\n"
		"S::S(const S&) = default;\n"
		"S::S(S&&) = default;\n"
		"S& S::operator=(const S&) = default;\n"
		"S& S::operator=(S&&) = default;\n"
		"struct V { int v; V() : v(1) { } };\n"
		"struct A : virtual V { P p; A() { } A(A&&); A& operator=(A&&); A(const A&) = default; };\n"
		"A::A(A&&) = default;\n"
		"A& A::operator=(A&&) = default;\n"
		"struct B : A { int b; B() : b(3) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a; S b(a); S c(static_cast<S&&>(a));\n"
		"	if (b.p.k == 11 && c.p.k == 21 && b.n == 5 && c.n == 5) r += 1;\n"
		"	S d; S e; d = a; e = static_cast<S&&>(a);\n"
		"	if (d.p.k == 101 && e.p.k == 201) r += 2;\n"
		"	A f; f.v = 7; A g(static_cast<A&&>(f));\n"
		"	if (g.p.k == 21 && g.v == 7) r += 4;\n"
		"	B h; h.v = 8; B i(static_cast<B&&>(h)); B j; j = static_cast<B&&>(h);\n"
		"	if (i.p.k == 21 && i.v == 8 && i.b == 3 && j.p.k == 201 && j.v == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//8.3.6 [dcl.fct.default] p4, p6: a member function's definition outside its class adds default arguments; a member template's and a class template's member's have the ones their class gave
	{
		"DclDefaultArgumentsOutsideClass", 118,
		"struct S { int F(int a, int b); template<class T> T G(T a, T b = T(2)); };\n"
		"int S::F(int a, int b = 2) { return a + b; }\n"
		"template<class T> T S::G(T a, T b) { return a + b; }\n"
		"template<class C> struct W { int F(int a, int b = 3); };\n"
		"template<class C> int W<C>::F(int a, int b) { return a + b; }\n"
		"int main() { S s; W<int> w; return s.F(1) + s.G(10) + w.F(100); }\n"
	},
	//8.3.6 [dcl.fct.default] p4: only a function that isn't a template is given default arguments by a later declaration
	{
		"DclDefaultArgumentAddedToTemplateRefused", CONFORMANCE_REFUSED,
		"template<class T> T Add(T a, T b) { return a + b; }\n"
		"template<class T> T Add(T a, T b = T(4));\n"
		"int main() { return Add(5); }\n"
	},
	//8.3.6 [dcl.fct.default] p4: nor by a second declaration before its definition
	{
		"DclDefaultArgumentAddedBeforeDefinitionRefused", CONFORMANCE_REFUSED,
		"template<class T> T Add(T a, T b);\n"
		"template<class T> T Add(T a, T b = T(4));\n"
		"template<class T> T Add(T a, T b) { return a + b; }\n"
		"int main() { return Add(5); }\n"
	},
	//8.3.6 [dcl.fct.default] p4, p6: a member template's definition outside its class adds none
	{
		"DclDefaultArgumentMemberTemplateOutsideRefused", CONFORMANCE_REFUSED,
		"struct S { template<class T> static T Mul(T a, T b); };\n"
		"template<class T> T S::Mul(T a, T b = T(3)) { return a * b; }\n"
		"int main() { return S::Mul(4); }\n"
	},
	//8.3.6 [dcl.fct.default] p6: a class template's member function is given its default arguments in the class
	{
		"DclDefaultArgumentClassTemplateMemberOutsideRefused", CONFORMANCE_REFUSED,
		"template<class C> struct W { int F(int a, int b); template<class T> T G(T a, T b); };\n"
		"template<class C> int W<C>::F(int a, int b = 2) { return a + b; }\n"
		"int main() { W<int> w; return w.F(1); }\n"
	},
};


//Clause 9, classes [class].
static const SConformanceCase gasConformanceClause9[] =
{
	//9 [class] p2, 9.2 [class.mem] p2: a class is complete in its member function bodies, though not before its }
	{
		"ClassCompleteInMemberBodies", 3,
		"struct S { int F() { return sizeof(S); } int a; int b; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.F() == (int)sizeof(S)) r += 1;\n"
		"	if (sizeof(S) >= 2 * sizeof(int)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9 [class] p4, 1.8 [intro.object] p5: an empty class's object has a size above zero, and two have different addresses
	{
		"ClassEmptyClassSize", 3,
		"struct E { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (sizeof(E) >= 1) r += 1;\n"
		"	E a;\n"
		"	E b;\n"
		"	if (&a != &b) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9 [class] p6, 3.9 [basic.types] p3: a trivially copyable object's bytes copied keep its value
	{
		"ClassTriviallyCopyable", 1,
		"struct T { int a; char b; };\n"
		"int main()\n"
		"{\n"
		"	T s = { 5, 'x' };\n"
		"	T t;\n"
		"	unsigned char* src = (unsigned char*)&s;\n"
		"	unsigned char* dst = (unsigned char*)&t;\n"
		"	for (unsigned i = 0; i < sizeof(T); i++) dst[i] = src[i];\n"
		"	return (t.a == 5 && t.b == 'x') ? 1 : 0;\n"
		"}\n"
	},
	//9.2 [class.mem] p10: a member isn't of the class being defined
	{
		"ClassMemberOfOwnTypeRefused", CONFORMANCE_REFUSED,
		"struct S { S s; };\n"
		"int main() { return 0; }\n"
	},
	//9.2 [class.mem] p10: nor of an incomplete class
	{
		"ClassMemberIncompleteRefused", CONFORMANCE_REFUSED,
		"struct T;\n"
		"struct S { T t; };\n"
		"int main() { return 0; }\n"
	},
	//9.4.2 [class.static.data] p2: a static member may be of the class's own type
	{
		"ClassStaticMemberOfOwnType", 1,
		"struct S { static S zero; int v; };\n"
		"S S::zero = { 1 };\n"
		"int main() { return S::zero.v; }\n"
	},
	//9.2 [class.mem] p1: a member is declared once in its class
	{
		"ClassMemberDeclaredTwiceRefused", CONFORMANCE_REFUSED,
		"struct S { int a; int a; };\n"
		"int main() { return 0; }\n"
	},
	//9.2 [class.mem] p1: a member function too
	{
		"ClassMemberFunctionDeclaredTwiceRefused", CONFORMANCE_REFUSED,
		"struct S { int F(); int F(); };\n"
		"int main() { return 0; }\n"
	},
	//9.2 [class.mem], 9.4 [class.static], 9.5 [class.union], 9.8 [class.local], 13.1 [over.load] p2: what those refusals leave allowed
	{
		"ClassMemberRulesKept", 127,
		"struct S\n"
		"{\n"
		"	int F();\n"
		"	int F() const { return 2; }\n"
		"	int G() & { return 4; }\n"
		"	int G() && { return 8; }\n"
		"	static const int k = 16;\n"
		"	static constexpr double d = 0.5;\n"
		"	static int H() { return 32; }\n"
		"};\n"
		"int S::F() { return 1; }\n"
		"namespace { static union { int ui; char uc; }; }\n"
		"union V { int i; float f; int Get() const { return i; } };\n"
		"int main()\n"
		"{\n"
		"	struct L { int v; int Get() const { return v; } };\n"
		"	L l = { 0 };\n"
		"	S s;\n"
		"	const S& c = s;\n"
		"	V v;\n"
		"	v.i = 64;\n"
		"	ui = 0;\n"
		"	int r = s.F() + c.F() + s.G() + S().G() + S::k + S::H() + v.Get() + l.Get() + ui;\n"
		"	return (S::d == 0.5) ? r : 0;\n"
		"}\n"
	},
	//9.2 [class.mem] p13: a static member doesn't have the class's name
	{
		"ClassMemberNamedAsClassRefused", CONFORMANCE_REFUSED,
		"struct S { static int S; };\n"
		"int main() { return 0; }\n"
	},
	//9.2 [class.mem] p4, 12.6.2 [class.base.init] p9: default member initialisers, one using another, a mem-initialiser taking over
	{
		"ClassDefaultMemberInitialisers", 15,
		"struct S { int a = 1; int b = a + 1; int c{3}; S() { } S(int x) : a(x) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.a == 1 && s.b == 2) r += 1;\n"
		"	if (s.c == 3) r += 2;\n"
		"	S t(10);\n"
		"	if (t.a == 10 && t.b == 11) r += 4;\n"
		"	if (t.c == 3) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//9.2 [class.mem] p15: members of one access are at increasing addresses
	{
		"ClassMemberAddressOrder", 1,
		"struct S { char a; int b; char c; };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	return ((char*)&s.a < (char*)&s.b && (char*)&s.b < (char*)&s.c) ? 1 : 0;\n"
		"}\n"
	},
	//9.2 [class.mem] p19: a standard layout object's address is its first member's
	{
		"ClassStandardLayoutFirstMember", 3,
		"struct S { int a; int b; };\n"
		"struct D : S { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { 5, 6 };\n"
		"	if (*reinterpret_cast<int*>(&s) == 5) r += 1;\n"
		"	D d;\n"
		"	d.a = 7;\n"
		"	if (reinterpret_cast<S*>(&d)->a == 7 && (void*)&d == (void*)&d.a) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.2 [class.mem] p18: the common initial sequence of two structs in a union read through the other
	{
		"ClassUnionCommonInitialSequence", 1,
		"struct A { int tag; int x; };\n"
		"struct B { int tag; char c; };\n"
		"union U { A a; B b; };\n"
		"int main()\n"
		"{\n"
		"	U u;\n"
		"	u.a.tag = 3;\n"
		"	u.a.x = 4;\n"
		"	return (u.b.tag == 3) ? 1 : 0;\n"
		"}\n"
	},
	//9.3 [class.mfct] p4, p5: a member function defined outside its class, its names looked up in the class
	{
		"ClassMemberFunctionDefinedOutside", 3,
		"struct S { typedef int I; I F(I); static const I k = 3; };\n"
		"S::I S::F(I x) { return x + k; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.F(1) == 4) r += 1;\n"
		"	r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.3 [class.mfct] p5: a member function is defined once
	{
		"ClassMemberFunctionDefinedTwiceRefused", CONFORMANCE_REFUSED,
		"struct S { int F() { return 1; } };\n"
		"int S::F() { return 2; }\n"
		"int main() { return 0; }\n"
	},
	//9.3 [class.mfct] p5: one defined outside its class is declared in it
	{
		"ClassMemberFunctionUndeclaredRefused", CONFORMANCE_REFUSED,
		"struct S { };\n"
		"int S::F() { return 1; }\n"
		"int main() { return 0; }\n"
	},
	//9.3.1 [class.mfct.non-static] p3, 9.3.2 [class.this] p1: const and volatile member functions, this's type, overloads on const
	{
		"ClassConstMemberFunctions", 7,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	int Get() { return 1; }\n"
		"	int Get() const { return 2; }\n"
		"	bool ThisConst() const { return Same<decltype(this), const S*>::V; }\n"
		"	int Vol() volatile { return 4; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	const S c = { 0 };\n"
		"	if (s.Get() == 1 && c.Get() == 2) r += 1;\n"
		"	if (c.ThisConst()) r += 2;\n"
		"	volatile S v;\n"
		"	if (v.Vol() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//9.3.2 [class.this] p3: a member function that isn't const isn't called on a const object
	{
		"ClassNonConstOnConstRefused", CONFORMANCE_REFUSED,
		"struct S { int v; int Set() { return v = 1; } };\n"
		"int main() { const S c = { 0 }; return c.Set(); }\n"
	},
	//9.3.2 [class.this] p1: a const member function doesn't change the object's members
	{
		"ClassConstFunctionChangesMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int v; void Set() const { v = 1; } };\n"
		"int main() { return 0; }\n"
	},
	//9.3.1 [class.mfct.non-static] p5, 13.3.1 [over.match.funcs] p4: & and && member functions, chosen by what the object is
	{
		"ClassRefQualifiers", 7,
		"struct S { int Which() & { return 1; } int Which() && { return 2; } int Which() const & { return 3; } };\n"
		"S Make() { return S(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.Which() == 1) r += 1;\n"
		"	if (Make().Which() == 2) r += 2;\n"
		"	const S c = S();\n"
		"	if (c.Which() == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.1 [over.load] p2: where one overload has a ref-qualifier, all do
	{
		"ClassRefQualifierMixedRefused", CONFORMANCE_REFUSED,
		"struct S { int F() & { return 1; } int F() { return 2; } };\n"
		"int main() { return 0; }\n"
	},
	//13.3.1 [over.match.funcs] p4: a && member function isn't called on an lvalue
	{
		"ClassRvalueOnLvalueRefused", CONFORMANCE_REFUSED,
		"struct S { int F() && { return 1; } };\n"
		"int main() { S s; return s.F(); }\n"
	},
	//9.3.2 [class.this] p1: this points to the object the function is called on
	{
		"ClassThis", 3,
		"struct S { int v; S* Self() { return this; } int Get() { return this->v + v; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.v = 2;\n"
		"	if (s.Self() == &s) r += 1;\n"
		"	if (s.Get() == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.4.1 [class.static.mfct]: a static member function, called through its class or an object, and its address a plain function pointer
	{
		"ClassStaticMemberFunction", 7,
		"struct S { static int Count; int v; static int Get() { return Count; } static int Twice(int x) { return 2 * x; } };\n"
		"int S::Count = 3;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (S::Get() == 3) r += 1;\n"
		"	S s;\n"
		"	if (s.Get() == 3) r += 2;\n"
		"	int (*p)(int) = &S::Twice;\n"
		"	if (p(2) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//9.4.1 [class.static.mfct] p2: a static member function has no this
	{
		"ClassStaticFunctionUsesMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int v; static int Get() { return v; } };\n"
		"int main() { return S::Get(); }\n"
	},
	//9.4.1 [class.static.mfct] p2: a static member function isn't const
	{
		"ClassStaticFunctionConstRefused", CONFORMANCE_REFUSED,
		"struct S { static int Get() const { return 1; } };\n"
		"int main() { return S::Get(); }\n"
	},
	//9.4.2 [class.static.data]: one object for the class, defined outside it; a const integer one's value in the class; none in the objects
	{
		"ClassStaticDataMember", 15,
		"struct S { static int n; static const int k = 4; int v; };\n"
		"int S::n = 1;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a;\n"
		"	S b;\n"
		"	a.n = 5;\n"
		"	if (b.n == 5 && S::n == 5) r += 1;\n"
		"	if (S::k == 4) r += 2;\n"
		"	int arr[S::k];\n"
		"	if (sizeof(arr) == 4 * sizeof(int)) r += 4;\n"
		"	if (sizeof(S) == sizeof(int)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//9.4.2 [class.static.data] p3: a constexpr float static member is a constant; one bound to a reference is defined
	{
		"ClassStaticConstexprFloat", 5,
		"struct S { static constexpr double d = 0.25; static constexpr float f = 1.5f; };\n"
		"constexpr double S::d;\n"
		"double Twice(const double& r) { return r * 2; }\n"
		"int main() { return (int)(Twice(S::d) * 4 + S::f * 2); }\n"
	},
	//9.4.2 [class.static.data] p3: only a const integer or a constexpr static member is initialised in its class
	{
		"ClassStaticInClassInitialiserRefused", CONFORMANCE_REFUSED,
		"struct S { static int n = 1; };\n"
		"int main() { return S::n; }\n"
	},
	//9.4.2 [class.static.data] p6: a local class has no static data member
	{
		"ClassLocalStaticMemberRefused", CONFORMANCE_REFUSED,
		"int main() { struct L { static int n; }; return 0; }\n"
	},
	//9.5 [class.union] p1: a union's members share its object, one living at a time; it holds the largest
	{
		"ClassUnion", 3,
		"union U { char c; int i; double d; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	U u;\n"
		"	u.i = 5;\n"
		"	if (u.i == 5) r += 1;\n"
		"	u.c = 'x';\n"
		"	if (u.c == 'x' && sizeof(U) >= sizeof(double)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//8.5.1 [dcl.init.aggr] p16, 9.5 [class.union]: a union's braced initialiser initialises its first member
	{
		"ClassUnionInitialiser", 1,
		"union U { int i; char c; };\n"
		"int main() { U u = { 3 }; return (u.i == 3) ? 1 : 0; }\n"
	},
	//9.5 [class.union] p2: a union may have member functions and a constructor
	{
		"ClassUnionMemberFunctions", 3,
		"union U { int i; float f; U(int x) : i(x) { } int Get() const { return i; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	U u(4);\n"
		"	if (u.Get() == 4) r += 1;\n"
		"	u.f = 1.5f;\n"
		"	if (u.f == 1.5f) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.5 [class.union] p2, p4: a union with a member that has a constructor: the union's own constructor and destructor make and destroy it
	{
		"ClassUnionClassMember", 3,
		"int giMade = 0;\n"
		"int giGone = 0;\n"
		"struct M { int v; M(int x) : v(x) { giMade++; } ~M() { giGone++; } };\n"
		"union U { M m; int i; U() : m(3) { } ~U() { m.~M(); } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		U u;\n"
		"		if (u.m.v == 3 && giMade == 1) r += 1;\n"
		"	}\n"
		"	if (giGone == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p8, 9.5 [class.union] p4: a union's constructor makes only the member it names, and none if it names none
	{
		"ClassUnionConstructorMakesNamedMember", 3,
		"int giMade = 0;\n"
		"struct M { int v; M(int x) : v(x) { giMade += 1; } ~M() { } };\n"
		"struct N { int w; N() : w(9) { giMade += 10; } ~N() { } };\n"
		"union U { M m; N n; U() : m(3) { } ~U() { } };\n"
		"union V { N n; M m; V() { } ~V() { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	U u;\n"
		"	if (u.m.v == 3 && giMade == 1) r += 1;\n"
		"	V v;\n"
		"	if (giMade == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.5 [class.union] p2, 12.1 [class.ctor] p4: a union's default constructor is deleted where a member's isn't trivial
	{
		"ClassUnionClassMemberDeletedRefused", CONFORMANCE_REFUSED,
		"struct M { M() { } };\n"
		"union U { M m; int i; };\n"
		"int main() { U u; return 0; }\n"
	},
	//9.5 [class.union] p2, p8, 12.6.2 [class.base.init] p8: an anonymous union's member with a constructor is its class's variant member, which the class's constructor makes or not
	{
		"ClassAnonymousUnionMemberWithConstructor", 3,
		"struct M { int v; M() : v(5) { } };\n"
		"struct S { int tag; union { M m; int i; }; S() : tag(0) { i = 3; } };\n"
		"int main() { S s; return s.i + s.tag; }\n"
	},
	//9.5 [class.union] p2: a union has no base class
	{
		"ClassUnionBaseRefused", CONFORMANCE_REFUSED,
		"struct B { };\n"
		"union U : B { int i; };\n"
		"int main() { return 0; }\n"
	},
	//9.5 [class.union] p2: and is no base class
	{
		"ClassUnionAsBaseRefused", CONFORMANCE_REFUSED,
		"union U { int i; };\n"
		"struct D : U { };\n"
		"int main() { return 0; }\n"
	},
	//9.5 [class.union] p2: a union has no virtual function
	{
		"ClassUnionVirtualRefused", CONFORMANCE_REFUSED,
		"union U { int i; virtual int F() { return 1; } };\n"
		"int main() { return 0; }\n"
	},
	//9.5 [class.union] p2: nor a reference member
	{
		"ClassUnionReferenceMemberRefused", CONFORMANCE_REFUSED,
		"union U { int& r; int i; };\n"
		"int main() { return 0; }\n"
	},
	//9.5 [class.union] p5: an anonymous union's members in a block, a class and a namespace, sharing one place
	{
		"ClassAnonymousUnion", 7,
		"struct S { int tag; union { int i; float f; }; };\n"
		"static union { int gi; char gc; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	union { int a; char b; };\n"
		"	a = 0x41;\n"
		"	if (b == 0x41) r += 1;\n"
		"	S s;\n"
		"	s.i = 3;\n"
		"	if (s.i == 3) r += 2;\n"
		"	gi = 2;\n"
		"	if (gi == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//9.5 [class.union] p5: an anonymous union at namespace scope is static
	{
		"ClassAnonymousUnionNotStaticRefused", CONFORMANCE_REFUSED,
		"union { int gi; char gc; };\n"
		"int main() { return 0; }\n"
	},
	//9.5 [class.union] p5: an anonymous union has only data members
	{
		"ClassAnonymousUnionFunctionRefused", CONFORMANCE_REFUSED,
		"int main() { union { int a; int F() { return 1; } }; return 0; }\n"
	},
	//9.6 [class.bit] p1, p2: a bit-field keeps its value to its width, an unsigned one wrapping; unnamed and zero width ones
	{
		"ClassBitFields", 15,
		"struct S { unsigned a : 3; unsigned b : 5; int : 0; unsigned c : 4; unsigned : 2; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.a = 9;\n"
		"	if (s.a == 1) r += 1;\n"
		"	s.b = 31;\n"
		"	if (s.b == 31) r += 2;\n"
		"	s.c = 15;\n"
		"	s.c++;\n"
		"	if (s.c == 0) r += 4;\n"
		"	s.a = 7;\n"
		"	s.b = 0;\n"
		"	if (s.a == 7) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//9.6 [class.bit] p3, p4: a bool bit-field, and one wide enough for all of an enum's values holding them
	{
		"ClassBitFieldBoolAndEnum", 7,
		"enum E { E0, E1, E2, E3 };\n"
		"struct S { bool f : 1; E e : 2; unsigned u : 1; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.f = true;\n"
		"	if (s.f) r += 1;\n"
		"	s.e = E3;\n"
		"	if (s.e == E3) r += 2;\n"
		"	s.u = 1;\n"
		"	s.f = false;\n"
		"	if (!s.f && s.u == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//9.6 [class.bit] p3: & isn't applied to a bit-field
	{
		"ClassBitFieldAddressRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 3; };\n"
		"int main() { S s; int* p = &s.b; return *p; }\n"
	},
	//9.6 [class.bit] p3: a non-const reference doesn't bind to a bit-field
	{
		"ClassBitFieldReferenceRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 3; };\n"
		"int main() { S s; s.b = 1; int& r = s.b; return r; }\n"
	},
	//9.6 [class.bit] p3: a const reference binds to a temporary of its value
	{
		"ClassBitFieldConstReference", 1,
		"struct S { int b : 3; };\n"
		"int main()\n"
		"{\n"
		"	S s;\n"
		"	s.b = 1;\n"
		"	const int& r = s.b;\n"
		"	s.b = 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.6 [class.bit] p3: a bit-field isn't static
	{
		"ClassBitFieldStaticRefused", CONFORMANCE_REFUSED,
		"struct S { static int b : 3; };\n"
		"int main() { return 0; }\n"
	},
	//9.6 [class.bit] p1: a bit-field's width isn't negative
	{
		"ClassBitFieldNegativeRefused", CONFORMANCE_REFUSED,
		"struct S { int b : -1; };\n"
		"int main() { return 0; }\n"
	},
	//9.6 [class.bit] p2: a zero width bit-field has no name
	{
		"ClassBitFieldZeroNamedRefused", CONFORMANCE_REFUSED,
		"struct S { int b : 0; };\n"
		"int main() { return 0; }\n"
	},
	//9.7 [class.nest] p1, p3: a nested class names the enclosing class's types and statics unqualified, and is defined outside it
	{
		"ClassNestedClass", 7,
		"struct Outer { typedef int Int; static const int k = 2; static int Get() { return 3; } struct In; struct Other { Int v = k; }; };\n"
		"struct Outer::In { Int Sum() { return k + Get(); } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Outer::In i;\n"
		"	if (i.Sum() == 5) r += 1;\n"
		"	Outer::Other o;\n"
		"	if (o.v == 2) r += 2;\n"
		"	Outer::Int n = 4;\n"
		"	if (n == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//9.7 [class.nest] p1: a nested class, defined in its class or outside it, calls the enclosing class's static member function
	{
		"ClassNestedCallsEnclosingStatic", 7,
		"struct Outer { static int Get() { return 3; } struct In { int F() { return Get(); } }; struct Out; };\n"
		"struct Outer::Out { int F() { return Get() + 1; } };\n"
		"int main() { Outer::In i; Outer::Out o; return i.F() + o.F(); }\n"
	},
	//9.7 [class.nest] p1: but not one that needs an object of the enclosing class
	{
		"ClassNestedCallsEnclosingMemberRefused", CONFORMANCE_REFUSED,
		"struct Outer { int F() { return 1; } struct In { int G() { return F(); } }; };\n"
		"int main() { Outer::In i; return i.G(); }\n"
	},
	//9.7 [class.nest], 11.7 [class.access.nest] p1: a nested class is a member, and reaches the enclosing class's private members
	{
		"ClassNestedPrivateAccess", 3,
		"class Outer\n"
		"{\n"
		"	int secret = 5;\n"
		"	static int hidden;\n"
		"public:\n"
		"	struct In { int Peek(const Outer& o) { return o.secret + hidden; } };\n"
		"};\n"
		"int Outer::hidden = 2;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Outer o;\n"
		"	Outer::In i;\n"
		"	if (i.Peek(o) == 7) r += 1;\n"
		"	r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.7 [class.nest] p1: a nested class's member function has no object of the enclosing class
	{
		"ClassNestedNoOuterThisRefused", CONFORMANCE_REFUSED,
		"struct Outer { int v; struct In { int Get() { return v; } }; };\n"
		"int main() { return 0; }\n"
	},
	//9.9 [class.nested.type]: a nested type named with its class outside it, alone inside
	{
		"ClassNestedTypeNames", 3,
		"struct S { typedef int I; enum E { A = 1 }; I Get() { return A; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S::I i = S::A;\n"
		"	if (i == 1) r += 1;\n"
		"	S s;\n"
		"	if (s.Get() == S::A) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//9.9 [class.nested.type]: outside its class a nested type needs its class's name
	{
		"ClassNestedTypeUnqualifiedRefused", CONFORMANCE_REFUSED,
		"struct S { typedef int I; };\n"
		"int main() { I i = 1; return i; }\n"
	},
	//9.8 [class.local], 14.5.2 [temp.mem] p2: a local class has no member template
	{
		"ClassLocalMemberTemplateRefused", CONFORMANCE_REFUSED,
		"int main() { struct L { template<class T> int F(T) { return 1; } }; return 0; }\n"
	},
	//9.8 [class.local] p2: a local class's member functions are defined in it
	{
		"ClassLocalMemberDefinedOutsideRefused", CONFORMANCE_REFUSED,
		"int main()\n"
		"{\n"
		"	struct L { int F(); };\n"
		"	int L::F() { return 1; }\n"
		"	return 0;\n"
		"}\n"
	},
};


//Clause 10, derived classes [class.derived].
static const SConformanceCase gasConformanceClause10[] =
{
	//10.2 [class.member.lookup] p3-p7: a member name found in one base, or as the same member through two, or hidden by a nearer class, is not ambiguous
	{
		"ClassMemberNameThroughBases", 15,
		"struct A { typedef int T; enum { K = 1 }; static int s; };\n"
		"int A::s = 2;\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"struct E : A { typedef char T; };\n"
		"struct F : E { };\n"
		"struct G { typedef long U; };\n"
		"struct H : A, G { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D::T x = 3;\n"
		"	F::T y = 4;\n"
		"	H::U z = 5;\n"
		"	if (x == 3 && sizeof(x) == sizeof(int)) r += 1;\n"
		"	if (y == 4 && sizeof(F::T) == 1) r += 2;\n"
		"	if (D::K == 1 && D::s == 2) r += 4;\n"
		"	if (z == 5 && sizeof(H::U) == sizeof(long) && sizeof(H::T) == sizeof(int)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.2 [class.member.lookup] p6: a name found in two base class subobjects as different members is ambiguous
	{
		"ClassMemberNameAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { typedef int T; };\n"
		"struct B { typedef char T; };\n"
		"struct C : A, B { };\n"
		"int main()\n"
		"{\n"
		"	C::T x = 0;\n"
		"	return x;\n"
		"}\n"
	},
	//10.3 [class.virtual] p5: a function marked override that overrides no member function of a base class is ill-formed, in a class nothing uses too
	{
		"ClassOverrideUnusedRefused", CONFORMANCE_REFUSED,
		"struct B { virtual void f(); };\n"
		"struct D : B { void g() override; };\n"
		"int main() { return 0; }\n"
	},
	//9.2 [class.mem] p8, 10.3 [class.virtual] p4: final appears only in the declaration of a virtual member function
	{
		"ClassFinalNotVirtualRefused", CONFORMANCE_REFUSED,
		"struct B { int h(); };\n"
		"struct D : B { int h() final; };\n"
		"int main() { return 0; }\n"
	},
	//10.3 [class.virtual] p4, p5: override and final on functions that override, and a final function called through its base
	{
		"ClassOverrideAndFinal", 3,
		"struct B { virtual int f() { return 1; } virtual int g() final { return 2; } int h() { return 9; } };\n"
		"struct D : B { int f() override { return 3; } };\n"
		"struct E final : D { int f() final { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	E e;\n"
		"	B* p = &d;\n"
		"	B* q = &e;\n"
		"	if (p->f() == 3 && p->g() == 2) r += 1;\n"
		"	if (q->f() == 4 && q->g() == 2 && e.h() == 9) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.1 [class.mi] p4, 12.8 [class.copy] p15 and p28, 8.4.2 [dcl.fct.def.default] p5: a copy constructor and a copy assignment defaulted after a class with virtual bases: called for a complete object and, as a base of another's, for a base subobject, which leaves the virtual bases to the complete object and is given its vtables by it
	{
		"ClassVirtualBaseCopyDefaultedOutside", 127,
		"struct V { int v; V() : v(1) { } };\n"
		"struct A : virtual V { int a; A() : a(2) { } A(const A&); A& operator=(const A&); virtual int F() { return a + v; } };\n"
		"A::A(const A&) = default;\n"
		"A& A::operator=(const A&) = default;\n"
		"struct B : A { int b; B() : b(3) { } virtual int F() { return b + v * 10; } };\n"
		"struct C : virtual V { int c; C() : c(4) { } C(const C&); virtual int G() { return c + v; } };\n"
		"C::C(const C&) = default;\n"
		"struct D : A, C { int d; D() : d(5) { } D(const D&); D& operator=(const D&); virtual int F() { return d + v * 100; } };\n"
		"D::D(const D&) = default;\n"
		"D& D::operator=(const D&) = default;\n"
		"struct E : D { int e; E() : e(6) { } virtual int G() { return e + v * 7; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A a; a.v = 11; a.a = 12;\n"
		"	A a2(a);\n"
		"	A* pa = &a2;\n"
		"	if (a2.v == 11 && a2.a == 12 && pa->F() == 23) r += 1;\n"
		"	B b; b.v = 21; b.a = 22; b.b = 23;\n"
		"	B b2(b);\n"
		"	pa = &b2;\n"
		"	if (b2.v == 21 && b2.a == 22 && b2.b == 23 && pa->F() == 233) r += 2;\n"
		"	D d; d.v = 31; d.a = 32; d.c = 34; d.d = 35;\n"
		"	D d2(d);\n"
		"	pa = &d2;\n"
		"	C* pc = &d2;\n"
		"	if (d2.v == 31 && d2.a == 32 && d2.c == 34 && d2.d == 35 && pa->F() == 3135 && pc->G() == 65 && &pc->v == &d2.v) r += 4;\n"
		"	E e; e.v = 41; e.a = 42; e.c = 44; e.d = 45; e.e = 46;\n"
		"	E e2(e);\n"
		"	pa = &e2;\n"
		"	pc = &e2;\n"
		"	if (e2.v == 41 && e2.a == 42 && e2.c == 44 && e2.d == 45 && e2.e == 46 && pa->F() == 4145 && pc->G() == 333 && &pc->v == &e2.v) r += 8;\n"
		"	A a3(e);\n"
		"	pa = &a3;\n"
		"	if (a3.v == 41 && a3.a == 42 && pa->F() == 83) r += 16;\n"
		"	D d3;\n"
		"	d3 = e;\n"
		"	pa = &d3;\n"
		"	if (d3.v == 41 && d3.a == 42 && d3.c == 44 && d3.d == 45 && pa->F() == 4145) r += 32;\n"
		"	A a4;\n"
		"	a4 = b;\n"
		"	if (a4.v == 21 && a4.a == 22) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//10.2 [class.member.lookup] p3-p7, 3.4.3.1 [class.qual] p1: a name two bases have is no ambiguity where it is said whose (a base named through the class, two instances of a template among them), where one base's hides a virtual base's, where a using-declaration makes it the class's own, and for a type that one class reached twice declares
	{
		"ClassMemberNameNotAmbiguous", 15,
		"template<class T> struct X { typedef T Type; T v; int F() { return 3; } static int s; enum { e = sizeof(T) }; };\n"
		"template<class T> int X<T>::s = 4;\n"
		"struct C : X<int>, X<char> { int G() { return X<int>::v + X<char>::F() + X<char>::e; } };\n"
		"struct V { int f; int H() { return 1; } typedef long Type; };\n"
		"struct A : virtual V { int f; int H() { return 2; } typedef char Type; };\n"
		"struct B : virtual V { };\n"
		"struct D : A, B { int K() { return f + H(); } };\n"
		"struct P { int m; int M() { return 5; } };\n"
		"struct Q { char m; int M(int) { return 6; } };\n"
		"struct R : P, Q { using P::m; using Q::M; int m2; };\n"
		"struct E1 : V { }; struct E2 : V { };\n"
		"struct W : E1, E2 { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c; c.X<int>::v = 1; c.X<char>::v = 2;\n"
		"	C::X<char>::Type t = 2;\n"
		"	if (c.X<int>::F() + c.X<int>::v + t == 6 && sizeof(t) == 1 && c.G() == 1 + 3 + 1 && C::X<int>::s == 4) r += 1;\n"
		"	D d; d.f = 7; d.V::f = 9;\n"
		"	D* pd = &d;\n"
		"	if (d.f == 7 && pd->H() == 2 && d.K() == 9 && sizeof(D::Type) == 1 && d.V::f == 9) r += 2;\n"
		"	R x; x.m = 300; x.m2 = 1;\n"
		"	if (x.m == 300 && x.M(1) == 6 && sizeof(x.m) == sizeof(int)) r += 4;\n"
		"	W w; w.E1::f = 1; w.E2::f = 2;\n"
		"	if (w.E1::f + w.E2::f == 3 && sizeof(W::Type) == sizeof(long)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.2 [class.member.lookup] p6: a type two instances of one template each have, both bases of the class
	{
		"ClassMemberTypeTwoInstancesRefused", CONFORMANCE_REFUSED,
		"template<class T> struct X { typedef T Type; };\n"
		"struct C : X<int>, X<char> { };\n"
		"int main() { C::Type t = 0; return t; }\n"
	},
	//10.2 [class.member.lookup] p6: a field of each of two instances of a template, named in an object of the class derived from both
	{
		"ClassMemberFieldTwoInstancesRefused", CONFORMANCE_REFUSED,
		"template<class T> struct X { T v; };\n"
		"struct C : X<int>, X<char> { };\n"
		"int main() { C c; c.v = 0; return 0; }\n"
	},
	//10.2 [class.member.lookup] p6: a static member of each
	{
		"ClassMemberStaticTwoInstancesRefused", CONFORMANCE_REFUSED,
		"template<class T> struct X { static int s; enum { e = 1 }; };\n"
		"template<class T> int X<T>::s = 2;\n"
		"struct C : X<int>, X<char> { };\n"
		"int main() { return C::s; }\n"
	},
	//10.2 [class.member.lookup] p6: object.name where two bases each have a field of the name
	{
		"ClassMemberFieldAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { int v; }; struct B { char v; };\n"
		"struct C : A, B { };\n"
		"int main() { C c; c.v = 0; return 0; }\n"
	},
	//10.2 [class.member.lookup] p6: pointer->name, and in sizeof
	{
		"ClassMemberArrowAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { int v; }; struct B { char v; };\n"
		"struct C : A, B { };\n"
		"int main() { C c; C* p = &c; return p->v + sizeof(c.v); }\n"
	},
	//10.2 [class.member.lookup] p6, 13.3 [over.match] p1: two bases' functions of one name aren't overloads of each other: the call is ambiguous though only one takes its arguments
	{
		"ClassMemberFunctionAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { int F() { return 1; } }; struct B { int F(int) { return 2; } };\n"
		"struct C : A, B { };\n"
		"int main() { C c; return c.F(1); }\n"
	},
	//10.2 [class.member.lookup] p6: the name alone, in a member function of the class derived from both
	{
		"ClassMemberUnqualifiedAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { int v; int F() { return 1; } static int s; }; struct B { char v; int F(int) { return 2; } static int s; };\n"
		"int A::s = 1; int B::s = 2;\n"
		"struct C : A, B { int G() { return v + F() + s; } };\n"
		"int main() { C c; return c.G(); }\n"
	},
	//9 [class] p3, 10.3 [class.virtual] p4: a class declared final derives from another, and a function that isn't the final one of its name is overridden
	{
		"ClassFinalAllowed", 0,
		"struct B { virtual void f(); };\n"
		"struct D final : B { void f(); };\n"
		"struct E { virtual void g() final; virtual void g(int); };\n"
		"struct F : E { void g(int); };\n"
		"int main() { return 0; }\n"
	},
	//10.3 [class.virtual] p4: a function declared final is overridden, in a class nothing uses
	{
		"ClassOverridesFinalUnusedRefused", CONFORMANCE_REFUSED,
		"struct B { virtual void f() final; };\n"
		"struct D : B { void f(); };\n"
		"int main() { return 0; }\n"
	},
	//10.3 [class.virtual] p4: one made final by a class between
	{
		"ClassOverridesFinalIndirectRefused", CONFORMANCE_REFUSED,
		"struct B { virtual void f(); };\n"
		"struct M : B { void f() final; };\n"
		"struct D : M { void f(); };\n"
		"int main() { return 0; }\n"
	},
	//9 [class] p3: a class declared final is no base
	{
		"ClassFinalBaseRefused", CONFORMANCE_REFUSED,
		"struct B final { int v; };\n"
		"struct D : B { };\n"
		"int main() { D d; d.v = 0; return d.v; }\n"
	},
	//9 [class] p3: nor of a class nothing uses
	{
		"ClassFinalBaseUnusedRefused", CONFORMANCE_REFUSED,
		"struct B final { int v; };\n"
		"struct D : B { };\n"
		"int main() { return 0; }\n"
	},
	//9 [class] p3: nor is an instance of a class template declared final
	{
		"ClassFinalTemplateBaseRefused", CONFORMANCE_REFUSED,
		"template<class T> struct B final { T v; };\n"
		"struct D : B<int> { };\n"
		"int main() { return 0; }\n"
	},
	//10 [class.derived] p2: a base class is complete
	{
		"DerivedBaseIncompleteRefused", CONFORMANCE_REFUSED,
		"struct B;\n"
		"struct D : B { };\n"
		"int main() { return 0; }\n"
	},
	//10 [class.derived] p1: a base is a class
	{
		"DerivedBaseNotClassRefused", CONFORMANCE_REFUSED,
		"typedef int I;\n"
		"struct D : I { };\n"
		"int main() { return 0; }\n"
	},
	//10.1 [class.mi] p3: a class is a direct base once
	{
		"DerivedDirectBaseTwiceRefused", CONFORMANCE_REFUSED,
		"struct B { };\n"
		"struct D : B, B { };\n"
		"int main() { return 0; }\n"
	},
	//10 [class.derived] p1, p2, 4.10 [conv.ptr] p3: members inherited; a derived object's pointer and reference converted to its base's; back by static_cast
	{
		"DerivedSingle", 15,
		"struct B { int b = 1; int GetB() { return b; } };\n"
		"struct D : B { int d = 2; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.b == 1 && d.GetB() == 1) r += 1;\n"
		"	B* pb = &d;\n"
		"	if (pb->b == 1) r += 2;\n"
		"	B& rb = d;\n"
		"	rb.b = 5;\n"
		"	if (d.b == 5) r += 4;\n"
		"	if (static_cast<D*>(pb)->d == 2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10 [class.derived] p2: an indirect base's members and a pointer to it
	{
		"DerivedIndirectBase", 3,
		"struct A { int a = 1; };\n"
		"struct B : A { int b = 2; };\n"
		"struct C : B { int c = 3; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	A* pa = &c;\n"
		"	if (pa->a == 1) r += 1;\n"
		"	if (c.a + c.b + c.c == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.1 [class.mi] p1, p2: two bases, each its own part; conversions to each, a null pointer staying null, back by static_cast
	{
		"DerivedMultipleBases", 15,
		"struct A { int a = 1; };\n"
		"struct B { int b = 2; };\n"
		"struct C : A, B { int c = 3; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	A* pa = &c;\n"
		"	B* pb = &c;\n"
		"	if (pa->a == 1 && pb->b == 2) r += 1;\n"
		"	if ((void*)pa != (void*)pb) r += 2;\n"
		"	if (static_cast<C*>(pb) == &c) r += 4;\n"
		"	C* pnull = 0;\n"
		"	B* pbn = pnull;\n"
		"	if (pbn == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.1 [class.mi] p3: an indirect base through two classes is two parts
	{
		"DerivedRepeatedBase", 3,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	d.B::a = 1;\n"
		"	d.C::a = 2;\n"
		"	if (d.B::a == 1 && d.C::a == 2) r += 1;\n"
		"	A* p = static_cast<B*>(&d);\n"
		"	if (p->a == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.1 [class.mi] p3, 4.10 [conv.ptr] p3: a conversion to a base that is there twice
	{
		"DerivedAmbiguousBaseConversionRefused", CONFORMANCE_REFUSED,
		"struct A { int a; };\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A* p = &d; return 0; }\n"
	},
	//10.1 [class.mi] p4: a virtual base is one part however often it is a base
	{
		"DerivedVirtualBaseShared", 15,
		"struct A { int a = 0; };\n"
		"struct B : virtual A { };\n"
		"struct C : virtual A { };\n"
		"struct D : B, C { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	d.B::a = 5;\n"
		"	if (d.C::a == 5) r += 1;\n"
		"	A* p = &d;\n"
		"	if (p->a == 5) r += 2;\n"
		"	B* pb = &d;\n"
		"	C* pc = &d;\n"
		"	if (static_cast<A*>(pb) == static_cast<A*>(pc)) r += 4;\n"
		"	d.a = 7;\n"
		"	if (pb->a == 7) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.1 [class.mi] p4: a class with virtual and non-virtual bases of one class has one part for the virtual ones and one for each other
	{
		"DerivedVirtualAndNonVirtual", 3,
		"struct A { int a = 0; };\n"
		"struct B : virtual A { };\n"
		"struct C : virtual A { };\n"
		"struct E : A { };\n"
		"struct D : B, C, E { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	d.B::a = 1;\n"
		"	d.E::a = 2;\n"
		"	if (d.C::a == 1) r += 1;\n"
		"	if (d.E::a == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//5.2.9 [expr.static.cast] p11: static_cast from a virtual base to a class derived from it
	{
		"DerivedVirtualBaseDowncastRefused", CONFORMANCE_REFUSED,
		"struct A { int a; virtual ~A() { } };\n"
		"struct B : virtual A { };\n"
		"int main() { B b; A* p = &b; B* q = static_cast<B*>(p); return 0; }\n"
	},
	//10.2 [class.member.lookup] p1, p4: a derived class's name hides the base's, a function of other parameters too
	{
		"DerivedLookupHides", 7,
		"struct B { int v = 1; int F(int) { return 1; } int G() { return 3; } };\n"
		"struct D : B { int v = 2; int F(char) { return 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.v == 2 && d.B::v == 1) r += 1;\n"
		"	if (d.F(1) == 2) r += 2;\n"
		"	if (d.G() == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//10.2 [class.member.lookup] p10: a name in a virtual base and in a class derived from it: the derived class's dominates
	{
		"DerivedLookupDominance", 7,
		"struct A { int F() { return 1; } int v = 1; };\n"
		"struct B : virtual A { int F() { return 2; } int v = 2; };\n"
		"struct C : virtual A { };\n"
		"struct D : B, C { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.F() == 2) r += 1;\n"
		"	if (d.v == 2) r += 2;\n"
		"	if (d.A::F() == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//10.2 [class.member.lookup] p9: a static member, a type or an enumerator found through two parts of one class is no ambiguity
	{
		"DerivedLookupStaticTwoPaths", 3,
		"struct A { static int s; typedef int T; enum { E = 4 }; };\n"
		"int A::s = 3;\n"
		"struct B : A { };\n"
		"struct C : A { };\n"
		"struct D : B, C { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.s == 3) r += 1;\n"
		"	D::T t = D::E;\n"
		"	if (t == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2, p15: a call through a base pointer or reference calls the final overrider; a qualified call doesn't
	{
		"DerivedVirtualFunction", 15,
		"struct B { virtual int F() { return 1; } int G() { return F(); } virtual ~B() { } };\n"
		"struct D : B { int F() { return 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	B* p = &d;\n"
		"	if (p->F() == 2) r += 1;\n"
		"	B& q = d;\n"
		"	if (q.F() == 2) r += 2;\n"
		"	if (p->B::F() == 1) r += 4;\n"
		"	if (d.G() == 2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2: an overrider is virtual without saying so; the nearest is called
	{
		"DerivedVirtualThroughLevels", 3,
		"struct A { virtual int F() { return 1; } virtual ~A() { } };\n"
		"struct B : A { int F() { return 2; } };\n"
		"struct C : B { };\n"
		"struct E : C { int F() { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	A* p = &c;\n"
		"	if (p->F() == 2) r += 1;\n"
		"	E e;\n"
		"	B* q = &e;\n"
		"	if (q->F() == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2: one function overriding two bases' virtual functions
	{
		"DerivedVirtualTwoBases", 7,
		"struct A { virtual int F() { return 1; } virtual ~A() { } };\n"
		"struct B { virtual int F() { return 2; } virtual ~B() { } };\n"
		"struct C : A, B { int F() { return 3; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	A* pa = &c;\n"
		"	B* pb = &c;\n"
		"	if (pa->F() == 3) r += 1;\n"
		"	if (pb->F() == 3) r += 2;\n"
		"	if (pb->B::F() == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2: a base function that isn't virtual isn't overridden: the static type's is called
	{
		"DerivedNonVirtualNotOverridden", 1,
		"struct B { int F() { return 1; } };\n"
		"struct D : B { int F() { return 2; } };\n"
		"int main() { D d; B* p = &d; return p->F(); }\n"
	},
	//10.2 [class.member.lookup], 10.3 [class.virtual]: an overrider hides the base's other functions of its name
	{
		"DerivedOverrideHidesOverloads", 3,
		"struct B { virtual int F(int) { return 1; } int F(char) { return 2; } virtual ~B() { } };\n"
		"struct D : B { int F(int) { return 3; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.F('c') == 3) r += 1;\n"
		"	B& b = d;\n"
		"	if (b.F('c') == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2: in a diamond, the overrider on one path is the final one
	{
		"DerivedFinalOverriderInDiamond", 1,
		"struct A { virtual int F() { return 1; } virtual ~A() { } };\n"
		"struct B : virtual A { int F() { return 2; } };\n"
		"struct C : virtual A { };\n"
		"struct D : B, C { };\n"
		"int main() { D d; A* p = &d; return (p->F() == 2) ? 1 : 0; }\n"
	},
	//10.3 [class.virtual] p2: two overriders of a virtual base's function, neither over the other
	{
		"DerivedFinalOverriderAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() { return 1; } };\n"
		"struct B : virtual A { int F() { return 2; } };\n"
		"struct C : virtual A { int F() { return 3; } };\n"
		"struct D : B, C { };\n"
		"int main() { D d; return 0; }\n"
	},
	//10.3 [class.virtual] p7: an overrider returning a pointer or reference to a class derived from the overridden's
	{
		"DerivedCovariantReturn", 7,
		"struct B { virtual B* Self() { return this; } virtual B& Ref() { return *this; } virtual int Id() { return 1; } virtual ~B() { } };\n"
		"struct D : B { D* Self() { return this; } D& Ref() { return *this; } int Id() { return 2; } int Only() { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	B* p = &d;\n"
		"	if (p->Self()->Id() == 2) r += 1;\n"
		"	if (p->Ref().Id() == 2) r += 2;\n"
		"	if (d.Self()->Only() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p7, p8: a covariant result through a base that isn't the first: the pointer is adjusted
	{
		"DerivedCovariantAdjusted", 3,
		"struct X { int x = 9; virtual ~X() { } };\n"
		"struct R { int r = 1; virtual ~R() { } };\n"
		"struct S : X, R { int s = 2; };\n"
		"struct B { virtual R* Get() { static R r0; return &r0; } virtual ~B() { } };\n"
		"struct D : B { S ms; S* Get() { return &ms; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	B* p = &d;\n"
		"	R* q = p->Get();\n"
		"	if (q->r == 1) r += 1;\n"
		"	if (q == static_cast<R*>(&d.ms)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p7, p8: a moved covariant result: a null pointer stays null, a reference, a call through the derived class, a class derived again
	{
		"DerivedCovariantAdjustedPaths", 15,
		"struct X { int x = 9; virtual ~X() { } };\n"
		"struct R { int r = 1; virtual ~R() { } };\n"
		"struct S : X, R { int s = 2; };\n"
		"struct B { virtual R* Get(bool b) { return 0; } virtual R& Ref() { static R r0; return r0; } virtual ~B() { } };\n"
		"struct D : B { S ms; S* Get(bool b) { return b ? &ms : 0; } S& Ref() { return ms; } };\n"
		"struct E : D { S other; S* Get(bool b) { return b ? &other : 0; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	B* p = &d;\n"
		"	if (p->Get(false) == 0) r += 1;\n"
		"	if (&p->Ref() == static_cast<R*>(&d.ms)) r += 2;\n"
		"	if (d.Get(true)->s == 2) r += 4;\n"
		"	E e;\n"
		"	B* q = &e;\n"
		"	D* qd = &e;\n"
		"	if (q->Get(true) == static_cast<R*>(&e.other) && qd->Get(true) == &e.other) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p2, p7, 10.4 [class.abstract] p3: what those refusals leave allowed
	{
		"DerivedOverriderRulesKept", 44,
		"struct A { virtual int F() { return 1; } virtual const A* Self() const { return this; } virtual int Id() const { return 0; } };\n"
		"struct B : virtual A { int F() { return 2; } };\n"
		"struct C : virtual A { int F() { return 3; } };\n"
		"struct D : B, C { int F() { return 4; } D* Self() const { return const_cast<D*>(this); } int Id() const { return 8; } };\n"
		"struct P { virtual int G() = 0; };\n"
		"struct Q : P { int G() { return 16; } };\n"
		"int Call(P& p) { return p.G(); }\n"
		"int CallPointer(P* p) { return p->G(); }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	A& a = d;\n"
		"	Q q;\n"
		"	return a.F() + a.Self()->Id() + Call(q) + CallPointer(&q);\n"
		"}\n"
	},
	//10.3 [class.virtual] p7: an overrider returning another type that isn't covariant
	{
		"DerivedReturnNotCovariantRefused", CONFORMANCE_REFUSED,
		"struct B { virtual int F() { return 1; } };\n"
		"struct D : B { long F() { return 2; } };\n"
		"int main() { return 0; }\n"
	},
	//10.3 [class.virtual] p7: nor a pointer to a class not derived from the overridden's
	{
		"DerivedCovariantUnrelatedRefused", CONFORMANCE_REFUSED,
		"struct X { };\n"
		"struct Y { };\n"
		"struct B { virtual X* F() { return 0; } };\n"
		"struct D : B { Y* F() { return 0; } };\n"
		"int main() { return 0; }\n"
	},
	//8.3.6 [dcl.fct.default] p10: a virtual call's default argument is the static type's
	{
		"DerivedDefaultArgumentStaticType", 3,
		"struct B { virtual int F(int x = 1) { return x; } virtual ~B() { } };\n"
		"struct D : B { int F(int x = 2) { return x * 10; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	B* p = &d;\n"
		"	if (p->F() == 10) r += 1;\n"
		"	if (d.F() == 20) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.4 [class.dtor] p9, 10.3 [class.virtual]: deleting through a base pointer runs the derived class's virtual destructor
	{
		"DerivedVirtualDestructor", 3,
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
		"int giLog = 0;\n"
		"struct B { virtual ~B() { giLog = giLog * 10 + 1; } };\n"
		"struct D : B { ~D() { giLog = giLog * 10 + 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B* p = new D;\n"
		"	delete p;\n"
		"	if (giLog == 21) r += 1;\n"
		"	if (giDeletes == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.4 [class.abstract] p2: a pure virtual function with a definition, called with a qualified name
	{
		"DerivedPureWithDefinition", 3,
		"struct A { virtual int F() = 0; virtual ~A() { } };\n"
		"int A::F() { return 1; }\n"
		"struct B : A { int F() { return A::F() + 1; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B b;\n"
		"	A* p = &b;\n"
		"	if (p->F() == 2) r += 1;\n"
		"	if (b.A::F() == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.4 [class.abstract] p4: a class is abstract until all its pure functions are overridden; pointers and references to one are used
	{
		"DerivedAbstractUntilOverridden", 3,
		"struct A { virtual int F() = 0; virtual int G() = 0; virtual ~A() { } };\n"
		"struct B : A { int F() { return 1; } };\n"
		"struct C : B { int G() { return 2; } };\n"
		"int Call(A& a) { return a.F() + a.G(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	A* p = &c;\n"
		"	if (p->G() == 2) r += 1;\n"
		"	if (Call(c) == 3) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//10.4 [class.abstract] p5: a pure function may override one that isn't pure
	{
		"DerivedPureOverridesVirtual", 1,
		"struct A { virtual int F() { return 1; } virtual ~A() { } };\n"
		"struct B : A { int F() = 0; };\n"
		"struct C : B { int F() { return 2; } };\n"
		"int main() { C c; A* p = &c; return (p->F() == 2) ? 1 : 0; }\n"
	},
	//10.4 [class.abstract] p3: no object of an abstract class
	{
		"DerivedAbstractObjectRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() = 0; };\n"
		"int main() { A a; return 0; }\n"
	},
	//10.4 [class.abstract] p4: one overriding only some of the pure functions is abstract
	{
		"DerivedAbstractStillRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() = 0; virtual int G() = 0; };\n"
		"struct B : A { int F() { return 1; } };\n"
		"int main() { B b; return 0; }\n"
	},
	//10.4 [class.abstract] p3: no parameter of an abstract class
	{
		"DerivedAbstractParameterRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() = 0; };\n"
		"int G(A a) { return 0; }\n"
		"int main() { return 0; }\n"
	},
	//10.4 [class.abstract] p3: nor a result
	{
		"DerivedAbstractResultRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() = 0; };\n"
		"A Make();\n"
		"int main() { Make(); return 0; }\n"
	},
	//10.4 [class.abstract] p3: nor an array of one
	{
		"DerivedAbstractArrayRefused", CONFORMANCE_REFUSED,
		"struct A { virtual int F() = 0; };\n"
		"A ga[2];\n"
		"int main() { return 0; }\n"
	},
	//10.2 [class.member.lookup] p6: a type two bases have, named alone, is ambiguous
	{
		"DerivedLookupAmbiguousTypeRefused", CONFORMANCE_REFUSED,
		"struct A { typedef int T; };\n"
		"struct B { typedef int T; };\n"
		"struct C : A, B { int f() { T t = 1; return t; } };\n"
		"int main() { C c; return c.f(); }\n"
	},
	//10.2 [class.member.lookup] p6: a static member two bases have, named alone, is ambiguous
	{
		"DerivedLookupAmbiguousStaticRefused", CONFORMANCE_REFUSED,
		"struct A { static int s; };\n"
		"struct B { static int s; };\n"
		"int A::s = 1;\n"
		"int B::s = 2;\n"
		"struct C : A, B { int f() { return s; } };\n"
		"int main() { C c; return c.f(); }\n"
	},
	//10.2 [class.member.lookup] p6: an enumerator two bases have, named alone, is ambiguous
	{
		"DerivedLookupAmbiguousEnumeratorRefused", CONFORMANCE_REFUSED,
		"struct A { enum { E = 1 }; };\n"
		"struct B { enum { E = 2 }; };\n"
		"struct C : A, B { int f() { return E; } };\n"
		"int main() { C c; return c.f(); }\n"
	},
	//10.2 [class.member.lookup] p6, p9: what the class declares hides its bases'; a qualified name picks one; a shared virtual base's member is one
	{
		"DerivedLookupOneMeaning", 63,
		"struct A { typedef char T; static int s; enum { E = 1 }; };\n"
		"struct B { typedef long T; static int s; enum { E = 2 }; };\n"
		"int A::s = 4;\n"
		"int B::s = 8;\n"
		"struct C : A, B { typedef short T; enum { E = 3 }; int f() { T t = 2; return t + E + A::s + B::E; } };\n"
		"struct V { static int v; enum { W = 5 }; typedef int U; };\n"
		"int V::v = 6;\n"
		"struct L : virtual V { };\n"
		"struct R : virtual V { };\n"
		"struct D : L, R { int g() { U u = v; return u + W; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	D d;\n"
		"	if (c.f() == 11) r += 1;\n"
		"	if (sizeof(C::T) == sizeof(short)) r += 2;\n"
		"	if (d.g() == 11) r += 4;\n"
		"	if (C::A::E == 1 && C::B::E == 2) r += 8;\n"
		"	if (A::s + B::s == 12) r += 16;\n"
		"	if (sizeof(D::U) == sizeof(int)) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//10.3 [class.virtual] p3, 9 [class] p3: a class nested in another, with nothing using it, doesn't derive from a final class
	{
		"DerivedNestedFinalBaseRefused", CONFORMANCE_REFUSED,
		"struct O { struct B final { int x; }; struct D : B { }; };\n"
		"int main() { return 0; }\n"
	},
	//10.3 [class.virtual] p5: override, in a nested class nothing uses, overrides a virtual function
	{
		"DerivedNestedOverrideNothingRefused", CONFORMANCE_REFUSED,
		"struct O { struct B { int f(); }; struct D : B { int f() override; }; };\n"
		"int main() { return 0; }\n"
	},
	//9 [class] p3, 9.8 [class.local]: a local class nothing uses doesn't derive from a final class
	{
		"DerivedLocalFinalBaseRefused", CONFORMANCE_REFUSED,
		"int main() { struct B final { int x; }; struct D : B { }; return 0; }\n"
	},
	//10.3 [class.virtual] p5, 9.8 [class.local]: override, in a local class, overrides a virtual function
	{
		"DerivedLocalOverrideNothingRefused", CONFORMANCE_REFUSED,
		"int main() { struct B { int f() { return 1; } }; struct D : B { int f() override { return 2; } }; return 0; }\n"
	},
	//10.3 [class.virtual] p4, p5: final and override where they are right, in nested and local classes
	{
		"DerivedNestedAndLocalVirtSpecifiers", 7,
		"struct O\n"
		"{\n"
		"	struct B { virtual int f() { return 1; } virtual ~B() { } };\n"
		"	struct D final : B { int f() override final { return 2; } };\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	struct L { virtual int f() { return 3; } virtual ~L() { } };\n"
		"	struct M : L { int f() override { return 4; } };\n"
		"	int r = 0;\n"
		"	O::D d;\n"
		"	O::B* b = &d;\n"
		"	M m;\n"
		"	L* l = &m;\n"
		"	if (b->f() == 2) r += 1;\n"
		"	if (l->f() == 4) r += 2;\n"
		"	if (d.f() + m.f() == 6) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
};


//Clause 11, member access control [class.access].
static const SConformanceCase gasConformanceClause11[] =
{
	//11 [class.access] p2: a class's members are private unless said, a struct's public
	{
		"AccessDefaults", 3,
		"class C { int v = 1; public: int Get() { return v; } };\n"
		"struct S { int v = 2; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (c.Get() == 1) r += 1;\n"
		"	S s;\n"
		"	if (s.v == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11 [class.access] p1: a private member is named only by the class's members and friends
	{
		"AccessPrivateMemberRefused", CONFORMANCE_REFUSED,
		"class C { int v = 1; };\n"
		"int main() { C c; return c.v; }\n"
	},
	//11 [class.access] p1: a private member function too
	{
		"AccessPrivateFunctionRefused", CONFORMANCE_REFUSED,
		"class C { int F() { return 1; } };\n"
		"int main() { C c; return c.F(); }\n"
	},
	//11 [class.access] p4: and a private member type
	{
		"AccessPrivateTypeRefused", CONFORMANCE_REFUSED,
		"class C { typedef int I; };\n"
		"int main() { C::I i = 0; return i; }\n"
	},
	//11 [class.access] p1: and a private static member
	{
		"AccessPrivateStaticRefused", CONFORMANCE_REFUSED,
		"class C { static int s; };\n"
		"int C::s = 1;\n"
		"int main() { return C::s; }\n"
	},
	//11 [class.access] p7: a private static member's definition and its initialiser are in its class's scope
	{
		"AccessPrivateStaticDefinition", 1,
		"class C { static int s; static int Seed() { return 1; } public: static int Get() { return s; } };\n"
		"int C::s = Seed();\n"
		"int main() { return C::Get(); }\n"
	},
	//11 [class.access] p4: access is to names: a public typedef of a private type is named
	{
		"AccessPublicTypedefOfPrivate", 1,
		"class C { struct P { int v = 1; }; public: typedef P Pub; };\n"
		"int main() { C::Pub p; return p.v; }\n"
	},
	//11.1 [class.access.spec] p1, p2: access specifiers in any order and repeated, each until the next
	{
		"AccessSpecifiersRepeated", 7,
		"class C\n"
		"{\n"
		"public:\n"
		"	int a = 1;\n"
		"private:\n"
		"	int b = 2;\n"
		"public:\n"
		"	int c = 4;\n"
		"	int B() { return b; }\n"
		"protected:\n"
		"	int d = 8;\n"
		"public:\n"
		"	int D() { return d; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (c.a == 1) r += 1;\n"
		"	if (c.B() == 2) r += 2;\n"
		"	if (c.c == 4 && c.D() == 8) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//11 [class.access] p1: a member reaches the private members of other objects of its class
	{
		"AccessOtherObjectOfClass", 3,
		"class C { int v; public: C(int x) : v(x) { } int Sum(const C& o) const { return v + o.v; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C a(1);\n"
		"	C b(2);\n"
		"	if (a.Sum(b) == 3) r += 1;\n"
		"	r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.2 [class.access.base] p1: a public base's public members are public in the derived class, its protected ones protected
	{
		"AccessPublicBase", 3,
		"struct B { int pub = 1; protected: int pro = 2; };\n"
		"struct D : public B { int Pro() { return pro; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.pub == 1) r += 1;\n"
		"	if (d.Pro() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.2 [class.access.base] p1: a protected base's public members are protected
	{
		"AccessProtectedBaseRefused", CONFORMANCE_REFUSED,
		"struct B { int pub = 1; };\n"
		"struct D : protected B { };\n"
		"int main() { D d; return d.pub; }\n"
	},
	//11.2 [class.access.base] p1: a private base's public members are private
	{
		"AccessPrivateBaseRefused", CONFORMANCE_REFUSED,
		"struct B { int pub = 1; };\n"
		"struct D : private B { };\n"
		"int main() { D d; return d.pub; }\n"
	},
	//11.2 [class.access.base] p1: a private base's members are used inside the derived class
	{
		"AccessPrivateBaseInside", 3,
		"struct B { int pub = 1; int F() { return 2; } };\n"
		"class D : private B { public: int Get() { return pub + F(); } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.Get() == 3) r += 1;\n"
		"	r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.2 [class.access.base] p2: a struct's bases are public unless said
	{
		"AccessStructBasePublic", 1,
		"struct B { int v = 1; };\n"
		"struct D : B { };\n"
		"int main() { D d; return d.v; }\n"
	},
	//11.2 [class.access.base] p2: a class's bases are private unless said
	{
		"AccessClassBasePrivateRefused", CONFORMANCE_REFUSED,
		"struct B { int v = 1; };\n"
		"class D : B { };\n"
		"int main() { D d; return d.v; }\n"
	},
	//11.2 [class.access.base] p5, 4.10 [conv.ptr] p3: no conversion to an inaccessible base
	{
		"AccessInaccessibleBaseConversionRefused", CONFORMANCE_REFUSED,
		"struct B { int v; };\n"
		"class D : private B { };\n"
		"int main() { D d; B* p = &d; return 0; }\n"
	},
	//11.2 [class.access.base] p5: inside the class its private base is accessible
	{
		"AccessBaseConversionInside", 1,
		"struct B { int v = 1; };\n"
		"class D : private B { public: B* Base() { return this; } };\n"
		"int main() { D d; return d.Base()->v; }\n"
	},
	//11.2 [class.access.base] p1: a private base's members aren't reached from a class derived from the derived one
	{
		"AccessPrivateBaseFromGrandchildRefused", CONFORMANCE_REFUSED,
		"struct B { int v = 1; };\n"
		"struct D : private B { };\n"
		"struct E : D { int Get() { return v; } };\n"
		"int main() { E e; return e.Get(); }\n"
	},
	//11.1 [class.access.spec] p5: a base's name found through a private base's injected name is inaccessible
	{
		"AccessInjectedNameThroughPrivateBaseRefused", CONFORMANCE_REFUSED,
		"class A { };\n"
		"class B : private A { };\n"
		"class C : public B { A* p; };\n"
		"int main() { return 0; }\n"
	},
	//11.1 [class.access.spec] p5: the same class named from the global scope is accessible
	{
		"AccessGlobalNameOfPrivateBase", 1,
		"class A { public: int v = 1; };\n"
		"class B : private A { };\n"
		"class C : public B { public: int Get() { ::A a; return a.v; } };\n"
		"int main() { C c; return c.Get(); }\n"
	},
	//11.3 [class.friend] p1, p6: a friend function reaches private members; one defined in its class
	{
		"AccessFriendFunction", 7,
		"class C { int v = 3; friend int Peek(const C& c); friend int Twice(const C& c) { return c.v * 2; } };\n"
		"int Peek(const C& c) { return c.v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (Peek(c) == 3) r += 1;\n"
		"	if (Twice(c) == 6) r += 2;\n"
		"	r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//11.3 [class.friend] p7, 7.3.1.2 [namespace.memdef] p3: a friend defined in its class isn't found by ordinary lookup
	{
		"AccessFriendDefinedInClassNotFoundRefused", CONFORMANCE_REFUSED,
		"class C { friend int Get() { return 1; } };\n"
		"int main() { return Get(); }\n"
	},
	//11.3 [class.friend] p2: a friend class's members reach the private members
	{
		"AccessFriendClass", 3,
		"class C { int v = 4; friend class F; friend struct G; };\n"
		"class F { public: int Get(const C& c) { return c.v; } };\n"
		"struct G { static int Twice(const C& c) { return c.v * 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	F f;\n"
		"	if (f.Get(c) == 4) r += 1;\n"
		"	if (G::Twice(c) == 8) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.3 [class.friend] p10: friendship isn't transitive
	{
		"AccessFriendNotTransitiveRefused", CONFORMANCE_REFUSED,
		"class A { int v = 1; friend class B; };\n"
		"class B { friend class C; };\n"
		"class C { public: int Get(const A& a) { return a.v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.3 [class.friend] p10: nor inherited
	{
		"AccessFriendNotInheritedRefused", CONFORMANCE_REFUSED,
		"class A { int v = 1; friend class B; };\n"
		"class B { };\n"
		"class D : public B { public: int Get(const A& a) { return a.v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.3 [class.friend] p4: a member function of another class made a friend
	{
		"AccessFriendMemberFunction", 1,
		"class C;\n"
		"struct M { int Get(const C& c); };\n"
		"class C { int v = 5; friend int M::Get(const C&); };\n"
		"int M::Get(const C& c) { return c.v; }\n"
		"int main() { C c; M m; return (m.Get(c) == 5) ? 1 : 0; }\n"
	},
	//11.3 [class.friend], 14.5.4 [temp.friend] p1: a function template and a class template made friends
	{
		"AccessFriendTemplates", 3,
		"class C { int v = 2; template<class T> friend int Peek(const T& t); template<class T> friend struct Box; };\n"
		"template<class T> int Peek(const T& t) { return t.v; }\n"
		"template<class T> struct Box { static int Get(const T& t) { return t.v * 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (Peek(c) == 2) r += 1;\n"
		"	if (Box<C>::Get(c) == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.4 [class.protected] p1: a protected member reached through an object of the derived class; a protected static one through the base
	{
		"AccessProtectedThroughDerived", 3,
		"struct B { protected: int v = 1; static int s; };\n"
		"int B::s = 2;\n"
		"struct D : B { int Get(D& o) { return o.v; } int Static() { return B::s; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.Get(d) == 1) r += 1;\n"
		"	if (d.Static() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//11.4 [class.protected] p1, 7.3.3 [namespace.udecl] p17: what those refusals leave allowed
	{
		"AccessProtectedRulesKept", 63,
		"struct B\n"
		"{\n"
		"protected:\n"
		"	int v = 1;\n"
		"	static int s;\n"
		"	B() { }\n"
		"	int Get() const { return v; }\n"
		"public:\n"
		"	virtual ~B() { }\n"
		"};\n"
		"int B::s = 2;\n"
		"struct D : B\n"
		"{\n"
		"	using B::Get;\n"
		"	D() : B() { v = 4; }\n"
		"	int Own(D& o) { return o.v + o.Get(); }\n"
		"	int Further(struct E& e);\n"
		"	int Static() { return B::s; }\n"
		"	int Pointer() { int D::* p = &D::v; return this->*p; }\n"
		"};\n"
		"struct E : D { E() { v = 16; } };\n"
		"int D::Further(E& e) { return e.v; }\n"
		"int main()\n"
		"{\n"
		"	D d;\n"
		"	E e;\n"
		"	return d.Own(d) + d.Further(e) + d.Static() + d.Pointer() + d.Get() + 29;\n"
		"}\n"
	},
	//11.4 [class.protected] p1, 11.2 [class.access.base] p5: B::F() through this, and a friend of D through a D
	{
		"AccessProtectedQualifiedAndFriend", 5,
		"struct B { protected: int v = 4; int F() { return 1; } };\n"
		"struct D : B { friend struct Fixer; int G() { return B::F() + B::v - 2; } };\n"
		"struct Fixer { static int Read(D* p) { return p->v; } };\n"
		"int main() { D d; return d.G() + Fixer::Read(&d) - 2; }\n"
	},
	//11.4 [class.protected] p1: not through an object of the base class
	{
		"AccessProtectedThroughBaseRefused", CONFORMANCE_REFUSED,
		"struct B { protected: int v = 1; };\n"
		"struct D : B { int Get(B& o) { return o.v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.4 [class.protected] p1: nor of another class derived from the base
	{
		"AccessProtectedThroughSiblingRefused", CONFORMANCE_REFUSED,
		"struct B { protected: int v = 1; };\n"
		"struct D1 : B { };\n"
		"struct D2 : B { int Get(D1& o) { return o.v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.4 [class.protected] p1: a pointer to a protected member formed through the derived class
	{
		"AccessProtectedMemberPointer", 1,
		"struct B { protected: int v = 1; };\n"
		"struct D : B { static int D::* Ptr() { return &D::v; } };\n"
		"int main() { D d; return d.*D::Ptr(); }\n"
	},
	//8.3.3 [dcl.mptr] p1, 8.3.5 [dcl.fct]: functions that return a pointer to a member, written without a typedef: a free function, a member function and a static one
	{
		"AccessMemberPointerResults", 7,
		"struct B { int v = 1; int w = 2; int B::* Mine() { return &B::w; } static int B::* Static() { return &B::v; } };\n"
		"int B::* Free() { return &B::v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B b;\n"
		"	if (b.*Free() == 1) r += 1;\n"
		"	if (b.*b.Mine() == 2) r += 2;\n"
		"	if (b.*B::Static() == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//11.4 [class.protected] p1: not through the base class
	{
		"AccessProtectedMemberPointerBaseRefused", CONFORMANCE_REFUSED,
		"struct B { protected: int v = 1; };\n"
		"struct D : B { static int B::* Ptr() { return &B::v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.5 [class.access.virt] p1: a virtual function's access is the static type's: a private overrider called through a public base's
	{
		"AccessVirtualByStaticType", 1,
		"struct B { virtual int F() { return 1; } virtual ~B() { } };\n"
		"class D : public B { int F() { return 2; } };\n"
		"int main() { D d; B* p = &d; return (p->F() == 2) ? 1 : 0; }\n"
	},
	//11.5 [class.access.virt] p1: through the derived class its private overrider isn't accessible
	{
		"AccessPrivateOverriderRefused", CONFORMANCE_REFUSED,
		"struct B { virtual int F() { return 1; } };\n"
		"class D : public B { int F() { return 2; } };\n"
		"int main() { D d; return d.F(); }\n"
	},
	//11.6 [class.paths] p1: a name reached by several paths has the access of the most accessible
	{
		"AccessMostAccessiblePath", 1,
		"struct V { int v = 1; };\n"
		"struct A : virtual V { };\n"
		"struct B : private virtual V { };\n"
		"struct C : A, B { };\n"
		"int main() { C c; return c.v; }\n"
	},
	//11.7 [class.access.nest] p1: the enclosing class has no special access to a nested class's private members
	{
		"AccessEnclosingToNestedPrivateRefused", CONFORMANCE_REFUSED,
		"class Outer { class In { int v = 1; }; public: int Get() { In i; return i.v; } };\n"
		"int main() { return 0; }\n"
	},
	//11.7 [class.access.nest], 11 [class.access] p4: a private nested class isn't named outside
	{
		"AccessPrivateNestedClassRefused", CONFORMANCE_REFUSED,
		"class Outer { struct In { int v; }; };\n"
		"int main() { Outer::In i; return 0; }\n"
	},
	//11 [class.access] p8: names in a default argument are checked where it is written
	{
		"AccessDefaultArgument", 1,
		"class C { static int Def() { return 1; } public: static int F(int x = Def()) { return x; } };\n"
		"int main() { return C::F(); }\n"
	},
	//11 [class.access] p4: a private type named as a template argument outside its class
	{
		"AccessPrivateTemplateArgumentRefused", CONFORMANCE_REFUSED,
		"class C { struct P { }; };\n"
		"template<class T> struct W { };\n"
		"W<C::P> w;\n"
		"int main() { return 0; }\n"
	},
	//7.3.3 [namespace.udecl] p17, 11 [class.access]: a using-declaration doesn't make a base's private member accessible
	{
		"AccessUsingDeclarationOfPrivateRefused", CONFORMANCE_REFUSED,
		"struct B { private: int v = 1; };\n"
		"struct D : B { using B::v; };\n"
		"int main() { return 0; }\n"
	},
	//11 [class.access]: a private constructor isn't used outside the class
	{
		"AccessPrivateConstructorRefused", CONFORMANCE_REFUSED,
		"class C { C() { } public: static C Make() { return C(); } };\n"
		"int main() { C c; return 0; }\n"
	},
	//11 [class.access] p1: a private constructor used by the class's own static member function
	{
		"AccessPrivateConstructorFactory", 1,
		"class C { int v; C(int x) : v(x) { } public: static C Make(int x) { return C(x); } int Get() const { return v; } };\n"
		"int main() { return C::Make(1).Get(); }\n"
	},
	//11 [class.access], 12.4 [class.dtor] p11: an object whose destructor is private isn't made outside the class
	{
		"AccessPrivateDestructorRefused", CONFORMANCE_REFUSED,
		"class C { ~C() { } };\n"
		"int main() { C c; return 0; }\n"
	},
};


//Clause 12, special member functions [special].
static const SConformanceCase gasConformanceClause12[] =
{
	//12.1 [class.ctor] p4, p7, 8.5 [dcl.init] p8: the implicit default constructor makes members with their own constructors; value-initialisation zeroes the rest
	{
		"SpecialImplicitDefaultConstructor", 7,
		"struct M { int v; M() : v(5) { } };\n"
		"struct S { M m; int i; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.m.v == 5) r += 1;\n"
		"	S z = S();\n"
		"	if (z.i == 0 && z.m.v == 5) r += 2;\n"
		"	S w{};\n"
		"	if (w.i == 0) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.1 [class.ctor] p4: the implicit default constructor is deleted where a reference member has no initialiser
	{
		"SpecialDefaultConstructorReferenceRefused", CONFORMANCE_REFUSED,
		"struct S { int& r; };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.1 [class.ctor] p4: or a const member of a type with no default constructor of its own
	{
		"SpecialDefaultConstructorConstRefused", CONFORMANCE_REFUSED,
		"struct S { const int k; };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.1 [class.ctor] p4: or a member has no default constructor
	{
		"SpecialDefaultConstructorMemberRefused", CONFORMANCE_REFUSED,
		"struct M { M(int) { } };\n"
		"struct S { M m; };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.1 [class.ctor] p4: a class that declares a constructor has no implicit default constructor
	{
		"SpecialNoImplicitDefaultRefused", CONFORMANCE_REFUSED,
		"struct S { S(int) { } };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.1 [class.ctor] p4, 8.4.2 [dcl.fct.def.default]: = default brings the default constructor back
	{
		"SpecialDefaultedConstructor", 3,
		"struct S { int v = 3; S() = default; S(int x) : v(x) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.v == 3) r += 1;\n"
		"	S t(4);\n"
		"	if (t.v == 4) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//8.4.3 [dcl.fct.def.delete] p2: a deleted constructor isn't used
	{
		"SpecialDeletedConstructorRefused", CONFORMANCE_REFUSED,
		"struct S { S() = delete; };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.1 [class.ctor] p4: a constructor isn't virtual
	{
		"SpecialConstructorVirtualRefused", CONFORMANCE_REFUSED,
		"struct S { virtual S() { } };\n"
		"int main() { return 0; }\n"
	},
	//12.1 [class.ctor] p4: nor static
	{
		"SpecialConstructorStaticRefused", CONFORMANCE_REFUSED,
		"struct S { static S() { } };\n"
		"int main() { return 0; }\n"
	},
	//12.1 [class.ctor] p6: nor has a return type
	{
		"SpecialConstructorReturnTypeRefused", CONFORMANCE_REFUSED,
		"struct S { int S() { return 0; } };\n"
		"int main() { return 0; }\n"
	},
	//12.1 [class.ctor] p5: a literal class's implicit default constructor is constexpr
	{
		"SpecialConstexprImplicitConstructor", 1,
		"struct P { int a = 1; int b = 2; };\n"
		"constexpr P p = P();\n"
		"int main()\n"
		"{\n"
		"	static_assert(p.a + p.b == 3, \"\");\n"
		"	return 1;\n"
		"}\n"
	},
	//12.2 [class.temporary] p3, p5: a temporary is destroyed at the end of its full expression, the last made first
	{
		"SpecialTemporaryLifetime", 7,
		"int giLog = 0;\n"
		"struct T { int i; T(int x) : i(x) { } ~T() { giLog = giLog * 10 + i; } int Get() const { return i; } };\n"
		"int Sum(const T& a, const T& b) { return a.Get() + b.Get(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int s = Sum(T(1), T(2));\n"
		"	if (s == 3) r += 1;\n"
		"	if (giLog == 21 || giLog == 12) r += 2;\n"
		"	giLog = 0;\n"
		"	int t = T(3).Get() + ((giLog == 0) ? 1 : 0);\n"
		"	if (t == 4 && giLog == 3) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.2 [class.temporary] p5: a temporary bound to a reference, or the temporary a member of which is bound, lives as long as the reference
	{
		"SpecialTemporaryBoundToReference", 7,
		"int giGone = 0;\n"
		"struct T { int i; T(int x) : i(x) { } ~T() { giGone++; } };\n"
		"struct H { T t; H(int x) : t(x) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		const T& a = T(1);\n"
		"		if (giGone == 0 && a.i == 1) r += 1;\n"
		"		T&& b = T(2);\n"
		"		if (giGone == 0 && b.i == 2) r += 2;\n"
		"		const T& c = H(3).t;\n"
		"		if (giGone == 0 && c.i == 3) r += 4;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//12.3.1 [class.conv.ctor] p1: a constructor taking one argument converts it: in initialisation, for an argument, in an assignment
	{
		"SpecialConvertingConstructor", 7,
		"struct N { int v; N(int x) : v(x) { } };\n"
		"int Get(N n) { return n.v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	N a = 4;\n"
		"	if (a.v == 4) r += 1;\n"
		"	if (Get(5) == 5) r += 2;\n"
		"	a = 6;\n"
		"	if (a.v == 6) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.3.1 [class.conv.ctor] p2, 13.5.3 [over.ass]: an assignment doesn't make its temporary with an explicit constructor
	{
		"SpecialAssignThroughExplicitRefused", CONFORMANCE_REFUSED,
		"struct N { explicit N(int x) : v(x) { } int v; };\n"
		"int main() { N a(1); a = 6; return a.v; }\n"
	},
	//12.3.1 [class.conv.ctor] p2: an explicit constructor isn't used for copy initialisation
	{
		"SpecialExplicitCopyInitialisationRefused", CONFORMANCE_REFUSED,
		"struct N { explicit N(int x) : v(x) { } int v; };\n"
		"int main() { N a = 4; return a.v; }\n"
	},
	//12.3.1 [class.conv.ctor] p2: nor for an argument
	{
		"SpecialExplicitArgumentRefused", CONFORMANCE_REFUSED,
		"struct N { explicit N(int x) : v(x) { } int v; };\n"
		"int Get(N n) { return n.v; }\n"
		"int main() { return Get(5); }\n"
	},
	//12.3.1 [class.conv.ctor] p2: an explicit constructor in direct initialisation and a cast
	{
		"SpecialExplicitDirect", 3,
		"struct N { explicit N(int x) : v(x) { } int v; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	N a(4);\n"
		"	if (a.v == 4) r += 1;\n"
		"	N b = static_cast<N>(5);\n"
		"	if (b.v == 5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.3.2 [class.conv.fct] p1: a conversion function for an initialisation, arithmetic and an argument
	{
		"SpecialConversionFunction", 7,
		"struct M { int v; operator int() const { return v; } };\n"
		"int Twice(int x) { return 2 * x; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	M m = { 3 };\n"
		"	int i = m;\n"
		"	if (i == 3) r += 1;\n"
		"	if (m + 1 == 4) r += 2;\n"
		"	if (Twice(m) == 6) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.3.2 [class.conv.fct] p2: an explicit conversion function in a condition and a cast
	{
		"SpecialExplicitConversionFunction", 3,
		"struct B { int v; explicit operator bool() const { return v != 0; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	B b = { 1 };\n"
		"	if (b) r += 1;\n"
		"	bool x = static_cast<bool>(b);\n"
		"	if (x && !B{0}) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.3.2 [class.conv.fct] p2: but not copy initialisation
	{
		"SpecialExplicitConversionCopyInitialisationRefused", CONFORMANCE_REFUSED,
		"struct B { int v; explicit operator bool() const { return v != 0; } };\n"
		"int main() { B b = { 1 }; bool x = b; return x; }\n"
	},
	//12.3 [class.conv] p4: at most one user-defined conversion in a sequence
	{
		"SpecialTwoUserConversionsRefused", CONFORMANCE_REFUSED,
		"struct A { A(int) { } };\n"
		"struct B { B(A) { } };\n"
		"int Take(B) { return 1; }\n"
		"int main() { return Take(1); }\n"
	},
	//12.3 [class.conv] p4, 13.3.3.1.2 [over.ics.user]: a user-defined conversion followed by a standard one
	{
		"SpecialUserThenStandardConversion", 1,
		"struct M { operator short() const { return 2; } };\n"
		"long Get(long x) { return x; }\n"
		"int main() { M m; return (Get(m) == 2) ? 1 : 0; }\n"
	},
	//12.3.2 [class.conv.fct] p1: a conversion function has no return type written
	{
		"SpecialConversionFunctionReturnTypeRefused", CONFORMANCE_REFUSED,
		"struct M { int operator int() { return 1; } };\n"
		"int main() { return 0; }\n"
	},
	//12.3.2 [class.conv.fct] p1: and no parameter
	{
		"SpecialConversionFunctionParameterRefused", CONFORMANCE_REFUSED,
		"struct M { operator int(int) { return 1; } };\n"
		"int main() { return 0; }\n"
	},
	//12.4 [class.dtor] p8: the destructor's body, then the members last declared first, then the bases last first
	{
		"SpecialDestructionOrder", 1,
		"char gacLog[8];\n"
		"int giLog = 0;\n"
		"void Log(char c) { gacLog[giLog++] = c; }\n"
		"struct L { char c; L(char x) : c(x) { } ~L() { Log(c); } };\n"
		"struct B1 { ~B1() { Log('7'); } };\n"
		"struct B2 { ~B2() { Log('8'); } };\n"
		"struct S : B1, B2 { L a; L b; S() : a('1'), b('2') { } ~S() { Log('9'); } };\n"
		"int main()\n"
		"{\n"
		"	{ S s; }\n"
		"	return (giLog == 5 && gacLog[0] == '9' && gacLog[1] == '2' && gacLog[2] == '1' && gacLog[3] == '8' && gacLog[4] == '7') ? 1 : 0;\n"
		"}\n"
	},
	//12.4 [class.dtor] p10, 12.6 [class.init]: an array's elements are destroyed the last first
	{
		"SpecialArrayDestroyedLastFirst", 1,
		"int giLog = 0;\n"
		"struct L { int i; ~L() { giLog = giLog * 10 + i; } };\n"
		"int main()\n"
		"{\n"
		"	{ L a[3] = { { 1 }, { 2 }, { 3 } }; }\n"
		"	return (giLog == 321) ? 1 : 0;\n"
		"}\n"
	},
	//12.4 [class.dtor] p13: a destructor called explicitly, on a union's member
	{
		"SpecialExplicitDestructorCall", 3,
		"int giGone = 0;\n"
		"struct D { int v; D(int x) : v(x) { } ~D() { giGone++; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		union U { D d; U() : d(4) { } ~U() { } } u;\n"
		"		if (u.d.v == 4) r += 1;\n"
		"		u.d.~D();\n"
		"	}\n"
		"	if (giGone == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.4 [class.dtor] p11: no object of a class whose destructor is deleted is made where it would be destroyed
	{
		"SpecialDeletedDestructorRefused", CONFORMANCE_REFUSED,
		"struct S { ~S() = delete; };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.4 [class.dtor] p2: a destructor takes no parameter
	{
		"SpecialDestructorParameterRefused", CONFORMANCE_REFUSED,
		"struct S { ~S(int) { } };\n"
		"int main() { return 0; }\n"
	},
	//12.4 [class.dtor] p9: a pure virtual destructor is defined, and the derived class's calls it
	{
		"SpecialPureVirtualDestructor", 1,
		"int giLog = 0;\n"
		"struct A { virtual ~A() = 0; };\n"
		"A::~A() { giLog += 1; }\n"
		"struct B : A { ~B() { giLog += 10; } };\n"
		"int main()\n"
		"{\n"
		"	{ B b; }\n"
		"	return (giLog == 11) ? 1 : 0;\n"
		"}\n"
	},
	//12.5 [class.free] p1, p4: a class's operator new and delete make and free its objects, static whether said or not; ::new uses the global one
	{
		"SpecialClassOperatorNew", 15,
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
		"int giClassNews = 0;\n"
		"int giClassDeletes = 0;\n"
		"struct S { int v; void* operator new(size_t n) { giClassNews++; return Allocate(n); } void operator delete(void* p) { if (p) giClassDeletes++; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S* p = new S;\n"
		"	if (giClassNews == 1 && giNews == 1) r += 1;\n"
		"	delete p;\n"
		"	if (giClassDeletes == 1 && giDeletes == 0) r += 2;\n"
		"	int* q = new int;\n"
		"	if (giClassNews == 1 && giNews == 2) r += 4;\n"
		"	S* g = ::new S;\n"
		"	if (giClassNews == 1 && giNews == 3) r += 8;\n"
		"	::delete g;\n"
		"	delete q;\n"
		"	return r;\n"
		"}\n"
	},
	//12.5 [class.free] p2: a class's operator new[] and delete[] for its arrays
	{
		"SpecialClassOperatorNewArray", 3,
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
		"int giArrays = 0;\n"
		"struct S { int v; void* operator new[](size_t n) { giArrays++; return Allocate(n); } void operator delete[](void* p) { giArrays += 10; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S* p = new S[3];\n"
		"	if (giArrays == 1) r += 1;\n"
		"	delete[] p;\n"
		"	if (giArrays == 11) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.5 [class.free] p4: deleting through a base with a virtual destructor uses the derived class's operator delete
	{
		"SpecialVirtualDestructorDeleteOperator", 1,
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
		"int giWhich = 0;\n"
		"struct B { virtual ~B() { } void operator delete(void* p) { giWhich = 1; } };\n"
		"struct D : B { void operator delete(void* p) { giWhich = 2; } };\n"
		"int main()\n"
		"{\n"
		"	B* p = new D;\n"
		"	delete p;\n"
		"	return (giWhich == 2) ? 1 : 0;\n"
		"}\n"
	},
	//12.5 [class.free] p5: a class's operator delete with a size is given the object's size
	{
		"SpecialSizedClassDelete", 1,
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
		"size_t guiFreed = 0;\n"
		"struct S { int a; int b; void operator delete(void* p, size_t n) { guiFreed = n; } };\n"
		"int main()\n"
		"{\n"
		"	S* p = new S;\n"
		"	delete p;\n"
		"	return (guiFreed == sizeof(S)) ? 1 : 0;\n"
		"}\n"
	},
	//12.6 [class.init] p2, 8.5.1 [dcl.init.aggr] p7: an array's objects made in order, by the constructors their initialisers choose and the default one for the rest
	{
		"SpecialArrayOfObjects", 7,
		"int giLog = 0;\n"
		"struct E { int v; E() : v(0) { giLog = giLog * 10 + 1; } E(int x) : v(x) { giLog = giLog * 10 + 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	E a[3] = { 5, 6 };\n"
		"	if (a[0].v == 5 && a[1].v == 6 && a[2].v == 0) r += 1;\n"
		"	if (giLog == 221) r += 2;\n"
		"	E b[2];\n"
		"	if (b[1].v == 0) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p11: bases first, then members in the order they are declared, whatever the list's order
	{
		"SpecialMemInitialiserOrder", 1,
		"int giLog = 0;\n"
		"struct L { L(int i) { giLog = giLog * 10 + i; } };\n"
		"struct B { B() { giLog = giLog * 10 + 9; } };\n"
		"struct S : B { L a; L b; S() : b(2), a(1) { } };\n"
		"int main() { S s; return (giLog == 912) ? 1 : 0; }\n"
	},
	//12.6.2 [class.base.init] p11: a member's initialiser uses one made before it
	{
		"SpecialMemInitialiserUsesEarlier", 1,
		"struct S { int a; int b; S(int x) : a(x), b(a * 2) { } };\n"
		"int main() { S s(3); return (s.b == 6) ? 1 : 0; }\n"
	},
	//12.6.2 [class.base.init] p6: a constructor that delegates runs the other first; the object is made once that one returns
	{
		"SpecialDelegatingConstructor", 7,
		"int giLog = 0;\n"
		"struct S\n"
		"{\n"
		"	int a;\n"
		"	int b;\n"
		"	S(int x, int y) : a(x), b(y) { giLog = giLog * 10 + 1; }\n"
		"	S(int x) : S(x, x * 2) { giLog = giLog * 10 + 2; }\n"
		"	S() : S(5) { giLog = giLog * 10 + 3; }\n"
		"	~S() { giLog = giLog * 10 + 4; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		S s;\n"
		"		if (s.a == 5 && s.b == 10) r += 1;\n"
		"		if (giLog == 123) r += 2;\n"
		"	}\n"
		"	if (giLog == 1234) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p6: a delegating constructor's mem-initialiser is its only one
	{
		"SpecialDelegatingWithOthersRefused", CONFORMANCE_REFUSED,
		"struct S { int a; S(int x) : a(x) { } S() : S(1), a(2) { } };\n"
		"int main() { return 0; }\n"
	},
	//12.6.2 [class.base.init] p9: a mem-initialiser takes the place of the member's default initialiser, which isn't run
	{
		"SpecialMemInitialiserOverDefault", 3,
		"int giRuns = 0;\n"
		"int Count() { return ++giRuns; }\n"
		"struct S { int a = Count(); S() { } S(int x) : a(x) { } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S t(7);\n"
		"	if (t.a == 7 && giRuns == 0) r += 1;\n"
		"	S s;\n"
		"	if (s.a == 1 && giRuns == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p7, p10: a virtual base is made by the most derived class; the classes between's mem-initialisers for it aren't used
	{
		"SpecialVirtualBaseMostDerived", 3,
		"struct V { int v; V(int x = 0) : v(x) { } };\n"
		"struct A : virtual V { A() : V(1) { } };\n"
		"struct B : virtual V { B() : V(2) { } };\n"
		"struct D : A, B { D() : V(3) { } };\n"
		"struct E : A, B { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.v == 3) r += 1;\n"
		"	E e;\n"
		"	if (e.v == 0) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.6.2 [class.base.init] p11: the virtual bases are made first, then the direct bases in order, then the class
	{
		"SpecialVirtualBasesFirst", 1,
		"int giLog = 0;\n"
		"struct V { V() { giLog = giLog * 10 + 1; } };\n"
		"struct A { A() { giLog = giLog * 10 + 2; } };\n"
		"struct B : virtual V { B() { giLog = giLog * 10 + 3; } };\n"
		"struct D : A, B { D() { giLog = giLog * 10 + 4; } };\n"
		"int main() { D d; return (giLog == 1234) ? 1 : 0; }\n"
	},
	//12.6.2 [class.base.init] p8: a constructor initialises a reference member
	{
		"SpecialReferenceMemberUninitialisedRefused", CONFORMANCE_REFUSED,
		"struct S { int& r; S() { } };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.6.2 [class.base.init] p8: and a const member of a type with no default constructor of its own
	{
		"SpecialConstMemberUninitialisedRefused", CONFORMANCE_REFUSED,
		"struct S { const int k; S() { } };\n"
		"int main() { S s; return 0; }\n"
	},
	//12.6.2 [class.base.init] p2: a mem-initialiser names a member or a base
	{
		"SpecialMemInitialiserNotMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int a; S() : b(1) { } };\n"
		"int main() { return 0; }\n"
	},
	//12.6.2 [class.base.init] p4: a member is named once in the list
	{
		"SpecialMemInitialiserTwiceRefused", CONFORMANCE_REFUSED,
		"struct S { int a; S() : a(1), a(2) { } };\n"
		"int main() { return 0; }\n"
	},
	//12.7 [class.cdtor] p4: a virtual call in a constructor or destructor is the function of the class being made or destroyed
	{
		"SpecialVirtualCallInConstructor", 7,
		"int giLog = 0;\n"
		"struct B { B() { giLog = giLog * 10 + Id(); } virtual ~B() { giLog = giLog * 10 + Id(); } virtual int Id() { return 1; } };\n"
		"struct D : B { D() { giLog = giLog * 10 + Id(); } ~D() { giLog = giLog * 10 + Id(); } int Id() { return 2; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	{\n"
		"		D d;\n"
		"		if (giLog == 12) r += 1;\n"
		"		B* p = &d;\n"
		"		if (p->Id() == 2) r += 2;\n"
		"	}\n"
		"	if (giLog == 1221) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.7 [class.cdtor] p6: dynamic_cast in a constructor sees the class being made, not the one derived from it
	{
		"SpecialDynamicCastInConstructor", 3,
		"struct B { int seen; B(); virtual ~B() { } };\n"
		"struct D : B { D() { } };\n"
		"B::B() { seen = dynamic_cast<D*>(this) ? 1 : 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.seen == 2) r += 1;\n"
		"	B* p = &d;\n"
		"	if (dynamic_cast<D*>(p) == &d) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.8 [class.copy] p7, p15, p18, p28: the implicit copy constructor and assignment copy each base and member, an array element by element
	{
		"SpecialImplicitCopy", 15,
		"struct B { int b; };\n"
		"struct S : B { int a[3]; char c; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.b = 1;\n"
		"	s.a[0] = 2;\n"
		"	s.a[2] = 3;\n"
		"	s.c = 'x';\n"
		"	S t(s);\n"
		"	if (t.b == 1 && t.a[2] == 3) r += 1;\n"
		"	S u;\n"
		"	u = s;\n"
		"	if (u.a[0] == 2 && u.c == 'x') r += 2;\n"
		"	t.a[2] = 9;\n"
		"	if (s.a[2] == 3) r += 4;\n"
		"	S v = s;\n"
		"	if (v.b == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//12.8 [class.copy] p15, p28: the implicit copies call the members' copy constructor and assignment
	{
		"SpecialImplicitCopyCallsMembers", 3,
		"int giCopies = 0;\n"
		"int giAssigns = 0;\n"
		"struct M { int v; M() : v(0) { } M(const M& o) : v(o.v) { giCopies++; } M& operator=(const M& o) { v = o.v; giAssigns++; return *this; } };\n"
		"struct S { M m; int i; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	s.m.v = 4;\n"
		"	S t(s);\n"
		"	if (t.m.v == 4 && giCopies == 1) r += 1;\n"
		"	S u;\n"
		"	u = s;\n"
		"	if (u.m.v == 4 && giAssigns == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.8 [class.copy] p7: a class that declares a move constructor has its copy constructor deleted
	{
		"SpecialMoveDeletesCopyRefused", CONFORMANCE_REFUSED,
		"struct S { S() { } S(S&&) { } };\n"
		"int main() { S a; S b(a); return 0; }\n"
	},
	//12.8 [class.copy] p18: and one that declares a move assignment its copy assignment
	{
		"SpecialMoveAssignDeletesCopyAssignRefused", CONFORMANCE_REFUSED,
		"struct S { S() { } S& operator=(S&&) { return *this; } };\n"
		"int main() { S a; S b; a = b; return 0; }\n"
	},
	//12.8 [class.copy] p9: a class that declares a copy constructor has no implicit move one: an rvalue is copied
	{
		"SpecialCopyDeclaredNoImplicitMove", 1,
		"int giCopies = 0;\n"
		"struct S { int v; S(int x) : v(x) { } S(const S& o) : v(o.v) { giCopies++; } };\n"
		"int main()\n"
		"{\n"
		"	S a(2);\n"
		"	S b(static_cast<S&&>(a));\n"
		"	return (b.v == 2 && giCopies == 1) ? 1 : 0;\n"
		"}\n"
	},
	//12.8 [class.copy] p9, p15: the implicit move constructor moves each member, by the member's own move constructor
	{
		"SpecialImplicitMove", 3,
		"int giMoves = 0;\n"
		"int giCopies = 0;\n"
		"struct M { M() { } M(const M&) { giCopies++; } M(M&&) { giMoves++; } };\n"
		"struct S { M m; int i; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a;\n"
		"	a.i = 3;\n"
		"	S b(static_cast<S&&>(a));\n"
		"	if (b.i == 3 && giMoves == 1 && giCopies == 0) r += 1;\n"
		"	S c(b);\n"
		"	if (giCopies == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.8 [class.copy] p32: a returned local is taken as an rvalue first: moved, or the copy left out, never copied
	{
		"SpecialReturnLocalMoved", 1,
		"int giCopies = 0;\n"
		"struct S { int v; S(int x) : v(x) { } S(const S& o) : v(o.v) { giCopies++; } S(S&& o) : v(o.v) { } };\n"
		"S Make() { S s(4); return s; }\n"
		"int main() { S t = Make(); return (t.v == 4 && giCopies == 0) ? 1 : 0; }\n"
	},
	//12.8 [class.copy] p32: a local of a class that can only be moved is returned
	{
		"SpecialMoveOnlyReturned", 1,
		"struct U { int v; U(int x) : v(x) { } U(U&& o) : v(o.v) { } U(const U&) = delete; };\n"
		"U Make() { U u(5); return u; }\n"
		"int main() { U t = Make(); return (t.v == 5) ? 1 : 0; }\n"
	},
	//8.4.3 [dcl.fct.def.delete] p2: a deleted copy constructor isn't used
	{
		"SpecialDeletedCopyRefused", CONFORMANCE_REFUSED,
		"struct S { S() { } S(const S&) = delete; };\n"
		"int main() { S a; S b = a; return 0; }\n"
	},
	//12.1 [class.ctor], 12.3.2 [class.conv.fct], 12.4 [class.dtor], 12.6.2 [class.base.init], 12.8 [class.copy]: what those refusals leave allowed
	{
		"SpecialMemberRulesKept", 127,
		"struct B { int b; B(int x) : b(x) { } };\n"
		"struct S : B\n"
		"{\n"
		"	int& r;\n"
		"	const int k;\n"
		"	const int d = 4;\n"
		"	S(int& x) : B(1), r(x), k(2) { }\n"
		"	S(int& x, int) : S(x) { }\n"
		"	explicit operator bool() const { return k == 2; }\n"
		"	~S(void) { }\n"
		"};\n"
		"struct A { const int k; int& r; };\n"
		"struct M { int v; M() : v(8) { } M(const M&) = delete; M(M&& o) : v(o.v) { } };\n"
		"struct H { M m; };\n"
		"struct C { M m; C() { } C(const C&) { } };\n"
		"int main()\n"
		"{\n"
		"	int x = 16;\n"
		"	S s(x, 0);\n"
		"	A a = { 32, x };\n"
		"	H h1;\n"
		"	H h2(static_cast<H&&>(h1));\n"
		"	C c1;\n"
		"	C c2(c1);\n"
		"	int r = s.b + s.k + s.d + h2.m.v + s.r + a.k + c2.m.v - 8 + 64;\n"
		"	return (s && a.r == 16) ? r : 0;\n"
		"}\n"
	},
	//12.8 [class.copy] p11: a member that can't be copied deletes the implicit copy constructor
	{
		"SpecialCopyDeletedByMemberRefused", CONFORMANCE_REFUSED,
		"struct M { M() { } M(const M&) = delete; };\n"
		"struct S { M m; };\n"
		"int main() { S a; S b(a); return 0; }\n"
	},
	//12.8 [class.copy] p23: a reference member deletes the implicit copy assignment
	{
		"SpecialAssignDeletedByReferenceRefused", CONFORMANCE_REFUSED,
		"struct S { int& r; };\n"
		"int main() { int x = 1; int y = 2; S a = { x }; S b = { y }; a = b; return 0; }\n"
	},
	//12.8 [class.copy] p28: an object assigned to itself keeps its value
	{
		"SpecialSelfAssignment", 1,
		"struct S { int a[2]; };\n"
		"int main() { S s = { { 3, 4 } }; s = s; return (s.a[0] == 3 && s.a[1] == 4) ? 1 : 0; }\n"
	},
	//12.8 [class.copy] p22, p28: the implicit copy assignment returns the object assigned to
	{
		"SpecialCopyAssignmentResult", 1,
		"struct S { int v; };\n"
		"int main()\n"
		"{\n"
		"	S a = { 1 };\n"
		"	S b = { 2 };\n"
		"	S c = { 3 };\n"
		"	a = b = c;\n"
		"	return (a.v == 3 && &(a = b) == &a) ? 1 : 0;\n"
		"}\n"
	},
	//12.9 [class.inhctor] p1, p3: using Base::Base inherits its constructors, one for each default argument left off
	{
		"SpecialInheritingConstructors", 7,
		"struct B { int a; int b; B(int x, int y = 7) : a(x), b(y) { } };\n"
		"struct D : B { using B::B; int c = 9; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d(1);\n"
		"	if (d.a == 1 && d.b == 7) r += 1;\n"
		"	D e(2, 3);\n"
		"	if (e.b == 3) r += 2;\n"
		"	if (d.c == 9) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//12.9 [class.inhctor] p3: the derived class keeps its implicit default constructor
	{
		"SpecialInheritingDefault", 3,
		"struct B { int v; B() : v(4) { } B(int x) : v(x) { } };\n"
		"struct D : B { using B::B; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D a;\n"
		"	if (a.v == 4) r += 1;\n"
		"	D b(6);\n"
		"	if (b.v == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//12.9 [class.inhctor] p3: a copy constructor isn't inherited
	{
		"SpecialInheritingCopyNotInheritedRefused", CONFORMANCE_REFUSED,
		"struct B { B() { } B(const B&) { } };\n"
		"struct D : B { using B::B; };\n"
		"int main() { B b; D d(b); return 0; }\n"
	},
	//12.9 [class.inhctor] p4: an inherited constructor is explicit if the base's is
	{
		"SpecialInheritingExplicitRefused", CONFORMANCE_REFUSED,
		"struct B { explicit B(int) { } };\n"
		"struct D : B { using B::B; };\n"
		"int main() { D d = 1; return 0; }\n"
	},
	//12.9 [class.inhctor] p4: an inherited constructor is constexpr if the base's is
	{
		"SpecialInheritingConstexpr", 1,
		"struct B { int v; constexpr B(int x) : v(x) { } };\n"
		"struct D : B { using B::B; };\n"
		"constexpr D d(3);\n"
		"int main()\n"
		"{\n"
		"	static_assert(d.v == 3, \"\");\n"
		"	return 1;\n"
		"}\n"
	},
	//12.9 [class.inhctor] p3: a constructor is left out only for one of the class's with the same signature: const int* is not int*
	{
		"SpecialInheritingConstBelowNotHidden", 1,
		"struct B { int v; B(int* p) : v(1) {} };\n"
		"struct D : B { using B::B; D(const int* p) : B((int*)0) { v = 2; } };\n"
		"int main() { int x; int* p = &x; D d(p); return d.v; }\n"
	},
};


//Clause 13, overloading [over].
static const SConformanceCase gasConformanceClause13[] =
{
	//13.1 [over.load] p2: function declarations that differ only in their return type can't be overloaded
	{
		"OverLoadReturnTypeOnlyRefused", CONFORMANCE_REFUSED,
		"int F(int a) { return a; }\n"
		"long F(int a) { return a; }\n"
		"int main() { return F(1); }\n"
	},
	//13.1 [over.load] p2: nor can a static and a non-static member function with the same parameter types
	{
		"OverLoadStaticMemberRefused", CONFORMANCE_REFUSED,
		"struct S { static int F(int a) { return a; } int F(int a) { return a + 1; } };\n"
		"int main() { S s; return s.F(1); }\n"
	},
	//13.1 [over.load] p2: a static and a non-static member function with other parameter types overload
	{
		"OverLoadStaticMemberOverloads", 7,
		"struct S\n"
		"{\n"
		"	static int F(int) { return 1; }\n"
		"	int F(long) { return 2; }\n"
		"	int G() const { return 4; }\n"
		"	static int G(int) { return 0; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (S::F(1) == 1) r += 1;\n"
		"	if (s.F(1L) == 2) r += 2;\n"
		"	if (s.G() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.1 [over.load] p3: parameter types that differ only by a typedef, by * or [], by an array's bound, by a function type or a pointer to one, by a top-level const, or by a default argument declare the same function
	{
		"OverLoadEquivalentDeclarations", 63,
		"typedef int Int;\n"
		"int A(Int a);\n"
		"int A(int a) { return a + 1; }\n"
		"int B(int a[]);\n"
		"int B(int* a) { return a[1]; }\n"
		"int C(int f(int));\n"
		"int C(int (*f)(int)) { return f(3); }\n"
		"int D(const int a);\n"
		"int D(int a) { a++; return a; }\n"
		"int E(int a, int b);\n"
		"int E(int a, int b = 4) { return a + b; }\n"
		"int G(int a[3]);\n"
		"int G(int a[5]) { return a[0]; }\n"
		"int Twice(int a) { return a * 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int ai[2] = { 7, 2 };\n"
		"	if (A(1) == 2) r += 1;\n"
		"	if (B(ai) == 2) r += 2;\n"
		"	if (C(Twice) == 6) r += 4;\n"
		"	if (D(4) == 5) r += 8;\n"
		"	if (E(1) == 5) r += 16;\n"
		"	if (G(ai) == 7) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.1 [over.load] p3: so defining both is defining one function twice
	{
		"OverLoadArrayPointerRedefinitionRefused", CONFORMANCE_REFUSED,
		"int B(int a[]) { return a[0]; }\n"
		"int B(int* a) { return a[1]; }\n"
		"int main() { int ai[2] = { 1, 2 }; return B(ai); }\n"
	},
	//13.1 [over.load] p3: a top-level const on a parameter doesn't make another function
	{
		"OverLoadConstParameterRedefinitionRefused", CONFORMANCE_REFUSED,
		"int D(int a) { return a; }\n"
		"int D(const int a) { return a + 1; }\n"
		"int main() { return D(1); }\n"
	},
	//13.1 [over.load] p3: const under a pointer or a reference does make another function
	{
		"OverLoadDistinctQualifiedParameters", 15,
		"int F(int*) { return 1; }\n"
		"int F(const int*) { return 2; }\n"
		"int G(int&) { return 4; }\n"
		"int G(const int&) { return 8; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = 0;\n"
		"	const int c = 0;\n"
		"	if (F(&i) == 1) r += 1;\n"
		"	if (F(&c) == 2) r += 2;\n"
		"	if (G(i) == 4) r += 4;\n"
		"	if (G(c) == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.1 [over.load] p3 note: functions told apart only by a defaulted parameter overload, and a call both take is ambiguous
	{
		"OverLoadDefaultArgumentAmbiguousRefused", CONFORMANCE_REFUSED,
		"int F(int a) { return a; }\n"
		"int F(int a, int b = 2) { return a + b; }\n"
		"int main() { return F(1); }\n"
	},
	//13.1 [over.load] p3 note: the same functions, called where only one fits
	{
		"OverLoadDefaultArgumentOverloads", 3,
		"int F(int a) { return a; }\n"
		"int F(int a, int b = 2) { return a + b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1, 2) == 3) r += 1;\n"
		"	int (*p)(int) = F;\n"
		"	if (p(2) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.2 [over.dcl] p1: a derived class's member function hides the base's of its name rather than overloading them
	{
		"OverDclDerivedHidesBaseRefused", CONFORMANCE_REFUSED,
		"struct B { int F(int) { return 1; } };\n"
		"struct D : B { int F(const char*) { return 2; } };\n"
		"int main() { D d; return d.F(1); }\n"
	},
	//13.2 [over.dcl] p1: the base's are named qualified, or brought in by a using-declaration
	{
		"OverDclDerivedHides", 15,
		"struct B { int F(int) { return 1; } int F(long) { return 8; } };\n"
		"struct D : B { int F(const char*) { return 2; } };\n"
		"struct U : B { using B::F; int F(const char*) { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d.B::F(1) == 1) r += 1;\n"
		"	if (d.F(\"x\") == 2) r += 2;\n"
		"	U u;\n"
		"	if (u.F(\"x\") == 4) r += 4;\n"
		"	if (u.F(1L) == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.2 [over.dcl] p2: a function declared in a block hides the outer ones of its name
	{
		"OverDclBlockDeclarationHidesRefused", CONFORMANCE_REFUSED,
		"int F(int a) { return a; }\n"
		"int F(const char*) { return 2; }\n"
		"int main() { extern int F(const char*); return F(1); }\n"
	},
	//13.2 [over.dcl] p2: the outer ones are found outside that block
	{
		"OverDclBlockDeclaration", 3,
		"int F(int a) { return a; }\n"
		"int F(const char*) { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1) == 1) r += 1;\n"
		"	{\n"
		"		extern int F(const char*);\n"
		"		if (F(\"x\") == 2) r += 2;\n"
		"	}\n"
		"	return r;\n"
		"}\n"
	},
	//13.2 [over.dcl] p3, 11 [class.access] p4: access is checked on the function chosen, after it is chosen
	{
		"OverDclAccessAfterChoiceRefused", CONFORMANCE_REFUSED,
		"struct S { int F(int) { return 1; } private: int F(long) { return 2; } };\n"
		"int main() { S s; return s.F(1L); }\n"
	},
	//13.2 [over.dcl] p3, 11 [class.access] p4: a private overload that isn't chosen is no error
	{
		"OverDclAccessAfterChoice", 3,
		"struct S { int F(int) { return 1; } int F(short) { return 2; } private: int F(long) { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s.F(1) == 1) r += 1;\n"
		"	if (s.F((short)1) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1 [over.match.funcs] p4, p5: the implicit object parameter: & for an lvalue, && for an rvalue, const for a const object; without a ref-qualifier an rvalue binds to it too; a static member function matches any object
	{
		"OverFuncsImplicitObjectParameter", 127,
		"struct S\n"
		"{\n"
		"	int F() & { return 1; }\n"
		"	int F() && { return 2; }\n"
		"	int G() { return 4; }\n"
		"	int G() const { return 8; }\n"
		"	int H() const { return 16; }\n"
		"	static int K(int) { return 32; }\n"
		"	int K(long) { return 0; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	const S cs = s;\n"
		"	if (s.F() == 1) r += 1;\n"
		"	if (S().F() == 2) r += 2;\n"
		"	if (s.G() == 4) r += 4;\n"
		"	if (cs.G() == 8) r += 8;\n"
		"	if (S().H() == 16) r += 16;\n"
		"	if (s.K(1) == 32) r += 32;\n"
		"	if (S().G() == 4) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1 [over.match.funcs] p4: a const object doesn't bind to a non-const member function's implicit object parameter
	{
		"OverFuncsConstObjectRefused", CONFORMANCE_REFUSED,
		"struct S { int G() { return 4; } };\n"
		"int main() { S s; const S cs = s; return cs.G(); }\n"
	},
	//13.3.1 [over.match.funcs] p4: an rvalue doesn't bind to a & member function's
	{
		"OverFuncsRvalueOnLvalueQualifiedRefused", CONFORMANCE_REFUSED,
		"struct S { int F() & { return 1; } };\n"
		"int main() { return S().F(); }\n"
	},
	//13.3.1.1.1 [over.call.func] p3: an unqualified call in a static member function offers the non-static functions too, and only choosing one is an error
	{
		"OverFuncsStaticContext", 3,
		"struct S\n"
		"{\n"
		"	static int F(int) { return 1; }\n"
		"	int F(long) { return 2; }\n"
		"	static int G() { return F(1); }\n"
		"	int H() { return F(1L); }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (S::G() == 1) r += 1;\n"
		"	S s;\n"
		"	if (s.H() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.1.1 [over.call.func] p3: a non-static member function chosen where there is no object
	{
		"OverFuncsStaticContextRefused", CONFORMANCE_REFUSED,
		"struct S\n"
		"{\n"
		"	static int F(int) { return 1; }\n"
		"	int F(long) { return 2; }\n"
		"	static int G() { return F(1L); }\n"
		"};\n"
		"int main() { return S::G(); }\n"
	},
	//13.3.1.1.2 [over.call.object] p1-p3: an object that is called offers its operator() functions and, for each conversion function to a pointer to function, a surrogate that calls through the pointer
	{
		"OverCallObjectSurrogate", 15,
		"typedef int (*FI)(int);\n"
		"int Twice(int a) { return a * 2; }\n"
		"struct P { operator FI() const { return Twice; } };\n"
		"struct R { int operator()(int) { return 4; } operator FI() { return Twice; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	P p;\n"
		"	if (p(3) == 6) r += 1;\n"
		"	const P cp = p;\n"
		"	if (cp(4) == 8) r += 2;\n"
		"	R q;\n"
		"	if (q(1) == 4) r += 4;\n"
		"	FI f = p;\n"
		"	if (f(5) == 10) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.1.2 [over.call.object], 13.3.3 [over.match.best] p2: operator()(long) is better for the object, the surrogate for the argument: ambiguous
	{
		"OverCallObjectAmbiguousRefused", CONFORMANCE_REFUSED,
		"typedef int (*FI)(int);\n"
		"int Twice(int a) { return a * 2; }\n"
		"struct Q { int operator()(long) { return 1; } operator FI() { return Twice; } };\n"
		"int main() { Q q; return q(2); }\n"
	},
	//13.3.1.2 [over.match.oper] p3, p6, p7: an operator's candidates are the left operand's member functions, the non-member functions found, and the built-in operators
	{
		"OverOperatorCandidates", 63,
		"struct S\n"
		"{\n"
		"	int operator+(int) const { return 1; }\n"
		"};\n"
		"int operator+(const S&, long) { return 2; }\n"
		"int operator-(int, const S&) { return 4; }\n"
		"enum E { A, B };\n"
		"int operator|(E, E) { return 8; }\n"
		"struct C { operator int() const { return 15; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s + 1 == 1) r += 1;\n"
		"	if (s + 1L == 2) r += 2;\n"
		"	if (1 - s == 4) r += 4;\n"
		"	if ((A | B) == 8) r += 8;\n"
		"	C c;\n"
		"	if (c + 1 == 16) r += 16;\n"
		"	if (A + B == 1) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.2 [over.match.oper], 13.3.3 [over.match.best] p2: a member and a non-member operator function as good as each other are ambiguous
	{
		"OverOperatorAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct S { int operator+(int) const { return 1; } };\n"
		"int operator+(const S&, int) { return 2; }\n"
		"int main() { S s; return s + 1; }\n"
	},
	//13.3.1.2 [over.match.oper] p3: member candidates come from the left operand only
	{
		"OverOperatorMemberLeftOperandRefused", CONFORMANCE_REFUSED,
		"struct S { int operator+(int) const { return 1; } };\n"
		"int main() { S s; return 1 + s; }\n"
	},
	//13.5 [over.oper] p6: an operator function has a parameter of class or enumeration type
	{
		"OverOperatorNoClassOperandRefused", CONFORMANCE_REFUSED,
		"int operator+(int a, int b) { return a - b; }\n"
		"int main() { return 1 + 2; }\n"
	},
	//13.3.1.3 [over.match.ctor] p1: direct initialisation offers every constructor, copy initialisation only the converting ones
	{
		"OverCtorCopyInitialisationConverting", 7,
		"struct S { explicit S(int) : v(1) {} S(long) : v(2) {} int v; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a(1);\n"
		"	if (a.v == 1) r += 1;\n"
		"	S b = 1;\n"
		"	if (b.v == 2) r += 2;\n"
		"	S c = S(1);\n"
		"	if (c.v == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.4 [over.match.copy], 13.3.3.2 [over.ics.rank] p3: copying a T into an S offers S's converting constructors and T's conversion functions; a T binds better to T& than to const T&
	{
		"OverCopyInitConversionFunctionChosen", 3,
		"struct T;\n"
		"struct S { int v; S(const T&) : v(1) {} S(int a) : v(a) {} };\n"
		"struct T { operator S() { return S(2); } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	T t;\n"
		"	S a = t;\n"
		"	if (a.v == 2) r += 1;\n"
		"	const T ct = t;\n"
		"	S b = ct;\n"
		"	if (b.v == 1) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.4 [over.match.copy], 13.3.3 [over.match.best] p2: with operator S() const the two are as good as each other
	{
		"OverCopyInitConversionAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct T;\n"
		"struct S { int v; S(const T&) : v(1) {} S(int a) : v(a) {} };\n"
		"struct T { operator S() const { return S(2); } };\n"
		"int main() { T t; S a = t; return a.v; }\n"
	},
	//13.3.1.5 [over.match.conv], 13.3.3 [over.match.best] p1: a non-class object initialised from a class object takes the conversion function whose result converts best
	{
		"OverConvFunctionNonClass", 15,
		"struct S { operator int() const { return 1; } operator double() const { return 2; } operator char() const { return 4; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	int i = s;\n"
		"	double d = s;\n"
		"	char c = s;\n"
		"	if (i == 1) r += 1;\n"
		"	if (d == 2) r += 2;\n"
		"	if (c == 4) r += 4;\n"
		"	if ((int)s == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.5 [over.match.conv], 13.3.3 [over.match.best] p2: a long from operator int or operator double, a conversion either way, is ambiguous
	{
		"OverConvFunctionAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct S { operator int() const { return 1; } operator double() const { return 2; } };\n"
		"int main() { S s; long l = s; return (int)l; }\n"
	},
	//13.3.1.6 [over.match.ref]: a reference bound to what a conversion function returns by reference
	{
		"OverConvFunctionReference", 3,
		"int g = 1;\n"
		"struct S { operator int&() { return g; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	int& ri = s;\n"
		"	ri = 5;\n"
		"	if (g == 5) r += 1;\n"
		"	const int& cr = s;\n"
		"	g = 6;\n"
		"	if (cr == 6) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.7 [over.match.list] p1: list initialisation tries the initializer_list constructors first, then all of them; empty braces take the default constructor
	{
		"OverListInitInitializerListFirst", 15,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	S() : v(1) {}\n"
		"	S(int, int) : v(2) {}\n"
		"	S(std::initializer_list<int>) : v(4) {}\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a{};\n"
		"	if (a.v == 1) r += 1;\n"
		"	S b{ 1, 2 };\n"
		"	if (b.v == 4) r += 2;\n"
		"	S c(1, 2);\n"
		"	if (c.v == 2) r += 4;\n"
		"	S d = { 3 };\n"
		"	if (d.v == 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.1.7 [over.match.list] p1, 8.5.4 [dcl.init.list] p3: the initializer_list constructor is chosen, and narrowing in it is an error, though another constructor would take the values
	{
		"OverListInitNarrowingRefused", CONFORMANCE_REFUSED,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"struct S { int v; S(double, int) : v(2) {} S(std::initializer_list<int>) : v(4) {} };\n"
		"int main() { S b{ 1.5, 2 }; return b.v; }\n"
	},
	//13.3.1.7 [over.match.list] p1: in copy list initialisation, choosing an explicit constructor is an error
	{
		"OverListInitExplicitRefused", CONFORMANCE_REFUSED,
		"struct E { int v; explicit E(int a) : v(a) {} };\n"
		"int main() { E e = { 1 }; return e.v; }\n"
	},
	//13.3.1.7 [over.match.list] p1: direct list initialisation may choose an explicit constructor
	{
		"OverListInitExplicitDirect", 3,
		"struct E { int v; explicit E(int a) : v(a) {} };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	E e{ 1 };\n"
		"	if (e.v == 1) r += 1;\n"
		"	E f(2);\n"
		"	if (f.v == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.2 [over.match.viable] p2: a function is viable with as many arguments as parameters, fewer where the rest have defaults, more where it ends with an ellipsis; an ellipsis is the worst match
	{
		"OverViableArgumentCounts", 31,
		"int F(int) { return 1; }\n"
		"int F(int, int, ...) { return 2; }\n"
		"int G(long) { return 4; }\n"
		"int G(...) { return 8; }\n"
		"int H(int a, int b = 15) { return a + b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1) == 1) r += 1;\n"
		"	if (F(1, 2, 3, 4) == 2) r += 2;\n"
		"	if (G(1) == 4) r += 4;\n"
		"	if (G() == 8) r += 8;\n"
		"	if (H(1) == 16) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3 [over.match.best] p1: with conversions as good, a function that isn't a template's specialisation beats one that is, and of two templates' the more specialised wins
	{
		"OverBestTemplateAndNonTemplate", 31,
		"template<class T> int F(T) { return 1; }\n"
		"int F(int) { return 2; }\n"
		"template<class T> int G(T) { return 4; }\n"
		"template<class T> int G(T*) { return 8; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1) == 2) r += 1;\n"
		"	if (F('a') == 1) r += 2;\n"
		"	if (F<>(1) == 1) r += 4;\n"
		"	int i = 0;\n"
		"	if (G(&i) == 8) r += 8;\n"
		"	if (G(i) == 4) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3 [over.match.best] p2: a call where neither function is better for every argument is ambiguous
	{
		"OverBestAmbiguousRefused", CONFORMANCE_REFUSED,
		"int F(int, long) { return 1; }\n"
		"int F(long, int) { return 2; }\n"
		"int main() { return F(1, 1); }\n"
	},
	//13.3.3 [over.match.best] p1: a function is better when no argument converts worse for it and one converts better
	{
		"OverBestEachArgumentNoWorse", 3,
		"int F(int, long) { return 1; }\n"
		"int F(long, long) { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1, 1) == 1) r += 1;\n"
		"	if (F(1L, 1) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.1.1 [over.ics.scs], 13.3.3.2 [over.ics.rank] p4: an exact match beats a promotion, a promotion beats a conversion
	{
		"OverRankPromotionsAndConversions", 127,
		"int F(int) { return 1; }\n"
		"int F(long) { return 2; }\n"
		"int G(double) { return 4; }\n"
		"int G(int) { return 8; }\n"
		"int H(long) { return 16; }\n"
		"int H(double) { return 32; }\n"
		"enum E { A };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	short sh = 1;\n"
		"	if (F('a') == 1 && F(sh) == 1 && F(true) == 1) r += 1;\n"
		"	if (F(1L) == 2) r += 2;\n"
		"	if (G(1.0f) == 4) r += 4;\n"
		"	if (G(sh) == 8) r += 8;\n"
		"	if (F(A) == 1) r += 16;\n"
		"	unsigned char uc = 1;\n"
		"	if (F(uc) == 1) r += 32;\n"
		"	if (H(1.0f) == 32) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.2 [over.ics.rank] p4: two conversions of one rank are ambiguous
	{
		"OverRankConversionsAmbiguousRefused", CONFORMANCE_REFUSED,
		"int H(long) { return 1; }\n"
		"int H(double) { return 2; }\n"
		"int main() { return H(1); }\n"
	},
	//13.3.3.1.1 [over.ics.scs] p3, 13.3.3.2 [over.ics.rank] p3: lvalue to rvalue, array to pointer, function to pointer and qualification conversions are exact matches, and the smaller qualification the better
	{
		"OverRankExactMatchCategory", 15,
		"int F(const char*) { return 1; }\n"
		"int F(bool) { return 0; }\n"
		"int Fn(int a) { return a; }\n"
		"int G(int (*)(int)) { return 2; }\n"
		"int G(bool) { return 0; }\n"
		"int H(const volatile int*) { return 0; }\n"
		"int H(const int*) { return 4; }\n"
		"int K(int) { return 8; }\n"
		"int K(long) { return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	char arr[2] = { 0, 0 };\n"
		"	if (F(arr) == 1) r += 1;\n"
		"	if (G(Fn) == 2) r += 2;\n"
		"	int i = 0;\n"
		"	if (H(&i) == 4) r += 4;\n"
		"	if (K(i) == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.2 [over.ics.rank] p4: a pointer to bool is worse than another conversion; to a base is better than to void*; to a nearer base better than to a farther one, by pointer or by reference; a pointer to member the other way round
	{
		"OverRankPointerConversions", 63,
		"struct A { int m; };\n"
		"struct B : A {};\n"
		"struct C : B {};\n"
		"int F(bool) { return 1; }\n"
		"int F(const void*) { return 2; }\n"
		"int G(A*) { return 4; }\n"
		"int G(void*) { return 0; }\n"
		"int H(A*) { return 0; }\n"
		"int H(B*) { return 8; }\n"
		"int K(A&) { return 0; }\n"
		"int K(B&) { return 16; }\n"
		"int M(int B::*) { return 32; }\n"
		"int M(int C::*) { return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	C c;\n"
		"	if (F(&c) == 2) r += 1;\n"
		"	if (G(&c) == 4) r += 2;\n"
		"	if (H(&c) == 8) r += 4;\n"
		"	if (K(c) == 16) r += 8;\n"
		"	if (M(&A::m) == 32) r += 16;\n"
		"	B b;\n"
		"	if (G(&b) == 4) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.2 [over.ics.rank] p3: an rvalue binds better to && than to const&, an lvalue better to & than to const&; a less qualified pointer target is better
	{
		"OverRankReferenceBindings", 127,
		"int F(const int&) { return 1; }\n"
		"int F(int&&) { return 2; }\n"
		"int G(int&) { return 4; }\n"
		"int G(const int&) { return 8; }\n"
		"int H(int*) { return 16; }\n"
		"int H(const int*) { return 32; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = 0;\n"
		"	const int ci = 0;\n"
		"	if (F(1) == 2) r += 1;\n"
		"	if (F(i) == 1) r += 2;\n"
		"	if (G(i) == 4) r += 4;\n"
		"	if (G(ci) == 8) r += 8;\n"
		"	if (H(&i) == 16) r += 16;\n"
		"	if (H(&ci) == 32) r += 32;\n"
		"	if (G(1) == 8) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.2 [over.ics.rank] p2, p3: a standard conversion beats a user-defined one, which beats an ellipsis; two by one conversion function are told apart by the standard conversion after it
	{
		"OverRankUserDefined", 7,
		"struct S { operator short() const { return 1; } };\n"
		"int F(int) { return 1; }\n"
		"int F(double) { return 2; }\n"
		"struct U { U(int) {} };\n"
		"int H(U) { return 0; }\n"
		"int H(long) { return 2; }\n"
		"int K(U) { return 4; }\n"
		"int K(...) { return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (F(s) == 1) r += 1;\n"
		"	if (H(1) == 2) r += 2;\n"
		"	if (K(1) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.1 [over.best.ics] p10, 13.3.3.2 [over.ics.rank] p3: two conversion functions that each give a conversion to the parameter make an ambiguous conversion
	{
		"OverRankUserDefinedAmbiguousRefused", CONFORMANCE_REFUSED,
		"struct T { operator int() const { return 1; } operator long() const { return 2; } };\n"
		"int G(double d) { return (int)d; }\n"
		"int main() { T t; return G(t); }\n"
	},
	//13.3.3.1.5 [over.ics.list], 13.3.3.2 [over.ics.rank] p3: a braced list converts to initializer_list<X> better than to anything else; a list of one element converts as the element; to an aggregate it is a user-defined conversion
	{
		"OverRankListInitialisation", 15,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"struct P { int a, b; };\n"
		"int F(std::initializer_list<int>) { return 1; }\n"
		"int F(int) { return 0; }\n"
		"int G(P) { return 2; }\n"
		"int G(int) { return 4; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F({ 1, 2 }) == 1) r += 1;\n"
		"	if (F({ 1 }) == 1) r += 2;\n"
		"	if (G({ 1, 2 }) == 2) r += 4;\n"
		"	if (G({ 1 }) == 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.4 [over.over] p1: an overloaded function's name chooses the one of the type the target wants: a pointer initialised or assigned, a parameter, a result, a cast, a member pointer, a reference
	{
		"OverAddressByTarget", 255,
		"int F(int) { return 1; }\n"
		"int F(long) { return 2; }\n"
		"int Call(int (*f)(int)) { return f(0); }\n"
		"int (*Get())(long) { return F; }\n"
		"struct S { int G(int) { return 4; } int G(long) { return 8; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int (*p)(long) = F;\n"
		"	if (p(0) == 2) r += 1;\n"
		"	int (*q)(int);\n"
		"	q = &F;\n"
		"	if (q(0) == 1) r += 2;\n"
		"	if (Call(F) == 1) r += 4;\n"
		"	if (Get()(0) == 2) r += 8;\n"
		"	if (((int (*)(long))F)(0) == 2) r += 16;\n"
		"	if (static_cast<int (*)(int)>(F)(0) == 1) r += 32;\n"
		"	int (S::*m)(long) = &S::G;\n"
		"	S s;\n"
		"	if ((s.*m)(0) == 8) r += 64;\n"
		"	int (&ref)(int) = F;\n"
		"	if (ref(0) == 1) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//13.4 [over.over] p2, p4: a function template's specialisation is deduced from the target, and a function that isn't a template is preferred
	{
		"OverAddressTemplates", 15,
		"template<class T> int H(T) { return 1; }\n"
		"int H(int) { return 2; }\n"
		"template<class T> int K(T*) { return 4; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int (*a)(int) = H;\n"
		"	if (a(0) == 2) r += 1;\n"
		"	int (*b)(char) = H;\n"
		"	if (b(0) == 1) r += 2;\n"
		"	int (*c)(char) = &H<char>;\n"
		"	if (c(0) == 1) r += 4;\n"
		"	int (*d)(int*) = K;\n"
		"	if (d(0) == 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//13.4 [over.over] p1: no function of the target's type
	{
		"OverAddressNoMatchRefused", CONFORMANCE_REFUSED,
		"int F(int) { return 1; }\n"
		"int F(long) { return 2; }\n"
		"int main() { int (*p)(double) = F; return p(0); }\n"
	},
	//13.4 [over.over] p1, 7.1.6.4 [dcl.spec.auto]: without a target type the name chooses nothing
	{
		"OverAddressNoTargetRefused", CONFORMANCE_REFUSED,
		"int F(int) { return 1; }\n"
		"int F(long) { return 2; }\n"
		"int main() { auto p = F; return p(0); }\n"
	},
	//13.5.1 [over.unary], 13.5.2 [over.binary], 13.5 [over.oper] p4: a unary operator is a member with no parameter or a non-member with one, a binary one a member with one or a non-member with two; -, * and & are either; an operator function is called by its name too
	{
		"OverOperatorUnaryAndBinary", 127,
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	int operator-() const { return 1; }\n"
		"	int operator-(const S&) const { return 2; }\n"
		"	int operator*() const { return 4; }\n"
		"};\n"
		"int operator!(const S&) { return 8; }\n"
		"int operator*(const S&, int) { return 16; }\n"
		"int operator&(const S&, const S&) { return 32; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { 0 };\n"
		"	if (-s == 1) r += 1;\n"
		"	if (s - s == 2) r += 2;\n"
		"	if (*s == 4) r += 4;\n"
		"	if (!s == 8) r += 8;\n"
		"	if (s * 1 == 16) r += 16;\n"
		"	if ((s & s) == 32) r += 32;\n"
		"	if (s.operator-() + operator!(s) == 9) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5 [over.oper] p8: an operator function has no default arguments
	{
		"OverOperatorDefaultArgumentRefused", CONFORMANCE_REFUSED,
		"struct S { int operator+(int a = 1) const { return a; } };\n"
		"int main() { S s; return s + 2; }\n"
	},
	//13.5 [over.oper] p8, 13.5.2 [over.binary]: a binary operator that is a member has one parameter
	{
		"OverOperatorArityRefused", CONFORMANCE_REFUSED,
		"struct S { int operator+(int a, int b) const { return a + b; } };\n"
		"int main() { S s; return s + 1; }\n"
	},
	//13.5.4 [over.call] p1: operator() may have any number of parameters, and default arguments
	{
		"OverOperatorCallDefaultArgument", 7,
		"struct S\n"
		"{\n"
		"	int operator()(int a, int b = 2) const { return a + b; }\n"
		"	int operator()() const { return 4; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	if (s(1) == 3) r += 1;\n"
		"	if (s(3, 3) == 6) r += 2;\n"
		"	if (s() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5 [over.oper], 13.5.3 [over.ass] p1: a base's operator functions are the derived class's, but for operator=, reached only qualified
	{
		"OverOperatorInherited", 3,
		"struct B { int v = 0; int operator+(int a) const { return a + 1; } B& operator=(int a) { v = a; return *this; } };\n"
		"struct D : B {};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D d;\n"
		"	if (d + 1 == 2) r += 1;\n"
		"	d.B::operator=(5);\n"
		"	if (d.v == 5) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5.3 [over.ass] p1: a base class's operator= is hidden by the derived class's copy assignment operator
	{
		"OverOperatorAssignmentHiddenRefused", CONFORMANCE_REFUSED,
		"struct B { int v = 0; B& operator=(int a) { v = a; return *this; } };\n"
		"struct D : B {};\n"
		"int main() { D d; d = 5; return d.v; }\n"
	},
	//13.5.3 [over.ass] p1: operator= is a non-static member function
	{
		"OverOperatorAssignmentNonMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"S& operator=(S& s, int a) { s.v = a; return s; }\n"
		"int main() { S s; s = 1; return s.v; }\n"
	},
	//13.5.4 [over.call] p1: operator() is a non-static member function
	{
		"OverOperatorCallNonMemberRefused", CONFORMANCE_REFUSED,
		"struct S {};\n"
		"int operator()(S, int a) { return a; }\n"
		"int main() { S s; return s(1); }\n"
	},
	//13.5.5 [over.sub]: operator[] is a member with one parameter, overloaded on const and on its parameter's type
	{
		"OverOperatorSubscript", 7,
		"struct S\n"
		"{\n"
		"	int a[3];\n"
		"	int& operator[](int i) { return a[i]; }\n"
		"	int operator[](int i) const { return a[i] + 1; }\n"
		"	int operator[](const char*) const { return 4; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { { 0, 0, 0 } };\n"
		"	s[1] = 1;\n"
		"	if (s.a[1] == 1) r += 1;\n"
		"	const S& cs = s;\n"
		"	if (cs[1] == 2) r += 2;\n"
		"	if (s[\"x\"] == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5.5 [over.sub]: operator[] is a non-static member function
	{
		"OverOperatorSubscriptNonMemberRefused", CONFORMANCE_REFUSED,
		"struct S { int a[2]; };\n"
		"int operator[](const S& s, int i) { return s.a[i]; }\n"
		"int main() { S s = { { 1, 2 } }; return s[1]; }\n"
	},
	//13.5.6 [over.ref]: x->m is (x.operator->())->m, the operator called again while what it gives is a class object
	{
		"OverOperatorArrowChain", 7,
		"struct A { int v; int F() { return 2; } };\n"
		"struct P { A a; A* operator->() { return &a; } };\n"
		"struct Q { P p; P& operator->() { return p; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	P p = { { 1 } };\n"
		"	if (p->v == 1) r += 1;\n"
		"	Q q = { { { 3 } } };\n"
		"	if (q->v == 3) r += 2;\n"
		"	if (q->F() == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5.6 [over.ref]: operator-> is a non-static member function
	{
		"OverOperatorArrowNonMemberRefused", CONFORMANCE_REFUSED,
		"struct A { int v; };\n"
		"struct P { A a; };\n"
		"A* operator->(P& p) { return &p.a; }\n"
		"int main() { P p = { { 1 } }; return p->v; }\n"
	},
	//13.5.7 [over.inc]: prefix ++ and -- have no parameter as members, one as non-members; postfix ones an int more, given 0 when the operator calls them
	{
		"OverOperatorIncrement", 63,
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	S& operator++() { v += 1; return *this; }\n"
		"	S operator++(int a) { S t = *this; v += 2 + a; return t; }\n"
		"};\n"
		"struct T { int v; };\n"
		"T& operator--(T& t) { t.v -= 1; return t; }\n"
		"T operator--(T& t, int) { T o = t; t.v -= 2; return o; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { 0 };\n"
		"	++s;\n"
		"	if (s.v == 1) r += 1;\n"
		"	S o = s++;\n"
		"	if (o.v == 1 && s.v == 3) r += 2;\n"
		"	s.operator++(4);\n"
		"	if (s.v == 9) r += 4;\n"
		"	T t = { 10 };\n"
		"	--t;\n"
		"	if (t.v == 9) r += 8;\n"
		"	T u = t--;\n"
		"	if (u.v == 9 && t.v == 7) r += 16;\n"
		"	if ((++s).v == 10) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5.7 [over.inc] p1: a postfix ++'s extra parameter is an int
	{
		"OverOperatorIncrementLongRefused", CONFORMANCE_REFUSED,
		"struct S { int v; S operator++(long) { return *this; } };\n"
		"int main() { S s = { 0 }; s++; return s.v; }\n"
	},
	//13.5.8 [over.literal], 2.14.8 [lex.ext]: a user-defined literal calls operator "": an integer's with unsigned long long, a floating one's with long double, a string's with its pointer and length, a character's with char; else the raw form gets the characters, the template form gets them as template arguments
	{
		"OverLiteralOperators", 63,
		"typedef decltype(sizeof(0)) size_t;\n"
		"int operator\"\" _k(unsigned long long n) { return (int)n * 2; }\n"
		"int operator\"\" _f(long double d) { return d > 1.4 && d < 1.6 ? 4 : 0; }\n"
		"int operator\"\" _s(const char* s, size_t n) { return n == 3 && s[0] == 'a' ? 8 : 0; }\n"
		"int operator\"\" _c(char c) { return c == 'x' ? 16 : 0; }\n"
		"int operator\"\" _raw(const char* s) { return s[0] == '1' && s[1] == '2' && s[2] == 0 ? 32 : 0; }\n"
		"template<char... C> int operator\"\" _t() { return sizeof...(C) == 3 ? 64 : 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (1_k == 2) r += 1;\n"
		"	if (1.5_f == 4) r += 2;\n"
		"	if (\"abc\"_s == 8) r += 4;\n"
		"	if ('x'_c == 16) r += 8;\n"
		"	if (12_raw == 32) r += 16;\n"
		"	if (123_t == 64) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//13.5.8 [over.literal] p3: a literal operator's parameters are one of the lists given
	{
		"OverLiteralBadParametersRefused", CONFORMANCE_REFUSED,
		"int operator\"\" _x(int n) { return n; }\n"
		"int main() { return 1_x; }\n"
	},
	//13.5 [over.oper] p4, 12.8 [class.copy] p18: the copy assignment operator the compiler declares is called by its name as a declared one is
	{
		"OverOperatorImplicitAssignmentByName", 3,
		"struct S { int v; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { 9 };\n"
		"	S t = { 10 };\n"
		"	s.operator=(t);\n"
		"	if (s.v == 10) r += 1;\n"
		"	s.v = 0;\n"
		"	if ((s = t).v == 10) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//13.3.3.2 [over.ics.rank] p3, 2.13.8 [lex.ext] p2: a user-defined literal of a literal operator returning a class by value is a prvalue: it binds R&&, and f(R&&) is chosen over f(R&)
	{
		"OverRankLiteralByValueBindsRvalue", 2,
		"struct R { unsigned long long v; };\n"
		"R operator\"\"_r(unsigned long long v) { R r = { v }; return r; }\n"
		"int f(R&) { return 1; }\n"
		"int f(R&&) { return 2; }\n"
		"int main() { return f(5_r); }\n"
	},
};


//Clause 14, templates [temp].
static const SConformanceCase gasConformanceClause14[] =
{
	//14.5.5 [temp.class.spec] p5, 14.5.1.3 [temp.static]: a class template's member variable template partially specialised outside the class, naming the class's parameters its own way
	{
		"TempMemberVariableTemplatePartialOutside", 31,
		"template<class C> struct W\n"
		"{\n"
		"	template<class T> static constexpr int size = 1;\n"
		"	template<class T> static constexpr bool same = false;\n"
		"};\n"
		"template<class D> template<class T> constexpr int W<D>::size<T*> = (int)sizeof(D) + 100;\n"
		"template<class C> template<class T> constexpr bool W<C>::same<T*> = true;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (W<int>::size<char> == 1) r += 1;\n"
		"	if (W<char>::size<int*> == 101) r += 2;\n"
		"	if (W<long>::size<char*> == (int)sizeof(long) + 100) r += 4;\n"
		"	if (W<int>::same<int*> && !W<int>::same<int>) r += 8;\n"
		"	if (W<char>::same<char**>) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.5 [temp.class.spec] p5: a member variable template is specialised outside its class (g++)
	{
		"TempMemberVariableTemplatePartialInClassRefused", CONFORMANCE_REFUSED,
		"template<class C> struct W\n"
		"{\n"
		"	template<class T> static constexpr bool same = false;\n"
		"	template<class T> static constexpr bool same<T*> = true;\n"
		"};\n"
		"int main() { return W<int>::same<int*> ? 1 : 0; }\n"
	},
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
	//14 [temp] p1, 14.5.1.3 [temp.static]: a variable template at class scope is a static data member template
	{
		"TempVariableTemplateStaticMember", 31,
		"struct S\n"
		"{\n"
		"	template<class T> static const int size = sizeof(T);\n"
		"	template<class T> static T value;\n"
		"	template<class T> static int kind;\n"
		"};\n"
		"template<class T> const int S::size;\n"
		"template<class T> T S::value = T(7);\n"
		"template<class T> int S::kind = 1;\n"
		"template<class T> int S::kind<T*> = 2;\n"
		"int gaiRow[S::size<char[3]>];\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (S::size<char> == 1 && S::size<char[2]> == 2) r += 1;\n"
		"	if (sizeof(gaiRow) == 3 * sizeof(int)) r += 2;\n"
		"	S::value<long> = 9;\n"
		"	if (S::value<long> == 9 && S::value<char> == 7 && sizeof(S::value<char>) == 1) r += 4;\n"
		"	if (S::kind<int> == 1 && S::kind<int*> == 2) r += 8;\n"
		"	if (S::value<int> == 7 && sizeof(S) == 1) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.3 [temp.variadic] p4, p5: a base-specifier-list's pack expansion makes a base for each element, of type packs and non-type packs expanded together
	{
		"TempPackBases", 31,
		"struct A { int a = 1; };\n"
		"struct B { int b = 2; };\n"
		"template<class... T> struct All : T... { };\n"
		"template<class T> struct Wrap : T { int w = 4; };\n"
		"template<class... T> struct Wrapped : Wrap<T>... { };\n"
		"template<int I, class T> struct Leaf { T v; };\n"
		"template<int... I> struct Index { };\n"
		"template<class X, class... T> struct Tuple;\n"
		"template<int... I, class... T> struct Tuple<Index<I...>, T...> : Leaf<I, T>... { };\n"
		"template<int... N> struct Sized : Leaf<N, char[N]>... { };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	All<A, B> x;\n"
		"	Wrapped<A, B> y;\n"
		"	Tuple<Index<0, 1>, int, char> t;\n"
		"	if (x.a == 1 && x.b == 2 && sizeof(All<>) == 1) r += 1;\n"
		"	if (static_cast<Wrap<A>&>(y).w == 4 && static_cast<Wrap<B>&>(y).b == 2) r += 2;\n"
		"	static_cast<Leaf<0, int>&>(t).v = 4;\n"
		"	static_cast<Leaf<1, char>&>(t).v = 3;\n"
		"	if (static_cast<Leaf<0, int>&>(t).v + static_cast<Leaf<1, char>&>(t).v == 7) r += 4;\n"
		"	if (sizeof(t) >= sizeof(int) + 1) r += 8;\n"
		"	if (sizeof(Sized<2, 3>) == 5) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.1.2 [temp.mem.class], 14.6.2 [temp.dep]: a member class of a class template is each instance's own, with the instance's template arguments
	{
		"TempMemberClassOfTemplate", 63,
		"template<class T> struct Outer\n"
		"{\n"
		"	struct Inner { T t; int Size() { return (int)sizeof(T); } };\n"
		"	struct Derived : T { int Next() { return this->v + 1; } };\n"
		"	struct Deep { struct Deeper { T d[2]; }; };\n"
		"	Inner i;\n"
		"	int Sizes() { return (int)sizeof(Inner) * 10 + i.Size(); }\n"
		"	static Inner Make(T t) { Inner n; n.t = t; return n; }\n"
		"};\n"
		"struct Base { int v = 4; };\n"
		"struct Big { long a; long b; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Outer<char> a;\n"
		"	Outer<Big> b;\n"
		"	Outer<char>::Inner ci;\n"
		"	Outer<Big>::Inner bi;\n"
		"	Outer<Base>::Derived d;\n"
		"	if (a.Sizes() == 11 && b.Sizes() == (int)sizeof(Big) * 11) r += 1;\n"
		"	if (sizeof(ci) == 1 && sizeof(bi) == sizeof(Big)) r += 2;\n"
		"	if (d.Next() == 5 && d.v == 4) r += 4;\n"
		"	if (sizeof(Outer<char>::Deep::Deeper) == 2 && sizeof(Outer<long>::Deep::Deeper) == 2 * sizeof(long)) r += 8;\n"
		"	bi = Outer<Big>::Make(Big());\n"
		"	ci = Outer<char>::Make('x');\n"
		"	if (ci.t == 'x') r += 16;\n"
		"	Outer<char>::Inner* p = &a.i;\n"
		"	p->t = 'y';\n"
		"	if (a.i.t == 'y') r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.6.4.1 [temp.point] p8, 14.5.6 [temp.fct], 8.3.6 [dcl.fct.default] p4: a function template is called where it is only declared, defined after, with the default arguments its declaration gave it
	{
		"TempFunctionTemplateDefinedLater", 15,
		"struct S { int v; };\n"
		"template<class T> T Twice(T);\n"
		"template<class T> T Add(T a, T b = T(4));\n"
		"template<class T> int operator+(S, T);\n"
		"template<class T> int Size(T t = T());\n"
		"int Early() { S s = { 2 }; return Twice(2) + (s + 3) * 10 + Add(5) * 100; }\n"
		"template<class U> U Twice(U u) { return u * 2; }\n"
		"template<class U> U Add(U a, U b) { return a + b; }\n"
		"template<class U> int operator+(S s, U u) { return s.v + (int)u; }\n"
		"template<class U> int Size(U u) { return (int)sizeof(U) + (int)u; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s = { 1 };\n"
		"	if (Early() == 954) r += 1;\n"
		"	if (Twice(3L) == 6 && Add(1, 1) == 2 && Add('a') == 'a' + 4 && s + 'b' == 'b' + 1) r += 2;\n"
		"	if (Size<char>() == 1 && Size<long>() == sizeof(long) && Size(2) == sizeof(int) + 2) r += 4;\n"
		"	if (sizeof(Twice('c')) == 1 && sizeof(Add(1L)) == sizeof(long)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p5, 13.3.3 [over.match.best] p1, 12.8 [class.copy] p2: a constructor template's instance competes with the copy constructor for an object of its class: S(T&&) takes one that isn't const, the copy constructor (never a template's instance) a const one and one that is going; S(T) never takes its own class
	{
		"TempConstructorTemplateCopies", 15,
		"int gCopies = 0;\n"
		"struct S { int v; S() : v(1) { } template<class T> S(T&& t) : v(100) { } };\n"
		"struct D : S { D() { } };\n"
		"struct V { int v; V() : v(1) { } template<class T> V(T t) : v(200) { } };\n"
		"struct M { int v; M() : v(1) { } M(const M& o) : v(o.v + 10) { } M(M&& o) : v(o.v + 20) { } template<class T> M(T&& t) : v(300) { } };\n"
		"M Make() { return M(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a; const S c; D d; V x; M m; const M cm;\n"
		"	S b(a); S e(c); S f(d); S g = a;\n"
		"	if (b.v == 100 && e.v == 1 && f.v == 100 && g.v == 100) r += 1;\n"
		"	V y(x); V z = x;\n"
		"	if (y.v == 1 && z.v == 1) r += 2;\n"
		"	M n(m); M o(cm); M p(static_cast<M&&>(m));\n"
		"	if (n.v == 300 && o.v == 11 && p.v == 21) r += 4;\n"
		"	S h(3); V w('c');\n"
		"	if (h.v == 100 && w.v == 200) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p5, 13.3.3 [over.match.best] p1: the same choice wherever an object is copied: an argument passed by value, a returned object, a member and a base of an object the compiler copies (const, so never the template), new, braces
	{
		"TempConstructorTemplateCopiesForms", 127,
		"struct S { int v; S() : v(1) { } template<class T> S(T&& t) : v(100) { } };\n"
		"struct O { S s; int k; O() : k(2) { } };\n"
		"struct N { int v; N() : v(1) { } N(const N& o) : v(o.v + 10) { } };\n"
		"struct Q { N n; int v; Q() : v(1) { } template<class T> Q(T&& t) : v(100) { } };\n"
		"struct B { int v; B() : v(1) { } template<class T> B(T&& t) : v(100) { } };\n"
		"struct E : B { int w; E() : w(3) { } };\n"
		"void* operator new(decltype(sizeof(0)), void* p) { return p; }\n"
		"int Take(S s) { return s.v; }\n"
		"S Give(S& s) { return s; }\n"
		"S GiveConst(const S& s) { return s; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S a; const S c;\n"
		"	if (Take(a) == 100 && Take(c) == 1 && Take(S()) == 1) r += 1;\n"
		"	if (Give(a).v == 100 && GiveConst(a).v == 1) r += 2;\n"
		"	O o; O p(o); O q = o;\n"
		"	if (p.s.v == 1 && p.k == 2 && q.s.v == 1) r += 4;\n"
		"	Q x; const Q cx; Q y(x); Q z(cx);\n"
		"	if (y.v == 100 && y.n.v == 1 && z.v == 1 && z.n.v == 11) r += 8;\n"
		"	E e; E f(e);\n"
		"	if (f.v == 1 && f.w == 3) r += 16;\n"
		"	S m1; S m2; S* h = new (&m1) S(a); S* i = new (&m2) S(c);\n"
		"	if (h->v == 100 && i->v == 1) r += 32;\n"
		"	S j = static_cast<S&&>(a); S k{a}; S l = {c};\n"
		"	if (j.v == 1 && k.v == 100 && l.v == 1) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p1, 14.6.4.1 [temp.point] p8, 8.3.6 [dcl.fct.default] p4: a member template is called where it is only declared, in its class and after it, and defined outside the class later, with the default arguments its declaration gave it (which name its template parameter)
	{
		"TempMemberTemplateDefinedLater", 31,
		"struct S\n"
		"{\n"
		"	int v;\n"
		"	template<class T> T Twice(T t);\n"
		"	template<class T> static T Half(T t);\n"
		"	template<class T> int Size(T t = T());\n"
		"	template<class T> static int Count(T t, int n = sizeof(T));\n"
		"	int Early() { return Twice(2) + Half(8) * 10 + Size<char>() * 100; }\n"
		"};\n"
		"template<class C> struct W\n"
		"{\n"
		"	C c;\n"
		"	template<class T> T Add(T t);\n"
		"	int Early() { return Add(3); }\n"
		"};\n"
		"int Before(S& s) { return s.Twice(5) + S::Half(6); }\n"
		"template<class U> U S::Twice(U u) { return u * 2 + v; }\n"
		"template<class U> U S::Half(U u) { return u / 2; }\n"
		"template<class U> int S::Size(U u) { return (int)sizeof(U) + (int)u; }\n"
		"template<class U> int S::Count(U u, int n) { return n + (int)u; }\n"
		"template<class C> template<class T> T W<C>::Add(T t) { return t + c; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s; s.v = 1;\n"
		"	W<int> w; w.c = 10;\n"
		"	if (s.Early() == 5 + 40 + 100) r += 1;\n"
		"	if (Before(s) == 11 + 3) r += 2;\n"
		"	if (w.Early() == 13 && w.Add('a') == 'a' + 10) r += 4;\n"
		"	if (s.Twice(3L) == 7 && sizeof(s.Twice('c')) == 1) r += 8;\n"
		"	if (S::Count('a') == 'a' + 1 && S::Count(2L) == sizeof(long) + 2 && S::Count(2, 5) == 7 && s.Size(3L) == sizeof(long) + 3) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.3 [temp.expl.spec] p21: an explicit specialisation of a function template is given no default argument
	{
		"TempExplicitSpecialisationDefaultArgumentRefused", CONFORMANCE_REFUSED,
		"template<class T> int Size(T t) { return sizeof(T); }\n"
		"template<> int Size<char>(char c = 'a') { return 7; }\n"
		"int main() { return Size('c'); }\n"
	},
	//14.7.3 [temp.expl.spec] p21: nor is one of a class template's member function
	{
		"TempMemberSpecialisationDefaultArgumentRefused", CONFORMANCE_REFUSED,
		"template<class C> struct W { int F(int a); };\n"
		"template<> int W<char>::F(int a = 2) { return a; }\n"
		"int main() { W<char> w; return w.F(1); }\n"
	},
	//14.5.1.2 [temp.mem.class] p1, 14.5.2 [temp.mem] p1: a member class of a class template, and a member class template of one, defined outside it (each instance's own, the definition naming the template's parameters its own way); a member function of such a member class defined outside it too
	{
		"TempMemberClassDefinedOutside", 15,
		"template<class T> struct Outer\n"
		"{\n"
		"	struct Inner;\n"
		"	struct Node;\n"
		"	template<class U> struct Pair;\n"
		"	Inner* Make(T t);\n"
		"	static int Size();\n"
		"};\n"
		"template<class T> struct Outer<T>::Inner { T t; int Twice() { return (int)t * 2; } };\n"
		"template<class T> struct Outer<T>::Node { T v; Node* next; Node(T a) : v(a), next(0) { } int Sum(); };\n"
		"template<class T> int Outer<T>::Node::Sum() { return (int)v + (next ? next->Sum() : 0); }\n"
		"template<class T> template<class U> struct Outer<T>::Pair { T a; U b; int Both() { return (int)a + (int)b; } };\n"
		"template<class T> typename Outer<T>::Inner* Outer<T>::Make(T t) { static Inner i; i.t = t; return &i; }\n"
		"template<class T> int Outer<T>::Size() { return (int)sizeof(Inner); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	Outer<int>::Inner i; i.t = 4;\n"
		"	Outer<char>::Inner c; c.t = 3;\n"
		"	if (i.Twice() == 8 && c.Twice() == 6 && sizeof(c) == 1) r += 1;\n"
		"	Outer<int>::Node a(1), b(2); a.next = &b;\n"
		"	if (a.Sum() == 3) r += 2;\n"
		"	Outer<char>::Pair<long> p; p.a = 1; p.b = 5;\n"
		"	if (p.Both() == 6 && sizeof(p.b) == sizeof(long)) r += 4;\n"
		"	Outer<int> o;\n"
		"	if (o.Make(7)->Twice() == 14 && Outer<char>::Size() == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.1.2 [temp.mem.class] p1, 9.7 [class.nest] p3: a member function of a class template's member class defined outside the template, a member class of the member class defined there, and a nested class of a class that is no template (its own names found before its enclosing class's)
	{
		"TempMemberClassMembersOutside", 135,
		"template<class T> struct Outer\n"
		"{\n"
		"	struct In { T t; int Get(); struct Deep; };\n"
		"};\n"
		"template<class T> int Outer<T>::In::Get() { return (int)t + 1; }\n"
		"template<class T> struct Outer<T>::In::Deep { T d[3]; };\n"
		"struct A { struct B; int k; };\n"
		"struct A::B { int x; int F() { return x + sizeof(A); } };\n"
		"int main()\n"
		"{\n"
		"	Outer<char>::In i; i.t = 4;\n"
		"	A::B b; b.x = 1;\n"
		"	return i.Get() + (int)sizeof(Outer<char>::In::Deep) * 10 + (b.F() == 1 + sizeof(A) ? 100 : 0);\n"
		"}\n"
	},
	//14.5.1.2 [temp.mem.class], 5.1.1 [expr.prim.general] p8: Outer<char>::Node::name is that instance's member class's member: a static function, a static constant, an enumerator, a typedef, a function whose address is taken
	{
		"TempMemberClassThroughInstance", 31,
		"template<class T> struct Outer\n"
		"{\n"
		"	struct Node\n"
		"	{\n"
		"		T v;\n"
		"		static int Count() { return (int)sizeof(T); }\n"
		"		static const int size = sizeof(T) + 1;\n"
		"		enum { twice = sizeof(T) * 2 };\n"
		"		typedef T Type;\n"
		"		static int Add(int a) { return a + Count(); }\n"
		"	};\n"
		"	static int Use() { return Node::Count() + Node::size * 10; }\n"
		"};\n"
		"int (*gp)(int) = &Outer<char>::Node::Add;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Outer<char>::Node::Count() == 1 && Outer<long>::Node::Count() == sizeof(long)) r += 1;\n"
		"	if (Outer<char>::Node::size == 2 && Outer<long>::Node::size == sizeof(long) + 1) r += 2;\n"
		"	if (Outer<char>::Node::twice == 2 && sizeof(Outer<long>::Node::Type) == sizeof(long)) r += 4;\n"
		"	if (Outer<char>::Use() == 21 && gp(4) == 5) r += 8;\n"
		"	if (sizeof(Outer<long>::Node::v) == sizeof(long)) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14 [temp] p1, 14.5.1.3 [temp.static] p1: a variable template that is a class template's static member is each instance's own and sees the instance's template arguments; defined in the class or outside it, named in the class, through an instance, an object and a derived class
	{
		"TempMemberVariableTemplateOfClassTemplate", 31,
		"template<class C> struct W\n"
		"{\n"
		"	template<class T> static const int size = sizeof(T) + sizeof(C);\n"
		"	template<class T> static T value;\n"
		"	static int Use() { return size<char> + value<short>; }\n"
		"};\n"
		"template<class C> template<class T> T W<C>::value = T(sizeof(C) + 2);\n"
		"struct D : W<char> { };\n"
		"int ga[W<char>::size<char>];\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (W<char>::size<char> == 2 && W<long>::size<char> == sizeof(long) + 1 && sizeof(ga) == 2 * sizeof(int)) r += 1;\n"
		"	if (W<char>::value<int> == 3 && W<long>::value<char> == (char)(sizeof(long) + 2)) r += 2;\n"
		"	W<char>::value<int> = 9;\n"
		"	if (W<char>::value<int> == 9 && W<char>::value<long> == 3) r += 4;\n"
		"	if (W<char>::Use() == 5 && W<long>::Use() == (int)sizeof(long) * 2 + 3) r += 8;\n"
		"	W<char> w; W<long>* p = 0;\n"
		"	if (w.size<long> == sizeof(long) + 1 && p->size<char> == sizeof(long) + 1 && D::size<char> == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14 [temp] p2: a template is declared at namespace or class scope only
	{
		"TempDeclarationInBlockRefused", CONFORMANCE_REFUSED,
		"int main() { template<class T> struct A {}; return 0; }\n"
	},
	//14 [temp] p4: a template has no C linkage
	{
		"TempLinkageCRefused", CONFORMANCE_REFUSED,
		"extern \"C\" { template<class T> int F(T) { return 1; } }\n"
		"int main() { return F(1); }\n"
	},
	//14 [temp] p5: a class template's name is the only thing of that name in its scope
	{
		"TempNameUniqueRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A {};\n"
		"int A = 1;\n"
		"int main() { return A; }\n"
	},
	//14.1 [temp.param] p4: a non-type template parameter is an integer, an enumeration, a pointer to an object or a function, an lvalue reference, a pointer to member or std::nullptr_t
	{
		"TempNonTypeParameterKinds", 127,
		"enum E { Two = 2 };\n"
		"int g = 4;\n"
		"int Eight() { return 8; }\n"
		"struct S { int m; };\n"
		"typedef decltype(nullptr) nullptr_t;\n"
		"template<int I> int FI() { return I; }\n"
		"template<E V> int FE() { return V; }\n"
		"template<int* P> int FP() { return *P; }\n"
		"template<int (*F)()> int FF() { return F(); }\n"
		"template<int& R> int FR() { return R * 4; }\n"
		"template<int S::*M> int FM() { S s = { 32 }; return s.*M; }\n"
		"template<nullptr_t N> int FN() { return N == nullptr ? 64 : 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (FI<1>() == 1) r += 1;\n"
		"	if (FE<Two>() == 2) r += 2;\n"
		"	if (FP<&g>() == 4) r += 4;\n"
		"	if (FF<Eight>() == 8) r += 8;\n"
		"	if (FR<g>() == 16) r += 16;\n"
		"	if (FM<&S::m>() == 32) r += 32;\n"
		"	if (FN<nullptr>() == 64) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//14.1 [temp.param] p7: a non-type template parameter isn't of a floating point type
	{
		"TempNonTypeFloatRefused", CONFORMANCE_REFUSED,
		"template<double D> int F() { return (int)D; }\n"
		"int main() { return F<1.0>(); }\n"
	},
	//14.1 [temp.param] p7: nor of a class type
	{
		"TempNonTypeClassRefused", CONFORMANCE_REFUSED,
		"struct S { int v; };\n"
		"template<S X> int F() { return X.v; }\n"
		"int main() { return 0; }\n"
	},
	//14.1 [temp.param] p6: a non-type parameter that isn't a reference is a prvalue: not assigned to
	{
		"TempNonTypeNotLvalueRefused", CONFORMANCE_REFUSED,
		"template<int I> int F() { I = 2; return I; }\n"
		"int main() { return F<1>(); }\n"
	},
	//14.1 [temp.param] p6: nor its address taken
	{
		"TempNonTypeAddressRefused", CONFORMANCE_REFUSED,
		"template<int I> int F() { const int* p = &I; return *p; }\n"
		"int main() { return F<1>(); }\n"
	},
	//14.1 [temp.param] p5, p9-p11, p14: default template arguments, naming earlier parameters; a function template's default before a parameter without one; a template template parameter's default; a default given by a later declaration; top-level const on a parameter ignored
	{
		"TempDefaultArguments", 63,
		"template<class T, class U = T*> struct A { static int F() { return sizeof(U) == sizeof(T*) ? 1 : 0; } };\n"
		"template<class T = int, class U> int G(U) { return sizeof(T) == sizeof(int) ? 2 : 0; }\n"
		"template<class T> struct Box { T v; };\n"
		"template<template<class> class TT = Box> struct H { TT<int> b; int F() { b.v = 4; return b.v; } };\n"
		"template<int I, int J = I + 1> int K() { return J; }\n"
		"template<class T, int N> struct L;\n"
		"template<class T, int N = 16> struct L { static int F() { return N; } };\n"
		"template<const int I> int C() { return I; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<char>::F() == 1) r += 1;\n"
		"	if (G('a') == 2) r += 2;\n"
		"	H<> h;\n"
		"	if (h.F() == 4) r += 4;\n"
		"	if (K<7>() == 8) r += 8;\n"
		"	if (L<int>::F() == 16) r += 16;\n"
		"	if (C<32>() == 32) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.1 [temp.param] p11: a class template's parameter with a default is followed only by ones with defaults
	{
		"TempDefaultBeforeNonDefaultClassRefused", CONFORMANCE_REFUSED,
		"template<class T = int, class U> struct A {};\n"
		"int main() { return 0; }\n"
	},
	//14.1 [temp.param] p12: a template parameter's default isn't given twice
	{
		"TempDefaultRedefinedRefused", CONFORMANCE_REFUSED,
		"template<class T = int> struct A;\n"
		"template<class T = int> struct A {};\n"
		"int main() { return 0; }\n"
	},
	//14.1 [temp.param] p9: a template parameter pack has no default
	{
		"TempDefaultForPackRefused", CONFORMANCE_REFUSED,
		"template<class... T = int> struct A {};\n"
		"int main() { return 0; }\n"
	},
	//14.1 [temp.param] p11: a class template's parameter pack is its last parameter
	{
		"TempPackNotLastClassRefused", CONFORMANCE_REFUSED,
		"template<class... T, class U> struct A {};\n"
		"int main() { return 0; }\n"
	},
	//14.2 [temp.names] p3: >> ends two template argument lists; in parentheses > and >> are operators
	{
		"TempNamesClosingAngles", 7,
		"template<class T> struct A { static int F() { return 1; } };\n"
		"template<int I> struct B { static int F() { return I; } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<A<int>>::F() == 1) r += 1;\n"
		"	if (B<(4 > 2)>::F() == 1) r += 2;\n"
		"	if (B<(8 >> 1)>::F() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.2 [temp.names] p4: a member template of a dependent object or class is named after the keyword template
	{
		"TempNamesTemplateKeyword", 15,
		"struct S\n"
		"{\n"
		"	template<int I> int F() { return I; }\n"
		"	template<class T> struct In { static int G() { return 4; } };\n"
		"	template<int I> static int H() { return I; }\n"
		"};\n"
		"template<class T> int Call(T t, T* p)\n"
		"{\n"
		"	int r = 0;\n"
		"	if (t.template F<1>() == 1) r += 1;\n"
		"	if (p->template F<2>() == 2) r += 2;\n"
		"	if (T::template In<int>::G() == 4) r += 4;\n"
		"	if (T::template H<8>() == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
		"int main() { S s; return Call(s, &s); }\n"
	},
	//14.2 [temp.names] p4: without it the < is less than
	{
		"TempNamesTemplateKeywordMissingRefused", CONFORMANCE_REFUSED,
		"struct S { template<int I> int F() { return I; } };\n"
		"template<class T> int Call(T t) { return t.F<1>(); }\n"
		"int main() { S s; return Call(s); }\n"
	},
	//14.3 [temp.arg] p2: an argument that could be a type or an expression is a type
	{
		"TempArgumentTypeIdPreferred", 3,
		"template<class T> int F() { return 1; }\n"
		"template<int I> int F() { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F<int()>() == 1) r += 1;\n"
		"	if (F<1>() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.3.1 [temp.arg.type] p1: a local type and an unnamed type are template arguments
	{
		"TempArgumentLocalAndUnnamedTypes", 3,
		"template<class T> int F(T t) { return t.v; }\n"
		"template<class T> int G(T t) { return (int)t; }\n"
		"enum { A = 2 };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	struct L { int v; } l = { 1 };\n"
		"	if (F(l) == 1) r += 1;\n"
		"	if (G(A) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.3.2 [temp.arg.nontype] p1: an argument is a constant expression, the address of an object or function with linkage (& left off for a function or an array), a null pointer, or an object with linkage for a reference
	{
		"TempNonTypeArguments", 63,
		"int ga[3] = { 1, 2, 3 };\n"
		"int g = 5;\n"
		"int F() { return 4; }\n"
		"constexpr int Two() { return 2; }\n"
		"template<int I> int A() { return I; }\n"
		"template<int* P> int B() { return P ? P[1] : 16; }\n"
		"template<int (*Fn)()> int C() { return Fn(); }\n"
		"template<const int& R> int D() { return R; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<Two() * 2 - 3>() == 1) r += 1;\n"
		"	if (B<ga>() == 2) r += 2;\n"
		"	if (C<F>() == 4 && C<&F>() == 4) r += 4;\n"
		"	if (B<nullptr>() == 16) r += 8;\n"
		"	if (D<g>() == 5) r += 16;\n"
		"	enum { E = 3 };\n"
		"	if (A<E + sizeof(char)>() == 4) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.3.2 [temp.arg.nontype] p1, p2: a string literal isn't an argument
	{
		"TempNonTypeStringLiteralRefused", CONFORMANCE_REFUSED,
		"template<const char* P> int F() { return P[0]; }\n"
		"int main() { return F<\"x\">(); }\n"
	},
	//14.3.2 [temp.arg.nontype] p1: nor the address of an object without linkage
	{
		"TempNonTypeNoLinkageRefused", CONFORMANCE_REFUSED,
		"template<int* P> int F() { return *P; }\n"
		"int main() { static int s = 1; return F<&s>(); }\n"
	},
	//14.3.2 [temp.arg.nontype] p1: nor the address of an array's element
	{
		"TempNonTypeSubobjectRefused", CONFORMANCE_REFUSED,
		"int ga[3] = { 1, 2, 3 };\n"
		"template<int* P> int F() { return *P; }\n"
		"int main() { return F<&ga[1]>(); }\n"
	},
	//14.3.2 [temp.arg.nontype] p1: an integer argument is a constant expression
	{
		"TempNonTypeNotConstantRefused", CONFORMANCE_REFUSED,
		"template<int I> int F() { return I; }\n"
		"int main() { int i = 1; return F<i>(); }\n"
	},
	//14.3.2 [temp.arg.nontype] p1, 5.19 [expr.const] p3: a converted constant expression doesn't narrow
	{
		"TempNonTypeNarrowingRefused", CONFORMANCE_REFUSED,
		"template<char C> int F() { return C; }\n"
		"int main() { return F<300>(); }\n"
	},
	//14.3.3 [temp.arg.template] p1, p3: a template template argument is a class template or an alias template whose parameters match; a pack matches any
	{
		"TempTemplateTemplateArguments", 7,
		"template<class T> struct Box { T v; };\n"
		"template<class T> using Ptr = T*;\n"
		"template<class T, class U = int> struct Pair { T a; U b; };\n"
		"template<template<class> class TT> int Size() { return sizeof(TT<char>); }\n"
		"template<template<class...> class TT> int SizeP() { return sizeof(TT<char, char>); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Size<Box>() == 1) r += 1;\n"
		"	if (Size<Ptr>() == sizeof(char*)) r += 2;\n"
		"	if (SizeP<Pair>() == 2) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.3.3 [temp.arg.template] p3: in C++14 the parameter lists match exactly: a template with a second, defaulted parameter isn't a template<class> class
	{
		"TempTemplateTemplateDefaultMismatchRefused", CONFORMANCE_REFUSED,
		"template<class T, class U = int> struct Pair { T a; U b; };\n"
		"template<template<class> class TT> int Size() { return sizeof(TT<char>); }\n"
		"int main() { return Size<Pair>(); }\n"
	},
	//14.4 [temp.type]: template-ids with the same arguments are one type: constant expressions by their values, a typedef, an alias template
	{
		"TempTypeEquivalence", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class T, int N> struct A {};\n"
		"typedef int Int;\n"
		"template<class T> using Al = A<T, 2>;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<A<int, 1 + 1>, A<int, 2>>::V) r += 1;\n"
		"	if (Same<A<Int, 2>, A<int, 2>>::V) r += 2;\n"
		"	if (Same<Al<int>, A<int, 2>>::V) r += 4;\n"
		"	if (!Same<A<int, 2>, A<int, 3>>::V) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.1.1 [temp.mem.func], 14.5.1.2 [temp.mem.class], 14.5.1.3 [temp.static], 14.5.1.4 [temp.mem.enum]: a class template's member function, static data member and member class defined outside the class, with the template's parameters, and its enumeration (g++ refuses one defined outside)
	{
		"TempClassMembersOutside", 63,
		"template<class T> struct A\n"
		"{\n"
		"	T v;\n"
		"	int F();\n"
		"	static int s;\n"
		"	struct In;\n"
		"	enum E : int { Eight = 8 };\n"
		"	int G() const;\n"
		"};\n"
		"template<class T> int A<T>::F() { return v + 1; }\n"
		"template<class T> int A<T>::s = 2;\n"
		"template<class T> struct A<T>::In { int H() { return 4; } };\n"
		"template<class T> int A<T>::G() const { return sizeof(T) == 1 ? 16 : 32; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a = { 0 };\n"
		"	if (a.F() == 1) r += 1;\n"
		"	if (A<int>::s == 2) r += 2;\n"
		"	A<int>::In in;\n"
		"	if (in.H() == 4) r += 4;\n"
		"	if (A<char>::Eight == 8) r += 8;\n"
		"	A<char> c = { 0 };\n"
		"	if (c.G() == 16) r += 16;\n"
		"	if (a.G() == 32) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p1, p5, p6: a member function template, one defined outside the class and its template, and a conversion function template
	{
		"TempMemberTemplates", 31,
		"template<class T> struct A\n"
		"{\n"
		"	T v;\n"
		"	template<class U> int F(U u) { return v + u; }\n"
		"	template<class U> int G(U);\n"
		"	template<class U> operator U() const { return (U)(v * 2); }\n"
		"};\n"
		"template<class T> template<class U> int A<T>::G(U) { return sizeof(U) == 1 ? 4 : 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a = { 0 };\n"
		"	if (a.F(1) == 1) r += 1;\n"
		"	a.v = 1;\n"
		"	if (a.F<char>(1) == 2) r += 2;\n"
		"	if (a.G('x') == 4) r += 4;\n"
		"	int i = a;\n"
		"	if (i == 2) r += 8;\n"
		"	long l = a;\n"
		"	if (l == 2) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.2 [temp.mem] p3: a member function template isn't virtual
	{
		"TempMemberTemplateVirtualRefused", CONFORMANCE_REFUSED,
		"struct S { template<class T> virtual int F(T) { return 1; } };\n"
		"int main() { S s; return s.F(1); }\n"
	},
	//14.5.2 [temp.mem] p4: a member function template's specialisation doesn't override a virtual function
	{
		"TempMemberTemplateDoesNotOverride", 1,
		"struct B { virtual int F(int) { return 1; } };\n"
		"struct D : B { template<class T> int F(T) { return 2; } };\n"
		"int main() { D d; B& b = d; return b.F(1); }\n"
	},
	//14.5.3 [temp.variadic] p5, 14.8.2.1 [temp.deduct.call] p1: a member template's parameters may be its class's pack, already known, and then its own, deduced from the rest
	{
		"TempVariadicClassPackThenOwn", 63,
		"template<class... A> struct Pair\n"
		"{\n"
		"	template<class... B> static int Mix(A... a, B... b) { return (int)sizeof...(A) * 10 + (int)sizeof...(B); }\n"
		"	template<class B> static int One(A... a, B b) { return (int)sizeof(B); }\n"
		"	template<class B, class... C> static int First(B b, A... a) { return (int)sizeof...(A); }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Pair<int, char>::Mix(1, 'a', 2.0, 3L, 4) == 23) r += 1;\n"
		"	if (Pair<int, char>::Mix(1, 'a') == 20) r += 2;\n"
		"	if (Pair<>::Mix(1, 2) == 2) r += 4;\n"
		"	if (Pair<int, char>::One(1, 'a', 2.0) == (int)sizeof(double)) r += 8;\n"
		"	if (Pair<int, char>::First(1.0, 1, 'a') == 2) r += 16;\n"
		"	if (Pair<long>::Mix(1L, 'x') == 11) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.3 [temp.variadic] p5: the class's pack takes as many arguments as it has
	{
		"TempVariadicClassPackTooFewRefused", CONFORMANCE_REFUSED,
		"template<class... A> struct Pair\n"
		"{\n"
		"	template<class... B> static int Mix(A... a, B... b) { return (int)sizeof...(B); }\n"
		"};\n"
		"int main() { return Pair<int, char>::Mix(1); }\n"
	},
	//14.5.3 [temp.variadic] p4, p5, p7: sizeof...; a pack expanded in a call's arguments, in a braced list, in a template argument list, with a pattern applied to each element, and empty
	{
		"TempVariadicSizeofAndExpansions", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class... T> struct Tup {};\n"
		"int Sum() { return 0; }\n"
		"template<class T, class... R> int Sum(T t, R... r) { return t + Sum(r...); }\n"
		"int Twice(int a) { return a * 2; }\n"
		"template<class... T> int Count(T...) { return sizeof...(T); }\n"
		"template<class... T> int Doubled(T... t) { return Sum(Twice(t)...); }\n"
		"template<class... T> int Arr(T... t) { int a[] = { 0, t... }; return sizeof(a) / sizeof(a[0]); }\n"
		"template<class... T> int Types(T...) { return Same<Tup<T...>, Tup<int, char>>::V; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Count() == 0 && Count(1, 'a', 2L) == 3) r += 1;\n"
		"	if (Sum(1, 2, 4) == 7) r += 2;\n"
		"	if (Doubled(1, 2) == 6) r += 4;\n"
		"	if (Arr(5, 6) == 3) r += 8;\n"
		"	if (Types(1, 'c')) r += 16;\n"
		"	if (Sum() == 0 && Doubled() == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.3 [temp.variadic] p4-p6: packs in one pattern expand together element by element; a mem-initializer list and a lambda's captures expand a pack
	{
		"TempVariadicLockstep", 7,
		"template<class... T> struct Types {};\n"
		"template<int... I> struct Ints {};\n"
		"int Sum() { return 0; }\n"
		"template<class T, class... R> int Sum(T t, R... r) { return t + Sum(r...); }\n"
		"template<class... T, int... I> int Pairs(Types<T...>, Ints<I...>) { return Sum((int)sizeof(T) * I...); }\n"
		"template<int N> struct Base { int v; Base() : v(N) {} };\n"
		"template<int... N> struct Derived : Base<N>... { Derived() : Base<N>()... {} int Total() { return Sum(Base<N>::v...); } };\n"
		"template<class... T> int Captured(T... t) { auto f = [t...] { return Sum(t...); }; return f(); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Pairs(Types<char, char>(), Ints<1, 2>()) == 3) r += 1;\n"
		"	Derived<1, 2> d;\n"
		"	if (d.Total() == 3) r += 2;\n"
		"	if (Captured(1, 3) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.3 [temp.variadic] p6: packs expanded together have one length
	{
		"TempVariadicLengthMismatchRefused", CONFORMANCE_REFUSED,
		"template<class... T> struct Types {};\n"
		"template<int... I> struct Ints {};\n"
		"int Sum() { return 0; }\n"
		"template<class T, class... R> int Sum(T t, R... r) { return t + Sum(r...); }\n"
		"template<class... T, int... I> int Pairs(Types<T...>, Ints<I...>) { return Sum((int)sizeof(T) * I...); }\n"
		"int main() { return Pairs(Types<char, char>(), Ints<1>()); }\n"
	},
	//14.5.3 [temp.variadic] p5: a pack is named only in an expansion
	{
		"TempVariadicUnexpandedRefused", CONFORMANCE_REFUSED,
		"template<class... T> int F(T...) { return sizeof(T); }\n"
		"int main() { return F(1); }\n"
	},
	//14.5.4 [temp.friend] p1, p4: a friend function template, a friend class template, one specialisation as a friend, a friend defined in a class template for each of its specialisations
	{
		"TempFriends", 15,
		"template<class T> class A;\n"
		"template<class T> int Peek(const A<T>& a) { return a.v; }\n"
		"template<class T> class A\n"
		"{\n"
		"	int v;\n"
		"public:\n"
		"	A(int x) : v(x) {}\n"
		"	template<class U> friend int Peek(const A<U>&);\n"
		"	template<class U> friend struct Reader;\n"
		"	friend int Get(const A& a) { return a.v * 2; }\n"
		"};\n"
		"template<class U> struct Reader { int Read(const A<U>& a) { return a.v; } };\n"
		"template<class T> int Only(T);\n"
		"class C { int w = 8; friend int Only<int>(int); };\n"
		"template<class T> int Only(T) { C c; return c.w; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a(1);\n"
		"	if (Peek(a) == 1) r += 1;\n"
		"	Reader<int> rd;\n"
		"	if (rd.Read(A<int>(2)) == 2) r += 2;\n"
		"	if (Get(A<char>(2)) == 4) r += 4;\n"
		"	if (Only(1) == 8) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.4 [temp.friend] p1: a specialisation that is a friend makes no other one a friend
	{
		"TempFriendSpecialisationOnlyRefused", CONFORMANCE_REFUSED,
		"template<class T> int Only(T);\n"
		"class C { int w = 8; friend int Only<int>(int); };\n"
		"template<class T> int Only(T) { C c; return c.w; }\n"
		"int main() { return Only('a'); }\n"
	},
	//14.6.5 [temp.inject] p2: a friend function defined in a class template is found only by argument dependent lookup
	{
		"TempFriendInjectedNotFoundRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A { friend int F() { return 1; } };\n"
		"int main() { A<int> a; return F(); }\n"
	},
	//14.5.5 [temp.class.spec], 14.5.5.1 [temp.class.spec.match], 14.5.5.2 [temp.class.order]: a partial specialisation matching the arguments is used rather than the primary template, the most specialised of those that match
	{
		"TempPartialSpecialisations", 63,
		"template<class T, class U> struct A { enum { V = 1 }; };\n"
		"template<class T> struct A<T, T> { enum { V = 2 }; };\n"
		"template<class T> struct A<T, int> { enum { V = 4 }; };\n"
		"template<class T, class U> struct A<T*, U> { enum { V = 8 }; };\n"
		"template<class T> struct A<T*, T*> { enum { V = 16 }; };\n"
		"template<int I, int J> struct B { enum { V = 0 }; };\n"
		"template<int I> struct B<I, 0> { enum { V = 32 }; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<char, long>::V == 1) r += 1;\n"
		"	if (A<char, char>::V == 2) r += 2;\n"
		"	if (A<char, int>::V == 4) r += 4;\n"
		"	if (A<char*, long>::V == 8) r += 8;\n"
		"	if (A<char*, char*>::V == 16) r += 16;\n"
		"	if (B<3, 0>::V == 32) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.5.1 [temp.class.spec.match] p1: two partial specialisations match and neither is more specialised
	{
		"TempPartialSpecialisationAmbiguousRefused", CONFORMANCE_REFUSED,
		"template<class T, class U> struct A { enum { V = 1 }; };\n"
		"template<class T> struct A<T, T> { enum { V = 2 }; };\n"
		"template<class T> struct A<T, int> { enum { V = 4 }; };\n"
		"int main() { return A<int, int>::V; }\n"
	},
	//14.5.5 [temp.class.spec] p8: a specialised non-type argument's parameter has no type that depends on the partial specialisation's parameters
	{
		"TempPartialSpecialisationDependentTypeRefused", CONFORMANCE_REFUSED,
		"template<class T, T V> struct C {};\n"
		"template<class T> struct C<T, 1> {};\n"
		"int main() { return 0; }\n"
	},
	//14.5.5 [temp.class.spec] p8: a partial specialisation's arguments aren't the primary template's
	{
		"TempPartialSpecialisationSameAsPrimaryRefused", CONFORMANCE_REFUSED,
		"template<class T, class U> struct A {};\n"
		"template<class T, class U> struct A<T, U> {};\n"
		"int main() { return 0; }\n"
	},
	//14.5.5 [temp.class.spec] p8: a partial specialisation's parameters have no defaults
	{
		"TempPartialSpecialisationDefaultRefused", CONFORMANCE_REFUSED,
		"template<class T, class U> struct A {};\n"
		"template<class T = int> struct A<T, char> {};\n"
		"int main() { return 0; }\n"
	},
	//14.5.5.3 [temp.class.spec.mfunc] p1: a partial specialisation's members are its own, defined outside with its parameter list
	{
		"TempPartialSpecialisationMembers", 7,
		"template<class T> struct A { int F() { return 1; } };\n"
		"template<class T> struct A<T*> { int G(); int H() { return 4; } };\n"
		"template<class T> int A<T*>::G() { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a;\n"
		"	if (a.F() == 1) r += 1;\n"
		"	A<int*> b;\n"
		"	if (b.G() == 2) r += 2;\n"
		"	if (b.H() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.5.3 [temp.class.spec.mfunc] p1: the primary template's members aren't a partial specialisation's
	{
		"TempPartialSpecialisationNoPrimaryMemberRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A { int F() { return 1; } };\n"
		"template<class T> struct A<T*> { int G() { return 2; } };\n"
		"int main() { A<int*> b; return b.F(); }\n"
	},
	//14.5.6.2 [temp.func.order], 14.8.2.4 [temp.deduct.partial]: of function templates that fit a call as well, the more specialised is chosen: T* before T, const T* before T*, const T& before T& for a const object, (T, T) before (T, U)
	{
		"TempFunctionPartialOrdering", 127,
		"template<class T> int F(T) { return 1; }\n"
		"template<class T> int F(T*) { return 2; }\n"
		"template<class T> int F(const T*) { return 4; }\n"
		"template<class T> int G(T&) { return 8; }\n"
		"template<class T> int G(const T&) { return 16; }\n"
		"template<class T, class U> int H(T, U) { return 32; }\n"
		"template<class T> int H(T, T) { return 64; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int i = 0;\n"
		"	const int ci = 0;\n"
		"	if (F(i) == 1) r += 1;\n"
		"	if (F(&i) == 2) r += 2;\n"
		"	if (F(&ci) == 4) r += 4;\n"
		"	if (G(i) == 8) r += 8;\n"
		"	if (G(ci) == 16) r += 16;\n"
		"	if (H(1, 'a') == 32) r += 32;\n"
		"	if (H(1, 2) == 64) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2.4 [temp.deduct.partial] p8, p11: a template without a pack is more specialised than one with
	{
		"TempFunctionPartialOrderingVariadic", 7,
		"template<class... T> int K(T...) { return 1; }\n"
		"template<class T> int K(T) { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (K(1) == 2) r += 1;\n"
		"	if (K(1, 2) == 1) r += 2;\n"
		"	if (K() == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.6.2 [temp.func.order] p5: neither template more specialised: the call is ambiguous
	{
		"TempFunctionOrderingAmbiguousRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T, int) { return 1; }\n"
		"template<class T> int F(int, T) { return 2; }\n"
		"int main() { return F(1, 1); }\n"
	},
	//14.5.6.1 [temp.over.link] p5, p6: function templates whose signatures are equivalent are one template defined twice
	{
		"TempFunctionRedefinitionRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T) { return 1; }\n"
		"template<class U> int F(U) { return 2; }\n"
		"int main() { return F(1); }\n"
	},
	//14.5.6.1 [temp.over.link] p2, 1.3.19 [defns.signature.templ]: a function template's return type is part of its signature: two that differ only there are two templates, told apart by a target type
	{
		"TempFunctionReturnTypeInSignature", 3,
		"template<class T> int F(T) { return 1; }\n"
		"template<class T> long F(T) { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int (*p)(int) = F;\n"
		"	if (p(0) == 1) r += 1;\n"
		"	long (*q)(char) = F;\n"
		"	if (q(0) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.7 [temp.alias] p1-p3: an alias template's specialisation is the type it names, deduced through, and it may have a pack
	{
		"TempAliasTemplates", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class T> struct Box { T v; };\n"
		"template<class T> using B = Box<T>;\n"
		"template<class T> using Ptr = T*;\n"
		"template<class... T> struct Tup {};\n"
		"template<class... T> using Tp = Tup<T..., int>;\n"
		"template<class T> int F(Ptr<T> p) { return *p; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same<B<int>, Box<int>>::V) r += 1;\n"
		"	int i = 2;\n"
		"	if (F(&i) == 2) r += 2;\n"
		"	if (Same<Tp<char>, Tup<char, int>>::V) r += 4;\n"
		"	B<char> b = { 'x' };\n"
		"	if (b.v == 'x') r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.7 [temp.alias] p4: an alias template's type doesn't name the alias template
	{
		"TempAliasSelfReferenceRefused", CONFORMANCE_REFUSED,
		"template<class T> struct S {};\n"
		"template<class T> using A = S<A<T>>;\n"
		"int main() { return 0; }\n"
	},
	//14.7.1 [temp.inst], 14.7.3 [temp.expl.spec]: a class template's specialisation instantiates others, ended by an explicit specialisation
	{
		"TempRecursiveSpecialisation", 1,
		"template<int N> struct Fact { enum { V = N * Fact<N - 1>::V }; };\n"
		"template<> struct Fact<0> { enum { V = 1 }; };\n"
		"int main() { return Fact<5>::V == 120 ? 1 : 0; }\n"
	},
	//14.6 [temp.res] p2, p3: a dependent qualified name is a type only when typename says so
	{
		"TempResTypenameRequiredRefused", CONFORMANCE_REFUSED,
		"struct S { typedef int I; };\n"
		"template<class T> int F() { T::I x = 1; return x; }\n"
		"int main() { return F<S>(); }\n"
	},
	//14.6 [temp.res] p3, p5, 14.6.2.1 [temp.dep.type] p1: typename before a dependent qualified type; a member of the current instantiation needs none; typename outside a template
	{
		"TempResTypename", 15,
		"struct S { typedef int I; struct In { int v; }; };\n"
		"template<class T> struct A\n"
		"{\n"
		"	typedef T Type;\n"
		"	typename T::I F() { typename T::In in = { 2 }; return in.v; }\n"
		"	int G() { typename A::Type* p = 0; A::Type* q = 0; return p == q ? 4 : 0; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<S> a;\n"
		"	if (a.F() == 2) r += 1;\n"
		"	if (a.G() == 4) r += 2;\n"
		"	typename S::I i = 8;\n"
		"	if (i == 8) r += 4;\n"
		"	if (sizeof(typename A<S>::Type) == sizeof(S)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.6.2 [temp.dep] p3: an unqualified name isn't looked for in a dependent base class
	{
		"TempResDependentBaseNotFoundRefused", CONFORMANCE_REFUSED,
		"template<class T> struct B { int v = 1; };\n"
		"template<class T> struct D : B<T> { int F() { return v; } };\n"
		"int main() { D<int> d; return d.F(); }\n"
	},
	//14.6.2 [temp.dep] p3: a dependent base's member is named through this-> or qualified; an unqualified name finds what is outside the class at the definition, not the base's
	{
		"TempResDependentBase", 7,
		"int v = 4;\n"
		"template<class T> struct B { int v = 1; int w = 2; };\n"
		"template<class T> struct D : B<T>\n"
		"{\n"
		"	int F() { return this->v; }\n"
		"	int G() { return B<T>::w; }\n"
		"	int H() { return v; }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	D<int> d;\n"
		"	if (d.F() == 1) r += 1;\n"
		"	if (d.G() == 2) r += 2;\n"
		"	if (d.H() == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.6.3 [temp.nondep] p1, 14.6.4.2 [temp.dep.candidate] p1: a non-dependent name is bound where the template is defined; a dependent call also finds later functions by argument dependent lookup
	{
		"TempResNonDependentBoundAtDefinition", 3,
		"int F(double) { return 1; }\n"
		"template<class T> int Call(T) { return F(1); }\n"
		"int F(int) { return 2; }\n"
		"namespace N { struct X {}; }\n"
		"template<class T> int Dep(T t) { return G(t); }\n"
		"namespace N { int G(X) { return 2; } }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Call(0) == 1) r += 1;\n"
		"	if (Dep(N::X()) == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.6.4.2 [temp.dep.candidate] p1: a function declared after the template isn't found by ordinary lookup at the instantiation
	{
		"TempResDependentOrdinaryLookupRefused", CONFORMANCE_REFUSED,
		"template<class T> int Dep(T t) { return G(t); }\n"
		"int G(int a) { return a; }\n"
		"int main() { return Dep(1); }\n"
	},
	//14.6 [temp.res] p8: an error no template argument could mend is an error though the template is never instantiated
	{
		"TempResNonDependentErrorRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T) { return Undeclared(); }\n"
		"int main() { return 0; }\n"
	},
	//14.6 [temp.res] p8, 14.7.1 [temp.inst] p1: an error that depends on the template's arguments is one only in a specialisation that is instantiated
	{
		"TempResDependentErrorNotInstantiated", 1,
		"template<class T> int F(T t) { return t.Missing(); }\n"
		"template<class T> struct A { int G() { T t; return t.Missing(); } int H() { return 1; } };\n"
		"int main() { A<int> a; return a.H(); }\n"
	},
	//14.6.1 [temp.local] p1-p3: in a class template its name alone is the current specialisation, with arguments a specialisation of the template, and as a template template argument the template
	{
		"TempLocalInjectedClassName", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<template<class> class TT> struct Use { static int V() { return Same<TT<int>, TT<int>>::V; } };\n"
		"template<class T> struct A\n"
		"{\n"
		"	A* self;\n"
		"	static int F() { return Same<A, A<T>>::V; }\n"
		"	static int G() { return Same<A<char>, ::A<char>>::V; }\n"
		"	static int H() { return Use<A>::V(); }\n"
		"};\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (A<int>::F()) r += 1;\n"
		"	if (A<int>::G()) r += 2;\n"
		"	if (A<long>::H()) r += 4;\n"
		"	if (Use<A>::V()) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.6.1 [temp.local] p7: in a member defined outside its class template, a member's name hides the template parameter of that name
	{
		"TempLocalMemberHidesParameter", 3,
		"template<class T> struct A { struct B { int v = 1; }; typedef long C; int F(); int G(); };\n"
		"template<class B> int A<B>::F() { B b; return b.v; }\n"
		"template<class C> int A<C>::G() { return sizeof(C) == sizeof(long) ? 2 : 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a;\n"
		"	if (a.F() == 1) r += 1;\n"
		"	A<char> c;\n"
		"	if (c.G() == 2) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.1 [temp.inst] p1-p3, p11: a class template specialisation's member functions, static members' initialisers and default arguments are instantiated only when used; the class only when it must be complete
	{
		"TempInstMembersOnDemand", 15,
		"template<class T> struct A\n"
		"{\n"
		"	int F() { return T::Missing; }\n"
		"	int G() { return 1; }\n"
		"	static int s;\n"
		"	int H(int a = T::Missing) { return a; }\n"
		"};\n"
		"template<class T> int A<T>::s = T::Missing;\n"
		"template<class T> struct Bad { typename T::Missing m; };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a;\n"
		"	if (a.G() == 1) r += 1;\n"
		"	if (a.H(2) == 2) r += 2;\n"
		"	Bad<int>* p = 0;\n"
		"	if (p == 0) r += 4;\n"
		"	if (sizeof(A<int>) > 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.1 [temp.inst] p2: a member function that is used is instantiated, and its error is one
	{
		"TempInstMemberUsedRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A { int F() { return T::Missing; } };\n"
		"int main() { A<int> a; return a.F(); }\n"
	},
	//14.7.1 [temp.inst] p1: a specialisation that must be complete is instantiated
	{
		"TempInstCompleteRequiredRefused", CONFORMANCE_REFUSED,
		"template<class T> struct Bad { typename T::Missing m; };\n"
		"int main() { Bad<int> b; return 0; }\n"
	},
	//14.7.2 [temp.explicit] p1-p4, p10: explicit instantiations of a class template, a function template (its arguments given or deduced) and a member function, and an explicit instantiation declaration
	{
		"TempExplicitInstantiation", 15,
		"template<class T> struct A { int F() { return 1; } int G() { return 2; } };\n"
		"template struct A<int>;\n"
		"template<class T> int H(T) { return 4; }\n"
		"template int H<char>(char);\n"
		"template int H(long);\n"
		"template<class T> struct B { int K() { return 8; } };\n"
		"template int B<int>::K();\n"
		"extern template struct A<char>;\n"
		"template struct A<char>;\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<int> a;\n"
		"	if (a.F() + a.G() == 3) r += 1;\n"
		"	if (H('a') == 4 && H(1L) == 4) r += 2;\n"
		"	B<int> b;\n"
		"	if (b.K() == 8) r += 4;\n"
		"	A<char> c;\n"
		"	if (c.G() == 2) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.2 [temp.explicit] p8: a class's explicit instantiation instantiates every member, so an error in one that is never called is one
	{
		"TempExplicitInstantiationAllMembersRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A { int F() { return T::Missing; } int G() { return 2; } };\n"
		"template struct A<int>;\n"
		"int main() { A<int> a; return a.G(); }\n"
	},
	//14.7 [temp.spec] p5: a specialisation is explicitly instantiated once only
	{
		"TempExplicitInstantiationTwiceRefused", CONFORMANCE_REFUSED,
		"template<class T> int H(T) { return 4; }\n"
		"template int H<int>(int);\n"
		"template int H<int>(int);\n"
		"int main() { return H(1); }\n"
	},
	//14.7.3 [temp.expl.spec] p1, p16, p18: explicit specialisations of a class template, of a function template (its arguments given or deduced), of a class template's member function and of a member template
	{
		"TempExplicitSpecialisations", 63,
		"template<class T> struct A { int F() { return 1; } };\n"
		"template<> struct A<int> { int G() { return 2; } };\n"
		"template<class T> int H(T) { return 0; }\n"
		"template<> int H<char>(char) { return 4; }\n"
		"template<> int H(long) { return 8; }\n"
		"template<class T> struct B { int K() { return 0; } template<class U> int M(U) { return 0; } };\n"
		"template<> int B<int>::K() { return 16; }\n"
		"template<> template<> int B<int>::M<char>(char) { return 32; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	A<char> a;\n"
		"	if (a.F() == 1) r += 1;\n"
		"	A<int> ai;\n"
		"	if (ai.G() == 2) r += 2;\n"
		"	if (H('a') == 4) r += 4;\n"
		"	if (H(1L) == 8) r += 8;\n"
		"	B<int> b;\n"
		"	if (b.K() == 16) r += 16;\n"
		"	if (b.M('c') == 32 && b.M(1) == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.7.3 [temp.expl.spec] p5: an explicit specialisation of a class template has only its own members
	{
		"TempExplicitSpecialisationNoPrimaryMemberRefused", CONFORMANCE_REFUSED,
		"template<class T> struct A { int F() { return 1; } };\n"
		"template<> struct A<int> { int G() { return 2; } };\n"
		"int main() { A<int> a; return a.F(); }\n"
	},
	//14.7.3 [temp.expl.spec] p6: an explicit specialisation is declared before a use that would instantiate the template for it
	{
		"TempExplicitSpecialisationAfterUseRefused", CONFORMANCE_REFUSED,
		"template<class T> int H(T) { return 0; }\n"
		"int Use() { return H('a'); }\n"
		"template<> int H<char>(char) { return 4; }\n"
		"int main() { return Use(); }\n"
	},
	//14.7.3 [temp.expl.spec] p2: an explicit specialisation is declared in a namespace enclosing the template
	{
		"TempExplicitSpecialisationOtherNamespaceRefused", CONFORMANCE_REFUSED,
		"namespace N { template<class T> struct A { enum { V = 0 }; }; }\n"
		"namespace M { template<> struct N::A<int> { enum { V = 1 }; }; }\n"
		"int main() { return N::A<int>::V; }\n"
	},
	//14.8.1 [temp.arg.explicit] p2-p4, p6: template arguments given explicitly, the trailing ones deduced, <> deducing all; an argument whose parameter's type is given explicitly is converted
	{
		"TempExplicitArguments", 31,
		"template<class T, class U> int F(U u) { return sizeof(T) == 1 && u == 2 ? 1 : 0; }\n"
		"template<class T> int G(T t) { return (int)t; }\n"
		"template<class T> T Make() { return T(4); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F<char>(2)) r += 1;\n"
		"	if (F<char, long>(2)) r += 2;\n"
		"	if (G<int>(2.7) == 2) r += 4;\n"
		"	if (Make<int>() == 4) r += 8;\n"
		"	if (G<>(16) == 16) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.1 [temp.arg.explicit] p8, 14.8.2 [temp.deduct] p2: more explicit template arguments than parameters
	{
		"TempExplicitArgumentsTooManyRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T) { return 1; }\n"
		"int main() { return F<int, int>(1); }\n"
	},
	//14.8.2.1 [temp.deduct.call] p2-p4: a parameter that isn't a reference takes an array's or function's pointer and drops top-level const; a reference keeps const; T&& from an lvalue makes T an lvalue reference; a derived class gives its base's template arguments
	{
		"TempDeductionFromCall", 127,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"template<class T> struct B {};\n"
		"struct D : B<long> {};\n"
		"template<class T> int Val(T) { return Same<T, int*>::V; }\n"
		"template<class T> int Fn(T) { return Same<T, int (*)(int)>::V; }\n"
		"template<class T> int NoConst(T) { return Same<T, int>::V; }\n"
		"template<class T> int Ref(T&) { return Same<T, const int>::V; }\n"
		"template<class T> int Fwd(T&&) { return Same<T, int&>::V; }\n"
		"template<class T> int FwdR(T&&) { return Same<T, int>::V; }\n"
		"template<class T> int Base(B<T>&) { return Same<T, long>::V; }\n"
		"int Id(int a) { return a; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int a[2] = { 0, 0 };\n"
		"	const int c = 1;\n"
		"	int i = 0;\n"
		"	D d;\n"
		"	if (Val(a)) r += 1;\n"
		"	if (Fn(Id)) r += 2;\n"
		"	if (NoConst(c)) r += 4;\n"
		"	if (Ref(c)) r += 8;\n"
		"	if (Fwd(i)) r += 16;\n"
		"	if (FwdR(1)) r += 32;\n"
		"	if (Base(d)) r += 64;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2.5 [temp.deduct.type] p2, 14.8.2.1 [temp.deduct.call]: one parameter deduced as two types
	{
		"TempDeductionConflictRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T, T) { return 1; }\n"
		"int main() { return F(1, 2L); }\n"
	},
	//14.8.2.1 [temp.deduct.call] p1, 14.8.2.5 [temp.deduct.type] p5: a braced list deduces nothing for a parameter T
	{
		"TempDeductionBracedListRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T) { return 1; }\n"
		"int main() { return F({ 1, 2 }); }\n"
	},
	//14.8.2.1 [temp.deduct.call] p1: a braced list deduces T for a parameter initializer_list<T>
	{
		"TempDeductionInitializerList", 3,
		"#ifdef CONFORMANCE_HOST\n"
		"#include <initializer_list>\n"
		"#else\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }\n"
		"#endif\n"
		"template<class T> int F(std::initializer_list<T> l) { return (int)l.size() * (int)sizeof(T); }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F({ 'a', 'b' }) == 2) r += 1;\n"
		"	if (F({ 1 }) == (int)sizeof(int)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2.5 [temp.deduct.type] p4, p5: T in a nested name specifier is deduced elsewhere or given; the argument then converts
	{
		"TempDeductionNonDeducedContext", 7,
		"template<class T> struct Id { typedef T Type; };\n"
		"template<class T> int F(T a, typename Id<T>::Type b) { return (int)(a + b); }\n"
		"template<class T> int G(typename Id<T>::Type v) { return (int)v; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F(1L, 'b' - 'a') == 2) r += 1;\n"
		"	if (G<int>(2.5) == 2) r += 2;\n"
		"	if (F(2, 2.9) == 4) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2.5 [temp.deduct.type] p5: a T only in a nested name specifier isn't deduced
	{
		"TempDeductionNonDeducedRefused", CONFORMANCE_REFUSED,
		"template<class T> struct Id { typedef T Type; };\n"
		"template<class T> int G(typename Id<T>::Type v) { return v; }\n"
		"int main() { return G(1); }\n"
	},
	//14.8.2.5 [temp.deduct.type] p8, p17: deduced from the forms of a type: a function pointer's result and parameter, a template's type and non-type arguments, a pointer to member's class and type, an array reference's bound
	{
		"TempDeductionFromTypes", 63,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"typedef decltype(sizeof(0)) size_t;\n"
		"template<size_t N> int Len(const char (&)[N]) { return (int)N; }\n"
		"template<class R, class A> int Sig(R (*)(A)) { return Same<R, long>::V && Same<A, char>::V; }\n"
		"template<class T, int N> struct Arr { T a[N]; };\n"
		"template<class T, int N> int ArrN(const Arr<T, N>&) { return N; }\n"
		"struct S { int m; };\n"
		"template<class C, class M> int Mem(M C::*) { return Same<C, S>::V && Same<M, int>::V; }\n"
		"template<class T, int N> int Raw(T (&)[N]) { return N; }\n"
		"long Fn(char) { return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Len(\"abc\") == 4) r += 1;\n"
		"	if (Sig(Fn)) r += 2;\n"
		"	Arr<int, 3> a3 = { { 0, 0, 0 } };\n"
		"	if (ArrN(a3) == 3) r += 4;\n"
		"	if (Mem(&S::m)) r += 8;\n"
		"	int x[5] = { 0 };\n"
		"	if (Raw(x) == 5) r += 16;\n"
		"	double y[2][3] = { { 0 } };\n"
		"	if (Raw(y) == 2) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2 [temp.deduct] p8: a type or expression that is invalid with the deduced arguments takes the template out of the overloads, with no error
	{
		"TempSfinae", 15,
		"struct S { typedef int Type; int m; };\n"
		"template<class T> int F(typename T::Type) { return 1; }\n"
		"template<class T> int F(T) { return 2; }\n"
		"template<bool B, class T = void> struct EnableIf {};\n"
		"template<class T> struct EnableIf<true, T> { typedef T Type; };\n"
		"template<class T> typename EnableIf<sizeof(T) == 1, int>::Type G(T) { return 4; }\n"
		"template<class T> typename EnableIf<sizeof(T) != 1, int>::Type G(T) { return 0; }\n"
		"template<class T> auto H(T t) -> decltype(t.m, 8) { return 8; }\n"
		"int H(...) { return 0; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (F<S>(0) == 1) r += 1;\n"
		"	if (F(1) == 2) r += 2;\n"
		"	if (G('a') == 4 && G(1L) == 0) r += 4;\n"
		"	S s = { 0 };\n"
		"	if (H(s) == 8 && H(1) == 0) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.2 [temp.deduct] p8: an error in the function's body is no substitution failure
	{
		"TempSfinaeBodyErrorRefused", CONFORMANCE_REFUSED,
		"template<class T> int F(T t) { return t.m; }\n"
		"int F(...) { return 0; }\n"
		"int main() { return F(1); }\n"
	},
	//14.8.2.3 [temp.deduct.conv]: a conversion function template's argument is deduced from the type converted to
	{
		"TempConversionFunctionTemplateDeduction", 7,
		"struct S { template<class T> operator T() const { return T(sizeof(T)); } };\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	S s;\n"
		"	char c = s;\n"
		"	if (c == 1) r += 1;\n"
		"	long l = s;\n"
		"	if (l == (long)sizeof(long)) r += 2;\n"
		"	double d = s;\n"
		"	if (d == sizeof(double)) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//14.8.3 [temp.over] p1: a template's specialisation, its arguments deduced, competes with the other functions; one whose deduction fails isn't a candidate
	{
		"TempOverloadResolution", 15,
		"template<class T> int Max(T, T) { return 1; }\n"
		"int Max(int, int) { return 2; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Max('a', 'b') == 1) r += 1;\n"
		"	if (Max(1, 'b') == 2) r += 2;\n"
		"	if (Max(1, 2) == 2) r += 4;\n"
		"	if (Max<>(1, 2) == 1) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.3.1 [temp.arg.type], 14.4 [temp.type], 3.4.3 [basic.lookup.qual] p4: a type named from the global namespace, ::B, is the type B, as a template argument and in sizeof
	{
		"TempArgumentGlobalQualifiedType", 15,
		"template<class T, class U> struct Same { enum { V = 0 }; };\n"
		"template<class T> struct Same<T, T> { enum { V = 1 }; };\n"
		"struct B { int v; };\n"
		"namespace N { struct C {}; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (Same< ::B, B>::V) r += 1;\n"
		"	if (Same<B, ::B>::V) r += 2;\n"
		"	if (Same<N::C, ::N::C>::V) r += 4;\n"
		"	if (sizeof(::B) == sizeof(B)) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//14.5.3 [temp.variadic] p4, p7: a pattern that expands the pack it is an element of (Count<Ts...> in Pair<Count<Ts...>, Ts>...)
	{
		"TempVariadicNestedSamePack", 43,
		"template<class... Ts> struct Count { enum { value = sizeof...(Ts) }; };\n"
		"template<class A, class B> struct Pair { enum { value = A::value * 10 + sizeof(B) }; };\n"
		"template<class... Ps> struct Sum;\n"
		"template<> struct Sum<> { enum { value = 0 }; };\n"
		"template<class P, class... Ps> struct Sum<P, Ps...> { enum { value = P::value + Sum<Ps...>::value }; };\n"
		"template<class... Ts> struct Outer { enum { value = Sum<Pair<Count<Ts...>, Ts>...>::value }; };\n"
		"int main() { return Outer<char, short>::value; }\n"
	},
	//14.5.3 [temp.variadic] p4, p7: the same in a function's parameters
	{
		"TempVariadicNestedInParameters", 45,
		"template<class... Ts> struct Count { enum { value = sizeof...(Ts) }; };\n"
		"template<class A, class B> struct Pair { enum { value = A::value * 10 + sizeof(B) }; };\n"
		"template<class... Ps> struct Sum;\n"
		"template<> struct Sum<> { enum { value = 0 }; };\n"
		"template<class P, class... Ps> struct Sum<P, Ps...> { enum { value = P::value + Sum<Ps...>::value }; };\n"
		"template<class... Ts> struct Holder { static int g(Pair<Count<Ts...>, Ts>... p) { return Sum<Pair<Count<Ts...>, Ts>...>::value + (int)sizeof...(p); } };\n"
		"int main() { return Holder<char, short>::g(Pair<Count<char, short>, char>(), Pair<Count<char, short>, short>()); }\n"
	},
	//14.5.3 [temp.variadic] p4, p7: the pack an element is of, expanded inside the pattern after the element (Pair<Ts, Count<Ts...>>...)
	{
		"TempVariadicNestedAfterElement", 43,
		"template<class... Ts> struct Count { enum { value = sizeof...(Ts) }; };\n"
		"template<class A, class B> struct Pair { enum { value = B::value * 10 + sizeof(A) }; };\n"
		"template<class... Ps> struct Sum;\n"
		"template<> struct Sum<> { enum { value = 0 }; };\n"
		"template<class P, class... Ps> struct Sum<P, Ps...> { enum { value = P::value + Sum<Ps...>::value }; };\n"
		"template<class... Ts> struct Outer { enum { value = Sum<Pair<Ts, Count<Ts...>>...>::value }; };\n"
		"int main() { return Outer<char, short>::value; }\n"
	},
};


//Clause 16, preprocessing directives [cpp].
static const SConformanceCase gasConformanceClause16[] =
{
	//16 [cpp] p1, p4: a directive starts with # first on its line, after spaces too; # alone is a null directive; a directive ends at its new-line
	{
		"CppDirectiveForms", 15,
		"  #  define ONE 1\n"
		"	#define TWO 2\n"
		"#\n"
		"# /* a comment */ define FOUR /* a comment */ 4\n"
		"#define EIGHT \\\n"
		"	8\n"
		"int main() { return ONE + TWO + FOUR + EIGHT; }\n"
	},
	//16 [cpp] p1: a # line with a name no directive has is no directive
	{
		"CppNonDirectiveRefused", CONFORMANCE_REFUSED,
		"#frobnicate\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond]: #if, #elif, #else, #ifdef, #ifndef and defined, nested
	{
		"CppConditionalBasics", 127,
		"#define A 2\n"
		"#define EMPTY\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#if A == 2\n"
		"	r += 1;\n"
		"#else\n"
		"	r += 100;\n"
		"#endif\n"
		"#if A == 1\n"
		"	r += 100;\n"
		"#elif A == 2\n"
		"	r += 2;\n"
		"#elif A == 2\n"
		"	r += 100;\n"
		"#else\n"
		"	r += 100;\n"
		"#endif\n"
		"#ifdef A\n"
		"	r += 4;\n"
		"#endif\n"
		"#ifndef B\n"
		"	r += 8;\n"
		"#endif\n"
		"#if defined A && defined(EMPTY) && !defined B\n"
		"	r += 16;\n"
		"#endif\n"
		"#if 0\n"
		"#if 1\n"
		"	r += 100;\n"
		"#else\n"
		"	r += 100;\n"
		"#endif\n"
		"#elif 1\n"
		"#ifdef A\n"
		"	r += 32;\n"
		"#endif\n"
		"#endif\n"
		"#ifdef EMPTY\n"
		"	r += 64;\n"
		"#endif\n"
		"	return r;\n"
		"}\n"
	},
	//16.1 [cpp.cond] p4: #if's arithmetic is intmax_t's and uintmax_t's; a name not a macro (but true) is 0; character literals are their values
	{
		"CppConditionalArithmetic", 255,
		"#define X 5\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#if -1 > 0u\n"
		"	r += 1;\n"
		"#endif\n"
		"#if 0x7FFFFFFFFFFFFFFF > 0x7FFFFFFF && 4294967296 > 0\n"
		"	r += 2;\n"
		"#endif\n"
		"#if undefined_name == 0 && true == 1 && false == 0\n"
		"	r += 4;\n"
		"#endif\n"
		"#if 'A' == 65 && '\\n' == 10\n"
		"	r += 8;\n"
		"#endif\n"
		"#if (X * 3 + 1) % 4 == 0 && (X << 2) == 20 && (X ? 1 : 2) == 1\n"
		"	r += 16;\n"
		"#endif\n"
		"#if 1 || (1 / 0)\n"
		"	r += 32;\n"
		"#endif\n"
		"#if 1 and not 0 and (6 bitand 3) == 2\n"
		"	r += 64;\n"
		"#endif\n"
		"#if 18446744073709551615u == -1\n"
		"	r += 128;\n"
		"#endif\n"
		"	return r;\n"
		"}\n"
	},
	//16.1 [cpp.cond] p6: a skipped group's directives are only followed to find its end; what isn't a directive there is skipped whatever it is
	{
		"CppConditionalSkippedGroups", 7,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#if 0\n"
		"#frobnicate\n"
		"#error not here\n"
		"#define r 100\n"
		"	this is not C++ ( {\n"
		"#else\n"
		"	r += 1;\n"
		"#endif\n"
		"#ifdef NOT_DEFINED\n"
		"#include \"no_such_file.h\"\n"
		"#else\n"
		"	r += 2;\n"
		"#endif\n"
		"#if 0\n"
		"#elif 1\n"
		"	r += 4;\n"
		"#endif\n"
		"	return r;\n"
		"}\n"
	},
	//16.1 [cpp.cond] p4, 5 [expr] p4: #if's expression is a constant expression
	{
		"CppConditionalDivisionByZeroRefused", CONFORMANCE_REFUSED,
		"#if 1 / 0\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #if has an expression
	{
		"CppConditionalNoExpressionRefused", CONFORMANCE_REFUSED,
		"#if\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #if's expression is a constant expression, without =
	{
		"CppConditionalAssignmentRefused", CONFORMANCE_REFUSED,
		"#if 1 = 1\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p4: #if's expression is integral
	{
		"CppConditionalFloatingRefused", CONFORMANCE_REFUSED,
		"#if 1.5 > 1\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p4: #if's expression has no string
	{
		"CppConditionalStringRefused", CONFORMANCE_REFUSED,
		"#if \"a\"\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #if ends with #endif
	{
		"CppConditionalUnterminatedRefused", CONFORMANCE_REFUSED,
		"#if 1\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #endif ends an #if
	{
		"CppConditionalEndifWithoutIfRefused", CONFORMANCE_REFUSED,
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: one #else, last
	{
		"CppConditionalElseAfterElseRefused", CONFORMANCE_REFUSED,
		"#if 1\n"
		"#else\n"
		"#else\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #elif before #else
	{
		"CppConditionalElifAfterElseRefused", CONFORMANCE_REFUSED,
		"#if 1\n"
		"#else\n"
		"#elif 1\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #ifdef names a macro
	{
		"CppConditionalIfdefNoNameRefused", CONFORMANCE_REFUSED,
		"#ifdef\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #ifdef has one name
	{
		"CppConditionalIfdefExtraTokensRefused", CONFORMANCE_REFUSED,
		"#ifdef A B\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #else has nothing after it
	{
		"CppConditionalElseExtraTokensRefused", CONFORMANCE_REFUSED,
		"#if 0\n"
		"#else X\n"
		"#endif\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1: #endif has nothing after it
	{
		"CppConditionalEndifExtraTokensRefused", CONFORMANCE_REFUSED,
		"#ifdef X\n"
		"#endif X\n"
		"int main() { return 0; }\n"
	},
	//16.1 [cpp.cond] p1, 2.8 [lex.comment]: a comment after #ifdef's name, #else and #endif is white space
	{
		"CppConditionalCommentsAfterDirectives", 3,
		"#define A\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#ifdef A // a comment\n"
		"	r += 1;\n"
		"#else /* a comment */\n"
		"	r += 100;\n"
		"#endif // A\n"
		"#ifndef A /* a comment */\n"
		"	r += 100;\n"
		"#else\n"
		"	r += 2;\n"
		"#endif /* a\n"
		"comment over two lines */\n"
		"	return r;\n"
		"}\n"
	},
	//16.2 [cpp.include]: <header> and "header", and a header named by a macro
	{
		"CppInclude", 3,
		"#include <stddef.h>\n"
		"#include \"stddef.h\"\n"
		"#define HEADER <stddef.h>\n"
		"#include HEADER\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	size_t n = sizeof(int);\n"
		"	if (n == sizeof(int)) r += 1;\n"
		"	if (sizeof(ptrdiff_t) == sizeof((char*)0 - (char*)0)) r += 2;\n"
		"	return r;\n"
		"}\n"
	},
	//16.2 [cpp.include] p2: a header that isn't there
	{
		"CppIncludeMissingRefused", CONFORMANCE_REFUSED,
		"#include \"no_such_header_file.h\"\n"
		"int main() { return 0; }\n"
	},
	//16.2 [cpp.include] p4: #include names a header
	{
		"CppIncludeNoHeaderRefused", CONFORMANCE_REFUSED,
		"#include\n"
		"int main() { return 0; }\n"
	},
	//16.3 [cpp.replace] p9: an object-like macro is its replacement list; an empty one is nothing; one redefined the same is allowed
	{
		"CppMacroObjectLike", 31,
		"#define SIX 6\n"
		"#define NOTHING\n"
		"#define EXPRESSION (SIX + 1)\n"
		"#define SIX 6\n"
		"#define SPACED  1   +   2\n"
		"#define SPACED 1 /* a comment */ + 2\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (SIX == 6) r += 1;\n"
		"	NOTHING r += 2; NOTHING\n"
		"	if (EXPRESSION * 2 == 14) r += 4;\n"
		"	if (SPACED == 3) r += 8;\n"
		"	int SIX_ = 1;\n"
		"	if (SIX_ == 1) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3 [cpp.replace] p10-p12: a function-like macro is called by its name and (; arguments split at commas outside parentheses; a name without ( isn't a call
	{
		"CppMacroFunctionLike", 255,
		"#define ADD(a, b) ((a) + (b))\n"
		"#define FIRST(a, b) a\n"
		"#define NONE() 7\n"
		"#define F(x) (x + 1)\n"
		"int Pair(int a, int b) { return a * 10 + b; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (ADD(1, 2) == 3) r += 1;\n"
		"	if (FIRST((1, 2), 3) == 2) r += 2;\n"
		"	if (NONE() == 7 && NONE( ) == 7) r += 4;\n"
		"	int F = 5;\n"
		"	if (F == 5) r += 8;\n"
		"	if (ADD\n"
		"		(\n"
		"		4,\n"
		"		5) == 9) r += 16;\n"
		"	if (FIRST(Pair(1, 2), 0) == 12) r += 32;\n"
		"	if (F(F) == 6) r += 64;\n"
		"	if (ADD(ADD(1, 1), ADD(2, 2)) == 6) r += 128;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3 [cpp.replace] p4, p10: an argument may be empty
	{
		"CppMacroEmptyArguments", 15,
		"#define SECOND(a, b) b\n"
		"#define WRAP(a) (a 3)\n"
		"#define ONE(a) a 4\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (SECOND(, 1) == 1) r += 1;\n"
		"	if (WRAP() == 3) r += 2;\n"
		"	if (WRAP(-) == -3) r += 4;\n"
		"	if (ONE() == 4) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3 [cpp.replace] p12, 16.3.1 [cpp.subst] p2: ... and __VA_ARGS__
	{
		"CppMacroVariadic", 63,
		"#define SUM(...) Sum(__VA_ARGS__)\n"
		"#define FIRST_REST(a, ...) a + Sum(__VA_ARGS__)\n"
		"#define STR(...) #__VA_ARGS__\n"
		"int Sum() { return 0; }\n"
		"int Sum(int a) { return a; }\n"
		"int Sum(int a, int b) { return a + b; }\n"
		"int Sum(int a, int b, int c) { return a + b + c; }\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (SUM(1, 2, 3) == 6) r += 1;\n"
		"	if (SUM() == 0) r += 2;\n"
		"	if (FIRST_REST(1, 2, 3) == 6) r += 4;\n"
		"	if (FIRST_REST(4,) == 4) r += 8;\n"
		"	if (sizeof(STR(a, b,c)) == sizeof(\"a, b,c\")) r += 16;\n"
		"	if (SUM((1, 2), 3) == 5) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3 [cpp.replace] p4: a call of a function-like macro has as many arguments as parameters
	{
		"CppMacroWrongArgumentCountRefused", CONFORMANCE_REFUSED,
		"#define ADD(a, b) a + b\n"
		"int main() { return ADD(1); }\n"
	},
	//16.3 [cpp.replace] p4: braces don't hold commas, parentheses do
	{
		"CppMacroTooManyArgumentsRefused", CONFORMANCE_REFUSED,
		"#define ONE(a) a\n"
		"int main() { int x[] = ONE({1, 2}); return x[0]; }\n"
	},
	//16.3 [cpp.replace] p4: a call has more arguments than the named parameters, for ...
	{
		"CppMacroVariadicNeedsArgumentRefused", CONFORMANCE_REFUSED,
		"#define FR(a, ...) a\n"
		"int main() { return FR(1); }\n"
	},
	//16.3 [cpp.replace] p10: a macro call ends with )
	{
		"CppMacroUnterminatedCallRefused", CONFORMANCE_REFUSED,
		"#define ID(a) a\n"
		"int main() { return ID(1; }\n"
	},
	//16.3 [cpp.replace] p2: a macro redefined has the same replacement list
	{
		"CppMacroRedefinedDifferentlyRefused", CONFORMANCE_REFUSED,
		"#define V 1\n"
		"#define V 2\n"
		"int main() { return V; }\n"
	},
	//16.3 [cpp.replace] p8, 16.8 [cpp.predefined] p4: defined isn't a macro's name
	{
		"CppMacroDefinedRefused", CONFORMANCE_REFUSED,
		"#define defined 1\n"
		"int main() { return 0; }\n"
	},
	//16.3 [cpp.replace] p5: __VA_ARGS__ only in a variadic macro's replacement list
	{
		"CppMacroVaArgsOutsideRefused", CONFORMANCE_REFUSED,
		"#define V __VA_ARGS__\n"
		"int main() { return 0; }\n"
	},
	//16.3 [cpp.replace] p6: a macro's parameters have different names
	{
		"CppMacroDuplicateParameterRefused", CONFORMANCE_REFUSED,
		"#define F(a, a) a\n"
		"int main() { return 0; }\n"
	},
	//16.3.2 [cpp.stringize]: # makes its argument a string, spaces between tokens as one, \ and " of strings and characters escaped
	{
		"CppStringize", 63,
		"#define STR(x) #x\n"
		"#define XSTR(x) STR(x)\n"
		"#define FOUR 4\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	const char* a = STR(  a   +    b  );\n"
		"	if (a[0] == 'a' && a[1] == ' ' && a[2] == '+' && a[3] == ' ' && a[4] == 'b' && a[5] == 0) r += 1;\n"
		"	const char* b = STR(\"x\\n\");\n"
		"	if (sizeof(STR(\"x\\n\")) == 6 && b[0] == '\"' && b[2] == '\\\\' && b[3] == 'n' && b[4] == '\"') r += 2;\n"
		"	if (sizeof(STR('\"')) == 4) r += 4;\n"
		"	if (sizeof(STR(FOUR)) == 5 && sizeof(XSTR(FOUR)) == 2 && XSTR(FOUR)[0] == '4') r += 8;\n"
		"	if (sizeof(STR()) == 1) r += 16;\n"
		"	if (sizeof(STR(a /* comment */ b)) == 4) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3.2 [cpp.stringize] p1: # in a function-like macro is followed by a parameter
	{
		"CppStringizeNotParameterRefused", CONFORMANCE_REFUSED,
		"#define F(x) # y\n"
		"int main() { return 0; }\n"
	},
	//16.3.3 [cpp.concat]: ## joins tokens; an empty argument joins as nothing; what is joined is rescanned
	{
		"CppTokenPasting", 63,
		"#define CAT(a, b) a ## b\n"
		"#define XCAT(a, b) CAT(a, b)\n"
		"#define ONE 1\n"
		"#define AB 40\n"
		"#define hash_hash # ## #\n"
		"#define mkstr(a) # a\n"
		"#define in_between(a) mkstr(a)\n"
		"#define join(c, d) in_between(c hash_hash d)\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	int xy = 1;\n"
		"	int ONE0 = 5;\n"
		"	if (CAT(x, y) == 1) r += 1;\n"
		"	if (CAT(1, 2) == 12 && CAT(0x, 1F) == 31) r += 2;\n"
		"	if (CAT(, xy) == 1 && CAT(xy, ) == 1) r += 4;\n"
		"	if (CAT(ONE, 0) == 5 && XCAT(ONE, 0) == 10) r += 8;\n"
		"	if (CAT(A, B) == 40) r += 16;\n"
		"	const char* s = join(x, y);\n"
		"	if (s[0] == 'x' && s[1] == ' ' && s[2] == '#' && s[3] == '#' && s[4] == ' ' && s[5] == 'y' && s[6] == 0) r += 32;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3.3 [cpp.concat] p3: what ## makes is one preprocessing token
	{
		"CppTokenPastingInvalidRefused", CONFORMANCE_REFUSED,
		"#define CAT(a, b) a ## b\n"
		"int main() { int x = 1; return CAT(x, +); }\n"
	},
	//16.3.3 [cpp.concat] p1: ## isn't first or last in a replacement list
	{
		"CppTokenPastingAtEdgeRefused", CONFORMANCE_REFUSED,
		"#define F(a) ## a\n"
		"int main() { return 0; }\n"
	},
	//16.3.4 [cpp.rescan]: a macro's replacement is rescanned; its own name isn't replaced again in it
	{
		"CppRescan", 31,
		"int x = 5;\n"
		"int AGAIN = 2;\n"
		"#define x (4 + x)\n"
		"#define f(a) a * g\n"
		"#define g(a) f(a)\n"
		"#define AGAIN AGAIN + 1\n"
		"#define OBJ ID\n"
		"#define ID(a) a\n"
		"#define ID2(a) a\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (AGAIN == 3) r += 1;\n"
		"	if (x == 9) r += 2;\n"
		"	int g = 6;\n"
		"	if (f(2) == 12) r += 4;\n"
		"	if (ID(ID2)(8) == 8) r += 8;\n"
		"	if (OBJ(3) == 3) r += 16;\n"
		"	return r;\n"
		"}\n"
	},
	//16.3.5 [cpp.scope]: #undef ends a macro; it may be defined again otherwise; #undef of no macro is nothing
	{
		"CppUndef", 7,
		"#define V 1\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	if (V == 1) r += 1;\n"
		"#undef V\n"
		"#define V 2\n"
		"	if (V == 2) r += 2;\n"
		"#undef V\n"
		"#undef V\n"
		"#undef NEVER_DEFINED\n"
		"#ifndef V\n"
		"	r += 4;\n"
		"#endif\n"
		"	return r;\n"
		"}\n"
	},
	//16.4 [cpp.line]: #line sets the next line's number, and the file's name; it may be made by macros
	{
		"CppLine", 15,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#line 100\n"
		"	if (__LINE__ == 100) r += 1;\n"
		"#line 200 \"other.cpp\"\n"
		"	if (__LINE__ == 200 && __FILE__[0] == 'o' && sizeof(__FILE__) == 10) r += 2;\n"
		"#define LINE_NUMBER 300\n"
		"#line LINE_NUMBER\n"
		"	if (__LINE__ == 300) r += 4;\n"
		"	if (__LINE__ == 301\n"
		"		&& __LINE__ == 302) r += 8;\n"
		"	return r;\n"
		"}\n"
	},
	//16.4 [cpp.line] p3: #line's number isn't 0
	{
		"CppLineZeroRefused", CONFORMANCE_REFUSED,
		"#line 0\n"
		"int main() { return 0; }\n"
	},
	//16.4 [cpp.line] p3: #line takes a number
	{
		"CppLineNotNumberRefused", CONFORMANCE_REFUSED,
		"#line abc\n"
		"int main() { return 0; }\n"
	},
	//16.5 [cpp.error]: #error stops the program
	{
		"CppErrorRefused", CONFORMANCE_REFUSED,
		"#error this program is refused\n"
		"int main() { return 0; }\n"
	},
	//16.6 [cpp.pragma], 16.9 [cpp.pragma.op]: a pragma the compiler doesn't know is ignored; _Pragma("...") is #pragma ..., from a macro too
	{
		"CppPragmas", 7,
		"#pragma not_a_pragma_anyone_knows\n"
		"#define IGNORED _Pragma(\"not_a_pragma_anyone_knows\") 1\n"
		"_Pragma(\"not_a_pragma_anyone_knows\")\n"
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"	r += 1;\n"
		"#pragma another_unknown_pragma with words\n"
		"	r += 2;\n"
		"	if (IGNORED == 1) r += 4;\n"
		"	return r;\n"
		"}\n"
	},
	//16.8 [cpp.predefined]: __cplusplus is 201402L; __LINE__, __FILE__, __DATE__, __TIME__ and __STDC_HOSTED__
	{
		"CppPredefinedMacros", 63,
		"int main()\n"
		"{\n"
		"	int r = 0;\n"
		"#if __cplusplus == 201402L\n"
		"	r += 1;\n"
		"#endif\n"
		"	if (__LINE__ == 7) r += 2;\n"
		"	if (sizeof(__FILE__) > 1) r += 4;\n"
		"	if (sizeof(__DATE__) == 12 && __DATE__[3] == ' ' && __DATE__[6] == ' ') r += 8;\n"
		"	if (sizeof(__TIME__) == 9 && __TIME__[2] == ':' && __TIME__[5] == ':') r += 16;\n"
		"#if defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0 || __STDC_HOSTED__ == 1)\n"
		"	r += 32;\n"
		"#endif\n"
		"	return r;\n"
		"}\n"
	},
};
