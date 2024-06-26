/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

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
#ifndef __HOLLOW_OBJECT_H__
#define __HOLLOW_OBJECT_H__
#include "BaseObject.h"


class CHollowObject : public CBaseObject
{
CONSTRUCTABLE(CHollowObject);
public:
								CHollowObject(void);
			void				Init(size iNumEmbedded);
			void				Kill(void) final;

			bool				IsHollow(void);

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;
			bool				IsCollection(void) override;
			bool				IsObject(void) override;
			void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
			void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
			size				NumPointerTos(void);
			size				GetNumEmbedded(void);
			CEmbeddedObject*	GetEmbeddedObject(size iIndex);
			size				GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
			size				NumHeapFroms(void);
			size				NumStackFroms(void);
			void				RemoveAllPointerTosDontKill(void);
			void				RemoveAllPointerTos(void);
			void				BaseValidatePointerTos(void);
			void				ValidateConsistency(void);

protected:
			void				EmbedFields(void);
			void				Class(void);
			CBaseObject*		Dehollow(void);

			void				FreePointers(void) override;
			void				Free(void);
			size				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
			void				RemovePointerTo(CEmbeddedObject* pcTo);
	virtual	CEmbeddedObject*	GetRemappedEmbeddedObject(size iIndex);
			void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
};


#endif // __HOLLOW_OBJECT_H__

