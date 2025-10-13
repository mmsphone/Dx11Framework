#include "Layer.h"
#include "Object.h"


Layer::Layer()
{
}

HRESULT Layer::AddObject(Object* pObject)
{
	if (nullptr == pObject)
		return E_FAIL;

	m_Objects.push_back(pObject);

	return S_OK;
}

Object* Layer::FindObject(_uint iIndex)
{
	if (iIndex >= m_Objects.size()) return nullptr;
	auto iter = m_Objects.begin();
	advance(iter, iIndex);
	return *iter;
}

void Layer::PriorityUpdate(_float fTimeDelta)
{
	for (auto& pGameObject : m_Objects)
	{
		pGameObject->PriorityUpdate(fTimeDelta);
	}
}

void Layer::Update(_float fTimeDelta)
{
	for (auto& pGameObject : m_Objects)
		pGameObject->Update(fTimeDelta);
}

void Layer::LateUpdate(_float fTimeDelta)
{
	for (auto& pGameObject : m_Objects)
		pGameObject->LateUpdate(fTimeDelta);
}

Layer* Layer::Create()
{
	return new Layer();
}

void Layer::Free()
{
	__super::Free();

	for (auto& pGameObject : m_Objects)
		SafeRelease(pGameObject);

	m_Objects.clear();
}