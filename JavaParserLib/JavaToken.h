#ifndef __JAVA_TOKEN_H__
#define __JAVA_TOKEN_H__
#include "BaseLib/Constructable.h"


class CJavaToken
{
CONSTRUCTABLE(CJavaToken);
protected:
	CJavaToken*		mpcNext;

public:
			void	Init(void);
	virtual void	Kill(void);
};


#endif // !__JAVA_TOKEN_H__

