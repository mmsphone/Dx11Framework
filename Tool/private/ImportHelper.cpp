#include "ImportHelper.h"

#include "Tool_Defines.h"

bool ImportHelper::ImportFBX(const std::string& filePath, ModelData& outModel)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if (!scene || !scene->mRootNode)
        return false;

    // 메시 변환
    outModel.meshes.clear();
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        aiMesh* mesh = scene->mMeshes[m];
        MeshData meshData;
        meshData.name = mesh->mName.C_Str();
        meshData.materialIndex = mesh->mMaterialIndex;

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

        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            aiFace face = mesh->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; ++idx)
                meshData.indices.push_back(face.mIndices[idx]);
        }

        // 본 처리 (간단히 변환)
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone* bone = mesh->mBones[b];
            MeshBone meshBone;
            meshBone.name = bone->mName.C_Str();
            meshBone.offsetMatrix = *reinterpret_cast<_float4x4*>(&bone->mOffsetMatrix);

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

    // 머티리얼, 노드, 애니메이션 등 필요시 추가 가능
    return true;
}
