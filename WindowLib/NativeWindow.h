#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Chars.h"
#include "SupportLib/Rectangle.h"
#include "BasicNativeComponent.h"


class CNativeWindowFactory;
class CNativeCanvas;
class CNativeWindow : public CBasicNativeComponent
{
CONSTRUCTABLE(CNativeWindow);
friend class CWindow;
protected:
	CWindow*	mpcWindow;
	CRectangle	mcLastRectangle;
	bool		mbPainting;

public:
			void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory);
			void	Kill(void);

			bool	Present(void);

protected:
			bool	BeginPresent(CRectangle* pcRectangle);
			void	EndPresent(CRectangle* pcRectangle);

	virtual	void	Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) =0;

	virtual bool	CreateNativeWindow(void) =0;
	virtual bool	ExecuteNativeWindow(void) =0;
	virtual void	SignalPresent(void) =0;
	virtual void	GetRectangle(CRectangle* pcDest) =0;
};


#endif // __NATIVE_WINDOW_H__


