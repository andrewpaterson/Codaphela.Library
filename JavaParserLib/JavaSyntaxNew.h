#ifndef __JAVA_SYNTAX_NEW_H__
#define __JAVA_SYNTAX_NEW_H__
#include "JavaSyntaxExpressionCommon.h"
#include "JavaSyntaxType.h"

// new Type(Arguments)

class CJavaSyntaxNew : public CJavaSyntaxExpressionCommon
{
CONSTRUCTABLE(CJavaSyntaxNew);
protected:
	CJavaSyntaxType*	mpcType;
	CExpressionArray	mapcArguments;

public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	bool				IsNew(void) override;
	bool				IsCompoundStatement(void) override;

	void				SetType(CJavaSyntaxType* pcType);
	CJavaSyntaxType*	GetNewType(void);
	void				AddArgument(CJavaSyntaxExpressionCommon* pcArgument);
	CExpressionArray*	GetArguments(void);
};


#endif // !__JAVA_SYNTAX_NEW_H__

