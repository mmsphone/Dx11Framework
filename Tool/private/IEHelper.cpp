#include "IEHelper.h"
#include "Tool_Defines.h"
struct AnimPruneThreshold {
    float posEps = 1e-4f;          // 한 채널의 위치 변경 허용치 (단위: 원본 단위)
    float rotDegEps = 0.1f;        // 한 채널의 회전 변경 허용치 (deg)
    float scaleEps = 1e-4f;        // 한 채널의 스케일 변경 허용치
    bool  requireRootMotion = false; // true면 root bone 이동/회전 없으면 제거
    std::string rootName = "root";   // root 본 이름 추정
};

static float AngleBetweenQuatDeg(const _float4& a, const _float4& b) {
    // 두 쿼터니언 사이 각도 (deg)
    // q, -q 동일 취급
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    dot = std::clamp(dot, -1.0f, 1.0f);
    float ang = 2.0f * acosf(fabsf(dot)); // rad
    return XMConvertToDegrees(ang);
}

static bool IsStaticChannel(const ChannelData& ch, const AnimPruneThreshold& th) {
    // 위치
    float maxPosDelta = 0.f;
    for (size_t i = 1; i < ch.positionKeys.size(); ++i) {
        auto& p0 = ch.positionKeys[i - 1].value;
        auto& p1 = ch.positionKeys[i].value;
        float dx = p1.x - p0.x, dy = p1.y - p0.y, dz = p1.z - p0.z;
        maxPosDelta = max(maxPosDelta, fabsf(dx));
        maxPosDelta = max(maxPosDelta, fabsf(dy));
        maxPosDelta = max(maxPosDelta, fabsf(dz));
        if (maxPosDelta > th.posEps) return false;
    }
    // 회전
    float maxRotDeg = 0.f;
    for (size_t i = 1; i < ch.rotationKeys.size(); ++i) {
        maxRotDeg = max(maxRotDeg, AngleBetweenQuatDeg(ch.rotationKeys[i - 1].value,
            ch.rotationKeys[i].value));
        if (maxRotDeg > th.rotDegEps) return false;
    }
    // 스케일
    float maxScaleDelta = 0.f;
    for (size_t i = 1; i < ch.scalingKeys.size(); ++i) {
        auto& s0 = ch.scalingKeys[i - 1].value;
        auto& s1 = ch.scalingKeys[i].value;
        float dx = s1.x - s0.x, dy = s1.y - s0.y, dz = s1.z - s0.z;
        maxScaleDelta = max(maxScaleDelta, fabsf(dx));
        maxScaleDelta = max(maxScaleDelta, fabsf(dy));
        maxScaleDelta = max(maxScaleDelta, fabsf(dz));
        if (maxScaleDelta > th.scaleEps) return false;
    }
    // 여기까지 오면 이 채널은 정지로 간주
    return true;
}

static bool AnimationIsStatic(const AnimationData& anim,
    const AnimPruneThreshold& th,
    bool* outOnlyOneKey = nullptr)
{
    // 1) 전체적으로 키가 사실상 1개뿐(=포즈)인지 확인
    size_t totalKeys = 0;
    for (auto& ch : anim.channels) {
        totalKeys += ch.positionKeys.size();
        totalKeys += ch.rotationKeys.size();
        totalKeys += ch.scalingKeys.size();
    }
    if (outOnlyOneKey) *outOnlyOneKey = (totalKeys <= 1);
    if (totalKeys <= 1) return true;

    // 2) 루트 모션 강제 조건
    if (th.requireRootMotion) {
        for (auto& ch : anim.channels) {
            if (ch.nodeName == th.rootName) {
                if (!IsStaticChannel(ch, th)) {
                    // 루트에서 변화가 있다면 통과
                    goto CHECK_ALL_CHANNELS;
                }
                else {
                    // 루트에 변화가 없으면 정지로 간주
                    return true;
                }
            }
        }
        // 루트 채널이 없으면 보수적으로 정지로 간주하지 않음
    }

CHECK_ALL_CHANNELS:
    // 3) 모든 채널이 정지면 이 애니는 정지
    bool anyDynamic = false;
    for (auto& ch : anim.channels) {
        if (!IsStaticChannel(ch, th)) { anyDynamic = true; break; }
    }
    return !anyDynamic;
}

