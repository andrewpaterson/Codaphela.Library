#ifndef __JAVA_SYNTAX_TYPE_COMMON_H__
#define __JAVA_SYNTAX_TYPE_COMMON_H__
#include "JavaTokenIdentifier.h"
#include "JavaSyntax.h"


class CJavaSyntaxTypeCommon : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTypeCommon);
protected:
	CJavaTokenIdentifier*	mpcName;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	BOOL	IsTypeCommon(void);

	void	SetName(CJavaTokenIdentifier* pcName);
};


typedef CArrayTemplatePtr<CJavaSyntaxTypeCommon>	CJavaSyntaxTypeCommonPtrArray;


#endif // !__JAVA_SYNTAX_TYPE_H__


