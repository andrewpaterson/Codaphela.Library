#include "BaseLib/EscapeCodes.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestEscapeCodes(void)
{
	BeginTests();

	char	sz[5];

	AssertString("\\0",   GetEscapeString((char)0, sz));
	AssertString("\\x01", GetEscapeString((char)1, sz));
	AssertString("\\x02", GetEscapeString((char)2, sz));
	AssertString("\\x03", GetEscapeString((char)3, sz));
	AssertString("\\x04", GetEscapeString((char)4, sz));
	AssertString("\\x05", GetEscapeString((char)5, sz));
	AssertString("\\x06", GetEscapeString((char)6, sz));
	AssertString("\\a",   GetEscapeString((char)7, sz));
	AssertString("\\b",   GetEscapeString((char)8, sz));
	AssertString("\\t",   GetEscapeString((char)9, sz));
	AssertString("\\n",   GetEscapeString((char)10, sz));
	AssertString("\\v",   GetEscapeString((char)11, sz));
	AssertString("\\f",   GetEscapeString((char)12, sz));
	AssertString("\\r",   GetEscapeString((char)13, sz));
	AssertString("\\x0e", GetEscapeString((char)14, sz));
	AssertString("\\x0f", GetEscapeString((char)15, sz));
	AssertString("\\x10", GetEscapeString((char)16, sz));
	AssertString("\\x11", GetEscapeString((char)17, sz));
	AssertString("\\x12", GetEscapeString((char)18, sz));
	AssertString("\\x13", GetEscapeString((char)19, sz));
	AssertString("\\x14", GetEscapeString((char)20, sz));
	AssertString("\\x15", GetEscapeString((char)21, sz));
	AssertString("\\x16", GetEscapeString((char)22, sz));
	AssertString("\\x17", GetEscapeString((char)23, sz));
	AssertString("\\x18", GetEscapeString((char)24, sz));
	AssertString("\\x19", GetEscapeString((char)25, sz));
	AssertString("\\x1a", GetEscapeString((char)26, sz));
	AssertString("\\x1b", GetEscapeString((char)27, sz));
	AssertString("\\x1c", GetEscapeString((char)28, sz));
	AssertString("\\x1d", GetEscapeString((char)29, sz));
	AssertString("\\x1e", GetEscapeString((char)30, sz));
	AssertString("\\x1f", GetEscapeString((char)31, sz));
	AssertString(" ",     GetEscapeString((char)32, sz));
	AssertString("!",     GetEscapeString((char)33, sz));
	AssertString("\\\"",  GetEscapeString((char)34, sz));
	AssertString("#",     GetEscapeString((char)35, sz));
	AssertString("$",     GetEscapeString((char)36, sz));
	AssertString("%",     GetEscapeString((char)37, sz));
	AssertString("&",     GetEscapeString((char)38, sz));
	AssertString("\\\'",  GetEscapeString((char)39, sz));
	AssertString("(",     GetEscapeString((char)40, sz));
	AssertString(")",     GetEscapeString((char)41, sz));
	AssertString("*",     GetEscapeString((char)42, sz));
	AssertString("+",     GetEscapeString((char)43, sz));
	AssertString(",",     GetEscapeString((char)44, sz));
	AssertString("-",     GetEscapeString((char)45, sz));
	AssertString(".",     GetEscapeString((char)46, sz));
	AssertString("/",     GetEscapeString((char)47, sz));
	AssertString("0",     GetEscapeString((char)48, sz));
	AssertString("1",     GetEscapeString((char)49, sz));
	AssertString("2",     GetEscapeString((char)50, sz));
	AssertString("3",     GetEscapeString((char)51, sz));
	AssertString("4",     GetEscapeString((char)52, sz));
	AssertString("5",     GetEscapeString((char)53, sz));
	AssertString("6",     GetEscapeString((char)54, sz));
	AssertString("7",     GetEscapeString((char)55, sz));
	AssertString("8",     GetEscapeString((char)56, sz));
	AssertString("9",     GetEscapeString((char)57, sz));
	AssertString(":",     GetEscapeString((char)58, sz));
	AssertString(";",     GetEscapeString((char)59, sz));
	AssertString("<",     GetEscapeString((char)60, sz));
	AssertString("=",     GetEscapeString((char)61, sz));
	AssertString(">",     GetEscapeString((char)62, sz));
	AssertString("\\\?",  GetEscapeString((char)63, sz));
	AssertString("@",     GetEscapeString((char)64, sz));
	AssertString("A",     GetEscapeString((char)65, sz));

	AssertString("[",     GetEscapeString((char)91, sz));
	AssertString("\\\\",  GetEscapeString((char)92, sz));
	AssertString("]",     GetEscapeString((char)93, sz));
	AssertString("^",     GetEscapeString((char)94, sz));
	AssertString("_",     GetEscapeString((char)95, sz));
	AssertString("`",     GetEscapeString((char)96, sz));
	AssertString("a",     GetEscapeString((char)97, sz));

	AssertString("|",     GetEscapeString((char)124, sz));
	AssertString("}",     GetEscapeString((char)125, sz));
	AssertString("~",     GetEscapeString((char)126, sz));
	AssertString("\\x7f", GetEscapeString((char)127, sz));

	AssertString("€",     GetEscapeString((uint8)128, sz));
	AssertString("\\x81", GetEscapeString((uint8)129, sz));
	AssertString("‚",     GetEscapeString((uint8)130, sz));
	AssertString("ƒ",     GetEscapeString((uint8)131, sz));
	AssertString("„",     GetEscapeString((uint8)132, sz));

	AssertString("ú",     GetEscapeString((uint8)250, sz));
	AssertString("û",     GetEscapeString((uint8)251, sz));
	AssertString("ü",     GetEscapeString((uint8)252, sz));
	AssertString("ý",     GetEscapeString((uint8)253, sz));
	AssertString("þ",     GetEscapeString((uint8)254, sz));
	AssertString("\\xff", GetEscapeString((uint8)255, sz));

	TestStatistics();
}

