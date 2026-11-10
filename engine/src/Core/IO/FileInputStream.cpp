#include <AssetEngine/Core/IO/FileInputStream.h>

#include <ios>

namespace AssetEngine
{
    FileInputStream::FileInputStream(const std::filesystem::path& path) : m_file(path, std::ios::binary)
    {
        if(!m_file.is_open())
            return;
        
        m_file.seekg(0, std::ios::end);
        m_size = static_cast<std::size_t>(m_file.tellg());
        m_file.seekg(0, std::ios::beg);
    }

    bool FileInputStream::IsOpen() const
    {
        return m_file.is_open();
    }

    std::size_t FileInputStream::Read(std::span<std::byte> buffer)
    {
        if(!m_file.is_open() || buffer.empty())
            return 0;

        m_file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        const auto count = static_cast<std::size_t>(m_file.gcount());
        
        if(!m_file)
            m_file.clear();
        return count;
    }

    std::size_t FileInputStream::Size() const
    {
        return m_size;
    }

    bool FileInputStream::Seek(std::size_t position)
    {
        if(!m_file.is_open() || position > m_size)
            return false;

        m_file.seekg(static_cast<std::streamoff>(position));
        return static_cast<bool>(m_file);
    }
}