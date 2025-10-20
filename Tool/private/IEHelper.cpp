#include "IEHelper.h"
#include "Tool_Defines.h"

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

    // ★ 프로퍼티: 본 가중치 제한, Collada 업축 무시(관성), 글로벌 스케일 옵션 등
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
    // 필요 시: importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);

    unsigned int flags =
        aiProcess_ConvertToLeftHanded |
        aiProcessPreset_TargetRealtime_MaxQuality;

    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || !scene->mRootNode)
        return false;

    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.animations.clear();
    outModel.bones.clear();

    std::filesystem::path modelFolder = std::filesystem::path(filePath).parent_path();

    // ---------- 1. Meshes ----------
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        MeshData meshData;
        meshData.name = mesh->mName.C_Str();
        meshData.materialIndex = mesh->mMaterialIndex;

        meshData.positions.reserve(mesh->mNumVertices);
        if (mesh->HasNormals())  meshData.normals.reserve(mesh->mNumVertices);
        if (mesh->HasTextureCoords(0)) meshData.texcoords.reserve(mesh->mNumVertices);
        if (mesh->HasTangentsAndBitangents()) meshData.tangents.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            meshData.positions.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });

            if (mesh->HasNormals())
                meshData.normals.push_back({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
            else
                meshData.normals.push_back({ 0.f, 1.f, 0.f });

            if (mesh->HasTextureCoords(0)) {
                meshData.texcoords.push_back({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
            }
            else {
                // ★ UV 없으면 0 채움(쉐이더 경로 보호)
                meshData.texcoords.push_back({ 0.f, 0.f });
            }

            if (mesh->HasTangentsAndBitangents()) {
                meshData.tangents.push_back({ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
            }
        }

        // 인덱스
        meshData.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                meshData.indices.push_back(face.mIndices[idx]);
        }

        // 본/가중치
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            const aiBone* bone = mesh->mBones[b];
            MeshBone meshBone;
            meshBone.name = bone->mName.C_Str();
            meshBone.offsetMatrix = ToF4x4(bone->mOffsetMatrix); // ★ reinterpret_cast 지양

            meshBone.weights.reserve(bone->mNumWeights);
            for (unsigned int w = 0; w < bone->mNumWeights; ++w)
            {
                VertexWeight vw;
                vw.vertexId = bone->mWeights[w].mVertexId;
                vw.weight = bone->mWeights[w].mWeight;
                meshBone.weights.push_back(vw);
            }
            meshData.bones.push_back(meshBone);

            // 전역 본 목록 중복 방지
            bool exists = std::any_of(outModel.bones.begin(), outModel.bones.end(),
                [&](const BoneData& bData) { return bData.name == meshBone.name; });
            if (!exists)
            {
                BoneData boneData;
                boneData.name = meshBone.name;
                boneData.offsetMatrix = meshBone.offsetMatrix;
                outModel.bones.push_back(boneData);
            }
        }

        outModel.meshes.push_back(std::move(meshData));
    }

    // ---------- 2. Materials ----------
    // ★ 임베디드 텍스처(*n) 보관용 폴더 (원하면 끄기 가능)
    std::filesystem::path embeddedOut = modelFolder / "_embedded_textures";

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
                aiTextureMapMode mapMode[2] = { aiTextureMapMode_Wrap, aiTextureMapMode_Wrap };
                if (mat->GetTexture(type, t, &texPath, nullptr, nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS)
                {
                    std::string p = texPath.C_Str();

                    // ★ 임베디드 *n 처리
                    if (!p.empty() && p[0] == '*' && scene->HasTextures())
                    {
                        int idx = std::atoi(p.c_str() + 1);
                        if (idx >= 0 && idx < (int)scene->mNumTextures)
                        {
                            const aiTexture* tex = scene->mTextures[idx];
                            std::string dumped = DumpEmbeddedTexture(tex, embeddedOut, idx);
                            if (!dumped.empty())
                                matData.texturePaths[eType].push_back(dumped);
                            else
                                matData.texturePaths[eType].push_back(p); // fallback
                        }
                        else {
                            matData.texturePaths[eType].push_back(p); // 잘못된 인덱스면 일단 보존
                        }
                    }
                    else
                    {
                        // ★ 파일 경로 보정(확장자 없는 케이스 회복/정규화)
                        std::string fixed = FixTexturePath(p, modelFolder);
                        matData.texturePaths[eType].push_back(fixed);
                    }
                }
            }
        };

        LoadTextures(aiTextureType_DIFFUSE, TextureType::Diffuse);
        LoadTextures(aiTextureType_SPECULAR, TextureType::Specular);
        LoadTextures(aiTextureType_NORMALS, TextureType::Normal);
        LoadTextures(aiTextureType_EMISSIVE, TextureType::Emissive);

        outModel.materials.push_back(std::move(matData));
    }

    // ---------- 3. Node Hierarchy ----------
    // ★ parentIndex 계산 버그 수정: 각 노드에 고유 index 부여
    int nodeCounter = 0;
    std::function<void(const aiNode*, NodeData&, int /*parentIndex*/)> processNode;
    processNode = [&](const aiNode* node, NodeData& outNode, int parentIndex)
    {
        const int myIndex = nodeCounter++;

        outNode.name = (node->mName.length > 0) ? node->mName.C_Str() : "Root";
        outNode.parentIndex = parentIndex;                          // ★ 올바른 부모 인덱스 기록
        outNode.transform = ToF4x4(node->mTransformation);          // ★ 안전 변환
        if (!(outNode.transform._11 || outNode.transform._22 || outNode.transform._33)) {
            outNode.transform = IdentityF4x4();
        }

        outNode.children.resize(node->mNumChildren);
        for (unsigned int c = 0; c < node->mNumChildren; ++c)
            processNode(node->mChildren[c], outNode.children[c], myIndex); // ★ myIndex 전달
    };

    outModel.rootNode = {};
    processNode(scene->mRootNode, outModel.rootNode, -1);

    // ---------- 4. Animations ----------
    for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
    {
        const aiAnimation* anim = scene->mAnimations[a];
        AnimationData animData;
        animData.name = anim->mName.C_Str();

        const double tps = (anim->mTicksPerSecond != 0.0) ? anim->mTicksPerSecond : 30.0;
        animData.ticksPerSecond = static_cast<float>(tps);
        animData.duration = static_cast<float>(anim->mDuration); // 필요 시 seconds로 바꿀 땐 /tps

        for (unsigned int c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim* channel = anim->mChannels[c];
            ChannelData channelData;
            channelData.nodeName = channel->mNodeName.C_Str();

            channelData.positionKeys.reserve(channel->mNumPositionKeys);
            for (unsigned int k = 0; k < channel->mNumPositionKeys; ++k) {
                KeyVector key;
                key.time = static_cast<float>(channel->mPositionKeys[k].mTime);
                key.value = { channel->mPositionKeys[k].mValue.x,
                              channel->mPositionKeys[k].mValue.y,
                              channel->mPositionKeys[k].mValue.z };
                channelData.positionKeys.push_back(key);
            }

            channelData.rotationKeys.reserve(channel->mNumRotationKeys);
            for (unsigned int k = 0; k < channel->mNumRotationKeys; ++k) {
                KeyQuat key;
                key.time = static_cast<float>(channel->mRotationKeys[k].mTime);
                key.value = { channel->mRotationKeys[k].mValue.x,
                              channel->mRotationKeys[k].mValue.y,
                              channel->mRotationKeys[k].mValue.z,
                              channel->mRotationKeys[k].mValue.w };
                NormalizeQuat(key.value); // ★ 보간 안정성
                channelData.rotationKeys.push_back(key);
            }

            channelData.scalingKeys.reserve(channel->mNumScalingKeys);
            for (unsigned int k = 0; k < channel->mNumScalingKeys; ++k) {
                KeyVector key;
                key.time = static_cast<float>(channel->mScalingKeys[k].mTime);
                key.value = { channel->mScalingKeys[k].mValue.x,
                              channel->mScalingKeys[k].mValue.y,
                              channel->mScalingKeys[k].mValue.z };
                channelData.scalingKeys.push_back(key);
            }

            animData.channels.push_back(std::move(channelData));
        }

        outModel.animations.push_back(std::move(animData));
    }

    return true;
}

