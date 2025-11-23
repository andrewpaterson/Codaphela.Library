#ifndef __BASIC_COMPONENT_H__
#define __COMPONENT_H__
#include "BasicComponent.h"


class CComponent : public CBasicComponent
{
CONSTRUCTABLE(CComponent);
public:
	void	Init(CBasicNativeComponent* pcNativeComponent);
	void	Class(void);
	void 	Free(void);

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


#endif // __BASIC_COMPONENT_H__

