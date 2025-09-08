#pragma once

#include "Engine_Defines.h"

NS_(Engine)

/* ReferenceCount Management */
class ENGINE_DLL Base abstract
{
protected:
	Base();
	virtual  ~Base() = default;

public:
	_uint AddRef();
	_uint Release();

protected:
	_uint referenceCount = 0;

public:
	virtual void Free();
};

_NS