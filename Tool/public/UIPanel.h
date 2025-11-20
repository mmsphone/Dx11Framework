#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

NS_BEGIN(Engine)
class UI;
enum UITYPE : int;
struct tagUIDesc;
typedef tagUIDesc UI_DESC;
NS_END

NS_BEGIN(Tool)

class UIPanel final : public Panel
{
    UIPanel(const std::string& panelName, bool open);
    virtual ~UIPanel() = default;

public:
    HRESULT Initialize();
    virtual void OnRender() override;

    static UIPanel* Create(const std::string& panelName, bool open = true);
    virtual void    Free() override;

private:
    void SyncFromSelectedUI();
    void ApplyToSelectedUI();

    void ResetEditFields();

private:
    // 선택된 UI 오브젝트
    Engine::UI* m_pSelectedUI = nullptr;

    // 편집용 버퍼 (생성 & 수정 공용)
    _int         m_EditType = 0;           // 0=Image, 1=Label, 2=Button
    std::string m_EditName;              // UI_DESC.name
    std::string m_EditFont;
    std::string m_EditTextUtf8;          // UI_DESC.text (UTF-8)
    std::string m_EditImagePathAnsi;     // UI_DESC.imagePath (ANSI path)

    _float         m_EditX = 100.f;
    _float         m_EditY = 100.f;
    _float         m_EditZ = 0.f;
    _float         m_EditW = 256.f;
    _float         m_EditH = 64.f;

    _bool        m_EditVisible = true;
    _bool        m_EditEnable = true;

    _float m_EditFontSize = 32.f;
    _float4 m_EditFontColor = { 1.f, 1.f, 1.f, 1.f };
};

NS_END
