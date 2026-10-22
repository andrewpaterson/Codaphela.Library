/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65_META_TRACE_H__
#define __65_META_TRACE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "65TraceValue.h"
#include "65MetaSignal.h"


extern S65TraceValue gs65MetaTraceUndriven;


class C65MetaTrace : public C65MetaSignal
{
protected:
	S65TraceValue	msValue;

public:
	void			Init(void);
	void			Init(bool bValue);
	void			Kill(void);

	void			Set(bool bValue);
	S65TraceValue	Get(void);
	void			Invert(void);
	bool			IsHigh(void);
	bool			IsLow(void);

	void			Print(CChars* psz);
};


#define Safe65SetTrace(p, b) if (p) { p->Set(b); }
#define Safe65GetTrace(p) if (p) { return p->Get(); } else { return gs65MetaTraceUndriven; }


#endif // __65_META_TRACE_H__

