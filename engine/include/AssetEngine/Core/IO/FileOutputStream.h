#pragma once

#include <AssetEngine/Core/IO/IOutputStream.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>

namespace AssetEngine
{
    class FileOutputStream final : public IOutputStream
    {
        public:
            explicit FileOutputStream(const std::filesystem::path& path);
            bool IsOpen() const;
            bool Write(std::span<const std::byte> data) override;
        private:
            std::ofstream m_file;
    };
}