#ifndef __SEQUENCE_CONFIG_H__
#define __SEQUENCE_CONFIG_H__
#include "LifeCycle.h"
#include "Constructable.h"
#include "ConstructorCall.h"
#include "Killable.h"
#include "DurableSync.h"


class CSequenceConfig : public CConstructable, public CKillable
{
CONSTRUCTABLE(CSequenceConfig)
public:
			void		Kill(void);

	virtual	const char*		Getdirectory(void) =0;
	virtual	EDurableSync	GetSync(void) =0;
};


#endif // __SEQUENCE_CONFIG_H__

