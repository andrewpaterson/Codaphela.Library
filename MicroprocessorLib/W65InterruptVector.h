#ifndef __INTERRUPT_VECTOR_H__
#define __INTERRUPT_VECTOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Refered.h"
#include "StandardLib/Unknown.h"


class CW65CCore;
class CW65InterruptVector : public CUnknown, public CRefered
{
protected:
public:
			void	Init(void);
			void	Kill(void) override;

	virtual uint16	GetAddress(CW65CCore* pcCPU) =0;
};


#endif // __INTERRUPT_VECTOR_H__

