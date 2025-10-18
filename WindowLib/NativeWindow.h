#ifndef __NATIVE_WINDOW_H__
#define __NATIVE_WINDOW_H__
#include "BaseLib/Constructable.h"


class CNativeWindow : public CConstructable
{
public:
	void Init(void);
	void Kill(void);
};


#endif // __NATIVE_WINDOW_H__


