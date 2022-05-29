#ifndef __JAVA_SYNTAX_CLASS_BLOCK_H__
#define __JAVA_SYNTAX_CLASS_BLOCK_H__
#include "JavaSyntax.h"
#include "JavaSyntaxStatement.h"

// { Statement ; ... Statement ; }

// Variable Declaration
// Code Block
// Constructor Declaration
// Method Declaration
// Class
// Enum
// Interface

class CJavaSyntaxClassBlock : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxClassBlock);
protected:
	CStatementArray		mapcStatements;
		
public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	BOOL				IsClassBlock(void) override;

	void				AddStatement(CJavaSyntaxStatement* pcStatement);

	CStatementArray*	GetStatements(void);
};


#endif // !__JAVA_SYNTAX_CLASS_BLOCK_H__

