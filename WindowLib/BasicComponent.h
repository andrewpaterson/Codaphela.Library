#ifndef __BASIC_COMPONENT_H__
#define __BASIC_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "SupportLib/ColourFormat.h"
#include"BasicNativeComponent.h"


class CBasicComponent : public CConstructable, public CKillable
{
CONSTRUCTABLE(CBasicComponent);
protected:
	CBasicNativeComponent*	mpcNativeComponent;

public:
	void	Init(CBasicNativeComponent* pcNativeComponent);
	void	Kill(void);
};


#endif // __BASIC_COMPONENT_H__

