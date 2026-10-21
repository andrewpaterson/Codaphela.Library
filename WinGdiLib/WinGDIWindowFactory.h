#ifndef __WIN_GDI_WINDOW_FACTORY_H__
#define __WIN_GDI_WINDOW_FACTORY_H__
#include <windows.h>
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "SupportLib/ColourFormat.h"
#include "WindowLib/NativeWindowFactory.h"


//Called with each WM_INPUT's RAWINPUT.  WinRawInputCallback in WinInputLib is one.
typedef void (*WinGDIRawInputFunc)(void* pvContext, void* pvRawInput);


class CWinGDIWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinGDIWindowFactory);
protected:
	CChars					mszWindowClass;
	HINSTANCE				mhInstance;
	HINSTANCE 				mhPrevInstance;
	HWND					mhWnd;
	int						miCmdShow;
	WinGDIRawInputFunc		mfRawInput;
	void*					mpvRawInputContext;

public:
	void			Init(CMallocator* pcMallocator, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(CWindow* pcWindow) override;
	CNativeCanvas*	CreateNativeCanvas(CCanvas* pcCanvas) override;

	void			SetHWnd(HWND hWnd);
	HWND			GetHWnd(void);

	void			SetRawInput(WinGDIRawInputFunc fRawInput, void* pvContext);
	void			RawInput(void* pvRawInput);

protected:

};


#endif // __WIN_GDI_WINDOW_FACTORY_H__

