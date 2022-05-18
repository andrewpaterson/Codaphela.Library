#ifndef __JAVA_ANNOTATION_H__
#define __JAVA_ANNOTATION_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaTokenAnnotation : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenAnnotation);
protected:
	char*	mszAnnotation;
	int		miLength;

public:
	void 	Init(STextPosition* psPosition, char* szAnnotation, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);

	BOOL	IsAnnotation(void);
	BOOL	Is(char* szAnnotation);
};


#endif // !__JAVA_ANNOTATION_H__

