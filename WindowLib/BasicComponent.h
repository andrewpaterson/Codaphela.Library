#ifndef __BASIC_COMPONENT_H__
#define __BASIC_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "SupportLib/ColourFormat.h"
#include "StandardLib/Object.h"
#include "BasicNativeComponent.h"


class CBasicComponent : public CObject
{
CONSTRUCTABLE(CBasicComponent);
protected:
	CBasicNativeComponent*	mpcNativeComponent;

public:
	void	Init(CBasicNativeComponent* pcNativeComponent);
	void	Class(void);
	void 	Free(void);

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


#endif // __BASIC_COMPONENT_H__

