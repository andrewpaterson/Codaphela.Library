#ifndef __JAVA_SYNTAX_INITIALISER_H__
#define __JAVA_SYNTAX_INITIALISER_H__
#include "JavaSyntax.h"
#include "JavaTokenLiteral.h"
#include "JavaSyntaxValueExpression.h"
#include "JavaSyntaxArrayValueExpression.h"


class CJavaSyntaxVariableInitialiser : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxVariableInitialiser);
protected:
	CJavaSyntaxArrayValueExpression*	mpcArrayValue;
	CJavaSyntaxValueExpression*			mpValue;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	char*	PrettyPrint(CChars* pszDest) override;
	BOOL	IsVariableInitialiser(void) override;
};


#endif // !__JAVA_SYNTAX_INITIALISER_H__

