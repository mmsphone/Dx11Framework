#include "SaveLoadManager.h"

#include "EngineUtility.h"

#include "object.h"
#include "layer.h"

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


SaveLoadManager* SaveLoadManager::Create()
{
    return new SaveLoadManager();
}

void SaveLoadManager::Free()
{
    __super::Free();

    SafeRelease(m_pEngineUtility);
}
