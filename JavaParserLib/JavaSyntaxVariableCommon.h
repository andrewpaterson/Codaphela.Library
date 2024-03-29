#ifndef __JAVA_SYNTAX_VARIABLE_COMMON_H__
#define __JAVA_SYNTAX_VARIABLE_COMMON_H__
#include "JavaSyntaxStatement.h"

// X x = y;  
// X (Assignment)

class CJavaSyntaxVariableCommon : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxVariableCommon);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;

	bool	IsVariableCommon(void) override;
};


#endif // !__JAVA_SYNTAX_VARIABLE_COMMON_H__

