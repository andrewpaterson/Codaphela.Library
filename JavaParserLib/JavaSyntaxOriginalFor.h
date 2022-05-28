#ifndef __JAVA_SYNTAX_ORIGINAL_FOR_H__
#define __JAVA_SYNTAX_ORIGINAL_FOR_H__
#include "JavaSyntaxStatement.h"

//for {? ; ? ; ? , ?} { ... }
class CJavaSyntaxOriginalFor : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxOriginalFor);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;

	BOOL	IsOriginalFor(void) override;
};


#endif // !__JAVA_SYNTAX_ORIGINAL_FOR_H__

