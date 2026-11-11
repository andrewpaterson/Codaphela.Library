#include "StandardLib/Array.h"


//The typed array's layer, from a unit that uses it: each is a call through CArray<M> to CArrayObject.
size UseArrayNumElements(CArray<CBaseObject>* pcArray)
{
	return pcArray->NumElements();
}


bool UseArrayIsEmpty(CArray<CBaseObject>* pcArray)
{
	return pcArray->IsEmpty();
}


bool UseArrayRemoveAt(CArray<CBaseObject>* pcArray, size uiIndex)
{
	return pcArray->RemoveAt(uiIndex);
}


bool UseArrayAdd(CArrayObject* pcArray, CPointer& pObject)
{
	return pcArray->Add(pObject);
}
