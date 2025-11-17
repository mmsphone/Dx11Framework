#include "SpawnerManager.h"

#include "Spawner.h"

SpawnerManager::SpawnerManager()
{
}

void SpawnerManager::AddSpawner(Spawner* pSpawner)
{
	if (pSpawner == nullptr)
		return;

	m_Spawners.push_back(pSpawner);
}

void SpawnerManager::Spawn(_uint iSpawnerIndex)
{
	if (iSpawnerIndex >= m_Spawners.size())
		return;

	Spawner* pSpawner = m_Spawners[iSpawnerIndex];
	if (pSpawner == nullptr)
		return;

	pSpawner->Spawn();
}

void SpawnerManager::RemoveSpawner(_uint iSpawnerIndex)
{
	if (iSpawnerIndex >= m_Spawners.size())
		return;

	m_Spawners.erase(m_Spawners.begin() + iSpawnerIndex);
}

void SpawnerManager::ClearSpawners()
{
	for (auto& spawner : m_Spawners)
	{
		SafeRelease(spawner);
	}
	m_Spawners.clear();
}

const vector<Spawner*>& SpawnerManager::GetSpawners() const
{
	return m_Spawners;
}

SpawnerManager* SpawnerManager::Create()
{
	return new SpawnerManager();
}

void SpawnerManager::Free()
{
	__super::Free();

	ClearSpawners();
}