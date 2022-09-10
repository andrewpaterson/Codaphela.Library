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
	bool				mbAbstract;
	bool				mbFinal;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	bool	IsInterface(void) override;
	bool	IsCompoundStatement(void);

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetAbstract(bool bAbstract);
	void	SetFinal(bool bFinal);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxInterface, 6>	CJavaSyntaxInterfacePtrArray;


#endif // !__JAVA_SYNTAX_INTERFACE_H__

