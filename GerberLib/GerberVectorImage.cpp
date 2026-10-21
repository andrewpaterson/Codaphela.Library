#include "GerberVectorImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObjects::Init(void)
{
	macObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberVectorObjects::Kill(void)
{
	size	i;
	size	uiNumObjects;

	uiNumObjects = macObjects.NumElements();
	for (i = 0; i < uiNumObjects; i++)
	{
		macObjects.Get(i)->Kill();
	}
	macObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberVectorObject* CGerberVectorObjects::Add(EGerberPolarity ePolarity)
{
	CGerberVectorObject*	pcObject;

	pcObject = macObjects.Add();
	pcObject->Init(ePolarity);
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberVectorObject* CGerberVectorObjects::Add(CGerberVectorObject* pcSource)
{
	CGerberVectorObject*	pcObject;

	//pcSource must not be in this list; adding can move the objects.
	pcObject = macObjects.Add();
	pcObject->Init(pcSource);
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberVectorObjects::NumObjects(void)
{
	return macObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberVectorObject* CGerberVectorObjects::GetObject(size uiIndex)
{
	return macObjects.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberVectorImage::GetBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	size					i;
	size					uiNumObjects;
	CGerberVectorObject*	pcObject;
	bool					bBounds;

	bBounds = false;
	uiNumObjects = NumObjects();
	for (i = 0; i < uiNumObjects; i++)
	{
		pcObject = GetObject(i);
		if (!pcObject->HasBounds())
		{
			continue;
		}
		if (!bBounds)
		{
			*psMinimum = *pcObject->GetMinimum();
			*psMaximum = *pcObject->GetMaximum();
			bBounds = true;
			continue;
		}
		if (pcObject->GetMinimum()->x < psMinimum->x) psMinimum->x = pcObject->GetMinimum()->x;
		if (pcObject->GetMinimum()->y < psMinimum->y) psMinimum->y = pcObject->GetMinimum()->y;
		if (pcObject->GetMaximum()->x > psMaximum->x) psMaximum->x = pcObject->GetMaximum()->x;
		if (pcObject->GetMaximum()->y > psMaximum->y) psMaximum->y = pcObject->GetMaximum()->y;
	}
	return bBounds;
}

