#include "IEHelper.h"

#include "Tool_Defines.h"

bool IEHelper::ImportFBX(const std::string& filePath, ModelData& outModel)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode)
        return false;

    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.animations.clear();

    // 모델 파일 폴더 경로
    std::filesystem::path modelFolder = std::filesystem::path(filePath).parent_path();

    // ---------------------
    // 1. Meshes
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* mesh = scene->mMeshes[m];
        MeshData meshData;
        meshData.name = mesh->mName.C_Str();
        meshData.materialIndex = mesh->mMaterialIndex;

        // Vertex attributes
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            meshData.positions.push_back({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
            if (mesh->HasNormals())
                meshData.normals.push_back({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
            if (mesh->HasTextureCoords(0))
                meshData.texcoords.push_back({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
            if (mesh->HasTangentsAndBitangents())
                meshData.tangents.push_back({ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
        }

        // Indices
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                meshData.indices.push_back(face.mIndices[idx]);
        }

        // Bones
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone* bone = mesh->mBones[b];
            MeshBone meshBone;
            meshBone.name = bone->mName.C_Str();
            meshBone.offsetMatrix = *reinterpret_cast<const _float4x4*>(&bone->mOffsetMatrix);

            for (unsigned int w = 0; w < bone->mNumWeights; ++w)
            {
                VertexWeight vw;
                vw.vertexId = bone->mWeights[w].mVertexId;
                vw.weight = bone->mWeights[w].mWeight;
                meshBone.weights.push_back(vw);
            }
            meshData.bones.push_back(meshBone);
        }

        outModel.meshes.push_back(meshData);
    }

    // ---------------------
    // 2. Materials
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* mat = scene->mMaterials[i];
        MaterialData matData;
        matData.name = mat->GetName().C_Str();

        auto LoadTextures = [&](aiTextureType type, TextureType eType)
        {
            unsigned int texCount = mat->GetTextureCount(type);
            for (unsigned int t = 0; t < texCount; ++t)
            {
                aiString texPath;
                if (mat->GetTexture(type, t, &texPath) == AI_SUCCESS)
                {
                    // 모델 폴더 기준으로 전체 경로 결합
                    std::filesystem::path fullPath = modelFolder / texPath.C_Str();
                    matData.texturePaths[eType].push_back(fullPath.string());
                }
            }
        };

        LoadTextures(aiTextureType_DIFFUSE, TextureType::Diffuse);
        LoadTextures(aiTextureType_SPECULAR, TextureType::Specular);
        LoadTextures(aiTextureType_NORMALS, TextureType::Normal);
        LoadTextures(aiTextureType_EMISSIVE, TextureType::Emissive);

        outModel.materials.push_back(matData);
    }

    // ---------------------
    // 3. Node Hierarchy
    std::function<void(aiNode*, NodeData&, int)> processNode;
    processNode = [&](aiNode* node, NodeData& outNode, int parentIndex)
    {
        outNode.name = node->mName.C_Str();
        outNode.parentIndex = parentIndex;
        outNode.transform = *reinterpret_cast<const _float4x4*>(&node->mTransformation);

        outNode.children.resize(node->mNumChildren);
        for (unsigned int c = 0; c < node->mNumChildren; ++c)
            processNode(node->mChildren[c], outNode.children[c], -1);
    };

    outModel.rootNode = {};
    processNode(scene->mRootNode, outModel.rootNode, -1);

    // ---------------------
    // 4. Animations + Channels
    for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
    {
        aiAnimation* anim = scene->mAnimations[a];
        AnimationData animData;
        animData.name = anim->mName.C_Str();
        animData.duration = static_cast<float>(anim->mDuration);
        animData.ticksPerSecond = static_cast<float>(
            anim->mTicksPerSecond != 0.0 ? anim->mTicksPerSecond : 30.0f);

        for (unsigned int c = 0; c < anim->mNumChannels; ++c)
        {
            aiNodeAnim* channel = anim->mChannels[c];
            ChannelData channelData;
            channelData.nodeName = channel->mNodeName.C_Str();

            // Position Keys
            for (unsigned int k = 0; k < channel->mNumPositionKeys; ++k)
            {
                KeyVector key;
                key.time = static_cast<float>(channel->mPositionKeys[k].mTime);
                key.value = { channel->mPositionKeys[k].mValue.x,
                              channel->mPositionKeys[k].mValue.y,
                              channel->mPositionKeys[k].mValue.z };
                channelData.positionKeys.push_back(key);
            }

            // Rotation Keys
            for (unsigned int k = 0; k < channel->mNumRotationKeys; ++k)
            {
                KeyQuat key;
                key.time = static_cast<float>(channel->mRotationKeys[k].mTime);
                key.value = { channel->mRotationKeys[k].mValue.x,
                              channel->mRotationKeys[k].mValue.y,
                              channel->mRotationKeys[k].mValue.z,
                              channel->mRotationKeys[k].mValue.w };
                channelData.rotationKeys.push_back(key);
            }

            // Scaling Keys
            for (unsigned int k = 0; k < channel->mNumScalingKeys; ++k)
            {
                KeyVector key;
                key.time = static_cast<float>(channel->mScalingKeys[k].mTime);
                key.value = { channel->mScalingKeys[k].mValue.x,
                              channel->mScalingKeys[k].mValue.y,
                              channel->mScalingKeys[k].mValue.z };
                channelData.scalingKeys.push_back(key);
            }

            animData.channels.push_back(channelData);
        }

        outModel.animations.push_back(animData);
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
        // name
        _uint nameLen = static_cast<_uint>(mesh.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(mesh.name.data(), nameLen);

        // material index
        file.write(reinterpret_cast<const char*>(&mesh.materialIndex), sizeof(_uint));

        // positions
        _uint numPos = static_cast<_uint>(mesh.positions.size());
        file.write(reinterpret_cast<const char*>(&numPos), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.positions.data()), sizeof(_float3) * numPos);

        // normals
        _uint numNormals = static_cast<_uint>(mesh.normals.size());
        file.write(reinterpret_cast<const char*>(&numNormals), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.normals.data()), sizeof(_float3) * numNormals);

        // texcoords
        _uint numTex = static_cast<_uint>(mesh.texcoords.size());
        file.write(reinterpret_cast<const char*>(&numTex), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.texcoords.data()), sizeof(_float2) * numTex);

        // tangents
        _uint numTang = static_cast<_uint>(mesh.tangents.size());
        file.write(reinterpret_cast<const char*>(&numTang), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.tangents.data()), sizeof(_float3) * numTang);

        // indices
        _uint numIndices = static_cast<_uint>(mesh.indices.size());
        file.write(reinterpret_cast<const char*>(&numIndices), sizeof(_uint));
        file.write(reinterpret_cast<const char*>(mesh.indices.data()), sizeof(_uint) * numIndices);

        // bones
        _uint numBones = static_cast<_uint>(mesh.bones.size());
        file.write(reinterpret_cast<const char*>(&numBones), sizeof(_uint));

        for (const auto& bone : mesh.bones)
        {
            // bone name
            _uint boneNameLen = static_cast<_uint>(bone.name.size());
            file.write(reinterpret_cast<const char*>(&boneNameLen), sizeof(_uint));
            file.write(bone.name.data(), boneNameLen);

            // weights
            _uint numWeights = static_cast<_uint>(bone.weights.size());
            file.write(reinterpret_cast<const char*>(&numWeights), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(bone.weights.data()), sizeof(VertexWeight) * numWeights);

            // offset matrix
            file.write(reinterpret_cast<const char*>(&bone.offsetMatrix), sizeof(_float4x4));
        }
    }

    // ---------------------
    // 2. Materials
    _uint numMaterials = static_cast<_uint>(model.materials.size());
    file.write(reinterpret_cast<const char*>(&numMaterials), sizeof(_uint));

    for (const auto& mat : model.materials)
    {
        // material name
        _uint nameLen = static_cast<_uint>(mat.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(mat.name.data(), nameLen);

        // textures (전체 경로 유지)
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
        _uint nameLen = static_cast<_uint>(node.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(_uint));
        file.write(node.name.data(), nameLen);

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

        // channels
        _uint numChannels = static_cast<_uint>(anim.channels.size());
        file.write(reinterpret_cast<const char*>(&numChannels), sizeof(_uint));

        for (const auto& ch : anim.channels)
        {
            _uint nodeNameLen = static_cast<_uint>(ch.nodeName.size());
            file.write(reinterpret_cast<const char*>(&nodeNameLen), sizeof(_uint));
            file.write(ch.nodeName.data(), nodeNameLen);

            // position keys
            _uint numPosKeys = static_cast<_uint>(ch.positionKeys.size());
            file.write(reinterpret_cast<const char*>(&numPosKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.positionKeys.data()), sizeof(KeyVector) * numPosKeys);

            // rotation keys
            _uint numRotKeys = static_cast<_uint>(ch.rotationKeys.size());
            file.write(reinterpret_cast<const char*>(&numRotKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.rotationKeys.data()), sizeof(KeyQuat) * numRotKeys);

            // scaling keys
            _uint numScaleKeys = static_cast<_uint>(ch.scalingKeys.size());
            file.write(reinterpret_cast<const char*>(&numScaleKeys), sizeof(_uint));
            file.write(reinterpret_cast<const char*>(ch.scalingKeys.data()), sizeof(KeyVector) * numScaleKeys);
        }
    }

    file.close();
    return true;
}
