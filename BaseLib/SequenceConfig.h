#ifndef __SEQUENCE_CONFIG_H__
#define __SEQUENCE_CONFIG_H__
#include "LifeCycle.h"
#include "Constructable.h"
#include "ConstructorCall.h"
#include "Killable.h"


class CSequenceConfig : public CConstructable, public CKillable
{
CONSTRUCTABLE(CSequenceConfig);
public:
			void		Kill(void);

	virtual	char*		Getdirectory(void) =0;
};


#endif // !__SEQUENCE_CONFIG_H__

