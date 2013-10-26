#ifndef __DIST_DETACHED_FROMS_H__
#define __DIST_DETACHED_FROMS_H__
#include "ObjectPointerArrays.h"


class CDistDetachedFroms
{
private:
	CArrayBaseObjectPtr		mapcDetachedFromRoot;
	CArrayBaseObjectPtr		mapcCompletelyDetached;

public:
	void			Init(void);
	void			Kill(void);

	void			AddDetachedFromRoot(CBaseObject* pcObject);
	int				NumDetachedFromRoot(void);
	CBaseObject*	GetDetachedFromRoot(int iIndex);

	void			AddCompletelyDetached(CBaseObject* pcObject);
	int				NumCompletelyDetached(void);
	CBaseObject*	GetCompletelyDetached(int iIndex);
	void			RemoveCompletelyDetached(int iIndex);

	void			CopyRootDetachedToCompletelyDetached(void);
};


#endif // __DIST_DETACHED_FROMS_H__


