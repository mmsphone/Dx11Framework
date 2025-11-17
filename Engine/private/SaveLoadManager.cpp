#include "SaveLoadManager.h"

#include "EngineUtility.h"

#include "object.h"
#include "layer.h"
#include "Light.h"
#include "ShadowLight.h"
#include "TriggerBox.h"
#include "UI.h"
#include "UIButton.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIPanel.h"

inline UICONTROLTYPE UIControlTypeFromString(const std::string& s)
{
    // 기본 타입들
    if (_stricmp(s.c_str(), "Label") == 0)
        return UICONTROLTYPE::UI_LABEL;

    if (_stricmp(s.c_str(), "Button") == 0)
        return UICONTROLTYPE::UI_BUTTON;

    if (_stricmp(s.c_str(), "ImagePanel") == 0)
        return UICONTROLTYPE::UI_IMAGE;

    if (_stricmp(s.c_str(), "EditablePanel") == 0 ||
        _stricmp(s.c_str(), "Panel") == 0 ||
        _stricmp(s.c_str(), "Frame") == 0 ||
        _stricmp(s.c_str(), "DropDownMenu") == 0 ||
        _stricmp(s.c_str(), "FlyoutMenu") == 0 ||
        _stricmp(s.c_str(), "RichText") == 0)
        return UICONTROLTYPE::UI_PANEL;

    // Alien Swarm / basemod 계열 특수 타입들
    if (_stricmp(s.c_str(), "BaseModHybridButton") == 0)
        return UICONTROLTYPE::UI_BUTTON;

    if (_stricmp(s.c_str(), "L4DMenuBackground") == 0)
        return UICONTROLTYPE::UI_IMAGE;

    // 그 외에는 일단 Unknown
    return UICONTROLTYPE::UI_UNKNOWN;
}

constexpr int UI_REF_WIDTH = 1280;
constexpr int UI_REF_HEIGHT = 720;

// "c-240", "r128", "f0", "200" 같은 문자열 → 픽셀 좌표로 대충 변환
inline int ResolveUIScalar(const std::string& token, int fullSize)
{
    if (token.empty())
        return 0;

    char c = token[0];

    // center 기준 (예: "c-240")
    if (c == 'c' || c == 'C')
    {
        int offset = 0;
        if (token.size() > 1)
            offset = std::stoi(token.substr(1));  // "c-240" → -240

        return fullSize / 2 + offset;
    }

    // right 기준 (예: "r128" → 전체폭 - 128)
    if (c == 'r' || c == 'R')
    {
        int off = 0;
        if (token.size() > 1)
            off = std::stoi(token.substr(1));
        return fullSize - off;
    }

    // full 비율 (예: wide "f0" → 전체, "f0.5" 이런 건 일단 무시하고 전체로)
    if (c == 'f' || c == 'F')
    {
        // 필요한 경우 f0.5 같은 것도 파싱해서 비율로 쓸 수 있음
        return fullSize;
    }

    // 그냥 숫자
    return std::stoi(token);
}

// xpos, ypos는 전체 해상도 기준으로
inline int ResolveXPos(const std::string& token)
{
    return ResolveUIScalar(token, UI_REF_WIDTH);
}

inline int ResolveYPos(const std::string& token)
{
    return ResolveUIScalar(token, UI_REF_HEIGHT);
}

// wide, tall도 f0 처리 정도만
inline int ResolveWidth(const std::string& token)
{
    return ResolveUIScalar(token, UI_REF_WIDTH);
}

inline int ResolveHeight(const std::string& token)
{
    return ResolveUIScalar(token, UI_REF_HEIGHT);
}

