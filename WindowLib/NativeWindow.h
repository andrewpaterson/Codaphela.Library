#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Chars.h"
#include "SupportLib/Int32Rect.h"
#include "NativeComponent.h"


class CNativeWindowFactory;
class CNativeCanvas;
class CWindow;
class CNativeWindow : public CNativeComponent
{
CONSTRUCTABLE(CNativeWindow);
friend class CWindow;
protected:
	CWindow*	mpcWindow;
	CInt32Rect	mcLastRectangle;
	bool		mbPainting;

public:
			void	Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory);
			void	Kill(void);

			bool	Present(void);
			void	Stop(void);

protected:
			bool	BeginPresent(void);
			void	EndPresent(void);

	virtual	void	Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) =0;

	virtual bool	CreateNativeWindow(void) =0;
	virtual bool	ExecuteNativeWindow(void) =0;
	virtual void	SignalPresent(void) =0;
	virtual void	GetRectangle(CInt32Rect* pcDest) =0;
};


#endif // __NATIVE_WINDOW_H__

