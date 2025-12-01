#ifndef __COMPLEX_COMPONENT_H__
#define __COMPLEX_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "BasicNativeComponent.h"
#include "BasicComponent.h"


class CComplexComponent : public CBasicComponent
{
CONSTRUCTABLE(CComplexComponent);
DESTRUCTABLE(CComplexComponent);
protected:
	CBasicNativeComponent*	mpcNativeComponent;

public:
	void	Init(Ptr<CWindow> pWindow, CBasicNativeComponent* pcNativeComponent);
	void	Class(void);
	void 	Free(void);

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


#endif // __BASIC_COMPONENT_H__

