#pragma once

#include "Engine_Defines.h"
#include "Engine_Typedef.h"

// 🔹 텍스처 타입 (aiTextureType 대체)
enum TextureType : int {
    Diffuse,
    Specular,
    Normal,
    Emissive,
    End
};

struct VertexWeight {
    _uint vertexId = 0;
    float weight = 0.f;
};

struct MeshBone {
    std::string name;
    std::vector<VertexWeight> weights;
    _float4x4 offsetMatrix = {};
};

// 🔹 메시 데이터 (정점 + 인덱스)
struct MeshData {
    std::string name;
    _uint       materialIndex = 0;

    std::vector<_float3> positions;
    std::vector<_float3> normals;
    std::vector<_float2> texcoords;
    std::vector<_float3> tangents;
    std::vector<_uint>   indices;

    std::vector<MeshBone> bones;
};

// 🔹 머티리얼 데이터
struct MaterialData {
    std::string name;
    std::vector<std::string> texturePaths[(int)TextureType::End];
};

// 🔹 본(스켈레톤) 데이터
struct BoneData {
    std::string name;
    _int        parentIndex = -1;
    _float4x4   offsetMatrix = {};
    _float4x4   transformMatrix = {};
};

// 🔹 노드 트리 데이터
struct NodeData {
    std::string name;
    _int        parentIndex = -1;
    _float4x4   transform = {};
    std::vector<NodeData> children;
};

// 🔹 애니메이션 데이터
struct KeyVector {
    float time;
    _float3 value;
};

struct KeyQuat {
    float time;
    _float4 value;
};

struct ChannelData {
    std::string nodeName;
    std::vector<KeyVector> positionKeys;
    std::vector<KeyQuat> rotationKeys;
    std::vector<KeyVector> scalingKeys;
};

struct AnimationData {
    std::string name;
    float duration = 0.f;
    float ticksPerSecond = 0.f;
    std::vector<ChannelData> channels;
};

// 🔹 모델 전체 데이터
struct ModelData {
    std::vector<MeshData>      meshes;
    std::vector<MaterialData>  materials;
    std::vector<BoneData>      bones;
    std::vector<AnimationData> animations;

    NodeData rootNode;
};