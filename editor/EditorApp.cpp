#include "EditorApp.h"

#include "DemoAssets.h"
#include "Window.h"

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileInputStream.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>
#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>
#include <AssetEngine/Serialization/Json/JsonWriter.h>

#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <utility>

using namespace AssetEngine;

namespace
{
	using Clock = std::chrono::steady_clock;

	// The JSON view of a big texture would be several megabytes: only its start is shown.
	constexpr std::size_t MaxContentSize = 4000;

	const ImVec4 Yellow{1.0f, 0.8f, 0.3f, 1.0f};
	const ImVec4 Green{0.4f, 0.9f, 0.5f, 1.0f};

	double MillisecondsSince(Clock::time_point start)
	{
		return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
	}

	std::string Hex(std::uint64_t value)
	{
		char text[17];
		std::snprintf(text, sizeof(text), "%016llx", static_cast<unsigned long long>(value));
		return text;
	}

	std::string Join(const std::vector<std::string>& items)
	{
		std::string text;
		for (const std::string& item : items)
			text += (text.empty() ? "" : ", ") + item;

		return text.empty() ? "-" : text;
	}
}

EditorApp::EditorApp(std::filesystem::path assetFolder, SecurityPolicy policy)
	: m_folder(std::move(assetFolder))
	, m_policy(std::move(policy))
	, m_registry(m_folder)
	, m_manager(m_registry, m_policy, &m_jobs)
{
	Scan();
}

EditorApp::~EditorApp()
{
	FreePreview();
}

void EditorApp::Draw()
{
	// The callbacks of the finished background loads run here, on the main thread.
	m_manager.Update();

	DrawRegistryWindow();
	DrawSelectionWindow();
	DrawMemoryWindow();
	DrawLogWindow();
}

void EditorApp::LoadAllAsync()
{
	for (const AssetRecord* record : m_registry.GetAll())
		LoadAsync(record->id);
}

