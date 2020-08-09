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
			void				Init(unsigned short iNumEmbedded);
			void				Class(void);
			void				Kill(void);
			CBaseObject*		Dehollow(void);

			BOOL				IsHollow(void);

			BOOL				Save(CObjectSerialiser* pcFile);
			BOOL				Load(CObjectDeserialiser* pcFile);
			BOOL				IsCollection(void);
			BOOL				IsObject(void);
			void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
			void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
			int					NumPointerTos(void);
			unsigned short int	GetNumEmbedded(void);
			CEmbeddedObject*	GetEmbeddedObject(unsigned short int iIndex);
			int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
			int					NumHeapFroms(void);
			int					NumStackFroms(void);
			void				RemoveAllPointerTosDontKill(void);
			void				RemoveAllPointerTos(void);
			void				BaseValidatePointerTos(void);
			void				ValidateConsistency(void);

protected:
			void				KillInternalData(void);
			void				KillData(void);
			int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
			void				RemovePointerTo(CEmbeddedObject* pcTo);
			void				BaseRemoveAllPointerTosDontKill(void);
	virtual	CEmbeddedObject*	GetRemappedEmbeddedObject(int iIndex);
			void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
};


#endif // __HOLLOW_OBJECT_H__

