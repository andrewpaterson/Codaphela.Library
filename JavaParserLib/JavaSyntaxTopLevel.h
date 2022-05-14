#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxTopLevel : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTopLevel);
protected:
	CJavaTokenIdentifier*	mpcName;
	BOOL				mbPublic;
	BOOL				mbAbstract;
	BOOL				mbFinal;

public:
	void 	Init(CJavaSyntaxTree* pcTree);
	void 	Kill(void);

	char*	GetType(void) override;

	BOOL	IsTopLevel(void) override;

	void	SetName(CJavaTokenIdentifier* pcName);
	void	SetPublic(BOOL bPublic);
	void	SetAbstract(BOOL bAbstract);
	void	SetFinal(BOOL bFinal);
};


typedef CArrayTemplatePtr<CJavaSyntaxTopLevel>	CJavaSyntaxTopLevelPtrArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

