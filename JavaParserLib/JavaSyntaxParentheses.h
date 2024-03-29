#ifndef __JAVA_SYNTAX_PARENTHESES_H__
#define __JAVA_SYNTAX_PARENTHESES_H__
#include "JavaSyntaxExpressionCommon.h"


class CJavaSyntaxParentheses : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxParentheses);
protected:
	CJavaSyntaxExpressionCommon*	mpcExpression;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	bool	IsParentheses(void) override;
	bool	IsCompoundStatement(void) override;

	void	SetExpression(CJavaSyntaxExpressionCommon* pcExpression);
};


#endif // !__JAVA_SYNTAX_PARENTHESES_H__

