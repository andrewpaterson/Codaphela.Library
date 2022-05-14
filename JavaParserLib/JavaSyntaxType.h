#ifndef __JAVA_SYNTAX_TYPE_H__
#define __JAVA_SYNTAX_TYPE_H__
#include "JavaSyntaxGeneric.h"


class CJavaSyntaxGeneric;
class CJavaSyntaxType : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxType);
protected:
	CJavaTokenIdentifier*		mpcName;
	CJavaSyntaxGenericPtrArray	mapcGenerics;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsType(void) override;
	void	AddGeneric(CJavaSyntaxGeneric* pcGeneric);
	void	SetName(CJavaTokenIdentifier* pcName);
};


#endif // !__JAVA_SYNTAX_TYPE_H__
