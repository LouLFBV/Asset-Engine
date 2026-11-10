#include <AssetEngine/Core/IO/FileOutputStream.h>

#include <ios>

namespace AssetEngine
{
    FileOutputStream::FileOutputStream(const std::filesystem::path& path) : m_file(path, std::ios::binary | std::ios::trunc)
    {
    }

    bool FileOutputStream::IsOpen() const
    {
        return m_file.is_open();
    }

    bool FileOutputStream::Write(std::span<const std::byte> data)
    {
        if(!m_file.is_open())
            return false;

        if (data.empty())
            return true;

        m_file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return static_cast<bool>(m_file);
    }
}