static void ExtractUIControlFromKV(const UI_KV* node, UIControlDesc& out)
{
    out = UIControlDesc{}; // 초기화
    out.name = node->key;  // "BtnStartGame", "LblSummaryLine1" 등

    std::string controlName;
    std::string xposStr, yposStr, wideStr, tallStr;

    for (UI_KV* child : node->children)
    {
        const std::string& k = child->key;
        const std::string& v = child->value;

        if (_stricmp(k.c_str(), "ControlName") == 0)
        {
            controlName = v;
        }
        else if (_stricmp(k.c_str(), "fieldName") == 0)
        {
            out.id = v;
        }
        else if (_stricmp(k.c_str(), "labelText") == 0)
        {
            out.text = v;
        }
        else if (_stricmp(k.c_str(), "image") == 0)
        {
            out.image = v;
        }
        else if (_stricmp(k.c_str(), "command") == 0)
        {
            out.command = v;
        }
        else if (_stricmp(k.c_str(), "xpos") == 0)
        {
            xposStr = v; // 나중에 ResolveXPos에서 해석
        }
        else if (_stricmp(k.c_str(), "ypos") == 0)
        {
            yposStr = v;
        }
        else if (_stricmp(k.c_str(), "wide") == 0)
        {
            wideStr = v;
        }
        else if (_stricmp(k.c_str(), "tall") == 0)
        {
            tallStr = v;
        }
        // 필요한 키(font, fgcolor_override 등)는 여기 계속 추가하면 됨
    }

    out.type = UIControlTypeFromString(controlName);

    // 좌표/사이즈 변환 (문자열 → 픽셀)
    if (!xposStr.empty()) out.x = ResolveXPos(xposStr);
    if (!yposStr.empty()) out.y = ResolveYPos(yposStr);
    if (!wideStr.empty()) out.w = ResolveWidth(wideStr);
    if (!tallStr.empty()) out.h = ResolveHeight(tallStr);
}

static void BuildUIControlsRecursive(const UI_KV* node, std::vector<UIControlDesc>& outControls)
{
    if (!node) return;

    // 이 node가 컨트롤 블록인지 판별:
    //  - value가 비어 있고(children 있음)
    //  - 그리고 안에 "ControlName"이 하나라도 있으면 컨트롤로 본다
    if (node->value.empty())
    {
        UI_KV* controlNameKV = node->FindChild("ControlName");
        if (controlNameKV)
        {
            UIControlDesc desc;
            ExtractUIControlFromKV(node, desc);

            if (desc.type != UICONTROLTYPE::UI_UNKNOWN)
            {
                outControls.push_back(desc);
            }
        }
    }

    // 자식들도 재귀 탐색 (자식 컨트롤)
    for (UI_KV* child : node->children)
    {
        BuildUIControlsRecursive(child, outControls);
    }
}

// -------------------- UI .res KeyValues Parser --------------------

