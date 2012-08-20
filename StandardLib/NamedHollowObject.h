#ifndef __NAMED_HOLLOW_OBJECT_H__
#define __NAMED_HOLLOW_OBJECT_H__
#include "HollowObject.h"


class CNamedHollowObject : public CHollowObject
{
BASE_FUNCTIONS(CNamedHollowObject);
protected:
	CChars	mon;

public:
	void	Kill(void);

	char*	GetName(void);
	BOOL	IsNamed(void);

private:
	BOOL	InitName(char* szName);
};


#endif // __NAMED_HOLLOW_OBJECT_H__

