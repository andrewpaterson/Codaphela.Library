#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaModifier.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxClassCommon : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxClassCommon);
protected:
	CJavaModifiers	mcModifiers;

public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;

	BOOL				IsClassCommon(void) override;

	CJavaModifiers*		GetModifiers(void);
	void				SetModifiers(CJavaModifiers cModifiers);
};


typedef CArrayTemplatePtr<CJavaSyntaxClassCommon>	CJavaSyntaxClassCommonPtrArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

