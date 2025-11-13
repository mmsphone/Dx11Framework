#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

namespace Engine
{
    class Object;
    struct tagPickResult;
}

NS_BEGIN(Tool)

class MapPanel final : public Panel
{
    MapPanel(const string& PanelName, bool open = true);
    virtual ~MapPanel() = default;

public:
    HRESULT Initialize();
    virtual void OnRender() override;

    void SetSelectedBinPath(const string& path);

    static MapPanel* Create(const string& PanelName, bool open = true);
    virtual void		Free() override;

private:
    Object* m_pSelectedObject = nullptr;
    class ObjectPanel* m_pObjectPanel = nullptr;

    _bool m_CellMode = false;
    _int   m_SelectedCellIndex = -1;

    string m_SelectedModelDataPath = {};
};

NS_END