/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_CHORD_DESCS_H__
#define __INPUT_CHORD_DESCS_H__
#include "BaseLib/ArrayTypedPointer.h"
#include "InputDeviceCopyContext.h"
#include "InputChordDesc.h"


class CInputChordDescs : public CUnknown
{
BASE_FUNCTIONS(CInputChordDescs);
protected:
	CSetInputChordDesc	mlcChordDescs;

public:
	void 				Init(void);
	void 				Kill(void);

	CInputChordDesc*	AddChordDesc(char* szActionName);
	void				GetInputSourceDescs(CArrayTypedPointer* apcDest, CInputDeviceDesc* pcDeviceDesc);
	void				CopyChordDescs(CInputChordDescs* pcSource, CInputDeviceCopyContext* pcContext);
	CInputChordDesc*	StartChordDescsIteration(SSetIterator* psIter);
	CInputChordDesc*	IterateChordDescs(SSetIterator* psIter);

private:
	void				CopyChordDesc(CInputChordDesc* pcDest, CInputChordCriteriaDesc* pcSource, CInputDeviceCopyContext* pcContext);
	void				CopyChordCriteriaDesc(CInputChordCriteriaDesc* pcDest, CInputChordCriteriaDesc* pcSource, CInputDeviceCopyContext* pcContext);
};


#endif // __INPUT_CHORD_DESCS_H__

