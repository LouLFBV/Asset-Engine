#pragma once

#include <AssetEngine/Asset/AssetHandle.h>
#include <AssetEngine/Asset/AssetManager.h>
#include <AssetEngine/Asset/AssetRegistry.h>
#include <AssetEngine/Asset/JobSystem.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <imgui.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// The editor: shows the registry, loads assets in both ways, and shows what is in memory.
// It only uses the public API of the engine, like a game would.
class EditorApp
{
public:
	EditorApp(std::filesystem::path assetFolder, AssetEngine::SecurityPolicy policy);

	// Frees the preview on the GPU: called while the window still exists.
	~EditorApp();

	EditorApp(const EditorApp&) = delete;
	EditorApp& operator=(const EditorApp&) = delete;

	// Called once per frame, between Window::BeginFrame and Window::EndFrame.
	void Draw();

	void LoadAllAsync();
	const std::vector<std::string>& GetLog() const { return m_log; }

private:
	void DrawRegistryWindow();
	void DrawSelectionWindow();
	void DrawMemoryWindow();
	void DrawLogWindow();

	void Scan();
	void LoadSync(AssetEngine::AssetId id);
	void LoadAsync(AssetEngine::AssetId id);
	void OnLoaded(const AssetEngine::AssetHandle<AssetEngine::Asset>& handle, std::string_view how, double milliseconds);
	void Release(AssetEngine::AssetId id);
	void CorruptFile(const AssetEngine::AssetRecord& record);
	void ConvertFile(const AssetEngine::AssetRecord& record);
	void ShowContent(AssetEngine::AssetId id);
	void DrawTexturePreview(const AssetEngine::Asset& asset, AssetEngine::AssetId id);
	void FreePreview();

	const AssetEngine::AssetHandle<AssetEngine::Asset>* FindHeld(AssetEngine::AssetId id) const;
	std::string GetPath(AssetEngine::AssetId id) const;
	std::string GetTypeName(const AssetEngine::AssetRecord& record) const;
	void Log(std::string line);

	std::filesystem::path m_folder;
	AssetEngine::SecurityPolicy m_policy;
	AssetEngine::AssetRegistry m_registry;

	// The manager is declared after the JobSystem: it is destroyed first, while the threads still exist.
	AssetEngine::JobSystem m_jobs;
	AssetEngine::AssetManager m_manager;

	// The handles the editor keeps: they keep their assets in memory.
	std::vector<AssetEngine::AssetHandle<AssetEngine::Asset>> m_held;

	AssetEngine::AssetId m_selected;
	char m_filter[64] = {};
	std::string m_content;
	std::vector<std::string> m_log;
	bool m_scrollLog = false;

	// The selected texture, sent to the GPU to be drawn. Only one at a time.
	AssetEngine::AssetId m_previewId;
	ImTextureID m_preview = ImTextureID_Invalid;
};
