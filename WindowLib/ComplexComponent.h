#ifndef __COMPLEX_COMPONENT_H__
#define __COMPLEX_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "BasicNativeComponent.h"
#include "BasicComponent.h"


//I don't know what this class achieves anymore.
class CComplexComponent : public CBasicComponent
{
CONSTRUCTABLE(CComplexComponent);
DESTRUCTABLE(CComplexComponent);
protected:
public:
	void	Init(Ptr<CWindow> pWindow);
	void	Class(void);
	void 	Free(void);

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
};


#endif // __BASIC_COMPONENT_H__

