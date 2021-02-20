#ifndef __SEQUENCE_CONFIG_H__
#define __SEQUENCE_CONFIG_H__
#include "BaseLib/LifeCycle.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Killable.h"


class CSequenceConfig : public CConstructable, public CKillable
{
CONSTRUCTABLE(CSequenceConfig);
public:
			void		Kill(void);

	virtual	char*		Getdirectory(void) = 0;
};


#endif // __SEQUENCE_CONFIG_H__

