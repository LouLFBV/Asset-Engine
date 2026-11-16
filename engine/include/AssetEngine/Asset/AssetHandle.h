#pragma once

#include <AssetEngine/Asset/AssetError.h>
#include <AssetEngine/Asset/AssetId.h>

#include <memory>
#include <utility>

namespace AssetEngine
{
	// Gives access to a loaded asset, and keeps it in memory while the handle exists.
	// When the last handle of an asset is destroyed, the asset is freed.
	// An empty handle tells why the asset could not be loaded.
	template <typename T>
	class AssetHandle
	{
	public:
		AssetHandle() = default;

		AssetHandle(AssetId id, std::shared_ptr<T> asset, AssetError error = AssetError::None)
			: m_id(id)
			, m_asset(std::move(asset))
			, m_error(error)
		{
		}

		T* Get() const { return m_asset.get(); }
		T* operator->() const { return m_asset.get(); }
		T& operator*() const { return *m_asset; }
		explicit operator bool() const { return m_asset != nullptr; }

		AssetId GetId() const { return m_id; }
		AssetError GetError() const { return m_error; }
		const std::shared_ptr<T>& GetShared() const { return m_asset; }

	private:
		AssetId m_id;
		std::shared_ptr<T> m_asset;
		AssetError m_error = AssetError::None;
	};
}
