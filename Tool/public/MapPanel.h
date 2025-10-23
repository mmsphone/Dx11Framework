#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

namespace Engine
{
    class Object;
}

NS_BEGIN(Tool)

class MapPanel final : public Panel
{
    MapPanel(const string& PanelName, bool open = true);
    virtual ~MapPanel() = default;

public:
    HRESULT Initialize();
    virtual void OnRender() override;

    void SortPointsClockWise(vector<_float3>& points);


    static MapPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;

private:
    Object* m_pTerrain = nullptr;
    Object* m_pSelectedObject = nullptr;
    class ObjectPanel* m_pObjectPanel = nullptr;
};

NS_END