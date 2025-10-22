#ifndef __WIN_GDI_WINDOW_FACTORY_H__
#define __WIN_GDI_WINDOW_FACTORY_H__
#include <windows.h>
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "WindowLib/NativeWindowFactory.h"


class CWinGDIWindowFactory : public CNativeWindowFactory
{
CONSTRUCTABLE(CWinGDIWindowFactory);
protected:
	CChars					mszWindowClass;
	HINSTANCE				mhInstance;
	HINSTANCE 				mhPrevInstance;
	int						miCmdShow;

public:
	void			Init(CMallocator* pcMallocator, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass);
	void			Kill(void);

	CNativeWindow*	CreateNativeWindow(char* szWindowTitle) override;

protected:

};


#endif // __WIN_GDI_WINDOW_FACTORY_H__

