#ifndef __JAVA_SYNTAX_VARIABLE_DECLARATION_H__
#define __JAVA_SYNTAX_VARIABLE_DECLARATION_H__
#include "JavaModifiers.h"
#include "JavaSyntaxStatement.h"

// X x = y;  
// X (Assignment)

class CJavaSyntaxVariableDeclaration : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxVariableDeclaration);
protected:
	CJavaModifiers	mcModifiers;
	

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsVariableDeclaration(void) override;
};


#endif // !__JAVA_SYNTAX_VARIABLE_DECLARATION_H__

