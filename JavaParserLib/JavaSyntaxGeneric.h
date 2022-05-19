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
	CJavaSyntaxType*		mpcGenericType;
	BOOL					mbWildCard;
	CJavaSyntaxType*		mpcExtends;  //NULL if does not extend a Type.

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsGeneric(void) override;

	void	SetGenericType(CJavaSyntaxType* pcGenericType);
	void	SetExtends(CJavaSyntaxType* pcType);
	void	SetWildCard(BOOL bWildCard);
};


typedef CArrayTemplatePtr<CJavaSyntaxGeneric>	CJavaSyntaxGenericPtrArray;


#endif // !__JAVA_SYNTAX_TYPE_GENERIC_H__

