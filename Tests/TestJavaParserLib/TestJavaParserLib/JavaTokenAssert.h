#ifndef __JAVA_TOKEN_ASSERT_H__
#define __JAVA_TOKEN_ASSERT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "JavaParserLib/JavaTokenMemory.h"


class CJavaTokenDefinitions;


CJavaToken* PrivateAssertKeyword(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenKeyword eKeyword, size iLine, char* szFile);
CJavaToken* PrivateAssertIdentifier(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char* szIdentifier, size iLine, char* szFile);
CJavaToken* PrivateAssertSeparator(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenSeparator eSeparator, size iLine, char* szFile);
CJavaToken* PrivateAssertOperator(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, EJavaTokenOperator eOperator, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char* szString, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, int64 lli, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, int32 i, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, float32 f, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, float64 d, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char8 c, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, char16 c, size iLine, char* szFile);
CJavaToken* PrivateAssertLiteral(CJavaTokenDefinitions* pcDefinitions, CJavaToken* pcToken, bool b, size iLine, char* szFile);


#define AssertKeyword(p, t, a)		PrivateAssertKeyword(p, t, a, __LINE__, __FILE__)
#define AssertIdentifier(p, t, a)	PrivateAssertIdentifier(p, t, a, __LINE__, __FILE__)
#define AssertSeparator(p, t, a)	PrivateAssertSeparator(p, t, a, __LINE__, __FILE__)
#define AssertOperator(p, t, a)		PrivateAssertOperator(p, t, a, __LINE__, __FILE__)
#define AssertLiteral(p, t, a)		PrivateAssertLiteral(p, t, a, __LINE__, __FILE__)


#endif // !__JAVA_TOKEN_ASSERT_H__

