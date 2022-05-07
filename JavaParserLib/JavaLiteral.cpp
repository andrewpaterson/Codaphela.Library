#include "BaseLib/NewLine.h"
#include "JavaLiteral.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaLiteral::Init(void)
{
	CJavaToken::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaLiteral::Kill(void)
{
	CJavaToken::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaLiteral::GetType(void) { return "Literal"; }
BOOL CJavaLiteral::IsLiteral(void) { return TRUE; }
BOOL CJavaLiteral::IsCharacter(void) { return FALSE; }
BOOL CJavaLiteral::IsBoolean(void) { return FALSE; }
BOOL CJavaLiteral::IsString(void) { return FALSE; }
BOOL CJavaLiteral::IsInteger(void) { return FALSE; }
BOOL CJavaLiteral::IsFloat(void) { return FALSE; }
BOOL CJavaLiteral::IsNull(void) { return FALSE; }

