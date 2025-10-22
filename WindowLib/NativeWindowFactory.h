#ifndef __NATIVE_WINDOW_FACTORY_H__
#define __NATIVE_WINDOW_FACTORY_H__
#include "BaseLib/Constructable.h"
#include "NativeWindow.h"


class CNativeWindowFactory : public CConstructable
{
CONSTRUCTABLE(CNativeWindowFactory);
friend class CWindowFactory;
protected:
	CMallocator*	mpcMallocator;

public:
			void			Init(CMallocator* pcMallocator);
			void			Kill(void);

	virtual CNativeWindow*	CreateNativeWindow(char* szWindowTitle) =0;
			bool			DestroyNativeWindow(CNativeWindow* pcWindow);

protected:
	template<class Class>
	Class* NativeMalloc(void)
	{
		Class* pcInstance;

		pcInstance = (Class*)mpcMallocator->Malloc(sizeof(Class));
		New<Class>(pcInstance);

		return pcInstance;
	}
};



#endif // __NATIVE_WINDOW_FACTORY_H__

