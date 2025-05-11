#ifndef __REFERED_H__
#define __REFERED_H__
#include "PrimitiveTypes.h"


class CRefered
{
protected:
	int32	miReferenceCount;

public:
			void	Init(void);
	
	virtual bool	CanKill(void);
	virtual void	ReferenceAdded(void);
	virtual void	ReferenceFreed(void);
};


#define TryKill(p) if (p) { if (p->CanKill()) { p->Kill(); free(p); p = NULL; } }


#endif // __REFERED_H__

