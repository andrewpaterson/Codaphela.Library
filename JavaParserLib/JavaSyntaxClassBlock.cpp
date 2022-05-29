#include "JavaSyntaxClassBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClassBlock::Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent)
{
	CJavaSyntax::Init(pcTree, pcParent);
	mapcStatements.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClassBlock::Kill(void)
{
	mapcStatements.Kill();
	CJavaSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaSyntaxClassBlock::GetType(void)
{
	return "Block";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClassBlock::TypePrint(CChars* pszDest, int iDepth)
{
	int						i;
	CJavaSyntaxStatement*	pcStatement;

	CJavaSyntax::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();

	for (i = 0; i < mapcStatements.NumElements(); i++)
	{
		pcStatement = mapcStatements.GetPtr(i);
		pcStatement->TypePrint(pszDest, iDepth + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClassBlock::PrettyPrint(CChars* pszDest, int iBlockDepth)
{
	int						i;
	CJavaSyntaxStatement* pcStatement;

	if (mapcStatements.IsNotEmpty())
	{
		pszDest->Append('\t', iBlockDepth);
		pszDest->Append('{');
		pszDest->AppendNewLine();
		for (i = 0; i < mapcStatements.NumElements(); i++)
		{
			pcStatement = mapcStatements.GetPtr(i);
			if (!pcStatement->IsCompoundStatement())
			{
				pszDest->Append('\t', iBlockDepth + 1);
				pcStatement->PrettyPrint(pszDest, iBlockDepth + 1);
				pszDest->Append(';');
				pszDest->AppendNewLine();
			}
			else
			{
				pcStatement->PrettyPrint(pszDest, iBlockDepth + 1);
			}
		}
		pszDest->Append('\t', iBlockDepth);
		pszDest->Append('}');
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaSyntaxClassBlock::IsClassBlock(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaSyntaxClassBlock::AddStatement(CJavaSyntaxStatement* pcStatement)
{
	mapcStatements.Add(pcStatement);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatementArray* CJavaSyntaxClassBlock::GetStatements(void)
{
	return &mapcStatements;
}

