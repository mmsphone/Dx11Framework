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

    auto R = [&](auto& v, size_t s) {_uint n; f.read((char*)&n, 4); v.resize(n); if (n)f.read((char*)v.data(), s * n); };

    // 1. Meshes
    _uint nm; f.read((char*)&nm, 4); m->meshes.resize(nm);
    for (auto& mesh : m->meshes) {
        _uint l; f.read((char*)&l, 4); mesh.name.resize(l); f.read(mesh.name.data(), l);
        f.read((char*)&mesh.materialIndex, 4);
        R(mesh.positions, sizeof(_float3)); R(mesh.normals, sizeof(_float3));
        R(mesh.texcoords, sizeof(_float2)); R(mesh.tangents, sizeof(_float3)); R(mesh.indices, sizeof(_uint));

        _uint nb; f.read((char*)&nb, 4); mesh.bones.resize(nb);
        for (auto& b : mesh.bones) {
            _uint ln; f.read((char*)&ln, 4); b.name.resize(ln); f.read(b.name.data(), ln);
            _uint nw; f.read((char*)&nw, 4); b.weights.resize(nw);
            if (nw)f.read((char*)b.weights.data(), sizeof(VertexWeight) * nw);
            f.read((char*)&b.offsetMatrix, sizeof(_float4x4));
        }
    }

    // 2. Materials
    _uint nmat; f.read((char*)&nmat, 4); m->materials.resize(nmat);
    for (auto& mt : m->materials) {
        _uint l; f.read((char*)&l, 4); mt.name.resize(l); f.read(mt.name.data(), l);
        for (int t = 0; t < (int)TextureType::End; t++) {
            _uint nt; f.read((char*)&nt, 4); mt.texturePaths[t].resize(nt);
            for (auto& p : mt.texturePaths[t]) {
                _uint pl; f.read((char*)&pl, 4); p.resize(pl); f.read(p.data(), pl);
            }
        }
    }

    // 3. Node
    std::function<void(NodeData&)> RN = [&](NodeData& n) {
        _uint l; f.read((char*)&l, 4); n.name.resize(l); f.read(n.name.data(), l);
        f.read((char*)&n.parentIndex, 4);
        f.read((char*)&n.transform, sizeof(_float4x4));
        _uint c; f.read((char*)&c, 4); n.children.resize(c);
        for (auto& ch : n.children)RN(ch);
        };
    RN(m->rootNode);

    // 4. Animations
    _uint na; f.read((char*)&na, 4); m->animations.resize(na);
    for (auto& a : m->animations) {
        _uint l; f.read((char*)&l, 4); a.name.resize(l); f.read(a.name.data(), l);
        f.read((char*)&a.duration, 4); f.read((char*)&a.ticksPerSecond, 4);
        _uint nc; f.read((char*)&nc, 4); a.channels.resize(nc);
        for (auto& ch : a.channels) {
            _uint ln; f.read((char*)&ln, 4); ch.nodeName.resize(ln); f.read(ch.nodeName.data(), ln);
            auto RK = [&](auto& v, size_t s) {_uint n; f.read((char*)&n, 4); v.resize(n); if (n)f.read((char*)v.data(), s * n); };
            RK(ch.positionKeys, sizeof(KeyVector));
            RK(ch.rotationKeys, sizeof(KeyQuat));
            RK(ch.scalingKeys, sizeof(KeyVector));
        }
    }

    // 5. Global Bones
    _uint nb; f.read((char*)&nb, 4); m->bones.resize(nb);
    for (auto& b : m->bones) {
        _uint l; f.read((char*)&l, 4); b.name.resize(l); f.read(b.name.data(), l);
        f.read((char*)&b.offsetMatrix, sizeof(_float4x4));
    }

    if (!f.eof()) {
        _uint len = 0;
        f.read((char*)&len, 4);
        if (len > 0) {
            m->modelDataFilePath.resize(len);
            f.read(m->modelDataFilePath.data(), len);
        }
    }
    else {
        m->modelDataFilePath = path; // fallback
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
