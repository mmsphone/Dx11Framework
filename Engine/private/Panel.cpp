#pragma once
#include "Panel.h"

#include "EngineUtility.h"

Panel::Panel(const string& PanelName, bool open)
    : m_PanelName(PanelName), m_IsOpen(open)
    , m_pEngineUtility { EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

void Panel::Draw()
{
    if (!m_IsOpen)
        return;

    ImGui::SetNextWindowPos(ImVec2(m_PanelPosition.x, m_PanelPosition.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(m_PanelSize.x, m_PanelSize.y), ImGuiCond_Once);
    if (ImGui::Begin(m_PanelName.c_str(), &m_IsOpen))
    {
        OnRender();
    }
    ImGui::End();
}

const string& Panel::GetPanelName() const
{
    return m_PanelName;
}

void Panel::SetPanelPos(_float2& vPos)
{
    m_PanelPosition = vPos;
}

void Panel::SetPanelSize(_float2& vSize)
{
    m_PanelSize = vSize;
}

void Panel::SetOpen(bool value)
{
    m_IsOpen = value;
}

bool Panel::IsOpen() const
{
    return m_IsOpen;
}

void Panel::Free()
{
    __super::Free();
    SafeRelease(m_pEngineUtility);
}
