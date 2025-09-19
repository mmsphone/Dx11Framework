#pragma once

#include "Base.h"

NS_(Engine)

class Light final : public Base
{
private:
	Light();
	virtual ~Light() = default;

public:
	HRESULT Initialize(const LIGHT_DESC& lightDesc);
	const LIGHT_DESC* GetLight() const;

	static Light* Create(const LIGHT_DESC& lightDesc);
	virtual void Free() override;
private:
	LIGHT_DESC m_LightDesc;
};

_NS