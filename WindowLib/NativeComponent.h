#ifndef __NATIVE_COMPONENT_H__
#define __NATIVE_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"


class CNativeWindowFactory;
class CNativeComponent : public CConstructable, public CKillable
{
CONSTRUCTABLE(CNativeComponent);
protected:
	CNativeWindowFactory*	mpcWindowFactory;

public:
	void					Init(CNativeWindowFactory* pcWindowFactory);
	void					Kill(void);

	CNativeWindowFactory*	GetFactory(void);
};


#endif // __NATIVE_COMPONENT_H__

