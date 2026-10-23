#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include "IntegratedCircuit.h"
#include "Passive.h"
#include "Trace.h"


//Logicim's Circuit: the integrated circuits, passives and traces of a circuit.  It owns all three; Logicim's traces
//are owned by its editor.  Killing it kills the components first, which takes their events off the timeline and
//disconnects their ports, then the traces.  So the circuit is killed before its timeline, see CCircuitSimulation.
class CTimeline;
class CCircuit
{
protected:
	CArrayIntegratedCircuitPtr	mapcIntegratedCircuits;
	CArrayPassivePtr			mapcPassives;
	CArrayTracePtr				mapcTraces;

public:
	void				Init(void);
	void				Kill(void);

	void				Add(CIntegratedCircuit* pcIntegratedCircuit);
	void				Add(CPassive* pcPassive);
	bool				Remove(CIntegratedCircuit* pcIntegratedCircuit);
	bool				Remove(CPassive* pcPassive);
	CTrace*				AddTrace(void);
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
	void				LogRemoveError(CCircuitComponent* pcComponent);
};


#endif // __CIRCUIT_H__

