#include "DemoAssets.h"
#include "EditorApp.h"
#include "ImageImporter.h"
#include "Window.h"

#include <AssetEngine/Asset/AssetManager.h>
#include <AssetEngine/Asset/AssetRegistry.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

// Usage: Editor [--frames N]
// With --frames, the editor loads every asset in the background, closes after N frames and prints its log:
// a quick check that the whole interface runs.
int main(int argc, char** argv)
{
	// Every type registered itself before main: from now on, the type registry never changes.
	AssetEngine::TypeRegistry::Global().Freeze();

	int frameLimit = 0;
	if (argc == 3 && std::string_view(argv[1]) == "--frames")
		frameLimit = std::atoi(argv[2]);

	Window window("Asset Engine - Editor", 1400, 860);
	if (!window.IsValid())
		return 1;

	const AssetEngine::SecurityPolicy policy = AssetEngine::SecurityPolicy::Checksum();
	const std::filesystem::path folder = std::filesystem::current_path() / "EditorAssets";
	Demo::CreateDemoAssets(folder, policy);
	if (!Demo::ImportDemoImage(folder, policy))
		std::cerr << "Could not import " << Demo::GetDemoImagePath().string() << '\n';

	EditorApp editor(folder, policy);
	if (frameLimit > 0)
		editor.LoadAllAsync();

	for (int frame = 0; window.BeginFrame(); ++frame)
	{
		editor.Draw();
		window.EndFrame();

		if (frameLimit > 0 && frame + 1 >= frameLimit)
			break;
	}

	if (frameLimit > 0)
	{
		for (const std::string& line : editor.GetLog())
			std::cout << line << '\n';

		// The real image must come back from its .asset file exactly as the importer read it.
		AssetEngine::AssetRegistry registry(folder);
		registry.Scan();
		AssetEngine::AssetManager manager(registry, policy);
		const AssetEngine::AssetHandle<Demo::TextureAsset> loaded = manager.Load<Demo::TextureAsset>("textures/dice.asset");
		const std::optional<Demo::TextureAsset> original = Demo::ImportImage(Demo::GetDemoImagePath());

		const bool same = loaded && original && loaded->width == original->width && loaded->height == original->height
			&& loaded->pixels == original->pixels;
		std::cout << "dice.asset: " << (same ? "same pixels as dice.png" : "DIFFERENT from dice.png") << '\n';
		if (!same)
			return 1;
	}

	return 0;
}
