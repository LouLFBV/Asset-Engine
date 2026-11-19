#include "ImageImporter.h"

// stb_image is a single header: this file holds its code. Only PNG and JPEG are compiled,
// and the image is decoded from memory, so stb never opens a file itself.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#include <stb_image.h>

#include <climits>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

namespace Demo
{
	std::optional<TextureAsset> ImportImage(const std::filesystem::path& file)
	{
		std::ifstream input(file, std::ios::binary);
		if (!input)
			return std::nullopt;

		const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
		if (bytes.empty() || bytes.size() > INT_MAX)
			return std::nullopt;

		// 4 asks for RGBA, whatever the file holds: an image without transparency gets an opaque alpha.
		int width = 0;
		int height = 0;
		int channels = 0;
		stbi_uc* rgba = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &width, &height, &channels, 4);
		if (rgba == nullptr)
			return std::nullopt;

		TextureAsset texture;
		texture.width = width;
		texture.height = height;
		texture.pixels.resize(static_cast<std::size_t>(width) * height);

		// One pixel is 4 bytes, R G B A: exactly one std::uint32_t. The bytes are copied in the same order,
		// so the GPU reads them back as they were in the image.
		std::memcpy(texture.pixels.data(), rgba, texture.pixels.size() * sizeof(std::uint32_t));
		stbi_image_free(rgba);
		return texture;
	}
}
