#ifndef __JAVA_ANNOTATION_H__
#define __JAVA_ANNOTATION_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


class CJavaAnnotation : public CJavaToken
{
	CONSTRUCTABLE(CJavaAnnotation);
protected:
	char* mszAnnotation;
	int		miLength;

public:
	void 	Init(char* szAnnotation, int iLength);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char* GetType(void);

	BOOL	IsAnnotation(void);
	BOOL	Is(char* szAnnotation);
};


#endif // !__JAVA_ANNOTATION_H__

