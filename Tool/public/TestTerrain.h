#pragma once

#include "Tool_Defines.h"

#include "Terrain.h"

NS_BEGIN(Tool)

class TestTerrain final : public Terrain
{
public:
	enum TEXTURETYPE : int { TEXTURE_DIFFUSE, TEXTURE_MASK, TEXTURE_BRUSH, TEXTURE_END };
private:
	TestTerrain();
	TestTerrain(const TestTerrain& Prototype);
	virtual ~TestTerrain() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	_float4 GetBrushPos() const;
	_float GetIncTexSize() const;

	static TestTerrain* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free();

private:
	HRESULT ReadyComponents();

private:
	_float4 m_vBrushPos = {};
	_float m_fBrushRange = {};
	_float m_fIncTexSize = {};
};

NS_END