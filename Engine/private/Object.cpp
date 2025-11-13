#include "Object.h"
#include "EngineUtility.h"

Object::Object()
    : m_pEngineUtility{EngineUtility::GetInstance()}
{
    SafeAddRef(m_pEngineUtility);
}

Object::Object(const Object& Prototype)
    :m_pEngineUtility{ Prototype.m_pEngineUtility }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT Object::InitializePrototype()
{
    return S_OK;
}

HRESULT Object::Initialize(void* pArg)
{
    Object::OBJECT_DESC* pDesc = static_cast<Object::OBJECT_DESC*>(pArg);

    Transform* pTransform = Transform::Create();
    if (pTransform == nullptr)
        return E_FAIL;

    if (FAILED(pTransform->Initialize(pArg)))
        return E_FAIL;

    m_Components.emplace(TEXT("Transform"), pTransform);

    return S_OK;
}

void Object::PriorityUpdate(_float fTimeDelta)
{
}

void Object::Update(_float fTimeDelta)
{
}

void Object::LateUpdate(_float fTimeDelta)
{
}

HRESULT Object::Render()
{
    return S_OK;
}

HRESULT Object::RenderShadow(_uint iIndex)
{
	return S_OK;
}

_bool Object::IsDead()
{
	return m_IsDead;
}

void Object::SetDead(_bool bDead)
{
	m_IsDead = bDead;
}

HRESULT Object::AddComponent(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, const _wstring& strComponentTag, Component** ppOut, void* pArg)
{
	if (nullptr != FindComponent(strComponentTag))
		return E_FAIL;

	Component* pComponent = static_cast<Component*>(m_pEngineUtility->ClonePrototype(PROTOTYPE::PROTOTYPE_COMPONENT, iPrototypeSceneId, strPrototypeTag, pArg));
	if (nullptr == pComponent)
		return E_FAIL;

	pComponent->SetOwner(this);
	m_Components.emplace(strComponentTag, pComponent);

	if (ppOut != nullptr)
	{
		*ppOut = pComponent;
	}

	return S_OK;
}

Component* Object::FindComponent(const _wstring& strComponentTag)
{
	auto iter = m_Components.find(strComponentTag);
	if (iter == m_Components.end())
		return nullptr;

	return iter->second;
}

void Object::Free()
{
	__super::Free();

	SafeRelease(m_pEngineUtility);

	for (auto& Pair : m_Components)
		SafeRelease(Pair.second);
	m_Components.clear();
}