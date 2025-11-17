#include "TriggerBoxManager.h"

#include "EngineUtility.h"

#include "TriggerBox.h"

TriggerBoxManager::TriggerBoxManager()
{
}

HRESULT TriggerBoxManager::Initialize()
{
    ClearTriggerBoxes();
    return S_OK;
}

HRESULT TriggerBoxManager::AddTriggerBox(TriggerBox* pTriggerBox)
{
    if (!pTriggerBox)
        return E_FAIL;

    m_TriggerBoxes.push_back(pTriggerBox);
    return S_OK;
}

HRESULT TriggerBoxManager::RemoveTriggerBox(_uint index)
{
    if (index >= m_TriggerBoxes.size())
        return E_FAIL;

    SafeRelease(m_TriggerBoxes[index]);
    m_TriggerBoxes.erase(m_TriggerBoxes.begin() + index);
    return S_OK;
}

void TriggerBoxManager::ClearTriggerBoxes()
{
    for (auto& triggerBox : m_TriggerBoxes)
    {
        SafeRelease(triggerBox);
    }
    m_TriggerBoxes.clear();
}

void TriggerBoxManager::UpdateTriggers()
{
    for (auto* pBox : m_TriggerBoxes)
    {
        if (pBox)
            pBox->UpdateTrigger();
    }
}

void TriggerBoxManager::RenderTriggerBoxes()
{
    for (auto* pBox : m_TriggerBoxes)
    {
        if (pBox)
            pBox->RenderTriggerBox();
    }
}

const vector<TriggerBox*>& TriggerBoxManager::GetTriggerBoxes() const
{
    return m_TriggerBoxes;
}

TriggerBoxManager* TriggerBoxManager::Create()
{
    TriggerBoxManager* pInstance = new TriggerBoxManager{};
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : TriggerBoxManager");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void TriggerBoxManager::Free()
{
    __super::Free();

    ClearTriggerBoxes();
}