void EditorApp::DrawRegistryWindow()
{
	ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({760, 520}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Registry");

	ImGui::Text("Folder: %s", m_folder.string().c_str());
	if (ImGui::Button("Scan"))
		Scan();

	ImGui::SameLine();
	if (ImGui::Button("Save registry.bin"))
		Log(m_registry.SaveCache(m_folder / "registry.bin") ? "registry.bin saved" : "registry.bin could not be saved");

	ImGui::SameLine();
	if (ImGui::Button("Load registry.bin"))
	{
		m_jobs.WaitIdle();
		Log(m_registry.LoadCache(m_folder / "registry.bin")
			? "registry.bin loaded: " + std::to_string(m_registry.Count()) + " assets, without scanning"
			: "registry.bin refused (missing or damaged): scan the folder again");
	}

	ImGui::SameLine();
	if (ImGui::Button("Load all async"))
		LoadAllAsync();

	ImGui::InputText("Filter", m_filter, sizeof(m_filter));

	if (ImGui::BeginTable("assets", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Path");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Encoding");
		ImGui::TableSetupColumn("Tags");
		ImGui::TableSetupColumn("State");
		ImGui::TableHeadersRow();

		for (const AssetRecord* record : m_registry.GetAll())
		{
			if (m_filter[0] != '\0' && record->path.find(m_filter) == std::string::npos)
				continue;

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(record->path.c_str(), record->id == m_selected, ImGuiSelectableFlags_SpanAllColumns))
			{
				m_selected = record->id;
				m_content.clear();
			}

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(GetTypeName(*record).c_str());
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(record->encoding == AssetEncoding::Json ? "JSON" : "Binary");
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(Join(record->metadata.tags).c_str());

			ImGui::TableNextColumn();
			if (m_manager.IsLoading(record->id))
				ImGui::TextColored(Yellow, "loading...");
			else if (m_manager.IsLoaded(record->id))
				ImGui::TextColored(Green, "in memory");
			else
				ImGui::TextDisabled("-");
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void EditorApp::DrawSelectionWindow()
{
	ImGui::SetNextWindowPos({780, 10}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({600, 520}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Selection");

	const AssetRecord* record = m_registry.Find(m_selected);
	if (record == nullptr)
	{
		ImGui::TextDisabled("Select an asset in the registry.");
		ImGui::End();
		return;
	}

	// What the registry knows, read from the header and the metadata only.
	ImGui::Text("Path: %s", record->path.c_str());
	ImGui::Text("Id: %s", Hex(record->id.value).c_str());
	ImGui::Text("Type: %s", GetTypeName(*record).c_str());
	ImGui::Text("Version: %u", record->metadata.version);
	ImGui::Text("Tags: %s", Join(record->metadata.tags).c_str());
	ImGui::Text("Dependencies: %s", Join(record->metadata.dependencies).c_str());
	ImGui::Separator();

	if (ImGui::Button("Load"))
		LoadSync(record->id);

	ImGui::SameLine();
	if (ImGui::Button("Load async"))
		LoadAsync(record->id);

	ImGui::SameLine();
	if (ImGui::Button("Release"))
		Release(record->id);

	ImGui::SameLine();
	if (ImGui::Button("Corrupt the file"))
		CorruptFile(*record);

	// Rewrites the file in the other encoding: the same Serialize writes binary or JSON.
	if (ImGui::Button(record->encoding == AssetEncoding::Json ? "Convert to binary" : "Convert to JSON"))
	{
		ConvertFile(*record);
		record = m_registry.Find(m_selected);
		if (record == nullptr)
		{
			ImGui::End();
			return;
		}
	}

	if (FindHeld(record->id) != nullptr && ImGui::Button("Show the content as JSON"))
		ShowContent(record->id);

	// A loaded texture is drawn: the pixels shown are the ones read back from the .asset file.
	if (const AssetHandle<Asset>* held = FindHeld(record->id))
		DrawTexturePreview(**held, record->id);

	if (!m_content.empty())
	{
		ImGui::BeginChild("content", {0, 0}, ImGuiChildFlags_Borders);
		ImGui::TextUnformatted(m_content.c_str());
		ImGui::EndChild();
	}

	ImGui::End();
}

void EditorApp::DrawMemoryWindow()
{
	ImGui::SetNextWindowPos({10, 540}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({760, 300}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Memory");

	ImGui::Text("Assets in memory: %zu", m_manager.GetLoadedCount());
	ImGui::SameLine();
	if (ImGui::Button("Release all"))
	{
		m_held.clear();
		m_content.clear();
		FreePreview();
		Log("Every handle released: the assets are freed");
	}

	ImGui::Separator();
	for (std::size_t i = 0; i < m_held.size(); ++i)
	{
		const AssetHandle<Asset>& handle = m_held[i];

		ImGui::PushID(static_cast<int>(i));
		const bool release = ImGui::SmallButton("Release");
		ImGui::SameLine();
		ImGui::Text("%s  (%s, %ld users)", GetPath(handle.GetId()).c_str(),
			std::string(handle->GetTypeInfo().GetName()).c_str(), handle.GetShared().use_count());
		ImGui::PopID();

		if (release)
		{
			Release(handle.GetId());
			break;
		}
	}

	ImGui::End();
}

void EditorApp::DrawLogWindow()
{
	ImGui::SetNextWindowPos({780, 540}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({600, 300}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Log");

	if (ImGui::Button("Clear"))
		m_log.clear();

	ImGui::BeginChild("lines");
	for (const std::string& line : m_log)
		ImGui::TextUnformatted(line.c_str());

	if (m_scrollLog)
	{
		ImGui::SetScrollHereY(1.0f);
		m_scrollLog = false;
	}

	ImGui::EndChild();
	ImGui::End();
}

void EditorApp::Scan()
{
	// The jobs read the registry: it is only changed once no load is running.
	m_jobs.WaitIdle();

	const ScanReport report = m_registry.Scan();
	Log("Scan: " + std::to_string(report.found) + " assets found");
	for (const std::string& error : report.errors)
		Log("  " + error);
}

void EditorApp::LoadSync(AssetId id)
{
	// Load blocks the frame until the file is read: the interface freezes during big loads.
	const Clock::time_point start = Clock::now();
	const AssetHandle<Asset> handle = m_manager.Load(id);
	OnLoaded(handle, "Load", MillisecondsSince(start));
}

void EditorApp::LoadAsync(AssetId id)
{
	// LoadAsync returns at once: the file is read by a job, and the callback runs in a later Update.
	const Clock::time_point start = Clock::now();
	m_manager.LoadAsync(id, [this, start](AssetHandle<Asset> handle) {
		OnLoaded(handle, "LoadAsync", MillisecondsSince(start));
	});
}

void EditorApp::OnLoaded(const AssetHandle<Asset>& handle, std::string_view how, double milliseconds)
{
	const std::string path = GetPath(handle.GetId());
	if (!handle)
	{
		Log(std::string(how) + " " + path + ": failed, " + std::string(ToString(handle.GetError())));
		return;
	}

	char time[32];
	std::snprintf(time, sizeof(time), "%.2f ms", milliseconds);
	Log(std::string(how) + " " + path + ": ok in " + time);

	if (FindHeld(handle.GetId()) == nullptr)
		m_held.push_back(handle);
}

void EditorApp::Release(AssetId id)
{
	std::erase_if(m_held, [id](const AssetHandle<Asset>& handle) { return handle.GetId() == id; });
	if (id == m_selected)
		m_content.clear();

	if (id == m_previewId)
		FreePreview();

	Log("Released " + GetPath(id) + (m_manager.IsLoaded(id) ? ": still used elsewhere" : ": freed"));
}

void EditorApp::CorruptFile(const AssetRecord& record)
{
	const std::filesystem::path file = m_registry.GetFullPath(record);

	std::vector<char> bytes;
	{
		std::ifstream input(file, std::ios::binary);
		bytes.assign(std::istreambuf_iterator<char>(input), {});
	}

	if (bytes.empty())
	{
		Log("Cannot read " + record.path);
		return;
	}

	// One bit in the middle of the file: the digest no longer matches.
	bytes[bytes.size() / 2] ^= 0x01;
	std::ofstream(file, std::ios::binary).write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

	Log("Corrupted one bit of " + record.path + ": the next load from the file fails with IntegrityFailure");
	if (m_manager.IsLoaded(record.id))
		Log("  The asset in memory is not affected: release it first");
}

void EditorApp::ConvertFile(const AssetRecord& record)
{
	// Scan replaces the records: everything needed is copied first.
	const std::string path = record.path;
	const std::filesystem::path file = m_registry.GetFullPath(record);
	const AssetEncoding encoding = record.encoding == AssetEncoding::Json ? AssetEncoding::Binary : AssetEncoding::Json;
	const std::string target = encoding == AssetEncoding::Json ? "JSON" : "binary";

	// No job may read the file while it is rewritten.
	m_jobs.WaitIdle();
	const Clock::time_point start = Clock::now();

	// The file is read directly, not through the manager: the copy in memory may be an older version.
	AssetLoadResult loaded;
	{
		FileInputStream input(file);
		if (!input.IsOpen())
		{
			Log("Convert " + path + ": cannot open the file");
			return;
		}
		loaded = ReadAsset(input, m_policy);
	}

	if (loaded.error != AssetError::None)
	{
		Log("Convert " + path + ": failed, " + std::string(ToString(loaded.error)));
		return;
	}

	// The content changes form, not meaning: same fields, same tags, a new version.
	AssetMetadata metadata = loaded.info.metadata;
	++metadata.version;

	// Written in memory first: if anything fails, the file on the disk is left untouched.
	MemoryStream converted;
	if (!WriteAsset(converted, *loaded.asset, metadata, encoding, m_policy))
	{
		Log("Convert " + path + ": the asset could not be written in " + target);
		return;
	}

	const std::uintmax_t before = std::filesystem::file_size(file);
	{
		FileOutputStream output(file);
		if (!output.IsOpen() || !output.Write(converted.Data()))
		{
			Log("Convert " + path + ": the file could not be written");
			return;
		}
	}

	char time[32];
	std::snprintf(time, sizeof(time), "%.2f ms", MillisecondsSince(start));
	Log("Converted " + path + " to " + target + " in " + time + ": " + std::to_string(before) + " -> "
		+ std::to_string(converted.Data().size()) + " bytes, version " + std::to_string(metadata.version));
	if (m_manager.IsLoaded(record.id))
		Log("  The asset in memory is the previous version: release it, then load it again");

	// The registry reads the new header: the Encoding column changes.
	Scan();
}

void EditorApp::ShowContent(AssetId id)
{
	const AssetHandle<Asset>* handle = FindHeld(id);
	if (handle == nullptr)
		return;

	// The JSON archive turns any asset into readable text, whatever its type.
	MemoryStream stream;
	JsonWriter writer(stream);
	writer.Value("asset", **handle);
	writer.Finish();

	const auto data = stream.Data();
	m_content.assign(reinterpret_cast<const char*>(data.data()), std::min(data.size(), MaxContentSize));
	if (data.size() > MaxContentSize)
		m_content += "\n... (" + std::to_string(data.size()) + " bytes in total)";
}

const AssetHandle<Asset>* EditorApp::FindHeld(AssetId id) const
{
	const auto held = std::ranges::find(m_held, id, &AssetHandle<Asset>::GetId);
	return held != m_held.end() ? &*held : nullptr;
}

std::string EditorApp::GetPath(AssetId id) const
{
	const AssetRecord* record = m_registry.Find(id);
	return record != nullptr ? record->path : Hex(id.value);
}

std::string EditorApp::GetTypeName(const AssetRecord& record) const
{
	const TypeInfo* type = TypeRegistry::Global().Find(record.type);
	return type != nullptr ? std::string(type->GetName()) : "unknown " + Hex(record.type.value);
}

void EditorApp::Log(std::string line)
{
	m_log.push_back(std::move(line));
	m_scrollLog = true;
}

void EditorApp::DrawTexturePreview(const Asset& asset, AssetId id)
{
	const auto* texture = dynamic_cast<const Demo::TextureAsset*>(&asset);
	if (texture == nullptr || texture->width <= 0 || texture->height <= 0)
		return;

	// The GPU reads width * height pixels: a file with fewer pixels than its size says is never sent.
	if (texture->pixels.size() != static_cast<std::size_t>(texture->width) * texture->height)
	{
		ImGui::TextColored(Yellow, "Preview: the pixel count does not match %d x %d", texture->width, texture->height);
		return;
	}

	// Sent to the GPU once, then drawn at every frame.
	if (m_preview == ImTextureID_Invalid || m_previewId != id)
	{
		FreePreview();
		m_preview = Window::CreateTexture(texture->width, texture->height, texture->pixels.data());
		m_previewId = id;
	}

	// Fitted in 320 pixels, keeping its proportions.
	const float scale = 320.0f / static_cast<float>(std::max(texture->width, texture->height));
	const ImVec2 size{texture->width * scale, texture->height * scale};
	ImGui::Text("Preview: %d x %d pixels", texture->width, texture->height);

	// A checkerboard behind the image: the transparent pixels show it, like in an image editor.
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	constexpr float Cell = 16.0f;
	for (float y = 0.0f; y < size.y; y += Cell)
	{
		for (float x = 0.0f; x < size.x; x += Cell)
		{
			const bool light = (static_cast<int>(x / Cell) + static_cast<int>(y / Cell)) % 2 == 0;
			const ImVec2 end{origin.x + std::min(x + Cell, size.x), origin.y + std::min(y + Cell, size.y)};
			draw->AddRectFilled({origin.x + x, origin.y + y}, end, light ? IM_COL32(200, 200, 200, 255) : IM_COL32(150, 150, 150, 255));
		}
	}

	ImGui::Image(m_preview, size);
}

void EditorApp::FreePreview()
{
	if (m_preview != ImTextureID_Invalid)
		Window::DestroyTexture(m_preview);

	m_preview = ImTextureID_Invalid;
	m_previewId = {};
}
