#pragma once

#include <AssetEngine/Core/IO/IOutputStream.h>
#include <AssetEngine/Core/IO/IInputStream.h>

#include <cstddef>
#include <span>
#include <vector>

namespace AssetEngine
{
    class MemoryStream final : public IInputStream, public IOutputStream
    {
        public:
            MemoryStream() = default;
            explicit MemoryStream(std::vector<std::byte> data);

            bool Write(std::span<const std::byte> data) override;
            std::size_t Read(std::span<std::byte> buffer) override;
            std::size_t Size() const override;
            bool Seek(std::size_t position) override;

            std::span<const std::byte> Data() const;
        
        private:
            std::vector<std::byte> m_buffer;
            std::size_t m_position = 0;
    };
}