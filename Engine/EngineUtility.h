#pragma once

#include "Base.h"

NS_(Engine)

class ENGINE_DLL EngineUtility final : public Base
{
	DECLARE_SINGLETON(EngineUtility)

private:
	EngineUtility();
	virtual ~EngineUtility() = default;

public:

};

_NS