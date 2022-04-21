#ifndef __JAVA_TOKEN_MEMORY_H__
#define __JAVA_TOKEN_MEMORY_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/MemoryStackExtended.h"
#include "JavaComment.h"


class CJavaTokenMemory
{
CONSTRUCTABLE(CJavaTokenMemory);
protected:
	CMemoryStackExtended	mcStack;

public:
	void			Init(void);
	void			Kill(void);

	CJavaComment*	CreateComment(char* szComment, int iLength);
};


#endif // !__JAVA_TOKEN_MEMORY_H__

	