#ifndef __JAVA_SYNTAX_INTERFACE_H__
#define __JAVA_SYNTAX_INTERFACE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxInterface : public CJavaSyntaxClassCommon
{
CONSTRUCTABLE(CJavaSyntaxInterface);
protected:
	CJavaSyntaxType*	mpcType;
	BOOL				mbAbstract;
	BOOL				mbFinal;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	BOOL	IsInterface(void) override;
	BOOL	IsCompoundStatement(void);

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetAbstract(BOOL bAbstract);
	void	SetFinal(BOOL bFinal);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxInterface, 6>	CJavaSyntaxInterfacePtrArray;


#endif // !__JAVA_SYNTAX_INTERFACE_H__

