#ifndef __CONTAINER_H__
#define __CONTAINER_H__
#include "BasicComponent.h"
#include "ContainerBounds.h"


class CContainer : public CBasicComponent
{
CONSTRUCTABLE(CContainer);
DESTRUCTABLE(CContainer);
protected:
	SContainerBounds		msBounds;

public:
			void	Init(Ptr<CWindow> pWindow);
			void	Class(void);
			void 	Free(void);

			bool	GetContainerBounds(SContainerBounds* psDest);
			void	SetBounds(SInt2 sPosition, SInt2 sAreaSize);

	virtual void	SetRequiredSize(void) =0;
			void	Layout(SInt2 sPosition, SInt2 sAreaSize) override;

			bool	Draw(void) override;
};


#endif // __CONTAINER_H__

