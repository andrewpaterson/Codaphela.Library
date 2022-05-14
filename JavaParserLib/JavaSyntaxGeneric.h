#ifndef __JAVA_SYNTAX_TYPE_GENERIC_H__
#define __JAVA_SYNTAX_TYPE_GENERIC_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxType;
class CJavaSyntaxGeneric : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxGeneric);
protected:
	CJavaTokenIdentifier*	mpcName;
	BOOL					mbWildCard;
	CJavaSyntaxType*		mpcExtends;  //NULL if does not extend a Type.

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsGeneric(void) override;

	void	SetName(CJavaTokenIdentifier* pcName);
	void	SetExtends(CJavaSyntaxType* pcType);
	void	SetWildCard(BOOL bWildCard);
};


typedef CArrayTemplatePtr<CJavaSyntaxGeneric>	CJavaSyntaxGenericPtrArray;


#endif // !__JAVA_SYNTAX_TYPE_GENERIC_H__

