#ifndef __WINDOW_H__
#define __WINDOW_H__
#include "BaseLib/Constructable.h"
#include "NativeWindow.h"


class CWindow : public CConstructable
{
CONSTRUCTABLE(CWindow); 
protected:
	CNativeWindow* mpcNativeWindow;

public:
			void	Init(CNativeWindow* pcNativeWindow);
			void	Kill(void);

	virtual bool	Show(void);
};


#endif // __WINDOW_H__

