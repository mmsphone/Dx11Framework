#pragma once
#include "NoAssimpModelStruct.h"

class ImportHelper
{
public:
    static bool ImportFBX(const std::string& filePath, ModelData& outModel);
};