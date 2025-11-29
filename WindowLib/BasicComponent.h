#ifndef __BASIC_COMPONENT_H__
#define __BASIC_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "ContainerBounds.h"


class CBasicComponent : public CObject
{
CONSTRUCTABLE(CBasicComponent);
DESTRUCTABLE(CBasicComponent);
protected:
	Ptr<CBasicComponent>		mpParent;
	CArray<CBasicComponent>		maChildren;

public:
			void	Init(void);
			void	Class(void);
			void 	Free(void);

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual	bool	Draw(void);
			bool	DrawChildren(void);
	virtual bool	GetContainerBounds(SContainerBounds* psDest);

			void	AddComponent(Ptr<CBasicComponent> pComponent);
			void	RemoveComponent(Ptr<CBasicComponent> pComponent);
			void	RemoveAllComponents(void);
};


#endif // __BASIC_COMPONENT_H__

