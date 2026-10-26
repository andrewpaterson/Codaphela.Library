#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include "StandardLib/Array.h"
#include "IntegratedCircuit.h"
#include "Passive.h"
#include "Trace.h"


//Logicim's Circuit: the integrated circuits, passives and traces of a circuit; Logicim's traces are owned by its
//editor.  A circuit is a CObject made with OMalloc and kept by its CCircuitSimulation.  Its pointers to its components
//keep them alive, and removing a component frees it unless something else points to it.  It owns its traces.
//
//Freeing the circuit frees its components and kills its traces, in no particular order: a trace that is killed
//first lets go of the ports still on it, and a component freed first takes its ports off their traces.  Freeing
//components takes their events off the timeline, so the circuit is freed before its timeline is killed, see
//CCircuitSimulation.
class CTimeline;
class CCircuit : public CObject
{
CONSTRUCTABLE(CCircuit);
protected:
	CArray<CIntegratedCircuit>	maIntegratedCircuits;
	CArray<CPassive>			maPassives;
	CArrayTracePtr				mapcTraces;

public:
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				Add(CIntegratedCircuit* pcIntegratedCircuit);
	void				Add(CPassive* pcPassive);
	bool				Remove(CIntegratedCircuit* pcIntegratedCircuit);
	bool				Remove(CPassive* pcPassive);
	bool				Remove(CCircuitComponent* pcComponent);
	CTrace*				AddTrace(void);
	bool				RemoveTrace(CTrace* pcTrace);
	void				DisconnectComponent(CCircuitComponent* pcComponent, CTimeline* pcTimeline);
	bool				IsEmpty(void);

	void				SimulationStarted(CTimeline* pcTimeline);
	void				GetIntegratedCircuits(const char* szType, CArrayIntegratedCircuitPtr* papcIntegratedCircuits);
	size				NumIntegratedCircuits(void);
	CIntegratedCircuit*	GetIntegratedCircuit(size uiIndex);
	size				NumPassives(void);
	CPassive*			GetPassive(size uiIndex);
	size				NumTraces(void);
	CTrace*				GetTrace(size uiIndex);

protected:
	template<class M>
	bool				RemoveFrom(CArray<M>* paComponents, CCircuitComponent* pcComponent);
	void				LogRemoveError(CCircuitComponent* pcComponent);
};


#endif // __CIRCUIT_H__

