#pragma once

#include <AssetEngine/Core/IO/IInputStream.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>

namespace AssetEngine
{
    class FileInputStream final : public IInputStream
    {
        public:
            explicit FileInputStream(const std::filesystem::path& path);

            bool IsOpen() const;

            std::size_t Read(std::span<std::byte> buffer) override;
            std::size_t Size() const override;
            bool Seek(std::size_t position) override;

        private:
            std::ifstream m_file;
            std::size_t m_size = 0;
    };
}