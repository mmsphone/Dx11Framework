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

HRESULT SaveLoadManager::SaveUI(const std::string& path)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        MessageBoxA(nullptr, "Failed to open UI file for writing.", "SaveUI Error", MB_OK);
        return E_FAIL;
    }

    _uint curSceneId = m_pEngineUtility->GetCurrentSceneId();

    const _tchar* layerTag = TEXT("UI");
    Layer* pLayer = m_pEngineUtility->FindLayer(curSceneId, layerTag);
    if (!pLayer)
        return E_FAIL;

    const auto& objects = pLayer->GetAllObjects();

    // UI 오브젝트만 필터링
    std::vector<UI*> uiObjects;
    uiObjects.reserve(objects.size());
    for (auto* pObj : objects)
    {
        if (UI* pUI = dynamic_cast<UI*>(pObj))
            uiObjects.push_back(pUI);
    }

    _uint count = static_cast<_uint>(uiObjects.size());
    f.write(reinterpret_cast<const char*>(&count), sizeof(_uint));

    auto WriteString = [&](const std::string& s)
        {
            _uint len = static_cast<_uint>(s.size());
            f.write(reinterpret_cast<const char*>(&len), sizeof(_uint));
            if (len)
                f.write(s.data(), len);
        };

    auto WriteWString = [&](const std::wstring& s)
        {
            _uint len = static_cast<_uint>(s.size());
            f.write(reinterpret_cast<const char*>(&len), sizeof(_uint));
            if (len)
                f.write(reinterpret_cast<const char*>(s.data()), len * sizeof(wchar_t));
        };

    for (UI* pUI : uiObjects)
    {
        // 타입 판별
        UITYPE type = UITYPE::UI_END;
        if (dynamic_cast<UIButton*>(pUI))
            type = UITYPE::UI_BUTTON;
        else if (dynamic_cast<UILabel*>(pUI))
            type = UITYPE::UI_LABEL;
        else if (dynamic_cast<UIImage*>(pUI))
            type = UITYPE::UI_IMAGE;
        else
            continue; // 알 수 없는 타입은 스킵

        const UI_DESC& d = pUI->GetUIDesc();

        // 타입
        _int typeInt = static_cast<_int>(type);
        f.write(reinterpret_cast<const char*>(&typeInt), sizeof(_int));

        // 문자열들
        WriteString(d.name);
        WriteWString(d.font);
        WriteWString(d.text);
        WriteWString(d.imagePath);

        // 위치/크기 (현재 UI_DESC는 _float)
        f.write(reinterpret_cast<const char*>(&d.x), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.y), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.z), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.w), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.h), sizeof(_float));

        // 플래그
        f.write(reinterpret_cast<const char*>(&d.visible), sizeof(_bool));
        f.write(reinterpret_cast<const char*>(&d.enable), sizeof(_bool));

        // UILabel 전용 속성
        f.write(reinterpret_cast<const char*>(&d.fontSize), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.fontColor), sizeof(_float4));

        // 트랜스폼용 속도
        f.write(reinterpret_cast<const char*>(&d.fSpeedPerSec), sizeof(_float));
        f.write(reinterpret_cast<const char*>(&d.fRotationPerSec), sizeof(_float));
    }

    f.close();
    return S_OK;
}

HRESULT SaveLoadManager::LoadUI(const std::string& path, _uint iSceneIndex)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
    {
        MessageBoxA(nullptr, "Failed to open UI file for reading.", "LoadUI Error", MB_OK);
        return E_FAIL;
    }

    const _tchar* layerTag = TEXT("UI");

    _uint count = 0;
    f.read(reinterpret_cast<char*>(&count), sizeof(_uint));
    if (!f.good())
        return E_FAIL;

    auto ReadString = [&](std::string& s) -> bool
        {
            _uint len = 0;
            if (!f.read(reinterpret_cast<char*>(&len), sizeof(_uint)))
                return false;
            s.clear();
            if (len == 0)
                return true;

            s.resize(len);
            return !!f.read(&s[0], len);
        };

    auto ReadWString = [&](std::wstring& s) -> bool
        {
            _uint len = 0;
            if (!f.read(reinterpret_cast<char*>(&len), sizeof(_uint)))
                return false;
            s.clear();
            if (len == 0)
                return true;

            s.resize(len);
            return !!f.read(reinterpret_cast<char*>(&s[0]), len * sizeof(wchar_t));
        };

    for (_uint i = 0; i < count; ++i)
    {
        _int typeInt = 0;
        if (!f.read(reinterpret_cast<char*>(&typeInt), sizeof(_int)))
            return E_FAIL;

        UITYPE type = static_cast<UITYPE>(typeInt);
        UI_DESC d{}; // 새 포맷 기본값으로 초기화
        d.type = type;

        // 문자열들 (Save 순서: name -> font -> text -> imagePath)
        if (!ReadString(d.name))       return E_FAIL;
        if (!ReadWString(d.font))      return E_FAIL;
        if (!ReadWString(d.text))      return E_FAIL;
        if (!ReadWString(d.imagePath)) return E_FAIL;

        // 위치/크기 (_float)
        if (!f.read(reinterpret_cast<char*>(&d.x), sizeof(_float))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.y), sizeof(_float))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.z), sizeof(_float))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.w), sizeof(_float))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.h), sizeof(_float))) return E_FAIL;

        // 플래그
        if (!f.read(reinterpret_cast<char*>(&d.visible), sizeof(_bool))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.enable), sizeof(_bool))) return E_FAIL;

        // UILabel 속성
        if (!f.read(reinterpret_cast<char*>(&d.fontSize), sizeof(_float)))  return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.fontColor), sizeof(_float4))) return E_FAIL;

        // 트랜스폼 속도
        if (!f.read(reinterpret_cast<char*>(&d.fSpeedPerSec), sizeof(_float))) return E_FAIL;
        if (!f.read(reinterpret_cast<char*>(&d.fRotationPerSec), sizeof(_float))) return E_FAIL;

        const _tchar* protoTag = nullptr;
        switch (type)
        {
        case UITYPE::UI_BUTTON: protoTag = TEXT("UIButton"); break;
        case UITYPE::UI_LABEL:  protoTag = TEXT("UILabel");  break;
        case UITYPE::UI_IMAGE:  protoTag = TEXT("UIImage");  break;
        default:
            // 알 수 없는 타입이면 스킵
            continue;
        }

        if (!protoTag)
            continue;

        if (FAILED(m_pEngineUtility->AddObject(0, protoTag, iSceneIndex, layerTag, &d)))
            return E_FAIL;
    }

    return S_OK;
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

