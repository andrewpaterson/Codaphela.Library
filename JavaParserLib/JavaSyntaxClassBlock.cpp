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
	size					i;
	CJavaSyntaxStatement*	pcStatement;
	size					uiNumElements;

	CJavaSyntax::TypePrint(pszDest, iDepth);
	pszDest->AppendNewLine();

	uiNumElements = mapcStatements.NumElements();
	for (i = 0; i < uiNumElements; i++)
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
	size					i;
	CJavaSyntaxStatement*	pcStatement;
	size					uiNumElements;

	if (mapcStatements.IsNotEmpty())
	{
		pszDest->Append('\t', iBlockDepth);
		pszDest->Append('{');
		pszDest->AppendNewLine();

		uiNumElements = mapcStatements.NumElements();
		for (i = 0; i < uiNumElements; i++)
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
bool CJavaSyntaxClassBlock::IsClassBlock(void)
{
	return true;
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

