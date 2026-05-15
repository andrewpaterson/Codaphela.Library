#ifndef __COMPONENT_H__
#define __COMPONENT_H__
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
class CComponent : public CObject, public CFocusListener, public CComponentListener
{
CONSTRUCTABLE(CComponent);
DESTRUCTABLE(CComponent);
protected:
	SInt32Vec2			msActualSize;
	SInt32Vec2	 		msPosition;
	SInt32Vec2			msDesiredSize;
	bool				mbCanGetFocus;
	SInt32Vec2			msRequiredSize;
	Ptr<CComponent>		mpParent;
	CArray<CComponent>	maChildren;
	Ptr<CWindow>		mpWindow;

public:
			void					Init(Ptr<CWindow> pWindow);
			void					Class(void);
			void 					Free(void);

			bool					Save(CObjectWriter* pcFile) override;
			bool					Load(CObjectReader* pcFile) override;

	virtual	bool					Draw(void);
			bool					DrawChildren(void);
	virtual bool					GetContainerBounds(SContainerBounds* psDest);

			void					AddComponent(Ptr<CComponent> pComponent);
			bool					RemoveComponent(Ptr<CComponent> pComponent);
			bool					RemoveAllComponents(void);
			size					NumComponents(void);

			bool					IsPointIn(int x, int y);
			bool					HasFocus(void);
			Ptr<CComponent>			FindComponentAt(int x, int y);
			void					ToChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);
			void					FromChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);

	virtual	void					Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			void					LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			size					GetDepth(void);

			bool					IsFocussed(void);

			void					SetActualSize(int fWidth, int fHeight);
			void					SetActualSize(SInt32Vec2 sSize);
			void					SetPosition(int x, int y);
			void					SetPosition(SInt32Vec2 sPosition);
			SInt32Vec2					GetPosition(void);
			SInt32Vec2					GetDesiredSize(void);
			void					SetDesiredSize(int fWidth, int fHeight);
			SInt32Vec2					GetActualSize(void);

	virtual void					SetRequiredSize(void) =0;
};


#endif // __COMPONENT_H__

