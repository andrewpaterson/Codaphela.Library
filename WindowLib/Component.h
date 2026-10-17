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
#include "SupportLib/Rectangle.h"


class CWindow;
class CComponent : public CObject, public CFocusListener, public CComponentListener
{
CONSTRUCTABLE(CComponent);
DESTRUCTABLE(CComponent);
protected:
	CRectangle			mcActualBounds;
	SInt32Vec2			msDesiredSize;
	bool				mbCanGetFocus;
	Ptr<CComponent>		mpParent;
	CArray<CComponent>	maChildren;
	Ptr<CWindow>		mpWindow;

public:
			void				Init(Ptr<CWindow> pWindow);
			void				Class(void) override;
			void 				Free(void) override;

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

	virtual	bool				Draw(void);
			bool				DrawChildren(void);
	virtual bool				GetContainerBounds(CRectangle* pcDest);

	virtual	bool				AddComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveComponent(Ptr<CComponent> pComponent);
	virtual	bool				RemoveAllComponents(void);
			size				NumComponents(void);

			bool				IsPointIn(int x, int y);
			bool				HasFocus(void);
			Ptr<CComponent>		FindComponentAt(int x, int y);
			void				ToChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);
			void				FromChildSpace(Ptr<CComponent> pcChildComponent, int x, int y, int* px, int* py);

	virtual	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			void				LayoutChildren(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize);
			size				GetDepth(void);

			bool				IsFocussed(void);

			void				SetActualSize(int fWidth, int fHeight);
			void				SetActualSize(SInt32Vec2 sSize);
			void				SetPosition(int x, int y);
			void				SetPosition(SInt32Vec2 sPosition);
			SInt32Vec2			GetPosition(void);
	virtual	SInt32Vec2			GetDesiredSize(void);
			void				SetDesiredSize(int fWidth, int fHeight);
			SInt32Vec2			GetActualSize(void);
};


#endif // __COMPONENT_H__

