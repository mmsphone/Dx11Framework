#include "PrototypeManager.h"

#include "Object.h"
#include "Component.h"
#include "EngineUtility.h"

PrototypeManager::PrototypeManager()
{
}

HRESULT PrototypeManager::Initialize(_uint iNumScenes)
{
    m_iNumScenes = iNumScenes;

    m_pPrototypes = new PROTOTYPES[iNumScenes];

    return S_OK;
}

_bool PrototypeManager::HasPrototype(_uint iSceneId, const _wstring& strPrototypeTag)
{
    Base* prototype = FindPrototype(iSceneId, strPrototypeTag);
    if (prototype == nullptr)
        return false;
    return true;
}

HRESULT PrototypeManager::AddPrototype(_uint iSceneId, const _wstring& strPrototypeTag, Base* pPrototype)
{
    if (iSceneId >= m_iNumScenes || FindPrototype(iSceneId, strPrototypeTag) != nullptr)
        return E_FAIL;

    m_pPrototypes[iSceneId].emplace(strPrototypeTag, pPrototype);

    return S_OK;
}

Base* PrototypeManager::ClonePrototype(PROTOTYPE eType, _uint iSceneId, const _wstring& strPrototypeTag, void* pArg)
{
    Base* pPrototype = FindPrototype(iSceneId, strPrototypeTag);
    if (pPrototype == nullptr)
        return nullptr;

    if (eType == PROTOTYPE::OBJECT)
        return dynamic_cast<Object*>(pPrototype)->Clone(pArg);
    else if (eType == PROTOTYPE::COMPONENT)
        return dynamic_cast<Component*>(pPrototype)->Clone(pArg);
    else
        return nullptr;
}

void PrototypeManager::Clear(_uint iSceneId)
{
    if (iSceneId >= m_iNumScenes)
        return;

    for (auto& Pair : m_pPrototypes[iSceneId])
    {
        SafeRelease(Pair.second);
    }
    m_pPrototypes[iSceneId].clear();
}

PrototypeManager* PrototypeManager::Create(_uint iNumLevels)
{
    PrototypeManager* pInstance = new PrototypeManager();

    if (FAILED(pInstance->Initialize(iNumLevels)))
    {
        MSG_BOX("Failed to Created : PrototypeManager");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void PrototypeManager::Free()
{
    __super::Free();

    for (size_t i = 0; i < m_iNumScenes; i++)
    {
        for (auto& Pair : m_pPrototypes[i])
            SafeRelease(Pair.second);
        m_pPrototypes[i].clear();
    }

    SafeDeleteArray(m_pPrototypes);
}

Base* PrototypeManager::FindPrototype(_uint iSceneId, const _wstring& strPrototypeTag)
{
    auto iter = m_pPrototypes[iSceneId].find(strPrototypeTag);

    if (iter == m_pPrototypes[iSceneId].end())
        return nullptr;

    return iter->second;
}
