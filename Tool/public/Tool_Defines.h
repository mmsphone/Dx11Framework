#pragma once

#include <Windows.h>
#include <process.h>

#include <vector>
#include <string>

#include <fstream>
#include <filesystem>
#include <commdlg.h>  

#ifdef new
#undef new
#endif
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Tool
{
	static const unsigned int g_iWinSizeX = 1280;
	static const unsigned int g_iWinSizeY = 720;

	enum SCENE : int { STATIC, MODEL, MAP, SCENE_END };
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWnd;

struct Vertex {
    float pos[3];
    float normal[3];
    float uv[2];
};

class ToolModel {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

using namespace Tool;