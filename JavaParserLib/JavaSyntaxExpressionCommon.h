#ifndef __JAVA_SYNTAX_EXPRESSION_COMMON_H__
#define __JAVA_SYNTAX_EXPRESSION_COMMON_H__
#include "JavaSyntaxStatement.h"

// (ExpressionCommon); x = (ExpressionCommon)

class CJavaSyntaxExpressionCommon : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxExpressionCommon);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;

	BOOL	IsExpressionCommon(void) override;
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxExpressionCommon, 6> CExpressionArray;


#endif // !__JAVA_SYNTAX_EXPRESSION_COMMON_H__

