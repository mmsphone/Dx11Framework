#pragma once

#include "Panel.h"

NS_BEGIN(Tool)

class AssetPanel final : public Panel
{
	AssetPanel(const string& PanelName, bool open = true);
	virtual ~AssetPanel() = default;

public:
	HRESULT Initialize();
	virtual void OnRender() override;

	static AssetPanel* Create(const string& PanelName, bool open = true);
	virtual void Free() override;

private:
	_bool LoadPreviewTexture(const std::filesystem::path& path);

private:
	std::filesystem::path m_CurrentDirectory;
	std::filesystem::path m_RootDirectory;
	std::string m_SelectedFile;

	ID3D11ShaderResourceView* m_pPreviewShaderResourceView = nullptr;
};

NS_END