static void PruneStaticAnimations(ModelData& model,
    const AnimPruneThreshold& th,
    const std::vector<std::string>& keepNameContains = {})
{
    std::vector<AnimationData> filtered;
    filtered.reserve(model.animations.size());

    for (auto& a : model.animations) {
        // 화이트리스트(이름 일부 포함)는 무조건 유지
        bool keepByName = false;
        for (auto& key : keepNameContains) {
            if (!key.empty() && a.name.find(key) != std::string::npos) {
                keepByName = true; break;
            }
        }
        if (keepByName) {
            filtered.push_back(a);
            continue;
        }

        bool onlyOne = false;
        bool isStatic = AnimationIsStatic(a, th, &onlyOne);
        if (isStatic) {
            char buf[256];
            sprintf_s(buf, "[Prune] drop anim '%s' (static%s)\n",
                a.name.c_str(), onlyOne ? "/oneKey" : "");
            OutputDebugStringA(buf);
            continue; // drop
        }
        filtered.push_back(a);
    }

    model.animations.swap(filtered);
}
// ★ 안전 행렬/쿼터니언 변환 유틸
static _float4x4 ToF4x4(const aiMatrix4x4& m) {
    _float4x4 r;
    r._11 = m.a1; r._12 = m.a2; r._13 = m.a3; r._14 = m.a4;
    r._21 = m.b1; r._22 = m.b2; r._23 = m.b3; r._24 = m.b4;
    r._31 = m.c1; r._32 = m.c2; r._33 = m.c3; r._34 = m.c4;
    r._41 = m.d1; r._42 = m.d2; r._43 = m.d3; r._44 = m.d4;
    return r;
}

static _float4x4 IdentityF4x4() {
    _float4x4 r{}; r._11 = r._22 = r._33 = r._44 = 1.0f; return r;
}

static void NormalizeQuat(_float4& q) {
    float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len > 1e-8f) { q.x /= len; q.y /= len; q.z /= len; q.w /= len; }
}

// ★ 임베디드 텍스처/경로 보정
static std::string NormalizePath(const std::string& p);
static std::string JoinPath(const std::filesystem::path& base, const std::string& rel) {
    return (base / rel).lexically_normal().string();
}
static bool FileExists(const std::string& p) {
    std::error_code ec; return std::filesystem::exists(p, ec);
}
static std::string FixTexturePath(std::string p, const std::filesystem::path& modelFolder) {
    if (p.empty()) return p;
    // 임베디드(*n)는 경로 보정 대상 아님
    if (p[0] == '*') return p;

    // 상대경로 → 절대/정규화
    if (!std::filesystem::path(p).is_absolute())
        p = JoinPath(modelFolder, p);

    // 확장자 없으면 후보 스캔
    std::string ext = std::filesystem::path(p).extension().string();
    if (ext.empty()) {
        static const char* exts[] = { ".png",".jpg",".jpeg",".tga",".bmp",".dds",".tif",".tiff" };
        for (auto e : exts) { if (FileExists(p + e)) return p + e; }
    }
    return p;
}

