#ifndef __BASIC_COMPONENT_H__
#define __COMPONENT_H__
#include "BasicComponent.h"


class CComponent : public CBasicComponent
{
CONSTRUCTABLE(CComponent);
public:
	void	Init(CBasicNativeComponent* pcNativeComponent);
	void	Kill(void);
};


#endif // __BASIC_COMPONENT_H__

