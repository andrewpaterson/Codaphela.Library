#ifndef __JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__
#define __JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxArrayValueExpression : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxArrayValueExpression);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	BOOL	IsArrayValueExpression(void) override;
	BOOL	IsCompoundStatement(void);
};


#endif // !__JAVA_SYNTAX_ARRAY_VALUE_EXPRESSION_H__

