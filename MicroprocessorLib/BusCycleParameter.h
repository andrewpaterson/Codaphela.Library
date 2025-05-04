#ifndef __BUS_CYCLE_PARAMETER_H__
#define __BUS_CYCLE_PARAMETER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Killable.h"
#include "BaseLib/Printable.h"


class CW65C816;
class CBusCycleParameter : public CKillable, CPrintable
{
public:
	virtual bool IsAddress(void) =0;
	virtual bool IsOperation(void) =0;
};


#endif // __BUS_CYCLE_PARAMETER_H__

