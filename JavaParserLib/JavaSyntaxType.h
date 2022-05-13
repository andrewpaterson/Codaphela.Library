#ifndef __JAVA_SYNTAX_TYPE_H__
#define __JAVA_SYNTAX_TYPE_H__
#include "JavaSyntax.h"


class CJavaSyntaxTypeGeneric;
class CJavaSyntaxType : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxType);
protected:
	CJavaSyntaxTypeGeneric* mpcGeneric;  //NULL if does not have a Generic.

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsType(void) override;
};


#endif // !__JAVA_SYNTAX_TYPE_H__

