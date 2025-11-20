#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Spawner final : public Base
{
	Spawner();
	virtual ~Spawner() = default;

public:
	HRESULT Initialize();

	void AddSpawnerMob(const _uint& iSceneId, const _wstring& prototypeKey, const _wstring& layerKey, _fvector pos, _float3 randomRange = _float3{0.f, 0.f, 0.f});
	void ClearSpawner();
	void Spawn();

	const vector<SPAWNER_MOB_DESC>& GetMobDescs() const;

	static Spawner* Create();
	virtual void    Free() override;

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	std::vector<SPAWNER_MOB_DESC> m_MobDescs{};
};

NS_END