bool IEHelper::ExportModel(const std::string& filePath, const ModelData& model)
{
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    // ---------------------
    // 1. Meshes
    _uint numMeshes = static_cast<_uint>(model.meshes.size());
    file.write(reinterpret_cast<const char*>(&numMeshes), sizeof(_uint));

    for (const auto& mesh : model.meshes)
    {
        _uint nameLen = static_cast<_uint>(mesh.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(mesh.name.data(), nameLen);

        file.write(reinterpret_cast<const char*>(&mesh.materialIndex), sizeof(_uint));

        _uint numPos = static_cast<_uint>(mesh.positions.size());
        file.write(reinterpret_cast<const char*>(&numPos), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.positions.data()), sizeof(_float3) * numPos);

        _uint numNormals = static_cast<_uint>(mesh.normals.size());
        file.write(reinterpret_cast<const char*>(&numNormals), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.normals.data()), sizeof(_float3) * numNormals);

        _uint numTex = static_cast<_uint>(mesh.texcoords.size());
        file.write(reinterpret_cast<const char*>(&numTex), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.texcoords.data()), sizeof(_float2) * numTex);

        _uint numTang = static_cast<_uint>(mesh.tangents.size());
        file.write(reinterpret_cast<const char*>(&numTang), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.tangents.data()), sizeof(_float3) * numTang);

        _uint numIndices = static_cast<_uint>(mesh.indices.size());
        file.write(reinterpret_cast<const char*>(&numIndices), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.indices.data()), sizeof(_uint) * numIndices);

        _uint numBones = static_cast<_uint>(mesh.bones.size());
        file.write(reinterpret_cast<const char*>(&numBones), sizeof(_uint));

        for (const auto& bone : mesh.bones)
        {
            _uint boneNameLen = static_cast<_uint>(bone.name.size());
            file.write(reinterpret_cast<const char*>(&boneNameLen), sizeof(_uint));
            file.write(bone.name.data(), boneNameLen);

            _uint numWeights = static_cast<_uint>(bone.weights.size());
            file.write(reinterpret_cast<const char*>(&numWeights), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(bone.weights.data()), sizeof(VertexWeight) * numWeights);

            file.write(reinterpret_cast<const char*>(&bone.offsetMatrix), sizeof(_float4x4));
        }
    }

    // ---------------------
    // 2. Materials
    _uint numMaterials = static_cast<_uint>(model.materials.size());
    file.write(reinterpret_cast<const char*>(&numMaterials), sizeof(_uint));

    for (const auto& mat : model.materials)
    {
        _uint nameLen = static_cast<_uint>(mat.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(mat.name.data(), nameLen);

        for (int t = 0; t < (int)TextureType::End; t++)
        {
            _uint numTex = static_cast<_uint>(mat.texturePaths[t].size());
            file.write(reinterpret_cast<const char*>(&numTex), sizeof(_uint));
            for (const auto& path : mat.texturePaths[t])
            {
                _uint pathLen = static_cast<_uint>(path.size());
                file.write(reinterpret_cast<const char*>(&pathLen), sizeof(_uint));
                file.write(path.data(), pathLen);
            }
        }
    }

    // ---------------------
    // 3. Node hierarchy
    std::function<void(const NodeData&)> writeNode;
    writeNode = [&](const NodeData& node)
    {
        std::string safeName = node.name.empty() ? "Unnamed" : node.name;
        _uint nameLen = static_cast<_uint>(safeName.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(safeName.data(), nameLen);

        file.write(reinterpret_cast<const char*>(&node.parentIndex), sizeof(_int));
        file.write(reinterpret_cast<const char*>(&node.transform), sizeof(_float4x4));

        _uint numChildren = static_cast<_uint>(node.children.size());
        file.write(reinterpret_cast<const char*>(&numChildren), sizeof(_uint));
        for (const auto& child : node.children)
            writeNode(child);
    };

    writeNode(model.rootNode);

    // ---------------------
    // 4. Animations
    _uint numAnims = static_cast<_uint>(model.animations.size());
    file.write(reinterpret_cast<const char*>(&numAnims), sizeof(_uint));

    for (const auto& anim : model.animations)
    {
        _uint nameLen = static_cast<_uint>(anim.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(anim.name.data(), nameLen);

        file.write(reinterpret_cast<const char*>(&anim.duration), sizeof(float));
        file.write(reinterpret_cast<const char*>(&anim.ticksPerSecond), sizeof(float));

        _uint numChannels = static_cast<_uint>(anim.channels.size());
        file.write(reinterpret_cast<const char*>(&numChannels), sizeof(_uint));

        for (const auto& ch : anim.channels)
        {
            _uint nodeNameLen = static_cast<_uint>(ch.nodeName.size());
            file.write(reinterpret_cast<const char*>(&nodeNameLen), sizeof(_uint));
            file.write(ch.nodeName.data(), nodeNameLen);

            _uint numPosKeys = static_cast<_uint>(ch.positionKeys.size());
            file.write(reinterpret_cast<const char*>(&numPosKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.positionKeys.data()), sizeof(KeyVector) * numPosKeys);

            _uint numRotKeys = static_cast<_uint>(ch.rotationKeys.size());
            file.write(reinterpret_cast<const char*>(&numRotKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.rotationKeys.data()), sizeof(KeyQuat) * numRotKeys);

            _uint numScaleKeys = static_cast<_uint>(ch.scalingKeys.size());
            file.write(reinterpret_cast<const char*>(&numScaleKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.scalingKeys.data()), sizeof(KeyVector) * numScaleKeys);
        }
    }

    // ---------------------
    // 5. Global Bones (선택적 — 구조 완결성용)
    _uint numBones = static_cast<_uint>(model.bones.size());
    file.write(reinterpret_cast<const char*>(&numBones), sizeof(_uint));
    for (const auto& bone : model.bones)
    {
        _uint nameLen = static_cast<_uint>(bone.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(bone.name.data(), nameLen);
        file.write(reinterpret_cast<const char*>(&bone.offsetMatrix), sizeof(_float4x4));
    }

    file.close();
    return true;
}
