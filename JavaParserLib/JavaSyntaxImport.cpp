#include "JavaSyntaxImport.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxImport::Init(CJavaSyntaxTree* pcTree)
{
	CJavaSyntax::Init(pcTree);
	mapcIdentifiers.Init();
	mbStatic = FALSE;
	mbWild = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxImport::Kill(void)
{
	mbStatic = FALSE;
	mbWild = FALSE;
	mapcIdentifiers.Kill();
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxImport::GetType(void)
{
	return "Import";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxImport::AddIdentifier(CJavaIdentifier* pcIdentifier)
{
	mapcIdentifiers.Add(pcIdentifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxImport::SetStatic(BOOL bStatic)
{
	mbStatic = bStatic;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxImport::SetWild(BOOL bWild)
{
	mbWild = bWild;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxImport::IsImport(void)
{
	return TRUE;
}

