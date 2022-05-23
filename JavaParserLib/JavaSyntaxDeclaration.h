#ifndef __JAVA_SYNTAX_DECLARATION_H__
#define __JAVA_SYNTAX_DECLARATION_H__
#include "JavaSyntaxStatement.h"

// X x = y;  
// X (Assignment)

class CJavaSyntaxDeclaration : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxDeclaration);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsDeclaration(void) override;
};


#endif // !__JAVA_SYNTAX_DECLARATION_H__

