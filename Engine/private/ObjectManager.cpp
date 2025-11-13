#include "ObjectManager.h"

#include "EngineUtility.h"
#include "Object.h"
#include "Layer.h"

NS_BEGIN(Engine)

ObjectManager::ObjectManager()
	: m_pEngineUtility { EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT ObjectManager::Initialize(_uint iNumScenes)
{
	m_iNumScenes = iNumScenes;

	m_pLayers = new map<const _wstring, Layer*>[iNumScenes];

	return S_OK;
}

HRESULT ObjectManager::AddObject(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, _uint iLayerSceneId, const _wstring& strLayerTag, void* pArg)
{
	// 원형 객체를 복제해서 가져오기
	Object* pObject = dynamic_cast<Object*>(m_pEngineUtility->ClonePrototype(PROTOTYPE::PROTOTYPE_OBJECT, iPrototypeSceneId, strPrototypeTag, pArg));
	if (pObject == nullptr)
		return E_FAIL;
	
	// 보관할 레이어 검색
	Layer* pLayer = FindLayer(iLayerSceneId, strLayerTag);
	if (pLayer == nullptr) // 레이어가 없으면
	{
		//새 레이어 생성
		pLayer = Layer::Create();
		pLayer->AddObject(pObject);
		m_pLayers[iLayerSceneId].emplace(strLayerTag, pLayer);
	}
	else // 레이어가 있으면
		pLayer->AddObject(pObject);

	return S_OK;
}

Object* ObjectManager::FindObject(_uint iLayerSceneId, const _wstring& strLayerTag, _uint iIndex)
{
	Layer* pLayer = FindLayer(iLayerSceneId, strLayerTag);
	if (pLayer == nullptr)
		return nullptr;
	return m_pLayers[iLayerSceneId].at(strLayerTag)->FindObject(iIndex);
}

void ObjectManager::Clear(_uint iSceneId)
{
	for (auto& Pair : m_pLayers[iSceneId])
	{
		auto& objects = Pair.second->GetAllObjects();
		for (auto& object : objects)
		{
			object->SetDead(true);
		}
		SafeRelease(Pair.second);
	}
	m_pLayers[iSceneId].clear();
}

void ObjectManager::ClearDeadObjects()
{
	for(_uint iSceneId = 0; iSceneId < m_iNumScenes ; ++iSceneId)
		for (auto& Pair : m_pLayers[iSceneId])
		{
			auto& objects = Pair.second->GetAllObjects();
			for (auto iter = objects.begin(); iter != objects.end(); )
			{
				if ((*iter)->IsDead())
				{
					SafeRelease(*iter);
					iter = objects.erase(iter);
				}
				else
					++iter;
			}
		}
}

void ObjectManager::PriorityUpdate(_float fTimeDelta)
{
	if (!m_pLayers || m_iNumScenes == 0)
		return;

	for (size_t i = 0; i < m_iNumScenes; ++i)
	{
		for (auto& Pair : m_pLayers[i])
		{
			Layer* pLayer = Pair.second;
			if (pLayer)
				pLayer->PriorityUpdate(fTimeDelta);
		}
	}
}


void ObjectManager::Update(_float fTimeDelta)
{
	if (!m_pLayers || m_iNumScenes == 0)
		return;

	for (size_t i = 0; i < m_iNumScenes; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			Layer* pLayer = Pair.second;
			if (pLayer)
				pLayer->Update(fTimeDelta);
		}
	}
}

void ObjectManager::LateUpdate(_float fTimeDelta)
{
	if (!m_pLayers || m_iNumScenes == 0)
		return;

	for (size_t i = 0; i < m_iNumScenes; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			Layer* pLayer = Pair.second;
			if (pLayer)
				pLayer->LateUpdate(fTimeDelta);
		}
	}
}

ObjectManager* ObjectManager::Create(_uint iNumScenes)
{
	ObjectManager* pInstance = new ObjectManager();
	if (FAILED(pInstance->Initialize(iNumScenes)))
	{
		MSG_BOX("Failed to Created : ObjectManager");
		SafeRelease(pInstance);
	}
	return pInstance;
}

void ObjectManager::Free()
{
	__super::Free();

	for (size_t i = 0; i < m_iNumScenes; i++)
	{
		for (auto& Pair : m_pLayers[i])
			SafeRelease(Pair.second);
		m_pLayers[i].clear();
	}
	SafeDeleteArray(m_pLayers);

	SafeRelease(m_pEngineUtility);
}

Layer* ObjectManager::FindLayer(_uint iSceneId, const _wstring& strLayerTag)
{
	auto	iter = m_pLayers[iSceneId].find(strLayerTag);
	if (iter == m_pLayers[iSceneId].end())
		return nullptr;

	return iter->second;
}

_uint ObjectManager::GetLayerSize(_uint iSceneId, const _wstring& strLayerTag)
{
	Layer* pLayer = FindLayer(iSceneId, strLayerTag);
	if (pLayer == nullptr) return 0;
	else return pLayer->GetLayerSize();
}

std::vector<class Object*> ObjectManager::GetAllObjects(_uint iSceneId)
{
	std::vector<Object*> vObjects{};

	for (auto& Pair : m_pLayers[iSceneId])
	{
		list<Object*> objects = Pair.second->GetAllObjects();
		for (auto& object : objects)
		{
			vObjects.push_back(object);
		}
	}

	return vObjects;
}

NS_END
