#ifndef __JAVA_SYNTAX_ASSIGNMENT_H__
#define __JAVA_SYNTAX_ASSIGNMENT_H__
#include "JavaSyntaxStatement.h"

// x = y; 
// x = (Expression);  =, +=, >>>=  etc...

class CJavaSyntaxAssignment : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxAssignment);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsAssignment(void) override;
};


#endif // !__JAVA_SYNTAX_ASSIGNMENT_H__

