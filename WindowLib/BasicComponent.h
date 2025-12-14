#ifndef __BASIC_COMPONENT_H__
#define __BASIC_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "SupportLib/ColourFormat.h"
#include "Focus.h"
#include "FocusListener.h"
#include "ComponentListener.h"
#include "ContainerBounds.h"


class CWindow;
class CBasicComponent : public CObject, public CFocusListener, public CComponentListener
{
CONSTRUCTABLE(CBasicComponent);
DESTRUCTABLE(CBasicComponent);
protected:
	SInt2						msActualSize;
	SInt2	 					msPosition;
	SInt2						msDesiredSize;
	bool						mbCanGetFocus;
	Ptr<CBasicComponent>		mpParent;
	CArray<CBasicComponent>		maChildren;
	Ptr<CWindow>				mpWindow;

public:
			void					Init(Ptr<CWindow> pWindow);
			void					Class(void);
			void 					Free(void);

			bool					Save(CObjectWriter* pcFile) override;
			bool					Load(CObjectReader* pcFile) override;

	virtual	bool					Draw(void);
			bool					DrawChildren(void);
	virtual bool					GetContainerBounds(SContainerBounds* psDest);

			void					AddComponent(Ptr<CBasicComponent> pComponent);
			bool					RemoveComponent(Ptr<CBasicComponent> pComponent);
			bool					RemoveAllComponents(void);
			size					NumComponents(void);

			bool					IsPointIn(int x, int y);
			bool					HasFocus(void);
			Ptr<CBasicComponent>	FindComponentAt(int x, int y);
			void					ToChildSpace(Ptr<CBasicComponent> pcChildComponent, int x, int y, int* px, int* py);
			void					FromChildSpace(Ptr<CBasicComponent> pcChildComponent, int x, int y, int* px, int* py);

	virtual	void					Layout(SInt2 sPosition, SInt2 sAreaSize);
			void					LayoutChildren(SInt2 sPosition, SInt2 sAreaSize);
			size					GetDepth(void);

			bool					IsFocussed(void);

			void					SetActualSize(int fWidth, int fHeight);
			void					SetActualSize(SInt2 sSize);
			void					SetPosition(int x, int y);
			void					SetPosition(SInt2 sPosition);
			SInt2					GetPosition(void);
			SInt2					GetDesiredSize(void);
			void					SetDesiredSize(int fWidth, int fHeight);
			SInt2					GetActualSize(void);
};


#endif // __BASIC_COMPONENT_H__