static bool ReadKVToken(std::istream& is, std::string& outToken, int& line)
{
    outToken.clear();
    char ch;

    // 공백 + 주석 스킵
    while (true)
    {
        if (!is.get(ch))
            return false;

        if (ch == '\n')
            ++line;

        // 공백류는 건너뜀
        if (isspace(static_cast<unsigned char>(ch)))
            continue;

        // 주석: // ... 줄 끝까지
        if (ch == '/')
        {
            if (is.peek() == '/')
            {
                while (is.get(ch))
                {
                    if (ch == '\n')
                    {
                        ++line;
                        break;
                    }
                }
                continue;
            }
        }

        // 유효한 문자 발견
        break;
    }

    // 중괄호 하나짜리 토큰
    if (ch == '{' || ch == '}')
    {
        outToken.assign(1, ch);
        return true;
    }

    // 따옴표 문자열
    if (ch == '\"')
    {
        while (true)
        {
            if (!is.get(ch))
                break;

            if (ch == '\n')
                ++line;

            if (ch == '\"')
                break;

            // 아주 간단한 escape 처리 (\" 만 신경씀)
            if (ch == '\\')
            {
                char next;
                if (is.get(next))
                {
                    if (next == '\"')
                        outToken.push_back('\"');
                    else
                    {
                        outToken.push_back(ch);
                        outToken.push_back(next);
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                outToken.push_back(ch);
            }
        }
        return true;
    }

    // bareword: 공백/중괄호 전까지
    outToken.push_back(ch);
    while (is.peek() != EOF)
    {
        char c = static_cast<char>(is.peek());
        if (isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}')
            break;

        is.get(c);
        outToken.push_back(c);
    }

    return true;
}
static UI_KV* ParseKVBlock(std::istream& is, int& line)
{
    std::string token;

    // key
    if (!ReadKVToken(is, token, line))
        return nullptr;

    if (token == "{" || token == "}")
        return nullptr;

    UI_KV* node = new UI_KV{};
    node->key = token;

    // 다음 토큰: value or '{'
    if (!ReadKVToken(is, token, line))
        return node;

    if (token == "{")
    {
        // children 블록
        while (true)
        {
            std::streampos pos = is.tellg();
            std::string t;
            if (!ReadKVToken(is, t, line))
                break;

            if (t == "}")
            {
                // 블록 끝
                break;
            }

            // 자식 key 시작이었으므로 되돌리고 재귀
            is.seekg(pos);
            UI_KV* child = ParseKVBlock(is, line);
            if (child)
                node->children.push_back(child);
            else
                break;
        }
    }
    else if (token == "}")
    {
        // 비정상 포맷이지만 그냥 value 없는 노드로 반환
        return node;
    }
    else
    {
        // key "value" 형태
        node->value = token;
    }

    return node;
}
static UI_KV* ParseUIResToKV(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return nullptr;

    int line = 1;
    UI_KV* root = ParseKVBlock(ifs, line);

    // 필요하다면 여기서 여러 루트를 지원하는 코드도 추가 가능 (대부분 1개라서 생략)
    return root;
}

static void FreeUIKV(UI_KV* node)
{
    if (!node) return;
    for (UI_KV* child : node->children)
        FreeUIKV(child);
    delete node;
}

SaveLoadManager::SaveLoadManager()
    :m_pEngineUtility{ EngineUtility::GetInstance()}
{
    SafeAddRef(m_pEngineUtility);
}

ModelData* SaveLoadManager::LoadNoAssimpModel(const _char* path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        return nullptr;

    auto m = new ModelData();

    auto ReadU32 = [&](uint32_t& v) -> bool { return !!f.read((char*)&v, 4); };
    auto ReadF4x4 = [&](_float4x4& M) -> bool { return !!f.read((char*)&M, sizeof(_float4x4)); };
    auto ReadF4 = [&](_float4& v) -> bool { return !!f.read((char*)&v, sizeof(_float4)); };
    auto ReadStr = [&](std::string& s) -> bool {
        _uint n = 0; if (!f.read((char*)&n, 4)) return false;
        s.resize(n); return n ? !!f.read(s.data(), n) : true;
        };
    auto ReadVec = [&](auto& v, size_t elemSize) -> bool {
        _uint n = 0; if (!f.read((char*)&n, 4)) return false;
        v.resize(n); return n ? !!f.read((char*)v.data(), elemSize * n) : true;
        };

    // --- 헤더 감지: "MDL2"(4) + version(4) ---
    char magic[4] = {};
    f.read(magic, 4);
    uint32_t version = 1; // 기본은 v1로 가정
    bool hasHeader = (f && magic[0] == 'M' && magic[1] == 'D' && magic[2] == 'L' && magic[3] == '2');
    if (hasHeader) {
        if (!ReadU32(version)) { delete m; return nullptr; }
    }
    else {
        // 구포맷: 헤더 없으므로 처음으로 되돌림
        f.clear();
        f.seekg(0, std::ios::beg);
        version = 1;
    }

    // 1) Meshes
    _uint nm = 0; f.read((char*)&nm, 4);
    if (!f) { delete m; return nullptr; }
    m->meshes.resize(nm);

    for (auto& mesh : m->meshes) {
        if (!ReadStr(mesh.name)) { delete m; return nullptr; }
        if (!f.read((char*)&mesh.materialIndex, 4)) { delete m; return nullptr; }

        if (!ReadVec(mesh.positions, sizeof(_float3))) { delete m; return nullptr; }
        if (!ReadVec(mesh.normals, sizeof(_float3))) { delete m; return nullptr; }
        if (!ReadVec(mesh.texcoords, sizeof(_float2))) { delete m; return nullptr; }
        if (!ReadVec(mesh.tangents, sizeof(_float3))) { delete m; return nullptr; }
        if (!ReadVec(mesh.indices, sizeof(_uint))) { delete m; return nullptr; }

        _uint nb = 0; f.read((char*)&nb, 4);
        if (!f) { delete m; return nullptr; }
        mesh.bones.resize(nb);

        for (auto& b : mesh.bones) {
            if (!ReadStr(b.name)) { delete m; return nullptr; }
            _uint nw = 0; f.read((char*)&nw, 4);
            if (!f) { delete m; return nullptr; }
            b.weights.resize(nw);
            if (nw && !f.read((char*)b.weights.data(), sizeof(VertexWeight) * nw)) { delete m; return nullptr; }
            if (!ReadF4x4(b.offsetMatrix)) { delete m; return nullptr; }
        }
    }

    // 2) Materials
    _uint nmat = 0; f.read((char*)&nmat, 4);
    if (!f) { delete m; return nullptr; }
    m->materials.resize(nmat);

    for (auto& mt : m->materials) {
        if (!ReadStr(mt.name)) { delete m; return nullptr; }

        // (a) 텍스처 경로 블록 (v1/v2 동일)
        for (int t = 0; t < (int)TextureType::End; ++t) {
            _uint nt = 0; f.read((char*)&nt, 4);
            if (!f) { delete m; return nullptr; }
            mt.texturePaths[t].resize(nt);
            for (auto& p : mt.texturePaths[t]) {
                if (!ReadStr(p)) { delete m; return nullptr; }
            }
        }

        // (b) v2부터 머티리얼 컬러 3종 추가
        if (version >= 2) {
            if (!ReadF4(mt.diffuseColor)) { delete m; return nullptr; }
            if (!ReadF4(mt.specularColor)) { delete m; return nullptr; }
            if (!ReadF4(mt.emissiveColor)) { delete m; return nullptr; }
        }
        else {
            // v1: 기본값 채움
            mt.diffuseColor = { 1.f, 1.f, 1.f, 1.f };
            mt.specularColor = { 0.f, 0.f, 0.f, 1.f };
            mt.emissiveColor = { 0.f, 0.f, 0.f, 1.f };
        }
    }

    // 3) Node
    std::function<bool(NodeData&)> RN = [&](NodeData& n) -> bool {
        if (!ReadStr(n.name)) return false;
        if (!f.read((char*)&n.parentIndex, 4)) return false;
        if (!ReadF4x4(n.transform)) return false;
        _uint c = 0; if (!f.read((char*)&c, 4)) return false;
        n.children.resize(c);
        for (auto& ch : n.children) { if (!RN(ch)) return false; }
        return true;
        };
    if (!RN(m->rootNode)) { delete m; return nullptr; }

    // 4) Animations
    _uint na = 0; f.read((char*)&na, 4);
    if (!f) { delete m; return nullptr; }
    m->animations.resize(na);

    for (auto& a : m->animations) {
        if (!ReadStr(a.name)) { delete m; return nullptr; }
        if (!f.read((char*)&a.duration, 4)) { delete m; return nullptr; }
        if (!f.read((char*)&a.ticksPerSecond, 4)) { delete m; return nullptr; }

        _uint nc = 0; f.read((char*)&nc, 4);
        if (!f) { delete m; return nullptr; }
        a.channels.resize(nc);

        for (auto& ch : a.channels) {
            if (!ReadStr(ch.nodeName)) { delete m; return nullptr; }

            auto RK = [&](auto& v, size_t s) -> bool {
                _uint n = 0; if (!f.read((char*)&n, 4)) return false;
                v.resize(n);
                return n ? !!f.read((char*)v.data(), s * n) : true;
                };
            if (!RK(ch.positionKeys, sizeof(KeyVector))) { delete m; return nullptr; }
            if (!RK(ch.rotationKeys, sizeof(KeyQuat))) { delete m; return nullptr; }
            if (!RK(ch.scalingKeys, sizeof(KeyVector))) { delete m; return nullptr; }
        }
    }

    // 5) Global Bones
    _uint nb = 0; f.read((char*)&nb, 4);
    if (!f) { delete m; return nullptr; }
    m->bones.resize(nb);
    for (auto& b : m->bones) {
        if (!ReadStr(b.name)) { delete m; return nullptr; }
        if (!ReadF4x4(b.offsetMatrix)) { delete m; return nullptr; }
    }

    // 6) 원본 경로(옵션)
    if (f.peek() != std::char_traits<char>::eof()) {
        // 문자열 하나 더 있으면 읽음
        if (!ReadStr(m->modelDataFilePath)) {
            // 일부 구파일은 경로가 없을 수 있음
            m->modelDataFilePath = path;
        }
    }
    else {
        m->modelDataFilePath = path;
    }

    f.close();
    return m;
}


std::vector<MAP_OBJECTDATA> SaveLoadManager::LoadMapData(const std::string& path)
{
    std::vector<MAP_OBJECTDATA> result;

    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        std::string msg = "Failed to open map data file:\n";
        msg += path;
        MessageBoxA(nullptr, msg.c_str(), "Load Error", MB_OK);
        return result;
    }

    _uint count = 0;
    f.read((char*)&count, sizeof(_uint));

    for (_uint i = 0; i < count; ++i)
    {
        MAP_OBJECTDATA data{};

        // 🔹 모델 경로 읽기
        _uint pathLen = 0;
        f.read((char*)&pathLen, sizeof(_uint));
        data.modelPath.resize(pathLen);
        f.read(data.modelPath.data(), pathLen);

        // 🔹 월드 행렬 읽기
        f.read((char*)&data.worldMatrix, sizeof(XMFLOAT4X4));

        // 🔹 파일명 추출 (Ground4_Mesh4 같은 오브젝트 이름)
        size_t lastSlash = data.modelPath.find_last_of("/\\");
        size_t lastDot = data.modelPath.find_last_of(".");
        if (lastSlash != std::string::npos && lastDot != std::string::npos)
            data.objectName = data.modelPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
        else
            data.objectName = "UnknownObject";

        result.push_back(std::move(data));
    }

    f.close();
    return result;
}

HRESULT SaveLoadManager::SaveMapData(const std::string& path)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        MessageBoxA(nullptr, "Failed to open file for writing.", "Save Error", MB_OK);
        return E_FAIL;
    }

    _uint iCurSceneId = m_pEngineUtility->GetCurrentSceneId();

    EngineUtility* pUtil = EngineUtility::GetInstance();
    Layer* pLayer = pUtil->FindLayer(iCurSceneId, TEXT("FieldObject"));
    if (!pLayer)
        return E_FAIL;

    const auto& objects = pLayer->GetAllObjects();
    _uint count = (_uint)objects.size();
    f.write((char*)&count, sizeof(_uint));

    for (auto& pObj : objects)
    {
        // 모델 경로
        Model* pModel = dynamic_cast<Model*>(pObj->FindComponent(TEXT("Model")));
        if (!pModel)
            continue;

        std::string modelPath = pModel->GetBinPath();
        if (modelPath.empty())
            modelPath = "UnknownModel.bin";

        _uint pathLen = (_uint)modelPath.size();
        f.write((char*)&pathLen, sizeof(_uint));
        f.write(modelPath.data(), pathLen);

        // 트랜스폼
        Transform* pTransform = dynamic_cast<Transform*>(pObj->FindComponent(TEXT("Transform")));
        if (pTransform)
        {
            _float4x4 worldMat = *pTransform->GetWorldMatrixPtr();
            f.write((char*)&worldMat, sizeof(_float4x4));
        }
        else
        {
            _float4x4 identity = {
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1
            };
            f.write((char*)&identity, sizeof(_float4x4));
        }
    }

    f.close();
    return S_OK;
}