// ★ 임베디드 텍스처를 파일로 풀어주는 헬퍼(필요 시 호출)
static std::string DumpEmbeddedTexture(const aiTexture* tex, const std::filesystem::path& outDir, int index) {
    if (!tex) return {};
    std::filesystem::create_directories(outDir);

    // 압축 이미지(mHeight==0) → 원본 포맷 그대로 저장
    if (tex->mHeight == 0) {
        // 포맷 힌트(tex->achFormatHint)로 확장자 유추
        std::string ext = ".bin";
        if (tex->achFormatHint[0]) {
            ext = "." + std::string(tex->achFormatHint);
        }
        std::string outPath = (outDir / ("embedded_" + std::to_string(index) + ext)).string();
        std::ofstream ofs(outPath, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth); // mWidth=byte size
        return outPath;
    }
    else {
        // 비압축 RGBA 텍스처(이미지 포맷이 아님). 필요하면 DDS/PNG 인코딩 추가.
        std::string outPath = (outDir / ("embedded_" + std::to_string(index) + ".rawrgba")).string();
        std::ofstream ofs(outPath, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth * tex->mHeight * 4);
        return outPath;
    }
}
bool IEHelper::ImportFBX(const std::string& filePath, ModelData& outModel)
{
    Assimp::Importer importer;
    importer.FreeScene();
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

    unsigned int flags =
        // aiProcess_ConvertToLeftHanded |  // Blender에서 이미 LH로 내보낸 경우 주석 유지
        aiProcessPreset_TargetRealtime_MaxQuality;

    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || !scene->mRootNode)
        return false;

    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.animations.clear();
    outModel.bones.clear();

    std::filesystem::path modelFolder = std::filesystem::path(filePath).parent_path();
    std::filesystem::path embeddedOut = modelFolder / "_embedded_textures";
    std::filesystem::create_directories(embeddedOut);

    // ---------- [1] Meshes ----------
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        MeshData meshData;
        meshData.name = mesh->mName.C_Str();
        meshData.materialIndex = (mesh->mMaterialIndex < scene->mNumMaterials)
            ? mesh->mMaterialIndex : 0;

        meshData.positions.reserve(mesh->mNumVertices);
        if (mesh->HasNormals())  meshData.normals.reserve(mesh->mNumVertices);
        if (mesh->HasTextureCoords(0)) meshData.texcoords.reserve(mesh->mNumVertices);
        if (mesh->HasTangentsAndBitangents()) meshData.tangents.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            meshData.positions.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
            meshData.normals.push_back(mesh->HasNormals() ?
                _float3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z } : _float3{ 0.f,1.f,0.f });
            meshData.texcoords.push_back(mesh->HasTextureCoords(0) ?
                _float2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y } : _float2{ 0.f,0.f });
            if (mesh->HasTangentsAndBitangents())
                meshData.tangents.push_back({ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
        }

        meshData.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                meshData.indices.push_back(face.mIndices[idx]);
        }

        // Bones
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            const aiBone* bone = mesh->mBones[b];
            MeshBone meshBone;
            meshBone.name = bone->mName.C_Str();
            meshBone.offsetMatrix = ToF4x4(bone->mOffsetMatrix);

            meshBone.weights.reserve(bone->mNumWeights);
            for (unsigned int w = 0; w < bone->mNumWeights; ++w)
            {
                VertexWeight vw;
                vw.vertexId = bone->mWeights[w].mVertexId;
                vw.weight = bone->mWeights[w].mWeight;
                meshBone.weights.push_back(vw);
            }
            meshData.bones.push_back(meshBone);

            if (std::none_of(outModel.bones.begin(), outModel.bones.end(),
                [&](const BoneData& bd) { return bd.name == meshBone.name; }))
            {
                BoneData bd;
                bd.name = meshBone.name;
                bd.offsetMatrix = meshBone.offsetMatrix;
                outModel.bones.push_back(bd);
            }
        }
        outModel.meshes.push_back(std::move(meshData));
    }

    // ---------- 2. Materials ----------
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* mat = scene->mMaterials[i];
        MaterialData matData;
        matData.name = mat->GetName().C_Str();
            
        auto LoadTextures = [&](aiTextureType type, TextureType eType)
            {
                const unsigned int texCount = mat->GetTextureCount(type);
                for (unsigned int t = 0; t < texCount; ++t)
                {
                    aiString texPath;
                    if (mat->GetTexture(type, t, &texPath) == AI_SUCCESS)
                    {
                        std::string p = texPath.C_Str();
                        // (여기 기존 FixTexturePath / 임베디드 텍스처 처리 코드 그대로)
                        std::string fixed = FixTexturePath(p, modelFolder);
                        matData.texturePaths[eType].push_back(fixed);
                    }
                }
            };

        LoadTextures(aiTextureType_DIFFUSE, TextureType::Diffuse);
        LoadTextures(aiTextureType_SPECULAR, TextureType::Specular);
        LoadTextures(aiTextureType_NORMALS, TextureType::Normal);
        LoadTextures(aiTextureType_EMISSIVE, TextureType::Emissive);

        // ★ 텍스처가 비어 있을 경우, 머티리얼 색상 직접 읽기
        aiColor3D diffuse(1, 1, 1);
        aiColor3D specular(0, 0, 0);
        aiColor3D emissive(0, 0, 0);

        mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        mat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);

        matData.diffuseColor = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
        matData.specularColor = { specular.r, specular.g, specular.b, 1.0f };
        matData.emissiveColor = { emissive.r, emissive.g, emissive.b, 1.0f };

        // Diffuse 텍스처가 아예 없으면 색상 렌더링용으로 표시
        if (matData.texturePaths[TextureType::Diffuse].empty())
        {
            char buf[256];
            sprintf_s(buf, "[ImportFBX] Material '%s' uses diffuse color (%.2f, %.2f, %.2f)\n",
                matData.name.c_str(), diffuse.r, diffuse.g, diffuse.b);
            OutputDebugStringA(buf);
        }

        outModel.materials.push_back(std::move(matData));
    }

    // ---------- [3] Node Hierarchy ----------
    int nodeCounter = 0;
    std::function<void(const aiNode*, NodeData&, int)> processNode;
    processNode = [&](const aiNode* node, NodeData& outNode, int parentIdx)
        {
            const int myIdx = nodeCounter++;
            outNode.name = (node->mName.length > 0) ? node->mName.C_Str() : "Root";
            outNode.parentIndex = parentIdx;
            outNode.transform = ToF4x4(node->mTransformation);
            if (!(outNode.transform._11 || outNode.transform._22 || outNode.transform._33))
                outNode.transform = IdentityF4x4();

            outNode.children.resize(node->mNumChildren);
            for (unsigned int c = 0; c < node->mNumChildren; ++c)
                processNode(node->mChildren[c], outNode.children[c], myIdx);
        };
    outModel.rootNode = {};
    processNode(scene->mRootNode, outModel.rootNode, -1);

    // ---------- [4] Animations ----------
    for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
    {
        const aiAnimation* anim = scene->mAnimations[a];
        AnimationData animData;
        animData.name = (anim->mName.length > 0)
            ? anim->mName.C_Str()
            : "Anim_" + std::to_string(a);

        double tps = (anim->mTicksPerSecond != 0.0) ? anim->mTicksPerSecond : 30.0;
        if (tps > 480.0) tps = 30.0; // 비정상 단위 보정
        animData.ticksPerSecond = (float)tps;
        animData.duration = (float)anim->mDuration;

        for (unsigned int c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim* ch = anim->mChannels[c];
            ChannelData cd;
            cd.nodeName = ch->mNodeName.C_Str();

            for (unsigned int k = 0; k < ch->mNumPositionKeys; ++k)
                cd.positionKeys.push_back({ (float)ch->mPositionKeys[k].mTime,
                    { ch->mPositionKeys[k].mValue.x, ch->mPositionKeys[k].mValue.y, ch->mPositionKeys[k].mValue.z } });

            for (unsigned int k = 0; k < ch->mNumRotationKeys; ++k)
            {
                _float4 q{ ch->mRotationKeys[k].mValue.x, ch->mRotationKeys[k].mValue.y,
                           ch->mRotationKeys[k].mValue.z, ch->mRotationKeys[k].mValue.w };
                NormalizeQuat(q);
                cd.rotationKeys.push_back({ (float)ch->mRotationKeys[k].mTime, q });
            }

            for (unsigned int k = 0; k < ch->mNumScalingKeys; ++k)
                cd.scalingKeys.push_back({ (float)ch->mScalingKeys[k].mTime,
                    { ch->mScalingKeys[k].mValue.x, ch->mScalingKeys[k].mValue.y, ch->mScalingKeys[k].mValue.z } });

            animData.channels.push_back(std::move(cd));
        }
        outModel.animations.push_back(std::move(animData));
    }

    outModel.modelDataFilePath = filePath;
    AnimPruneThreshold th;
    th.requireRootMotion = true;
    th.posEps = 1e-4f;
    th.rotDegEps = 0.1f;
    th.scaleEps = 1e-4f;

    std::vector<std::string> keep = { "BindPose"};

    PruneStaticAnimations(outModel, th, keep);

    return true;
}
bool IEHelper::ExportModel(const std::string& filePath, const ModelData& model)
{
    // ============= [1] 전체 모델 내보내기 =============
    {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        auto WriteU32 = [&](uint32_t v) { file.write((char*)&v, 4); };
        auto WriteF4x4 = [&](const _float4x4& m) { file.write((char*)&m, sizeof(_float4x4)); };
        auto WriteF4 = [&](const _float4& v) { file.write((char*)&v, sizeof(_float4)); };
        auto WriteStr = [&](const std::string& s) {
            _uint n = (_uint)s.size(); file.write((char*)&n, 4);
            if (n) file.write(s.data(), n);
            };
        auto WriteVector = [&](auto& v) {
            _uint n = (_uint)v.size(); file.write((char*)&n, 4);
            if (n) file.write((char*)v.data(), sizeof(v[0]) * n);
            };

        // [NEW] 헤더: "MDL2" + version(2)
        const char magic[4] = { 'M','D','L','2' };
        file.write(magic, 4);
        WriteU32(2u); // version = 2

        // 1. Meshes
        _uint numMeshes = (_uint)model.meshes.size();
        file.write((char*)&numMeshes, 4);
        for (auto& mesh : model.meshes)
        {
            WriteStr(mesh.name);
            file.write((char*)&mesh.materialIndex, 4);
            WriteVector(mesh.positions);
            WriteVector(mesh.normals);
            WriteVector(mesh.texcoords);
            WriteVector(mesh.tangents);
            WriteVector(mesh.indices);

            _uint numBones = (_uint)mesh.bones.size();
            file.write((char*)&numBones, 4);
            for (auto& b : mesh.bones)
            {
                WriteStr(b.name);
                _uint w = (_uint)b.weights.size();
                file.write((char*)&w, 4);
                if (w) file.write((char*)b.weights.data(), sizeof(VertexWeight) * w);
                WriteF4x4(b.offsetMatrix);
            }
        }

        // 2. Materials
        _uint numMat = (_uint)model.materials.size();
        file.write((char*)&numMat, 4);
        for (auto& m : model.materials)
        {
            WriteStr(m.name);

            // (a) 텍스처 경로들 (기존과 동일)
            for (int t = 0; t < (int)TextureType::End; ++t)
            {
                _uint numTex = (_uint)m.texturePaths[t].size();
                file.write((char*)&numTex, 4);
                for (auto& p : m.texturePaths[t])
                    WriteStr(p);
            }

            // (b) [NEW] 컬러 3종 (_float4): diffuse/specular/emissive
            WriteF4(m.diffuseColor);
            WriteF4(m.specularColor);
            WriteF4(m.emissiveColor);
        }

        // 3. Node 트리
        std::function<void(const NodeData&)> WNode = [&](const NodeData& n)
            {
                WriteStr(n.name);
                file.write((char*)&n.parentIndex, 4);
                WriteF4x4(n.transform);
                _uint c = (_uint)n.children.size();
                file.write((char*)&c, 4);
                for (auto& ch : n.children) WNode(ch);
            };
        WNode(model.rootNode);

        // 4. Animations
        _uint numAnim = (_uint)model.animations.size();
        file.write((char*)&numAnim, 4);
        for (auto& a : model.animations)
        {
            WriteStr(a.name);
            file.write((char*)&a.duration, 4);
            file.write((char*)&a.ticksPerSecond, 4);

            _uint c = (_uint)a.channels.size();
            file.write((char*)&c, 4);
            for (auto& ch : a.channels)
            {
                WriteStr(ch.nodeName);

                auto WK = [&](auto& v) {
                    _uint n = (_uint)v.size();
                    file.write((char*)&n, 4);
                    if (n) file.write((char*)v.data(), sizeof(v[0]) * n);
                    };
                WK(ch.positionKeys);
                WK(ch.rotationKeys);
                WK(ch.scalingKeys);
            }
        }

        // 5. Global Bones
        _uint nb = (_uint)model.bones.size();
        file.write((char*)&nb, 4);
        for (auto& b : model.bones)
        {
            WriteStr(b.name);
            WriteF4x4(b.offsetMatrix);
        }

        // 6. 원본 경로
        WriteStr(model.modelDataFilePath);

        file.close();
    }

    // ============= [2] 각 Mesh별 개별 bin 내보내기 =============
    std::filesystem::path basePath = std::filesystem::path(filePath).parent_path();
    std::string baseName = std::filesystem::path(filePath).stem().string();

    for (size_t i = 0; i < model.meshes.size(); ++i)
    {
        std::string meshFile = (basePath / (baseName + "_Mesh" + std::to_string(i) + ".bin")).string();
        std::ofstream mfile(meshFile, std::ios::binary);
        if (!mfile.is_open())
            continue;

        const auto& mesh = model.meshes[i];

        auto WriteU32 = [&](uint32_t v) { mfile.write((char*)&v, 4); };
        auto WriteF4x4 = [&](const _float4x4& m) { mfile.write((char*)&m, sizeof(_float4x4)); };
        auto WriteF4 = [&](const _float4& v) { mfile.write((char*)&v, sizeof(_float4)); };
        auto WriteStr = [&](const std::string& s) {
            _uint n = (_uint)s.size(); mfile.write((char*)&n, 4);
            if (n) mfile.write(s.data(), n);
            };
        auto W = [&](auto& v) { _uint n = (_uint)v.size(); mfile.write((char*)&n, 4); if (n) mfile.write((char*)v.data(), sizeof(v[0]) * n); };

        // [NEW] 헤더: "MDL2" + version(2)
        const char magic[4] = { 'M','D','L','2' };
        mfile.write(magic, 4);
        WriteU32(2u);

        // --- Meshes (단일)
        _uint numMeshes = 1;
        mfile.write((char*)&numMeshes, 4);
        {
            WriteStr(mesh.name);
            _uint fixedMatIdx = 0; // 단일 파일 안에서는 0번만 사용
            mfile.write((char*)&fixedMatIdx, 4);

            W(mesh.positions); W(mesh.normals); W(mesh.texcoords); W(mesh.tangents); W(mesh.indices);

            _uint numBones = (_uint)mesh.bones.size();
            mfile.write((char*)&numBones, 4);
            for (auto& b : mesh.bones)
            {
                WriteStr(b.name);
                _uint w = (_uint)b.weights.size(); mfile.write((char*)&w, 4);
                if (w) mfile.write((char*)b.weights.data(), sizeof(VertexWeight) * w);
                WriteF4x4(b.offsetMatrix);
            }
        }

        // --- Materials (메쉬가 참조하는 하나만 포함)
        _uint numMat = 0;
        if (mesh.materialIndex < model.materials.size())
        {
            numMat = 1;
            mfile.write((char*)&numMat, 4);

            const auto& mat = model.materials[mesh.materialIndex];
            WriteStr(mat.name);

            // (a) 텍스처 경로들
            for (int t = 0; t < (int)TextureType::End; ++t)
            {
                _uint numTex = (_uint)mat.texturePaths[t].size();
                mfile.write((char*)&numTex, 4);
                for (auto& p : mat.texturePaths[t]) WriteStr(p);
            }
            // (b) [NEW] 컬러 3종
            WriteF4(mat.diffuseColor);
            WriteF4(mat.specularColor);
            WriteF4(mat.emissiveColor);
        }
        else
        {
            mfile.write((char*)&numMat, 4);
        }

        // --- Node (간단 루트만)
        {
            NodeData root{};
            root.name = "Root";
            root.parentIndex = -1;
            root.transform = IdentityF4x4();
            _uint c = 0;

            WriteStr(root.name);
            mfile.write((char*)&root.parentIndex, 4);
            WriteF4x4(root.transform);
            mfile.write((char*)&c, 4);
        }

        // --- Animations (개별 메쉬 bin에는 없음)
        _uint numAnim = 0;
        mfile.write((char*)&numAnim, 4);

        // --- Global Bones (해당 메쉬 본만)
        _uint nb = (_uint)mesh.bones.size();
        mfile.write((char*)&nb, 4);
        for (auto& b : mesh.bones)
        {
            WriteStr(b.name);
            WriteF4x4(b.offsetMatrix);
        }

        // 원본 경로
        WriteStr(model.modelDataFilePath);

        char buf[256];
        sprintf_s(buf, "[Export] %s : vtx=%zu, idx=%zu, matIdx=%d\n",
            mesh.name.c_str(), mesh.positions.size(), mesh.indices.size(), mesh.materialIndex);
        OutputDebugStringA(buf);

        mfile.close();
    }

    return true;
}
