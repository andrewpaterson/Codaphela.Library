#ifndef __BASIC_NATIVE_COMPONENT_H__
#define __BASIC_NATIVE_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"


class CNativeWindowFactory;
class CBasicNativeComponent : public CConstructable, public CKillable
{
CONSTRUCTABLE(CBasicNativeComponent);
protected:
	CNativeWindowFactory*	mpcWindowFactory;

public:
	void					Init(CNativeWindowFactory* pcWindowFactory);
	void					Kill(void);

	CNativeWindowFactory*	GetFactory(void);
};


#endif // __BASIC_NATIVE_COMPONENT_H__