HRESULT SaveLoadManager::SaveLights(const std::string& path)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        MessageBoxA(nullptr, "Failed to open file for writing.", "Save Error", MB_OK);
        return E_FAIL;
    }

    list<Light*> lights = m_pEngineUtility->GetAllLights();
    list<LIGHT_DESC> lightDescs{};
    for (auto& light : lights)
    {
        LIGHT_DESC lightDesc{};
        lightDesc = *light->GetLight();
        lightDescs.push_back(lightDesc);
    }
     list<ShadowLight*> shadowLights = m_pEngineUtility->GetAllShadowLights();
     list<SHADOW_DESC> shadowLightDescs{};
     for (auto& shadowlight : shadowLights)
     {
         SHADOW_DESC shadowLightDesc{};
         shadowLightDesc = *shadowlight->GetShadowLight();
         shadowLightDescs.push_back(shadowLightDesc);
     }

    //lightDescs랑 shadowLightDescs 데이터들을 바이너리화해서 저장
    _uint lightCount = (_uint)lightDescs.size();
    _uint shadowCount = (_uint)shadowLightDescs.size();
    f.write((const char*)&lightCount, sizeof(_uint));
    f.write((const char*)&shadowCount, sizeof(_uint));

    for (const auto& L : lightDescs)
    {
        // enum은 고정폭으로 보관
        uint32_t eTypeU32 = static_cast<uint32_t>(L.eType);
        f.write((const char*)&eTypeU32, sizeof(uint32_t));

        f.write((const char*)&L.vDiffuse, sizeof(XMFLOAT4));
        f.write((const char*)&L.vAmbient, sizeof(XMFLOAT4));
        f.write((const char*)&L.vSpecular, sizeof(XMFLOAT4));

        f.write((const char*)&L.vDirection, sizeof(XMFLOAT4));

        f.write((const char*)&L.vPosition, sizeof(XMFLOAT4));
        f.write((const char*)&L.fRange, sizeof(float));

        f.write((const char*)&L.fInnerCone, sizeof(float));  // Spot 아니면 0
        f.write((const char*)&L.fOuterCone, sizeof(float));  // Spot 아니면 0
    }

    for (const auto& S : shadowLightDescs)
    {
        f.write((const char*)&S.vEye, sizeof(XMFLOAT3));
        f.write((const char*)&S.vAt, sizeof(XMFLOAT3));

        f.write((const char*)&S.fFovy, sizeof(float));
        f.write((const char*)&S.fNear, sizeof(float));
        f.write((const char*)&S.fFar, sizeof(float));
        f.write((const char*)&S.fAspect, sizeof(float)); // 0 저장 시 로드시 화면비로 대체 가능
    }

    f.close();
    return S_OK;
}

