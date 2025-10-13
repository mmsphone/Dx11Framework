#pragma once
#include "NoAssimpModelStruct.h"

class IEHelper
{
public:
    static bool ImportFBX(const std::string& filePath, ModelData& outModel);
    static bool ExportModel(const std::string& filePath, const ModelData& model);
};