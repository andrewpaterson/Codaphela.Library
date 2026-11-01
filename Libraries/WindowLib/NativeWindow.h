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
	CWindow*		mpcWindow;
	CNativeWindow*	mpcOwner;  //The window a dialog is shown over, NULL for a main window.
	CInt32Rect		mcLastRectangle;
	SInt32Vec2		msRequestedSize;  //The client size to create the window with, -1 for the native default.
	bool			mbPainting;
	bool			mbEnabled;  //A disabled window, the owner of a modal dialog, ignores the keyboard and pointer.

public:
			void			Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory);
			void			Kill(void);

			bool			Present(void);
			void			Stop(void);

			void			SetOwner(CNativeWindow* pcOwner);
			CNativeWindow*	GetOwner(void);
			void			SetRequestedSize(int32 iWidth, int32 iHeight);
			SInt32Vec2		GetRequestedSize(void);
	virtual	void			SetEnabled(bool bEnabled);
			bool			IsEnabled(void);

protected:
			bool	BeginPresent(void);
			void	EndPresent(void);

	virtual	void	Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight) =0;

	virtual bool	CreateNativeWindow(void) =0;
	virtual void	DestroyNativeWindow(void) =0;  //Takes the window off the screen, as when a dialog closes.  It can be created again.
	virtual bool	ExecuteNativeWindow(void) =0;
	virtual void	SignalPresent(void) =0;
	virtual void	GetRectangle(CInt32Rect* pcDest) =0;
};


#endif // __NATIVE_WINDOW_H__