HRESULT SaveLoadManager::ReadyLightsFromFile(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        return E_FAIL;

    _uint lightCount = 0, shadowCount = 0;
    f.read((char*)&lightCount, sizeof(_uint));
    f.read((char*)&shadowCount, sizeof(_uint));
    if (!f) return E_FAIL;

    // 현재 화면비(Aspect==0 저장된 항목 대체용)
    _uint iNumView = 1;
    D3D11_VIEWPORT view{};
    m_pEngineUtility->GetContext()->RSGetViewports(&iNumView, &view);
    const float aspectNow = view.Width / view.Height;

    // 1) 라이트들
    for (_uint i = 0; i < lightCount; ++i)
    {
        LIGHT_DESC L{}; // 기본 0
        uint32_t eTypeU32 = 0;

        f.read((char*)&eTypeU32, sizeof(uint32_t));               L.eType = (LIGHT)eTypeU32;

        f.read((char*)&L.vDiffuse, sizeof(XMFLOAT4));
        f.read((char*)&L.vAmbient, sizeof(XMFLOAT4));
        f.read((char*)&L.vSpecular, sizeof(XMFLOAT4));

        f.read((char*)&L.vDirection, sizeof(XMFLOAT4));

        f.read((char*)&L.vPosition, sizeof(XMFLOAT4));
        f.read((char*)&L.fRange, sizeof(float));

        f.read((char*)&L.fInnerCone, sizeof(float));
        f.read((char*)&L.fOuterCone, sizeof(float));
        if (!f) return E_FAIL;

        if (FAILED(m_pEngineUtility->AddLight(L)))
            return E_FAIL;
    }

    // 2) 섀도우들
    for (_uint i = 0; i < shadowCount; ++i)
    {
        SHADOW_DESC S{}; // 기본 0
        f.read((char*)&S.vEye, sizeof(XMFLOAT3));
        f.read((char*)&S.vAt, sizeof(XMFLOAT3));
        f.read((char*)&S.fFovy, sizeof(float));
        f.read((char*)&S.fNear, sizeof(float));
        f.read((char*)&S.fFar, sizeof(float));
        f.read((char*)&S.fAspect, sizeof(float));
        if (!f) return E_FAIL;

        if (S.fAspect == 0.0f) S.fAspect = aspectNow;

        if (FAILED(m_pEngineUtility->AddShadowLight(S)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT SaveLoadManager::SaveTriggerBoxes(const std::string& path)
{
    vector<TriggerBox*> triggerBoxes = m_pEngineUtility->GetTriggerBoxes();
    vector<TRIGGERBOX_DESC> triggers{};
    for (auto& triggerBox : triggerBoxes)
    {
        triggers.push_back(triggerBox->GetTriggerBoxDesc());
    }

    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
        return E_FAIL;

    _uint count = static_cast<_uint>(triggers.size());
    f.write(reinterpret_cast<const char*>(&count), sizeof(_uint));

    for (const auto& t : triggers)
    {
        f.write(reinterpret_cast<const char*>(&t.center), sizeof(_float3));
        f.write(reinterpret_cast<const char*>(&t.Extents), sizeof(_float3));
    }

    return S_OK;
}

HRESULT SaveLoadManager::LoadTriggerBoxes(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        return E_FAIL;

    _uint count = 0;
    f.read(reinterpret_cast<char*>(&count), sizeof(_uint));
    if (!f.good())
        return E_FAIL;

    for (_uint i = 0; i < count; ++i)
    {
        TRIGGERBOX_DESC desc{};
        f.read(reinterpret_cast<char*>(&desc.center), sizeof(_float3));
        f.read(reinterpret_cast<char*>(&desc.Extents), sizeof(_float3));
        if (!f.good())
            return E_FAIL;

        TriggerBox* pTriggerBox = TriggerBox::Create(&desc);
        m_pEngineUtility->AddTriggerBox(pTriggerBox);
    }

    return S_OK;
}

HRESULT SaveLoadManager::BuildUIFromRes(const std::string& path, const UIPrototypeTags& protoTags, std::vector<class UI*>& outUIObjects)
{
    outUIObjects.clear();

    // 1) res -> UIControlDesc 리스트
    std::vector<UIControlDesc> controls;
    if (FAILED(LoadUIFromRes(path, controls)))
        return E_FAIL;

    // 2) 현재 씬 ID 가져오기 (이미 SaveMapData에서 쓰는 방식 그대로 사용)
    _uint curSceneId = m_pEngineUtility->GetCurrentSceneId();

    // 3) 컨트롤마다 적당한 프로토타입으로 UI 오브젝트 생성
    for (const UIControlDesc& c : controls)
    {
        // 4) UI_DESC 채워서 UI 베이스에 넘기기
        UI::UI_DESC desc{};

        const _tchar* prototypeTag = nullptr;
        switch (c.type)
        {
        case UICONTROLTYPE::UI_BUTTON:
            prototypeTag = protoTags.buttonProto;
            desc.fZ = 0.3f;
            break;
        case UICONTROLTYPE::UI_LABEL:
            prototypeTag = protoTags.labelProto;
            desc.fZ = 0.2f;
            break;
        case UICONTROLTYPE::UI_IMAGE:
            prototypeTag = protoTags.imageProto;
            desc.fZ = 0.4f;
            break;
        case UICONTROLTYPE::UI_PANEL:
            prototypeTag = protoTags.panelProto;
            desc.fZ = 0.5f;
            break;
        default:
            // 아직 지원 안 하는 타입은 스킵
            continue;
        }

        if (!prototypeTag)
            continue;

        desc.fX = static_cast<_float>(c.x);
        desc.fY = static_cast<_float>(c.y);
        desc.fSizeX = static_cast<_float>(c.w);
        desc.fSizeY = static_cast<_float>(c.h);
        desc.fRotationPerSec = 0.f;
        desc.fSpeedPerSec = 0.f;

        if (FAILED(m_pEngineUtility->AddObject( curSceneId, prototypeTag, curSceneId, protoTags.layerTag, &desc)))
            continue;
        Layer* pLayer = m_pEngineUtility->FindLayer(curSceneId, protoTags.layerTag);
        if (!pLayer)
            continue;
        auto& objs = pLayer->GetAllObjects();
        if (objs.empty())
            continue;

        Object* pObj = objs.back();

        UI* pUI = dynamic_cast<UI*>(pObj);
        if (!pUI)
            continue;

        // 5) 타입별로 텍스트/커맨드/이미지 등 추가 속성 세팅
        //    UIButton / UILabel / UIImage 같은 파생 클래스가 있다고 가정
        if (c.type == UICONTROLTYPE::UI_BUTTON)
        {
            UIButton* pBtn = dynamic_cast<UIButton*>(pUI);
            if (pBtn)
            {
                pBtn->SetText(c.text);
                pBtn->SetCommand(c.command);
            }
        }
        else if (c.type == UICONTROLTYPE::UI_LABEL)
        {
            UILabel* pLabel = dynamic_cast<UILabel*>(pUI);
            if (pLabel)
                pLabel->SetText(c.text);
        }
        else if (c.type == UICONTROLTYPE::UI_IMAGE)
        {
            UIImage* pImage = dynamic_cast<UIImage*>(pUI);
            if (pImage)
                pImage->SetImagePath(c.image);
        }

        outUIObjects.push_back(pUI);
    }

    return outUIObjects.empty() ? E_FAIL : S_OK;
}

HRESULT SaveLoadManager::LoadUIFromRes(const std::string& path, vector<UIControlDesc>& outControls)
{
    outControls.clear();

    UI_KV* pRoot = ParseUIResToKV(path);
    if (!pRoot)
        return E_FAIL;

    // 2) 루트 아래의 모든 컨트롤을 재귀적으로 추출
    BuildUIControlsRecursive(pRoot, outControls);

    // 3) 여기서 pRoot 메모리 해제
    FreeUIKV(pRoot);

    return outControls.empty() ? E_FAIL : S_OK;
}


SaveLoadManager* SaveLoadManager::Create()
{
    return new SaveLoadManager();
}

void SaveLoadManager::Free()
{
    __super::Free();

    SafeRelease(m_pEngineUtility);
}

