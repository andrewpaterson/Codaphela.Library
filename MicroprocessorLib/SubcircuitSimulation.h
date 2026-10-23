/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SUBCIRCUIT_SIMULATION_H__
#define __SUBCIRCUIT_SIMULATION_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"


//Logicim's SubcircuitSimulation: where in a CCircuitSimulation a component is being simulated.  The top level circuit
//has a CSubcircuitTopSimulation and each instance of a subcircuit in it a CSubcircuitInstanceSimulation; they all
//share the circuit simulation's circuit and timeline.  The circuit simulation owns them.  Every one gets the next id
//unless it is given one.  Logicim's save data is not ported.
class CTimeline;
class CCircuit;
class CCircuitSimulation;
class CSubcircuitSimulation : public CConstructable
{
CONSTRUCTABLE(CSubcircuitSimulation);
protected:
	CCircuitSimulation*		mpcCircuitSimulation;
	int64					miID;

public:
			void				Init(CCircuitSimulation* pcCircuitSimulation);
			void				Init(CCircuitSimulation* pcCircuitSimulation, int64 iID);
	virtual void				Kill(void);

	virtual const char*			GetType(void) =0;
	virtual bool				IsTop(void);
	virtual bool				IsInstance(void);
	virtual void				AppendDescription(CChars* psz);

			int64				GetID(void);
			void				SetID(int64 iID);
			CCircuitSimulation*	GetCircuitSimulation(void);
			CCircuit*			GetCircuit(void);
			CTimeline*			GetTimeline(void);
			int64				GetTime(void);
			char*				GetName(void);
			bool				RunSimultaneous(void);
			bool				RunToTime(int64 iTimeForward);

	static	void				ResetNextID(void);
};


typedef CArrayTemplatePtr<CSubcircuitSimulation>	CArraySubcircuitSimulationPtr;


//Logicim's SubcircuitTopSimulation.
class CSubcircuitTopSimulation : public CSubcircuitSimulation
{
CONSTRUCTABLE(CSubcircuitTopSimulation);
public:
	virtual const char*		GetType(void);
	virtual bool			IsTop(void);
};


//Logicim's SubcircuitInstanceSimulation: the simulation of a CSubcircuitInstance's subcircuit.
class CSubcircuitInstance;
class CSubcircuitInstanceSimulation : public CSubcircuitSimulation
{
CONSTRUCTABLE(CSubcircuitInstanceSimulation);
protected:
	CSubcircuitInstance*	mpcSubcircuitInstance;

public:
			void					Init(CCircuitSimulation* pcCircuitSimulation, CSubcircuitInstance* pcSubcircuitInstance);
			void					Init(CCircuitSimulation* pcCircuitSimulation, int64 iID);

	virtual const char*				GetType(void);
	virtual bool					IsInstance(void);
	virtual void					AppendDescription(CChars* psz);

			void					SetSubcircuitInstance(CSubcircuitInstance* pcSubcircuitInstance);
			CSubcircuitInstance*	GetSubcircuitInstance(void);
};


#endif // __SUBCIRCUIT_SIMULATION_H__

