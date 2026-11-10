#include <AssetEngine/Core/IO/MemoryStream.h>

#include <algorithm>
#include <cstring>
#include <utility>

namespace AssetEngine
{
    MemoryStream::MemoryStream(std::vector<std::byte> data)
        : m_buffer(std::move(data))
    {
    }

    bool MemoryStream::Write(std::span<const std::byte> data)
    {
        if (data.empty())
            return true;
        
        const std::size_t end = m_position + data.size();
        if (end > m_buffer.size())
        {
            m_buffer.resize(end);
        }

        std::memcpy(m_buffer.data() + m_position, data.data(), data.size());
        m_position = end;
        return true;
    }

    std::size_t MemoryStream::Read(std::span<std::byte> buffer)
    {
        const std::size_t available = m_buffer.size() - m_position;
        const std::size_t count = std::min(buffer.size(), available);
        if (count > 0)
        {
            std::memcpy(buffer.data(), m_buffer.data() + m_position, count);
        }
        m_position += count;
        return count;
    }

    std::size_t MemoryStream::Size() const
    {
        return m_buffer.size();
    }

    bool MemoryStream::Seek(std::size_t position)
    {
        if (position > m_buffer.size())
            return false;
        m_position = position;
        return true;
    }

    std::span<const std::byte> MemoryStream::Data() const
    {
        return m_buffer;
    }
}