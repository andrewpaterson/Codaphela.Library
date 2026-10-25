/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __VIEW_PATH_H__
#define __VIEW_PATH_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Chars.h"


class CCircuitView;
class CCircuitSimulation;
class CSubcircuitSimulation;
class CViewPaths;
class CPortView;


//A port view of a subcircuit instance's pin, in the circuit the instance is in, and the port view of the pin inside
//the subcircuit, which are one net.
struct SPinPortJoin
{
	CPortView*	pcOutside;
	CPortView*	pcInside;
};


typedef CArrayTemplate<SPinPortJoin>	CArrayPinPortJoin;


//What a view path runs through, as Logicim's CircuitInstanceView: a circuit shown on its own at the top of a path, or
//an instance of a subcircuit below it.  Its instance circuit view is the circuit it shows.  An instance joins the
//port views of its pins to those inside (GetPinPortJoins); a circuit on its own has nothing outside it to join.
class CCircuitInstanceView
{
public:
	virtual CCircuitView*	GetInstanceCircuitView(void) =0;
	virtual void			AppendDescription(CChars* psz) =0;
	virtual void			GetPinPortJoins(CArrayPinPortJoin* pasJoins);
};


typedef CArrayTemplatePtr<CCircuitInstanceView>	CArrayCircuitInstanceViewPtr;


struct SViewPathSimulation
{
	CCircuitSimulation*		pcCircuitSimulation;
	CSubcircuitSimulation*	pcSubcircuitSimulation;
};


typedef CArrayTemplate<SViewPathSimulation>	CArrayViewPathSimulation;


//Logicim's ViewPath: the circuit instance views from a top circuit down to one use of a circuit, which says which of
//the components a circuit's views make is meant when the circuit is used in more than one place.  Its first view is
//the top circuit and each after that an instance of a subcircuit in the circuit of the one before.  For each circuit
//simulation it is simulated in, it has the subcircuit simulation its last circuit's components are made in (the top
//simulation for a top path).  The previous path is the one without the last view; a child is the path with an
//instance view of the last circuit on the end.  Paths belong to a CViewPaths, which makes children as they're asked
//for.  A subcircuit can't be inside itself, so a path never has a circuit twice.  Every one gets the next id.
class CViewPath
{
protected:
	CArrayCircuitInstanceViewPtr	mapcPath;
	CArrayViewPathSimulation		masSimulations;
	int64							miID;
	CViewPath*						mpcPrevious;
	CViewPaths*						mpcViewPaths;

public:
			void					Init(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcPrevious, CViewPaths* pcViewPaths);
			void					Kill(void);

			bool					AddSubcircuitSimulation(CSubcircuitSimulation* pcSubcircuitSimulation);
			bool					RemoveSubcircuitSimulation(CCircuitSimulation* pcCircuitSimulation);
			bool					ContainsCircuitSimulation(CCircuitSimulation* pcCircuitSimulation);
			CSubcircuitSimulation*	GetSubcircuitSimulation(CCircuitSimulation* pcCircuitSimulation);
			size					NumCircuitSimulations(void);
			CCircuitSimulation*		GetCircuitSimulation(size uiIndex);

			size					Length(void);
			CCircuitInstanceView*	Get(size uiIndex);
			CCircuitInstanceView*	GetFirst(void);
			CCircuitInstanceView*	GetLast(void);
			CCircuitView*			GetCircuitView(void);
			bool					IsTop(void);
			bool					EqualsPath(CArrayCircuitInstanceViewPtr* papcPath);
			CViewPath*				GetPrevious(void);
			CViewPath*				GetTop(void);
			CViewPath*				GetChild(CCircuitInstanceView* pcInstanceView);
			CViewPath*				FindChild(CCircuitInstanceView* pcInstanceView);
			bool					ContainsCircuitView(CCircuitView* pcCircuitView);
			int64					GetID(void);
			void					AppendDescription(CChars* psz);

	static	void					ResetNextID(void);

protected:
			SViewPathSimulation*	FindSimulation(CCircuitSimulation* pcCircuitSimulation, size* puiIndex);
};


typedef CArrayTemplatePtr<CViewPath>	CArrayViewPathPtr;


//The element of an array of structs with pcViewPath and pcCircuitSimulation fields (what a view keeps for each path
//and circuit simulation) that's for the path and circuit simulation; NULL if there isn't one.  If puiIndex isn't NULL
//it's given the element's index.
template<class M>
M* FindViewPathSimulation(CArrayTemplate<M>* pasElements, CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex)
{
	size	i;
	M*		psElement;

	for (i = 0; i < pasElements->NumElements(); i++)
	{
		psElement = pasElements->Get(i);
		if ((psElement->pcViewPath == pcViewPath) && (psElement->pcCircuitSimulation == pcCircuitSimulation))
		{
			if (puiIndex != NULL)
			{
				*puiIndex = i;
			}
			return psElement;
		}
	}
	return NULL;
}


//Logicim's ViewPaths: every path from each top circuit down through the subcircuit instances in it.  It owns them;
//they keep their place (and so the components keyed by them) for as long as the views they run through.
class CViewPaths
{
protected:
	CArrayViewPathPtr	mapcViewPaths;

public:
	void			Init(void);
	void			Kill(void);

	CViewPath*		AddTop(CCircuitView* pcCircuitView);
	CViewPath*		GetTop(CCircuitView* pcCircuitView);
	CViewPath*		GetViewPath(CViewPath* pcParent, CCircuitInstanceView* pcInstanceView);
	CViewPath*		GetOrAddViewPath(CViewPath* pcParent, CCircuitInstanceView* pcInstanceView);
	void			GetPathsEndingWith(CCircuitView* pcCircuitView, CArrayViewPathPtr* papcViewPaths);
	void			RemovePathsThrough(CCircuitInstanceView* pcInstanceView);

	size			NumViewPaths(void);
	CViewPath*		GetViewPath(size uiIndex);

protected:
	CViewPath*		Find(CArrayCircuitInstanceViewPtr* papcPath);
	void			MakeTopPath(CArrayCircuitInstanceViewPtr* papcPath, CCircuitView* pcCircuitView);
	void			MakeChildPath(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcParent, CCircuitInstanceView* pcInstanceView);
	CViewPath*		Add(CArrayCircuitInstanceViewPtr* papcPath, CViewPath* pcPrevious);
};


#endif // __VIEW_PATH_H__

