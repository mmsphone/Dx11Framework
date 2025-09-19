#include "RenderManager.h"
#include "EngineUtility.h"
#include "Object.h"

RenderManager::RenderManager()
    :m_pEngineUtility{ EngineUtility::GetInstance() }
{
    Safe_AddRef(m_pEngineUtility);
}

HRESULT RenderManager::Initialize()
{
    return S_OK;
}

HRESULT RenderManager::JoinRenderGroup(RENDERGROUP eGroupId, Object* pObject)
{
    m_RenderObjects[eGroupId].push_back(pObject);
    Safe_AddRef(pObject);

    return S_OK;
}

void RenderManager::Draw()
{
    RenderPriority();
    RenderNonBlend();
    RenderBlend();
    RenderUI();
}

RenderManager* RenderManager::Create()
{
    RenderManager* pInstance = new RenderManager();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : RenderManager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void RenderManager::Free()
{
    __super::Free();
    
    Safe_Release(m_pEngineUtility);
}

void RenderManager::RenderPriority()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::PRIORITY])
    {
        CHECKPTR(pObject)
            pObject->Render();
        Safe_Release(pObject);
    }
    m_RenderObjects[RENDERGROUP::PRIORITY].clear();
}

void RenderManager::RenderNonBlend()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::NONBLEND])
    {
        CHECKPTR(pObject)
            pObject->Render();
        Safe_Release(pObject);
    }
    m_RenderObjects[RENDERGROUP::NONBLEND].clear();
}

void RenderManager::RenderBlend()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::BLEND])
    {
        CHECKPTR(pObject)
            pObject->Render();
        Safe_Release(pObject);
    }
    m_RenderObjects[RENDERGROUP::BLEND].clear();
}

void RenderManager::RenderUI()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::UI])
    {
        CHECKPTR(pObject)
            pObject->Render();
        Safe_Release(pObject);
    }
    m_RenderObjects[RENDERGROUP::UI].clear();
}
