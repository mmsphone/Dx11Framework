#pragma once

#include "EngineUtility.h"

class AssimpTool
{
public:
    static AssimpTool* GetInstance();

    bool LoadModel(const std::string& path);      // FBX/OBJ 등 읽기
    void ExportBinary(const std::string& path);   // 바이너리 변환 후 저장

    // DX11용 데이터 전달
    void FillMeshBuffers(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);

private:
    AssimpTool() = default;
    const aiScene* m_pScene = nullptr;
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
};
