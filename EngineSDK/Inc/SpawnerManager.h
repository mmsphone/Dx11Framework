#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class SpawnerManager final : public Base
{
	SpawnerManager();
	virtual ~SpawnerManager() = default;

public:
	void AddSpawner(class Spawner* pSpawner);
	void Spawn(_uint iSpawnerIndex);
	void RemoveSpawner(_uint iSpawnerIndex);
	void ClearSpawners();

	const vector<Spawner*>& GetSpawners() const;

	static SpawnerManager* Create();
	virtual void Free() override;

private:
	vector<class Spawner*> m_Spawners{};
};

NS_END