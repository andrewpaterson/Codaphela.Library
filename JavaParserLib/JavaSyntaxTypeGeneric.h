#ifndef __JAVA_SYNTAX_TYPE_GENERIC_H__
#define __JAVA_SYNTAX_TYPE_GENERIC_H__
#include "JavaSyntax.h"
#include "JavaIdentifier.h"


class CJavaSyntaxType;
class CJavaSyntaxTypeGeneric : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTypeGeneric);
protected:
	CJavaIdentifier* mpcName;
	CJavaSyntaxType* mpcType;  //NULL if does not extend a Type.

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsTypeGeneric(void) override;
};


#endif // !__JAVA_SYNTAX_TYPE_GENERIC_H__

