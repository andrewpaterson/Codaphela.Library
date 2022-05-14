#include "BaseLib/NewLine.h"
#include "JavaTokenLiteral.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenLiteral::Init(EJavaTokenLiteralType eType)
{
	CJavaToken::Init();
	meLiteralType = eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenLiteral::Kill(void)
{
	CJavaToken::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenLiteral::IsLiteralType(EJavaTokenLiteralType eType)
{
	return meLiteralType == eType;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaTokenLiteral::GetType(void) { return "Literal"; }
BOOL CJavaTokenLiteral::IsLiteral(void) { return TRUE; }
BOOL CJavaTokenLiteral::IsCharacter(void) { return FALSE; }
BOOL CJavaTokenLiteral::IsBoolean(void) { return FALSE; }
BOOL CJavaTokenLiteral::IsString(void) { return FALSE; }
BOOL CJavaTokenLiteral::IsInteger(void) { return FALSE; }
BOOL CJavaTokenLiteral::IsFloat(void) { return FALSE; }
BOOL CJavaTokenLiteral::IsNull(void) { return FALSE; }
