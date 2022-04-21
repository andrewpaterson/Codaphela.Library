#ifndef __JAVA_TOKENS_H__
#define __JAVA_TOKENS_H__
#include "BaseLib/Constructable.h"
#include "JavaTokenMemory.h"


class CJavaTokens
{
CONSTRUCTABLE(CJavaTokens);
protected:
	CJavaTokenMemory	mcMemory;

public:
	void			Init(void);
	void			Kill(void);

	CJavaComment*	AddComment(char* szComment, int iLength);
};


#endif // !__JAVA_TOKENS_H__

