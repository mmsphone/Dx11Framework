#include "UIPanel.h"

#include "EngineUtility.h"

#include "UI.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"
#include "Layer.h"

UIPanel::UIPanel(const std::string& panelName, bool open)
    : Panel{ panelName, open }
{
}

HRESULT UIPanel::Initialize()
{
    m_PanelPosition = _float2(900.f, 200.f);
    m_PanelSize = _float2(400.f, 500.f);

    ResetEditFields();
    return S_OK;
}

static std::wstring Utf8ToWString(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring ws(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

static std::string WStringToUtf8(const std::wstring& ws)
{
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &s[0], len, nullptr, nullptr);
    return s;
}

// ANSI 경로 <-> wstring
static std::wstring AnsiToWString(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring ws(len - 1, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

static std::string WStringToAnsi(const std::wstring& ws)
{
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, &s[0], len, nullptr, nullptr);
    return s;
}

void UIPanel::ResetEditFields()
{
    m_pSelectedUI = nullptr;

    m_EditType = 0;                  // Image
    m_EditName = "NewUI";
    m_EditFont.clear();
    m_EditTextUtf8 = "Label Text";
    m_EditImagePathAnsi.clear();

    m_EditX = 100.f;
    m_EditY = 100.f;
    m_EditZ = 0.f;
    m_EditW = 256.f;
    m_EditH = 64.f;

    m_EditVisible = true;
    m_EditEnable = true;

    m_EditFontSize = 32.f;
    m_EditFontColor = { 1.f, 1.f, 1.f, 1.f };
}

void UIPanel::SyncFromSelectedUI()
{
    if (!m_pSelectedUI)
        return;

    const UI_DESC& d = m_pSelectedUI->GetUIDesc();

    // 타입은 UILabel/UIImage/UIButton 캐스팅해서 판별
    if (dynamic_cast<UIImage*>(m_pSelectedUI))
        m_EditType = 0;
    else if (dynamic_cast<UILabel*>(m_pSelectedUI))
        m_EditType = 1;
    else if (dynamic_cast<UIButton*>(m_pSelectedUI))
        m_EditType = 2;
    else
        m_EditType = 0;

    m_EditName = d.name;
    m_EditFont = WStringToUtf8(d.font);
    m_EditTextUtf8 = WStringToUtf8(d.text);
    m_EditImagePathAnsi = WStringToAnsi(d.imagePath);

    m_EditX = d.x;
    m_EditY = d.y;
    m_EditZ = d.z;
    m_EditW = d.w;
    m_EditH = d.h;

    m_EditVisible = d.visible ? true : false;
    m_EditEnable = d.enable ? true : false;

    m_EditFontSize = d.fontSize;
    m_EditFontColor = d.fontColor;
}

void UIPanel::ApplyToSelectedUI()
{
    if (!m_pSelectedUI)
        return;

    UI_DESC d = m_pSelectedUI->GetUIDesc();

    // 타입 변경은 여기서는 안 건드리는 걸로 (기존 타입 유지)
    // 위치/크기/텍스트/이미지/플래그만 수정
    d.name = m_EditName;

    switch (m_EditType)
    {
    case 1:
        d.font = Utf8ToWString(m_EditFont);
        d.text = Utf8ToWString(m_EditTextUtf8);
        d.fontSize = m_EditFontSize;
        d.fontColor = m_EditFontColor;
        break;
    case 0: case 2:
        d.imagePath = AnsiToWString(m_EditImagePathAnsi);
        break;
    default:
        break;
    }

    d.x = m_EditX;
    d.y = m_EditY;
    d.z = m_EditZ;
    d.w = m_EditW;
    d.h = m_EditH;

    d.visible = m_EditVisible;
    d.enable = m_EditEnable;

    m_pSelectedUI->ApplyUIDesc(d);
}

void UIPanel::OnRender()
{
    if (ImGui::Button("Test"))
    {
        m_pEngineUtility->LoadUI("../bin/data/minimap.dat", SCENE::MAP);
        D3D11_RECT rect{};
        rect.left = 1132.f;
        rect.top = 552.f;
        rect.right = 1262.f;
        rect.bottom = 685.f;
        list<Object*> pObjects= m_pEngineUtility->FindLayer(SCENE::MAP, L"UI")->GetAllObjects();
        auto iter = pObjects.begin();
        advance(iter, 4);
        dynamic_cast<UIImage*>((*iter))->SetScissor(rect);
    }

    ImGui::Text("UI Panel");
    ImGui::Separator();

    _uint curSceneId = m_pEngineUtility->GetCurrentSceneId();
    const _tchar* uiLayerTag = TEXT("UI");

    Layer* pLayer = m_pEngineUtility->FindLayer(curSceneId, uiLayerTag);
    std::vector<UI*> uiList;

    ImGui::Text("UI Objects");
    ImGui::BeginChild("UIList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (pLayer)
    {
        const auto& objs = pLayer->GetAllObjects();
        int idx = 0;
        for (auto* pObj : objs)
        {
            UI* pUI = dynamic_cast<UI*>(pObj);
            if (!pUI)
                continue;

            uiList.push_back(pUI);

            const UI_DESC& d = pUI->GetUIDesc();

            const char* typeName = "Unknown";
            if (dynamic_cast<UIImage*>(pUI))  typeName = "Image";
            else if (dynamic_cast<UILabel*>(pUI)) typeName = "Label";
            else if (dynamic_cast<UIButton*>(pUI)) typeName = "Button";

            char label[128];
            sprintf_s(label, "[%d] %s (%s)", idx, d.name.c_str(), typeName);

            bool selected = (pUI == m_pSelectedUI);
            if (ImGui::Selectable(label, selected))
            {
                m_pSelectedUI = pUI;
                SyncFromSelectedUI();
            }

            ++idx;
        }
    }

    ImGui::EndChild();

    ImGui::Separator();

    // -------------------------
    // 선택된 UI 편집
    // -------------------------
    if (m_pSelectedUI)
    {
        ImGui::Text("Selected UI: %s", m_EditName.c_str());

        // 타입은 읽기 전용 표시
        const char* typeItems[] = { "Image", "Label", "Button" };
        int readType = m_EditType;
        ImGui::Combo("Type", &readType, typeItems, IM_ARRAYSIZE(typeItems));
        ImGui::SameLine();
        ImGui::TextDisabled("(type change not supported)");

        // Name
        {
            char buf[128] = {};
            strncpy_s(buf, m_EditName.c_str(), sizeof(buf) - 1);
            if (ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf)))
                m_EditName = buf;
        }

        if (m_EditType == 1)
        {
            // Font
            {
                char buf[256] = {};
                strncpy_s(buf, m_EditFont.c_str(), sizeof(buf) - 1);
                if (ImGui::InputText("Font", buf, IM_ARRAYSIZE(buf)))
                    m_EditFont = buf;
            }

            // Text
            {
                char buf[256] = {};
                strncpy_s(buf, m_EditTextUtf8.c_str(), sizeof(buf) - 1);
                if (ImGui::InputText("Text", buf, IM_ARRAYSIZE(buf)))
                    m_EditTextUtf8 = buf;
            }

            // ★ Font Size
            ImGui::DragFloat("Font Size", &m_EditFontSize, 1.0f, 4.0f, 200.0f);

            ImVec4 col = ImVec4(
                m_EditFontColor.x,
                m_EditFontColor.y,
                m_EditFontColor.z,
                m_EditFontColor.w
            );
            if (ImGui::ColorEdit4("Font Color", (float*)&col))
            {
                m_EditFontColor.x = col.x;
                m_EditFontColor.y = col.y;
                m_EditFontColor.z = col.z;
                m_EditFontColor.w = col.w;
            }
        }
        else
        {
            ImGui::TextDisabled("Font / Text : Label type only");
        }

        if (m_EditType == 0 || m_EditType == 2)
        {
            // Image Path
            ImGui::Text("Image Path:");
            ImGui::TextWrapped("%s", m_EditImagePathAnsi.c_str());
            if (ImGui::Button("Browse Image##Edit"))
            {
                char szFile[MAX_PATH] = {};
                OPENFILENAMEA ofn = {};
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = GetActiveWindow();
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.dds;*.tga\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

                if (GetOpenFileNameA(&ofn))
                {
                    m_EditImagePathAnsi = szFile;
                }
            }
        }
        else
        {
            ImGui::TextDisabled("Image Path : Image / Button type only");
        }

        ImGui::Separator();
        ImGui::Text("Transform");
        ImGui::DragFloat("X", &m_EditX, 1.0f);
        ImGui::DragFloat("Y", &m_EditY, 1.0f);
        ImGui::DragFloat("Z", &m_EditZ, 0.01f);
        ImGui::DragFloat("Width", &m_EditW, 1.0f, 0, 4096);
        ImGui::DragFloat("Height", &m_EditH, 1.0f, 0, 4096);

        ImGui::Checkbox("Visible", &m_EditVisible);
        ImGui::SameLine();
        ImGui::Checkbox("Enable", &m_EditEnable);

        if (ImGui::Button("Apply##UIEdit"))
        {
            ApplyToSelectedUI();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Selection##UI"))
        {
            m_pSelectedUI = nullptr;
        }
    }
    else
    {
        ImGui::TextDisabled("No UI selected.");
    }

    ImGui::Separator();
    ImGui::Text("Create New UI");

    // 타입 선택 (이미지/라벨/버튼) - 생성용
    const char* typeItems[] = { "Image", "Label", "Button" };
    ImGui::Combo("Type##New", &m_EditType, typeItems, IM_ARRAYSIZE(typeItems));

    // Name
    {
        char buf[128] = {};
        strncpy_s(buf, m_EditName.c_str(), sizeof(buf) - 1);
        if (ImGui::InputText("Name##New", buf, IM_ARRAYSIZE(buf)))
            m_EditName = buf;
    }

    if (m_EditType == 1)
    {
        // Font
        {
            char buf[128] = {};
            strncpy_s(buf, m_EditFont.c_str(), sizeof(buf) - 1);
            if (ImGui::InputText("Font##New", buf, IM_ARRAYSIZE(buf)))
                m_EditFont = buf;
        }

        // Text
        {
            char buf[256] = {};
            strncpy_s(buf, m_EditTextUtf8.c_str(), sizeof(buf) - 1);
            if (ImGui::InputText("Text##New", buf, IM_ARRAYSIZE(buf)))
                m_EditTextUtf8 = buf;
        }

        // ★ Font Size
        ImGui::DragFloat("Font Size##New", &m_EditFontSize, 1.0f, 4.0f, 200.0f);

        // ★ Font Color
        ImVec4 col = ImVec4(
            m_EditFontColor.x,
            m_EditFontColor.y,
            m_EditFontColor.z,
            m_EditFontColor.w
        );
        if (ImGui::ColorEdit4("Font Color##New", (float*)&col))
        {
            m_EditFontColor.x = col.x;
            m_EditFontColor.y = col.y;
            m_EditFontColor.z = col.z;
            m_EditFontColor.w = col.w;
        }
    }
    else
    {
        ImGui::TextDisabled("Font / Text : Label type only");
    }
    
    if (m_EditType == 0 || m_EditType == 2)
    {
        // Image Path
        ImGui::Text("Image Path (New):");
        ImGui::TextWrapped("%s", m_EditImagePathAnsi.c_str());
        if (ImGui::Button("Browse Image##New"))
        {
            char szFile[MAX_PATH] = {};
            OPENFILENAMEA ofn = {};
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.dds;*.tga\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameA(&ofn))
            {
                m_EditImagePathAnsi = szFile;
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Image Path : Image / Button type only");
    }

    ImGui::DragFloat("X##New", &m_EditX, 1.0f);
    ImGui::DragFloat("Y##New", &m_EditY, 1.0f);
    ImGui::DragFloat("Z##New", &m_EditZ, 0.01f);
    ImGui::DragFloat("Width##New", &m_EditW, 1.0f, 0, 4096);
    ImGui::DragFloat("Height##New", &m_EditH, 1.0f, 0, 4096);

    ImGui::Checkbox("Visible##New", &m_EditVisible);
    ImGui::SameLine();
    ImGui::Checkbox("Enable##New", &m_EditEnable);

    if (ImGui::Button("Create UI"))
    {
        UI_DESC desc{};

        // 타입 매핑
        switch (m_EditType)
        {
        case 0: desc.type = UITYPE::UI_IMAGE;  break;
        case 1: desc.type = UITYPE::UI_LABEL;  break;
        case 2: desc.type = UITYPE::UI_BUTTON; break;
        default: desc.type = UITYPE::UI_IMAGE; break;
        }

        desc.name = m_EditName;

        desc.x = m_EditX;
        desc.y = m_EditY;
        desc.z = m_EditZ;
        desc.w = m_EditW;
        desc.h = m_EditH;

        desc.visible = m_EditVisible;
        desc.enable = m_EditEnable;

        desc.fSpeedPerSec = 0.f;
        desc.fRotationPerSec = 0.f;

        switch (desc.type)
        {
        case UITYPE::UI_LABEL:
            desc.font = Utf8ToWString(m_EditFont);
            desc.text = Utf8ToWString(m_EditTextUtf8);
            desc.fontSize = m_EditFontSize;
            desc.fontColor = m_EditFontColor;
            // imagePath는 사용 안 함
            break;

        case UITYPE::UI_IMAGE:
        case UITYPE::UI_BUTTON:
            desc.imagePath = AnsiToWString(m_EditImagePathAnsi);
            // font/text/fontSize/fontColor는 기본값 그대로 두어도 무방
            break;

        default:
            break;
        }

        const _tchar* protoTag = nullptr;
        switch (desc.type)
        {
        case UITYPE::UI_IMAGE:  protoTag = TEXT("UIImage");  break;
        case UITYPE::UI_LABEL:  protoTag = TEXT("UILabel");  break;
        case UITYPE::UI_BUTTON: protoTag = TEXT("UIButton"); break;
        default: break;
        }

        if (protoTag)
        {
            if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, protoTag, curSceneId, uiLayerTag, &desc)))
            {
                MessageBoxA(nullptr, "AddObject(UI) failed.", "UIPanel", MB_OK);
            }
        }
        else
        {
            MessageBoxA(nullptr, "Invalid UITYPE for create.", "UIPanel", MB_OK);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset##UIFields"))
    {
        ResetEditFields();
    }

    ImGui::Separator();

    // Save / Load UI (EngineUtility 래핑 함수 있다고 가정: SaveUI, LoadUI)
    if (ImGui::Button("Save UI..."))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "UI Layout (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameA(&ofn))
        {
            if (FAILED(m_pEngineUtility->SaveUI(szFile)))
            {
                MessageBoxA(nullptr, "SaveUI failed.", "UIPanel", MB_OK);
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load UI..."))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "UI Layout (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameA(&ofn))
        {
            if (FAILED(m_pEngineUtility->LoadUI(szFile, SCENE::MAP)))
            {
                MessageBoxA(nullptr, "LoadUI failed.", "UIPanel", MB_OK);
            }
        }
    }
}

UIPanel* UIPanel::Create(const std::string& panelName, bool open)
{
    UIPanel* pInstance = new UIPanel(panelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : UIPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void UIPanel::Free()
{
    __super::Free();
}
