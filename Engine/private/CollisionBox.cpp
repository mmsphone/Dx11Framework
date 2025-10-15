#include "CollisionBox.h"

#include "EngineUtility.h"

CollisionBox::CollisionBox()
	:Base{}
	,m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

void CollisionBox::Free()
{
	__super::Free();

	SafeRelease(m_pEngineUtility);
}
