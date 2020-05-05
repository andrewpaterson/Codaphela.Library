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
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "CoreLib/ValueNamedIndexesConfig.h"
#include "CoreLib/ValueIndexedDataConfig.h"
#include "CodaBase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodaBase::Init(CDurableFileController* pcController, size_t uiDataCacheSize, size_t uiIndexCacheSize, size_t uiNamedCacheSize)
{
	CValueIndexedDataConfig*			pcIndexConfig;
	CValueNamedIndexesConfig*			pcNamedConfig;
	CIndexTreeEvictionStrategyRandom*	pcIndexEvictionStrategy;
	CIndexTreeEvictionStrategyRandom*	pcNamedEvictionStrategy;

	pcNamedEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyRandom>();
	pcNamedEvictionStrategy->Init();

	pcIndexEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyRandom>();
	pcIndexEvictionStrategy->Init();

	pcIndexConfig = NewMalloc<CValueIndexedDataConfig>();
	pcNamedConfig = NewMalloc<CValueNamedIndexesConfig>();

	pcIndexConfig->Init("IndexData", uiDataCacheSize, uiIndexCacheSize, IWT_No);
	pcNamedConfig->Init("Names", uiNamedCacheSize, pcNamedEvictionStrategy, IWT_No);

	mcConfig.Init(pcIndexConfig, pcNamedConfig, FALSE);
	mcNamedIndexData.Init(pcController, &mcConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCodaBase::Kill(void)
{
	CIndexedDataConfig*			pcIndexConfig;
	CNamedIndexesConfig*		pcNamedConfig;
	CIndexTreeEvictionStrategy* pcEvictionStrategy;

	pcIndexConfig = mcConfig.GetIndexConfig();
	pcNamedConfig = mcConfig.GetNamedConfig();

	pcEvictionStrategy = pcNamedConfig->GetEvictionStrategy();
	SafeKill(pcEvictionStrategy);

	mcConfig.Kill();
	SafeKill(pcIndexConfig);
	SafeKill(pcNamedConfig);
}

